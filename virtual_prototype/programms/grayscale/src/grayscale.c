#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>

// Define common configurations
#define CONFIG_BUS_ADDRESS(addr) asm volatile ("l.nios_rrr r0,%[in1],%[in2],0xD"::[in1]"r"((0x3) << 9),[in2]"r"(addr))
#define CONFIG_MEM_ADDRESS(addr) asm volatile ("l.nios_rrr r0,%[in1],%[in2],0xD"::[in1]"r"((0x5) << 9),[in2]"r"(addr))
#define CONFIG_BLOCK_SIZE(size) asm volatile ("l.nios_rrr r0,%[in1],%[in2],0xD"::[in1]"r"((0x7) << 9),[in2]"r"(size))
#define CONFIG_BURST_SIZE(size) asm volatile ("l.nios_rrr r0,%[in1],%[in2],0xD"::[in1]"r"((0x9) << 9),[in2]"r"(size))
#define START_TRANSFER(cmd) asm volatile ("l.nios_rrr r0,%[in1],%[in2],0xD"::[in1]"r"((0xB) << 9),[in2]"r"(cmd))
#define READ_STATUS_REG(out) asm volatile ("l.nios_rrr %[out1],%[in1],r0,0xD":[out1]"=r"(out):[in1]"r"((0x0A) << 9))
#define READ_BURST_SIZE(size) asm volatile ("l.nios_rrr %[out1],%[in1],r0,0xD":[out1]"=r"(size):[in1]"r"((0x8) << 9))
#define READ_BLOCK_SIZE(size) asm volatile ("l.nios_rrr %[out1],%[in1],r0,0xD":[out1]"=r"(size):[in1]"r"((0x6) << 9))
#define READ_BUS_ADRESS(addr) asm volatile ("l.nios_rrr %[out1],%[in1],r0,0xD":[out1]"=r"(addr):[in1]"r"((0x2) << 9))
#define READ_MEM_ADRESS(addr) asm volatile ("l.nios_rrr %[out1],%[in1],r0,0xD":[out1]"=r"(addr):[in1]"r"((0x4) << 9))

#define BURST_SIZE 0x00000004

// is this even used or defined in our verilog code?
#define READ_MEM_VALUE(addr, out) asm volatile ("l.nios_rrr %[out1],%[in1],r0,0xD":[out1]"=r"(out):[in1]"r"(addr))
// difference between writing to CI with this or with DMA??
#define WRITE_MEM_VALUE(addr, in) asm volatile ("l.nios_rrr r0,%[in1],%[in2],0xD"::[in1]"r"((0x1) << 9),[in2]"r"(in))

// Write from bus to CI memory
void read_from_bus(uint32_t memoryAddress, uint32_t blockSize) {
    CONFIG_MEM_ADDRESS(memoryAddress);
    CONFIG_BLOCK_SIZE(blockSize);
    START_TRANSFER(0x00000001);
    uint32_t read_status_register;
    READ_STATUS_REG(read_status_register);
    printf("Checking status register value : %d\n", read_status_register);

    //how does this work '000b 1b Write to memory location A[8..0]' ???????
    for (uint32_t i = memoryAddress; i < memoryAddress + blockSize + 1; i++) {
        uint32_t read_value;
        READ_MEM_VALUE(i, read_value);
        printf("Value at SRAM memory location 0x%3x: 0x%8x\n", i, read_value);
    }
    
}

void write_to_bus(uint32_t* memoryArray, uint32_t blockSize, uint32_t write_to){
    CONFIG_BUS_ADDRESS((uint32_t) &memoryArray[write_to]); 
    CONFIG_BLOCK_SIZE(blockSize);

    // read status of bus before transfer
    printf("Status of bus before transfer\n");
    for (uint32_t array_element = write_to; array_element < write_to+blockSize+1;array_element++) {
      printf("Value at bus location %d: %d\n",array_element,memoryArray[array_element]);
    }
    START_TRANSFER(0x00000002);
    printf("Status of bus after transfer\n");
    for (uint32_t array_element = write_to;array_element < write_to+blockSize+1;array_element++) {
      printf("Value at bus location %d: %d\n",array_element,swap_u32(memoryArray[array_element])); 
    }
}

int main () {



  CONFIG_BURST_SIZE(BURST_SIZE);
  printf("Start Program\n");
  // array used as a slave to read and write on the bus
  uint32_t arraySize = 512;
  volatile uint32_t memoryArray[512];
  for (uint32_t i = 0 ; i < arraySize ; i++) {
    memoryArray[i] = swap_u32(i+1);
  }

  CONFIG_BUS_ADDRESS((uint32_t)&memoryArray[0]);

    volatile unsigned int *vga = (unsigned int *) 0X50000020;
  camParameters camParams;
  vga_clear();

  // WRITE FROM BUS TO CI MEMORY

  // Write to CI memory adress = 1
  printf("\n===== Single burst transfer from bus to CI-memory with blocksize = 5 =====\n");
  read_from_bus(0x00000001, 0x00000005);

  // Write to CI memory adress = 16
  printf("\n===== Single burst transfer from bus to CI-memory with blocksize = 3 =====\n");
  read_from_bus(0x00000010, 0x00000003);

  // Write to CI memory adress = 32
  printf("\n===== Multiple burst transfer from bus to CI-memory, blocksize = 7 =====\n");
  read_from_bus(0x00000020, 0x00000007);


  // CHECK STUFF

  // Read burst size to check if it is correctly set
  uint32_t read_burst_size;
  READ_BURST_SIZE(read_burst_size);
  printf("Check burst size %d\n",read_burst_size);

  // Read block size to check if it is correctly set
  uint32_t read_block_size;
  READ_BLOCK_SIZE(read_block_size);
  printf("Check block size %d\n",read_block_size);

  // Read bus address to check if it is correctly set
  uint32_t read_bus_start_address;
  READ_BUS_ADRESS(read_bus_start_address);
  printf("Check bus start address %d\n", read_bus_start_address);

  // Read memory address to check if it is correctly set
  uint32_t read_mem_start_address;
  READ_MEM_ADRESS(read_mem_start_address);
  printf("Check memory start address %d\n", read_mem_start_address);


  // READ FROM CI MEMORY TO BUS

  // Read element 128 of array
  printf("\n===== Single burst transfer from CI-memory to bus with blocksize = 5 =====\n");
  write_to_bus(memoryArray,  0x00000005 , 128);

  // Read element 256 of array
  printf("\n===== Single burst transfer from CI-memory to bus with blocksize = 3 =====\n");
  write_to_bus(memoryArray,  0x00000003 , 256);

  // Read element 384 of array
  printf("\n===== Multiple burst transfer from CI-memory to bus, blocksize = 7 =====\n");
  write_to_bus(memoryArray,  0x00000007 , 384);
}