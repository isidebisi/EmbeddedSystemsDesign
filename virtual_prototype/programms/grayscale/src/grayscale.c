#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>

#define ENABLE_ALL 0x000F
#define DISABLE_ALL 0x00F0
#define RESET_ALL 0x0F00

int main () {
  volatile uint16_t rgb565[640*480];
  volatile uint8_t grayscale[640*480];
  volatile uint32_t result, cycles,stall,idle;
  volatile unsigned int *vga = (unsigned int *) 0X50000020;
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
  uint32_t * rgb = (uint32_t *) &rgb565[0];
  uint32_t grayPixels;
  vga[2] = swap_u32(2);
  vga[3] = swap_u32((uint32_t) &grayscale[0]);

  uint32_t control = ENABLE_ALL;
  uint32_t result0, result1, result2, result3, counterId;

  

  while(1) {

    //start counters
    control = ENABLE_ALL;
    asm volatile("l.nios_rrr r0,r0,%[in2],0x0B"::[in2]"r"(control));

    uint32_t * gray = (uint32_t *) &grayscale[0];
    takeSingleImageBlocking((uint32_t) &rgb565[0]);
    for (int line = 0; line < camParams.nrOfLinesPerImage; line++) {
      for (int pixel = 0; pixel < camParams.nrOfPixelsPerLine; pixel++) {
        uint16_t rgb = swap_u16(rgb565[line*camParams.nrOfPixelsPerLine+pixel]);

        uint32_t gray;
        asm volatile("l.nios_rrr %[out1], r0,%[in2],0x0C":
                          [out1]"=r"(gray):
                          [in2]"r"(rgb));

        //uint32_t red1 = ((rgb >> 11) & 0x1F) << 3;
        //uint32_t green1 = ((rgb >> 5) & 0x3F) << 2;
        //uint32_t blue1 = (rgb & 0x1F) << 3;
        //uint32_t gray = ((red1*54+green1*183+blue1*19) >> 8)&0xFF;
        grayscale[line*camParams.nrOfPixelsPerLine+pixel] = gray;
      }
    }

    //stop counters and take results
    counterId = 0;
    control = DISABLE_ALL;
    asm volatile("l.nios_rrr %[out1],%[in1],%[int2],0x0B":
                  [out1]"=r"(result0):
                  [in1]"r"(counterId),
                  [int2]"r"(control));

    counterId = 1;
    asm volatile("l.nios_rrr %[out1],%[in1],r0,0x0B":
                  [out1]"=r"(result1):
                  [in1]"r"(counterId));

    counterId = 2;
    asm volatile("l.nios_rrr %[out1],%[in1],r0,0x0B":
                  [out1]"=r"(result2):
                  [in1]"r"(counterId));

    //reset counters
    control = RESET_ALL;
    asm volatile("l.nios_rrr r0,r0,%[in2],0x0B"::[in2]"r"(control));
                  
    printf("Results: \n");
    printf("number of CPU execution cycles:    %d\n", result0);
    printf("number of CPU stall cycles:        %d\n", result1);
    printf("number of CPU bus-idle cycles:     %d\n", result2);

  }
}
