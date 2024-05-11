# Hand ins Practical Works 2 and 4


| Student Name     | Student ID |
|------------------|------------|
| Charlotte Heibig | 300855     |
| Ismael Frei      | 301225     |


## Practical Work 6

Task 1: virtual_prototype_task_1.zip \
Task 2: virtual_prototype_task_2.zip \
Task 3: virtual_prototype_task_3.zip 


## Task 1

File modified: \
`programms/grayscale/src/grayscale.c`

Results Task1 | cycles w/o pingPongBuffer | cycles with pingPongBuffer | with divided by w/o in %
--- | --- | --- | ---
CPU execution cycles | 9804046 | 3324236 | 34%
CPU stall cycles | 8267693 | 404643 | 5%
CPU bus-idle cycles | 3562424 | 673355 | 19%

We can see that the CPU execution cycles are reduced by 66% when using the pingPongBuffer. The CPU stall cycles are reduced by 95% and the CPU bus-idle cycles are reduced by 81%.

That might seem unintuitive at first because we start by putting all the rgb565 pixels on the buffer just to remove them and put them back on. But when we look at where those rgb565 are stored we can observe that by putting them on the ping-pong buffer we can avoid using the bus to load them from the ram. 

## Task 2

File modified: \
`modules/camera/verilog/camera.v`

## Task 3

Files modified: \
`modules/camera/verilog/camera.v` \
`programms/streaming/src/streaming.c` (just commented the define)
