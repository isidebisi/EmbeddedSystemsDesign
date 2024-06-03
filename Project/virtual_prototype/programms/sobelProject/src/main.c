#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>
#include <floyd_steinberg.h>
#include <sobel.h>

#define SOBEL_THRESHOLD 64
#define MOVEMENT_THRESHOLD 4000

uint32_t sobelCi(uint32_t valueA, uint32_t valueB);

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
      uint32_t valueA, valueB, result, resultSWdX, resultSWdY,resultSW;
      for(uint16_t line = 1; line < camParams.nrOfLinesPerImage-1; line++) {
        for(uint16_t pixel = 1; pixel < camParams.nrOfPixelsPerLine-1; pixel++) {
          valueA =  grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel-1] << 24 +
                    grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel] << 16 +
                    grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel+1] << 8 +
                    grayscale[(line * camParams.nrOfPixelsPerLine) + pixel-1];
          valueB =  grayscale[(line * camParams.nrOfPixelsPerLine) + pixel+1] << 24 +
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel-1] << 16 +
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel] << 8 +
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel+1];
          
        result = sobelCi(valueA, valueB);
        resultSWdX =  -grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel-1] +
                    grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel+1] -
                    2*grayscale[(line * camParams.nrOfPixelsPerLine) + pixel-1] +
                    2*grayscale[(line * camParams.nrOfPixelsPerLine) + pixel+1] -
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel-1] +
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel+1];

        resultSWdY =  grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel-1] +
                    2*grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel] +
                    grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel+1] -
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel-1] -
                    2*grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel] -
                    grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel+1];
        
        resultSW = abs(resultSWdX - resultSWdY);
/*
        if (result != resultSW) {
          printf("Error at pixel %d \n", pixel);
          printf("Error at line %d, pixel %d: %d != %d\n", line, pixel, result, resultSW);
          printf("at pix: %d pixels: %d %d %d %d %d %d %d %d %d\n", pixel, grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel-1],
                                                        grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel],
                                                        grayscale[(line-1 * camParams.nrOfPixelsPerLine) + pixel+1],
                                                        grayscale[(line * camParams.nrOfPixelsPerLine) + pixel-1],
                                                        grayscale[(line * camParams.nrOfPixelsPerLine) + pixel],
                                                        grayscale[(line * camParams.nrOfPixelsPerLine) + pixel+1],
                                                        grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel-1],
                                                        grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel],
                                                        grayscale[(line+1 * camParams.nrOfPixelsPerLine) + pixel+1]);
          printf("End Error at pixel %d \n", pixel);
        }
*/
        sobel[line * camParams.nrOfPixelsPerLine + pixel] = (resultSW>SOBEL_THRESHOLD) ? 0xFF : 0;
        //printf("result: %d\n", result);
        }
      }

          
      //edgeDetection(grayscale,sobel, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage,64);
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


uint32_t sobelCi(uint32_t valueA, uint32_t valueB)
{
  uint32_t result;
  asm volatile ("l.nios_rrr %[out1],%[in1],%[in2],0xA":[out1]"=r"(result):[in1]"r"(valueA),[in2]"r"(valueB));
  return result;
}
