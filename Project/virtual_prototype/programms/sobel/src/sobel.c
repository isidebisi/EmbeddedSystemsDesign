#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>

#define __WITH_CI

//PARAMETERS
#define BLOCKSIZE 256
#define BURSTSIZE 15

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_SIZE   (SCREEN_WIDTH*SCREEN_HEIGHT)
#define PI_PO_BUFFER_SIZE_32B 256
#define PI_PO_BUFFER_SIZE_16B PI_PO_BUFFER_SIZE_32B/2
#define BUFFER_ITERATIONS (SCREEN_SIZE/PI_PO_BUFFER_SIZE_16B)

//private functions
void      ci_writeToMemory(uint32_t address, uint32_t value);
uint32_t  ci_readFromMemory(uint32_t address);

void      dma_writeBusAddress(uint32_t address);
uint32_t  dma_readBusAddress();
void      dma_writeMemoryStart(uint32_t address);
uint32_t  dma_readMemoryStart();
void      dma_writeBlockSize(uint32_t blockSize);
uint32_t  dma_readBlockSize();
void      dma_writeBurstSize(uint32_t burstSize);
uint32_t  dma_readBurstSize();
void      dma_startWriteTransfer();
void      dma_startReadTransfer();
uint32_t  dma_readStatus();
void      dma_waitTransferComplete();
uint32_t  dma_switchBuffer(int buffer);

uint32_t grayscale_convert2Pixels(uint32_t pixel1, uint32_t pixel2);



int main () {
  const uint8_t sevenSeg[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
  volatile uint8_t grayscale_in[640*480];
  volatile uint8_t grayscale_out[640*480];
  volatile uint32_t result, cycles,stall,idle;
  volatile unsigned int *vga = (unsigned int *) 0X50000020;
  volatile unsigned int *gpio = (unsigned int *) 0x40000000;
  camParameters camParams;
  vga_clear();
  
  printf("Initialising camera (this takes up to 3 seconds)!\n" );
  camParams = initOv7670(VGA);
  printf("Done!\n" );
  printf("NrOfPixels : %d\n", camParams.nrOfPixelsPerLine );
  result = (camParams.nrOfPixelsPerLine <= 320) ? camParams.nrOfPixelsPerLine | 0x80000000 : camParams.nrOfPixelsPerLine;
  vga[0] = swap_u32(result);
  printf("NrOfLines  : %d\n", camParams.nrOfLinesPerImage );
  result =  (camParams.nrOfLinesPerImage <= 240) ? camParams.nrOfLinesPerImage | 0x80000000 : camParams.nrOfLinesPerImage;
  vga[1] = swap_u32(result);
  printf("PCLK (kHz) : %d\n", camParams.pixelClockInkHz );
  printf("FPS        : %d\n", camParams.framesPerSecond );
  uint32_t grayPixels;
  vga[2] = swap_u32(2);
  vga[3] = swap_u32((uint32_t) &grayscale_out[0]);

  //Initialize DMA
  dma_writeMemoryStart(0);
  dma_writeBlockSize(BLOCKSIZE);
  dma_writeBurstSize(BURSTSIZE);
  
  printf("DMA initialized with MemoryStart: %d, BlockSize: %d, BurstSize: %d\n", dma_readMemoryStart(), dma_readBlockSize(), dma_readBurstSize());

  uint32_t ping_pong_start_Addr = 0;

  while(1) {
    takeSingleImageBlocking((uint32_t) &grayscale_in[0]);

    memcpy((uint32_t) &grayscale_out[0], (uint32_t) &grayscale_in[0], SCREEN_SIZE);

    asm volatile ("l.nios_rrr %[out1],r0,%[in2],0xC":[out1]"=r"(cycles):[in2]"r"(1<<8|7<<4));
    asm volatile ("l.nios_rrr %[out1],%[in1],%[in2],0xC":[out1]"=r"(stall):[in1]"r"(1),[in2]"r"(1<<9));
    asm volatile ("l.nios_rrr %[out1],%[in1],%[in2],0xC":[out1]"=r"(idle):[in1]"r"(2),[in2]"r"(1<<10));
    printf("nrOfCycles: %d %d %d\n", cycles, stall, idle);
  }
}



void ci_writeToMemory(uint32_t address, uint32_t value){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(address | 0x200), [in2] "r"(value));
}

uint32_t ci_readFromMemory(uint32_t address){
  uint32_t res = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(res) : [in1] "r"(address));
  return res;
}

void dma_writeBusAddress(uint32_t address){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x600), [in2] "r"(address));
}

uint32_t dma_readBusAddress(){
  uint32_t busAddress = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(busAddress) : [in1] "r"(0x400));
  return busAddress;
}

void dma_writeMemoryStart(uint32_t address){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0xA00), [in2] "r"(address));
}

uint32_t dma_readMemoryStart(){
  uint32_t memoryAddress = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(memoryAddress) : [in1] "r"(0x800));
  return memoryAddress;
}

void dma_writeBlockSize(uint32_t blockSize){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0xE00), [in2] "r"(blockSize));
}

uint32_t dma_readBlockSize(){
  uint32_t blockSize = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(blockSize) : [in1] "r"(0xC00));
  return blockSize;
}

void dma_writeBurstSize(uint32_t burstSize){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x1200), [in2] "r"(burstSize));
}

uint32_t dma_readBurstSize(){
  uint32_t burstSize = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(burstSize) : [in1] "r"(0x1000));
  return burstSize;
}

void dma_startWriteTransfer(){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x1600), [in2] "r"(1));
}

void dma_startReadTransfer(){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x1600), [in2] "r"(2));
}

uint32_t dma_readStatus(){
  uint32_t status = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(status) : [in1] "r"(0x1400));
  return status;
}

void dma_waitTransferComplete(){
  uint32_t status = dma_readStatus();
  while (status != 0)
  {
    status = dma_readStatus();
  }
}

uint32_t dma_switchBuffer(int buffer){
  return buffer == 0 ? (uint32_t)256 : (uint32_t)0;
}

uint32_t grayscale_convert2Pixels(uint32_t pixel1, uint32_t pixel2){
  uint32_t grayPixelsOut;
  asm volatile("l.nios_rrr %[out1],%[in1],%[in2],0x9" : [out1] "=r"(grayPixelsOut) : [in1] "r"(pixel1), [in2] "r"(pixel2));
  return grayPixelsOut;
}