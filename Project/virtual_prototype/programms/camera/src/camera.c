#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>
#include <floyd_steinberg.h>
#include <sobel.h>

#define threshold 100

volatile uint16_t rgb565[640*480];
volatile uint8_t grayscale[640*480];
volatile uint8_t floyd[640*480];
volatile uint8_t previous_floyd[640*480];
volatile int16_t error_array[642<<1];
volatile int first_frame = 1; // Flag to indicate if it's the first frame

int main () {
  volatile int result;
  volatile unsigned int *vga = (unsigned int *) 0X50000020;
  int reg;
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

  while(1) {
    // vga[2] = swap_u32(2);
    // vga[3] = swap_u32((uint32_t) &grayscale[0]);
    // asm volatile ("l.nios_rrr r0,%[in1],%[in2],0x6"::[in1]"r"(5000000),[in2]"r"(1)); // set 5 seconds
    // asm volatile ("l.nios_rrr %[out1],r0,r0,0x4":[out1]"=r"(reg));

    // do {
    //   takeSingleImageBlocking((uint32_t) &grayscale[0]);
    //   asm volatile ("l.nios_rrr %[out1],r0,%[in2],0x6":[out1]"=r"(result):[in2]"r"(3));
    // } while (result != 0 || ((reg&0x8) != 0));


    // vga[2] = swap_u32(2);
    // vga[3] = swap_u32((uint32_t) &grayscale[0]);
    // asm volatile ("l.nios_rrr r0,%[in1],%[in2],0x6"::[in1]"r"(5000000),[in2]"r"(1)); // set 5 seconds

    // do {
    //   takeSingleImageBlocking((uint32_t) &grayscale[0]);
    //   asm volatile ("l.nios_rrr %[out1],r0,%[in2],0x6":[out1]"=r"(result):[in2]"r"(3));
    // } while (result != 0);


    // vga[2] = swap_u32(2);
    // vga[3] = swap_u32((uint32_t) &floyd[0]);
    // asm volatile ("l.nios_rrr r0,%[in1],%[in2],0x6"::[in1]"r"(5000000),[in2]"r"(1)); // set 5 seconds

    // do {
    //   takeSingleImageBlocking((uint32_t) &grayscale[0]);
    //   floyd_steinberg(grayscale, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage, floyd, error_array);
    //   asm volatile ("l.nios_rrr %[out1],r0,%[in2],0x6":[out1]"=r"(result):[in2]"r"(3));
    // } while (result != 0);


    // asm volatile ("l.nios_rrr r0,%[in1],%[in2],0x6"::[in1]"r"(5000000),[in2]"r"(1)); // set 5 seconds

    do {
      takeSingleImageBlocking((uint32_t) &grayscale[0]);
      edgeDetection(grayscale,floyd, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage,128);
      asm volatile ("l.nios_rrr %[out1],r0,%[in2],0x6":[out1]"=r"(result):[in2]"r"(3));
    } while (result != 0);

    if (first_frame) {
      for (int i = 0; i < camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage; i++) {
        previous_floyd[i] = floyd[i];
      }
      first_frame = 0; // Update the flag
      
    } else {
      // Compare the current frame with the previous frame
      int changed_pixels = 0;
      for (int i = 0; i < camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage; i++) {
        // Subtract corresponding pixels and check if the difference is significant
        int diff = floyd[i] - previous_floyd[i];
        if (diff != 0) {
          changed_pixels++;
        }
      }

      // If there are changed pixels, movement is detected
      if (changed_pixels > threshold) {
        printf("Movement detected!\n");
        // Do something here, e.g., sound an alarm, send a notification, etc.
      }
    }

      // Update the previous frame with the current frame
      for (int i = 0; i < camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage; i++) {
        previous_floyd[i] = floyd[i];
      }
      vga[2] = swap_u32(2);
      vga[3] = swap_u32((uint32_t) &floyd[0]);
  }
}
