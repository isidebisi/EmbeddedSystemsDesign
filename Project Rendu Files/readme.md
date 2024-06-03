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
In the hand-in folder there is a zip file for every step of the project.
The programm folders only contain the programms of the project.


## Step 1 and 2: Sobel in Software and Hardware

After Implementing the Sobel algorithm in Software and Hardware we compared the results with the profiler.

Results | cycles without HW acceleration | cycles with HW acceleration | 
--- | --- | ---
CPU execution cycles | 334'856'828 | 102'773'340
CPU stall cycles | 255'656'675 | 81'355'177
CPU bus-idle cycles | 155'509'118 | 46'019'168

We can see that the HW acceleration reduces the CPU cycles by a factor of 3.25. The CPU stall cycles are reduced by a factor of 3.15 and the CPU bus-idle cycles are reduced by a factor of 3.38.

This is already something, but could be accelerated even more using a DMA controller and some sort of a Ping pong buffer like in PW6.

