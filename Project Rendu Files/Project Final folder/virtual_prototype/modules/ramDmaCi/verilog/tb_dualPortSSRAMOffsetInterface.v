`timescale 1ns / 1ps

module testbench;
    reg clockA, clockB, writeEnableA, writeEnableB;
    reg [1:0] offsetA;
    reg [5:0] addressA, addressB; // Assuming nrOfEntries = 64
    reg [31:0] dataInA, dataInB; // 32-bit values
    wire [31:0] dataOutA, dataOutB;

    dualPortSSRAMOffsetInterface #(.bitwidth(32), .nrOfEntries(64)) uut (
        .clockA(clockA), .clockB(clockB),
        .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
        .offsetA(offsetA),
        .addressA(addressA), .addressB(addressB),
        .dataInA(dataInA), .dataInB(dataInB),
        .dataOutA(dataOutA), .dataOutB(dataOutB)
    );

    initial begin
        // Initialize signals
        clockA = 0; clockB = 0;
        writeEnableA = 0; writeEnableB = 0;
        offsetA = 0;
        addressA = 0; addressB = 0;
        dataInA = 0; dataInB = 0;

        // Generate clock
        forever #10 clockA = ~clockA;

        // Start writing data after some delay
        #100;

        integer i;
        for (i = 0; i < 64; i = i + 1) begin
            writeEnableA = 1;
            offsetA = 0;
            addressA = i;
            dataInA = ((4*i) << 24) | ((4*i+1) << 16) | ((4*i+2) << 8) | (4*i+3);
            #20; // Wait for some time
            writeEnableA = 0;
            #20; // Wait for some time
        end

        // Start reading data after some delay
        #100;

        for (i = 0; i < 64; i = i + 1) begin
            writeEnableB = 0;
            offsetA = 0;
            addressB = i;
            #20; // Wait for some time
            if (dataOutB !== ((4*i) << 24) | ((4*i+1) << 16) | ((4*i+2) << 8) | (4*i+3)) begin
                $display("Read incorrect value at address %0d: expected %0d, got %0d", i, ((4*i) << 24) | ((4*i+1) << 16) | ((4*i+2) << 8) | (4*i+3), dataOutB);
            end
            #20; // Wait for some time
        end
    end
endmodule