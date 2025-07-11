#include <stdio.h>
#include <ov7670.h>
#include <swap.h>
#include <vga.h>
#include <dmaCi.h>
#include <profileCi.h>
#include <sobel.h>


#define ENABLE_PROFILING 0  //Feel free to enable/disable profiling
#define ENABLE_SOBEL_HW_NO_DMA 0 //Enable the sobel algorithm without DMA
#define ENABLE_DYNAMIC_THRESHOLD 1 //Enable dynamic thresholding

#define SOBEL_MIN 5000
#define SOBEL_MAX 30000
uint16_t sobel_thresholds[6] = {16, 32, 64, 128, 256, 512};
uint8_t sobel_threshold_index = 3;

#define MOVEMENT_THRESHOLD 4000

//Dma parameters
#define BLOCKSIZE 160
#define BURSTSIZE 15
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_SIZE   (SCREEN_WIDTH*SCREEN_HEIGHT)
#define SSRAM_SIZE 640
#define PI_PO_BUFFER_SIZE_32B 160     // 640 PIXELS / 4 BYTES_PER_WORD * 4 LINES / 4 BUFFER_ZONES
#define BUFFER_ITERATIONS (SCREEN_HEIGHT) // WE NEED TO ITERATE OVER EVERY LINE BUT THE FIRST AND LAST



//function prototypes
uint32_t sobelCi(uint32_t valueA, uint32_t valueB);
void doSobelHW(uint8_t grayscale[], uint8_t sobel[], uint32_t width, uint32_t height, uint32_t threshold);
uint32_t movementDetectCi(uint32_t valueA, uint32_t valueB);
uint32_t movmentDetectSW(uint8_t sobel[], uint8_t previous_sobel[], uint32_t width, uint32_t height);
uint32_t movmentDetectHW(uint8_t sobel[], uint8_t previous_sobel[], uint32_t width, uint32_t height);



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

  //Initialize DMA
  dma_writeMemoryStart(0);
  dma_writeBlockSize(BLOCKSIZE);
  dma_writeBurstSize(BURSTSIZE);


  while(1) {


    takeSingleImageBlocking((uint32_t) &grayscale[0]);
    uint32_t ping_pong_start_Addr = 0;
    uint32_t iter_address = 0;
    uint32_t row1, row2, row3;
    uint32_t valueA1, valueB1, valueA2, valueB2, resultSobel, writePixels;
    uint8_t pixel1, pixel2;
    uint16_t keep2pixels, sobelEdgeCount = 0;
#if ENABLE_PROFILING
    printf("Starting HW (with DMA) Sobel\n");
    profileCiResetCounters();
    profileCiEnableCounters();
#endif
    for(uint16_t i=0; i < BUFFER_ITERATIONS; i++){
    
      iter_address = &grayscale[0] + 4*i* PI_PO_BUFFER_SIZE_32B;
      dma_writeBusAddress(iter_address);
      dma_writeBlockSize(BLOCKSIZE);
      dma_startWriteTransfer();

      if(i<3){
        //do the first 3 rows before starting the sobel algorithm
        ping_pong_start_Addr = dma_switchBuffer(ping_pong_start_Addr);
        dma_waitTransferComplete();
        dma_writeMemoryStart(ping_pong_start_Addr);
        continue;
      }

      ping_pong_start_Addr = dma_switchBuffer(ping_pong_start_Addr);

      //sobel algorithm
      // note that as we take 4 pixels per row instead of 3 we can apply sobel for 2 pixels at the time
      for(uint16_t j=0; j < SCREEN_WIDTH-3; j+=2){
        
        uint32_t row1 = (ci_readFromMemory(ping_pong_start_Addr+j/4, j%4));
        uint32_t row2 = (ci_readFromMemory((ping_pong_start_Addr+j/4+PI_PO_BUFFER_SIZE_32B)%SSRAM_SIZE, j%4));
        uint32_t row3 = (ci_readFromMemory((ping_pong_start_Addr+j/4+2*PI_PO_BUFFER_SIZE_32B)%SSRAM_SIZE, j%4));
                
        valueA1 = (row1 & 0xFFFFFF00) + ((row2 >> 24)&0x000000FF);
        valueB1 = ((row2 << 16)&0xFF000000) + ((row3 >>8) & 0x00FFFFFF);
        valueA2 = ((row1 << 8) & 0xFFFFFF00) + ((row2 >> 16)&0x000000FF);
        valueB2 = ((row2 << 24)&0xFF000000) + (row3 & 0x00FFFFFF);

        resultSobel = sobelCi(valueA1, valueB1);
        pixel1 = (resultSobel>sobel_thresholds[sobel_threshold_index]) ? 0xFF : 0; //(row2>>8);
        resultSobel = sobelCi(valueA2, valueB2);
        pixel2 = (resultSobel>sobel_thresholds[sobel_threshold_index]) ? 0xFF : 0; //(row2>>16);

#if ENABLE_DYNAMIC_THRESHOLD          
        sobelEdgeCount += (pixel1 == 0xFF) + (pixel2 == 0xFF);
#endif
        //We only write 4 pixels at a time into the memory so we need to keep 2 pixels for the next iteration
        if(j%4 != 0){
          writePixels = (uint32_t)(keep2pixels << 16) + (uint32_t)(pixel1 << 8) + pixel2;
          ci_writeToMemory(ping_pong_start_Addr+j/4, writePixels, 0);
        } else {
          keep2pixels = (pixel1 << 8) + pixel2;
        }
      }
        

      dma_waitTransferComplete();
      dma_writeMemoryStart(ping_pong_start_Addr);
      dma_writeBusAddress(&sobel[0] + (i-2) * SCREEN_WIDTH);
      dma_writeBlockSize(BLOCKSIZE);
      dma_writeBurstSize(BURSTSIZE);
      dma_startReadTransfer();
      dma_waitTransferComplete();
    }
    


#if ENABLE_PROFILING
    profileCiDisableCounters();
    profileCiPrintCounters();
#endif

#if ENABLE_SOBEL_HW_NO_DMA
  #if ENABLE_PROFILING
    printf("Starting HW (pixel per pixel) Sobel\n");
    profileCiResetCounters();
    profileCiEnableCounters();
  #endif

    //That's the non-optimized sobel algorithm. It is already a Ci function but without DMA and buffer use.
    doSobelHW(grayscale, sobel, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage, sobel_thresholds[sobel_threshold_index]);

  #if ENABLE_PROFILING
    profileCiDisableCounters();
    profileCiPrintCounters();
  #endif
#endif

    if (first_frame) {
      // first frame has nothing to compare to so we wait for the second frame for movement detection
      memcpy(previous_sobel, sobel, camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage * sizeof(*sobel));

      first_frame = 0; // Update the flag

    } else {

      uint32_t movement_result;

#if ENABLE_PROFILING
    printf("Starting HW Movement\n");
    profileCiResetCounters();
    profileCiEnableCounters();
#endif
      movement_result = movmentDetectHW(sobel, previous_sobel, camParams.nrOfPixelsPerLine, camParams.nrOfLinesPerImage);
  #if ENABLE_PROFILING
    profileCiDisableCounters();
    profileCiPrintCounters();
  #endif

      if(movement_result){
        printf("Movement detected ! %d times\n", ++movement_detected_counter);
      }
      // Update the previous frame with the current frame
      memcpy(previous_sobel, sobel, camParams.nrOfPixelsPerLine * camParams.nrOfLinesPerImage * sizeof(*sobel));
    }

#if ENABLE_DYNAMIC_THRESHOLD    
      if(sobelEdgeCount < SOBEL_MIN){
        if (sobel_threshold_index!=0){
          sobel_threshold_index--;
          printf("Sobel threshold dynamically decreased to %d because edgeCounter=%d \n", sobel_thresholds[sobel_threshold_index], sobelEdgeCount);
          first_frame = 1; // Reset the movement detect flag
        }
      } else if(sobelEdgeCount > SOBEL_MAX){
        if (sobel_threshold_index!=5){
          sobel_threshold_index++; ;
          printf("Sobel threshold dynamically increased to %d because edgeCounter=%d\n", sobel_thresholds[sobel_threshold_index], sobelEdgeCount);
          first_frame = 1; // Reset the movement detect flag
        }
      }
#endif
  }
}


uint32_t sobelCi(uint32_t valueA, uint32_t valueB)
{
  uint32_t result;
  asm volatile ("l.nios_rrr %[out1],%[in1],%[in2],10":[out1]"=r"(result):[in1]"r"(valueA),[in2]"r"(valueB));
  return result;
}

void doSobelHW(uint8_t grayscale[], uint8_t sobel[], uint32_t width, uint32_t height, uint32_t threshold)
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
        sobel[line * width + pixel] = (resultHW>threshold) ? 0xFF : 0;
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

uint32_t movmentDetectSW(uint8_t sobel[], uint8_t previous_sobel[], uint32_t width, uint32_t height)
{
// Compare the current frame with the previous frame
  int changed_pixels = 0;
  for (int i = 0; i < width * height; i++) {
    // Subtract corresponding pixels and check if the difference is significant
    int diff = sobel[i] - previous_sobel[i];
    if (diff != 0) {
      changed_pixels++;
    }
  }

  // If there are changed pixels, movement is detected
  if (changed_pixels > MOVEMENT_THRESHOLD) {
    return 1;
    // Do something here, e.g., sound an alarm, send a notification, etc.
  }
  return 0;
}

uint32_t movementDetectCi(uint32_t valueA, uint32_t valueB)
{
  uint32_t result;
  asm volatile ("l.nios_rrr %[out1],%[in1],%[in2],11":[out1]"=r"(result):[in1]"r"(valueA),[in2]"r"(valueB));
  return result;
}

uint32_t movmentDetectHW(uint8_t sobel[], uint8_t previous_sobel[], uint32_t width, uint32_t height)
{
  uint32_t changed_pixels = 0;
  uint32_t valueA, valueB;
  for (int i = 0; i < width * height; i=i+4) {
    // Subtract corresponding pixels and check if the difference is significant
    valueA = ((sobel[i] << 24) & 0xFF000000) +
              ((sobel[i+1] << 16) & 0x00FF0000) +
              ((sobel[i+2] << 8) & 0x0000FF00) +
              (sobel[i+3] & 0x000000FF);
    valueB = ((previous_sobel[i] << 24) & 0xFF000000) +
              ((previous_sobel[i+1] << 16) & 0x00FF0000) +
              ((previous_sobel[i+2] << 8) & 0x0000FF00) +
              (previous_sobel[i+3] & 0x000000FF);
    
    changed_pixels += movementDetectCi(valueA, valueB);
  }

  if(changed_pixels > MOVEMENT_THRESHOLD) return 1;
  return 0;
}