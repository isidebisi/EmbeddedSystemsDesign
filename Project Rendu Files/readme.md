# Hand in Project for course CS-476 Embedded System Design


| Student Name     | Student ID |
|------------------|------------|
| Charlotte Heibig | 300855     |
| Ismael Frei      | 301225     |


## Introduction

The project is done in several steps:
1. Implement Sobel and movement detection in Software
2. Implement Sobel in hardware and compare the results
3. Implement movement detection in hardware and compare the results
4. Further optimization

## Hand in and files
In the hand-in folder there is a zip file for every step of the project. \
The programm folders only contain the programms of the project. \
Note that the Step folders are works in progress and therefore not perfectly cleaned up. They are mere checkpoints of our progress.


## Step 1 and 2: Sobel in Software and Hardware

After Implementing the Sobel algorithm in Software and Hardware we compared the results with the profiler.

Results | cycles without HW acceleration | cycles with HW acceleration | 
--- | --- | ---
CPU execution cycles | 334'856'828 | 102'773'340
CPU stall cycles | 255'656'675 | 81'355'177
CPU bus-idle cycles | 155'509'118 | 46'019'168

We can see that the HW acceleration reduces the CPU cycles by a factor of 3.25. The CPU stall cycles are reduced by a factor of 3.15 and the CPU bus-idle cycles are reduced by a factor of 3.38.

This is already something, but could be accelerated even more using a DMA controller and some sort of a Ping pong buffer like in PW6.

## Step 3: Movement Detection in Hardware
The movement detection is implemented in hardware. \
4 pixels can be compared at the time thanks to valueA and B being `uint32_t` and the pixels being `uint8_t`. \
The results are compared with the profiler.

Results | cycles without HW acceleration | cycles with HW acceleration | 
--- | --- | ---
CPU execution cycles | 24'880'570 | 22'525'469
CPU stall cycles | 18'655'980 | 18'378'947
CPU bus-idle cycles | 11'741'333 | 9'698'014

We can note 2 things:
1. The HW acceleration reduces the CPU cycles by a factor of 1.1. This is not a lot.
2. The movement detection is more than 10 times quicker than the sobel algorithm. This means that for further optimization we should focus on the Sobel algorithm.

## Step 4: Further optimization
Le'ts optimize the Sobel algorithm. \
Idea: Use a DMA controller and with a SRAM as a pingpong buffer. But instead of seperating it into 2 sections we could use 4 sections. \
The sobel algorithm needs 3 rows of pixels to calculate the edges. With 4 buffers we could roll from row to row. \
That means that 3 rows are in the buffer and worked with while one new row is added to the buffer by the background. \
The results overwrite the buffer space of the highest row. \
Each buffer needs 640 bytes of space, which means that in total we need 640 words for the four buffers (2560 bytes). 

We can take the ramDmaCi.v as a template but need to adapt it to have one more byte in the address space.
1. All the bits in valueA are shifted to the left by 1 bit. (example: ValueA\[10] stores read/Write now instead of valueA\[9])
2. Customize the DualPort SSRAM to be able to read from specific addresses with byte offset (for example from adress 4 with a byte offset of 1 to get bytes 1-3 from adress 4 and byte 0 from address 5).

visualization of RAM:
Word 0 | Word 1 | Word 2 | Word 3 |
--- | --- | --- | ---
B0 B1 B2 B3 | B0 B1 B2 B3 | B0 B1 B2 B3 | B0 B1 B2 B3

#### That was the plan for the RAM anyway. - Deep dive into RAM Architecture in verilog
Right after the presentation, we realized that our solution woth a RAM with byte offset didn't synthesize. The demo ran effectively on a previous version without byte offset.

When defining a DualPort SSRAM, the synthesizer recognizes it and automatically implements it in optimized memory blocks.

However, for our usecase where we want to break the words in the RAM up there is a problem. Any tried configuration with clockA and clockB in separate always loops produced some net errors stating that the reg `memoryContent[]` is potentially written by two signals at the same time. This is understandable as Port A and B could write to the same address at the same time if they are badly configured. But even when taking care of those cases by creating conditions that prevent this, the error still occurs.

The second strategy would then be to just implement the two ports in one single always loop. The problem there was, however, that it didn't recognize it as a memory block and wanted to implement it in logic. We don't have enough space in logic to store so many bytes and it would take a huge space on the FPGA for nothing.

**Conclusion:** The synthesizer is not able to implement the RAM in memory blocks if we add some fancy bitshift functionnality.

**Solution:** We create instead of one single RAM with 4 buffers, 4 RAMs with 4 buffers each. Each RAMs stores 1 byte. There is an interface layer between the DMA and the RAMs that assembles the signals together to form a word. This solution is not the most efficient neither the most elegant, and certainly not worth the effort of fighting the uphill battle of managing adresses, memory blocks and byteshifts. But as it was already presented in this way we felt the need to make it right and prove our concept.



#### Results of Sobel algorithm with optimized HW acceleration: 

Results | cycles without HW acceleration | cycles with HW acceleration | cycles with optimized HW acceleration
--- | --- | --- | ---
CPU execution cycles | 334'856'828 | 101'867'190 | 22'353'305
CPU stall cycles | 255'656'675 | 80'459'872 | 18'282'876
CPU bus-idle cycles | 155'509'118 | 46'030'981 | 9'571'337