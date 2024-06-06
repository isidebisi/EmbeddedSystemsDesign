#include <stdint.h>
#include <stdio.h>

#include <profileCi.h>

//private variables
uint32_t profileCPUExecute, profileCPUStall, profileBusIdle;

void profileCiEnableCounters()
{
  asm volatile("l.nios_rrr r0,r0,%[in2],12"::[in2]"r"(0x000F));
}

void profileCiDisableCounters()
{
  asm volatile("l.nios_rrr r0,r0,%[in2],12"::[in2]"r"(0x00F0));
}

void profileCiResetCounters()
{
  asm volatile("l.nios_rrr r0,r0,%[in2],12"::[in2]"r"(0x0F00));
}

void profileCiPrintCounters()
{
  asm volatile("l.nios_rrr %[out1],%[in1],r0,12":
                [out1]"=r"(profileCPUExecute):
                [in1]"r"(0));
  asm volatile("l.nios_rrr %[out1],%[in1],r0,12":
                [out1]"=r"(profileCPUStall):
                [in1]"r"(1));
  asm volatile("l.nios_rrr %[out1],%[in1],r0,12":
                [out1]"=r"(profileBusIdle):
                [in1]"r"(2));

  printf("CPU Execute cycles: %d\n", profileCPUExecute);
  printf("CPU Stall cycles:   %d\n", profileCPUStall);
  printf("Bus Idle cycles:    %d\n", profileBusIdle);

}