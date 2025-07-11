#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>
#include <floyd_steinberg.h>
#include <sobel.h>

#define SOBEL_THRESHOLD 64
#define MOVEMENT_THRESHOLD 4000


void profileCiEnableCounters();
void profileCiDisableCounters();
void profileCiResetCounters();
void profileCiPrintCounters();

uint32_t sobelCi(uint32_t valueA, uint32_t valueB);
void doSobelHW(uint8_t grayscale[], uint8_t sobel[], uint32_t width, uint32_t height);

uint32_t profileCPUExecute, profileCPUStall, profileBusIdle;

volatile uint16_t rgb565[640*480];
volatile uint8_t grayscale[640*480];
volatile uint8_t sobel[640*480];
volatile uint8_t previous_sobel[640*480];
volatile int16_t error_array[642<<1];
volatile int first_frame = 1; // Flag to indicate if it's the first frame

int main () {
  volatile int result;
  volatile unsigned int *vga = (unsigned int *) 0X50000020;
  int reg;
  camParameters camParams;
  vga_clear();

  uint8_t movement_detected_counter = 0;

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

  vga[2] = swap_u32(2);
  vga[3] = swap_u32((uint32_t) &sobel[0]);

  while(1) {

    do {
      takeSingleImageBlocking((uint32_t) &grayscale[0]);

      printf("Starting HW Sobel\n");
      profileCiResetCounters();
      profileCiEnableCounters();
      doSobelHW(grayscale, sobel, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage);
      profileCiDisableCounters();
      profileCiPrintCounters();
      

      printf("\nStarting SW Sobel\n");
      profileCiResetCounters();
      profileCiEnableCounters();    
      edgeDetection(grayscale,sobel, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage,SOBEL_THRESHOLD);
      profileCiDisableCounters();
      profileCiPrintCounters();

      asm volatile ("l.nios_rrr %[out1],r0,%[in2],0x6":[out1]"=r"(result):[in2]"r"(3));
    } while (result != 0);

    if (first_frame) {
      for (int i = 0; i < camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage; i++) {
        previous_sobel[i] = sobel[i];
      }
      first_frame = 0; // Update the flag
      
    } else {
      // Compare the current frame with the previous frame
      int changed_pixels = 0;
      for (int i = 0; i < camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage; i++) {
        // Subtract corresponding pixels and check if the difference is significant
        int diff = sobel[i] - previous_sobel[i];
        if (diff != 0) {
          changed_pixels++;
        }
      }

      // If there are changed pixels, movement is detected
      if (changed_pixels > MOVEMENT_THRESHOLD) {
        printf("Movement detected! %d times\n", ++movement_detected_counter);
        // Do something here, e.g., sound an alarm, send a notification, etc.
      }
    }

      // Update the previous frame with the current frame
      for (int i = 0; i < camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage; i++) {
        previous_sobel[i] = sobel[i];
      }
  }
}

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




uint32_t sobelCi(uint32_t valueA, uint32_t valueB)
{
  uint32_t result;
  asm volatile ("l.nios_rrr %[out1],%[in1],%[in2],10":[out1]"=r"(result):[in1]"r"(valueA),[in2]"r"(valueB));
  return result;
}

void doSobelHW(uint8_t grayscale[], uint8_t sobel[], uint32_t width, uint32_t height)
{
  uint8_t valueA1, valueA2, valueA3, valueA4, valueB1, valueB2, valueB3, valueB4;
  uint32_t valueA, valueB, result, resultHW;
  for(uint16_t line = 0; line < height; line++) {
    for(uint16_t pixel = 0; pixel < width; pixel++) {
      if (line == 0 || line == height || pixel == 0 || pixel == width-1) {
        sobel[line * width + pixel] = 0;
      } else {

        // load 8 neighburing pixels into valueA and B
        valueA1 = grayscale[(line-1) * width + pixel-1];
        valueA2 = grayscale[(line-1) * width + pixel];
        valueA3 = grayscale[(line-1) * width + pixel+1];
        valueA4 = grayscale[(line * width) + pixel-1];
        valueB1 = grayscale[(line * width) + pixel+1];
        valueB2 = grayscale[(line+1) * width + pixel-1];
        valueB3 = grayscale[(line+1) * width + pixel];
        valueB4 = grayscale[(line+1) * width + pixel+1];

        valueA = ((valueA1 << 24) & 0xFF000000) +
                  ((valueA2 << 16) & 0x00FF0000) +
                  ((valueA3 << 8) & 0x0000FF00) +
                  (valueA4 & 0x000000FF);
        valueB = ((valueB1 << 24) & 0xFF000000) +
                  ((valueB2 << 16) & 0x00FF0000) +
                  ((valueB3 << 8) & 0x0000FF00) +
                  (valueB4 & 0x000000FF);

        resultHW = sobelCi(valueA, valueB);
        sobel[line * width + pixel] = (resultHW>SOBEL_THRESHOLD) ? 0xFF : 0;
 /* DEBUGGING
          if (line ==20 && pixel == 100) {
            printf("At pixel %d \n", pixel);
            printf("valueA: %d, valueB: %d \n", valueA, valueB);
            printf("values A1: %d A2: %d A3: %d A4: %d B1: %d B2: %d B3: %d B4: %d\n", valueA1, valueA2, valueA3, valueA4, valueB1, valueB2, valueB3, valueB4);
            printf("valueA and B = %d %d %d %d %d %d %d %d\n", valueA>>24, (valueA>>16)&0xFF, (valueA>>8)&0xFF, valueA&0xFF, valueB>>24, (valueB>>16)&0xFF, (valueB>>8)&0xFF, valueB&0xFF);
            printf("At line %d, pixel %d: %d \n", line, pixel, result, );
            printf("at pix: %d pixels: %d %d %d %d %d %d %d %d %d\n", pixel, grayscale[(line-1) * width + pixel-1],
                                                          grayscale[(line-1) * width + pixel],
                                                          grayscale[(line-1) * width + pixel+1],
                                                          grayscale[(line * width) + pixel-1],
                                                          grayscale[(line * width) + pixel],
                                                          grayscale[(line * width) + pixel+1],
                                                          grayscale[(line+1) * width + pixel-1],
                                                          grayscale[(line+1) * width + pixel],
                                                          grayscale[(line+1) * width + pixel+1]);
            printf("End at pixel %d \n", pixel);
          }
*/
      }
    }
  }
}