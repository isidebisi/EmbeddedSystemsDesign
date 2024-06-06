#include <stdint.h>
#include <dmaCi.h>





void ci_writeToMemory(uint32_t address, uint32_t value, uint32_t offset){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(address | 0x400 | offset <<30), [in2] "r"(value));
}

uint32_t ci_readFromMemory(uint32_t address, uint32_t offset){
  uint32_t res = 0;

  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(res) : [in1] "r"(address | offset <<30));
  return res;
}

void dma_writeBusAddress(uint32_t address){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0xC00), [in2] "r"(address));
}

uint32_t dma_readBusAddress(){
  uint32_t busAddress = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(busAddress) : [in1] "r"(0x800));
  return busAddress;
}

void dma_writeMemoryStart(uint32_t address){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x1400), [in2] "r"(address));
}

uint32_t dma_readMemoryStart(){
  uint32_t memoryAddress = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(memoryAddress) : [in1] "r"(0x1000));
  return memoryAddress;
}

void dma_writeBlockSize(uint32_t blockSize){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x1C00), [in2] "r"(blockSize));
}

uint32_t dma_readBlockSize(){
  uint32_t blockSize = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(blockSize) : [in1] "r"(0x1800));
  return blockSize;
}

void dma_writeBurstSize(uint32_t burstSize){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x2400), [in2] "r"(burstSize));
}

uint32_t dma_readBurstSize(){
  uint32_t burstSize = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(burstSize) : [in1] "r"(0x2000));
  return burstSize;
}

void dma_startWriteTransfer(){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x2C00), [in2] "r"(1));
}

void dma_startReadTransfer(){
  asm volatile("l.nios_rrr r0, %[in1],%[in2],20" ::[in1] "r"(0x2C00), [in2] "r"(2));
}

uint32_t dma_readStatus(){
  uint32_t status = 0;
  asm volatile("l.nios_rrr %[out1],%[in1],r0,20" : [out1] "=r"(status) : [in1] "r"(0x2800));
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
  switch (buffer)
  {
  case 0:
    return 160;
    break;
  case 160:
    return 320;
    break;
  case 320:
    return 480;
    break;
  default:
    return 0;
    break;
  }
}