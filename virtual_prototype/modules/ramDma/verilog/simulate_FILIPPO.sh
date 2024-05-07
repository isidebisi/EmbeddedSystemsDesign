#!/bin/bash
if [ -f "testbench" ]; then
    rm testbench
fi
iverilog -s DMATestBench -o testbench BusTransaction_tb.v ramDmaCi.v DMAController.v dualPortSSRAM.v
./testbench
#gtkwave dma_tb.vcd &
