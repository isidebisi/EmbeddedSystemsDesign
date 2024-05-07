`timescale 1ps/1ps // set the time-units for simulation

module ramDmaCiTestbench;
    reg start;
    reg clock;
    reg reset;
    reg [31:0] valueA;
    reg [31:0] valueB;
    reg [7:0] ciN;
    wire done;
    wire [31:0] result;

    initial begin
        reset = 1'b1;
        clock = 1'b0; // set the initial values
        repeat (4) #5 clock = ~clock; // generate 2 clock periods
        reset = 1'b0; // de-activate the reset
        forever #5 clock = ~clock; // generate a clock with a period of 10 time-units
    end

    ramDmaCi #(.customId(8'h0B)) DUT (
        .start(start), 
        .clock(clock), 
        .reset(reset), 
        .valueA(valueA), 
        .valueB(valueB),
        .ciN(ciN),
        .done(), 
        .result(result)
    );

    initial begin
        ciN = 8'h0B;
        start = 1'b0;
        @(negedge reset); /* wait for the reset period to end */
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */

        /*
        Perform a write operation to the RAM
        Reminder: 
        ValueA is a 32-bit signal containing the following information:
            - Bits [8:0] corresponds to the address
            - Bit 9 corresponds to the write-enable signal
            - Bits [31:10] Should be zeros (for now)

        ValueB is a 32-bit signal containing the data to be written
        */
        start = 1'b1; // set the start signal to 1
        valueA = 32'b 0000_0000_0000_0000_0000_0010_0000_0001; // set address 1 and write-enable signal to 1
        valueB = 32'h 00000011; // Data to send is 1
        repeat(2) @(posedge clock); /* wait for the next clock cycle */
        start = 1'b0; // set the start signal to 0
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */

        start = 1'b1; // set the start signal to 1
        valueA = 32'b 0000_0000_0000_0000_0000_0010_0000_0010; // set address 2 and write-enable signal to 1
        valueB = 32'h 00000012; // Data to send is 1
        repeat(2) @(posedge clock); /* wait for the next clock cycle */
        start = 1'b0; // set the start signal to 0
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */

        /*
        Perform a read operation from the RAM
        */
        start = 1'b1; // set the start signal to 1
        valueA = 32'b 0000_0000_0000_0000_0000_0000_0000_0001; // set a random address and write-enable signal to 0
        valueB = 32'h 00000000; // Data to send is 0
        repeat(2)@(posedge clock); /* wait for the next clock cycle */
        start = 1'b0; // set the start signal to 0
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */

        start = 1'b1; // set the start signal to 1
        valueA = 32'b 0000_0000_0000_0000_0000_0000_0000_0010; // set a random address and write-enable signal to 0
        valueB = 32'h 00000000; // Data to send is 0
        repeat(2)@(posedge clock); /* wait for the next clock cycle */
        start = 1'b0; // set the start signal to 0
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */


        // Perform an illicit write operation to the RAM
        start = 1'b1; // set the start signal to 1
        valueA = 32'b 0001_0000_0000_0000_0000_0010_0000_0001; // set the same address and write-enable signal to 1
        valueB = 32'h 00000001; // Data to send is 1
        repeat(2) @(posedge clock); /* wait for the next clock cycle */
        start = 1'b0; // set the start signal to 0
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */

        // Read the last illegal write operation
        start = 1'b1; // set the start signal to 1
        valueA = 32'b 0000_0000_0000_0000_0000_0000_0000_0001; // set a random address and write-enable signal to 0
        valueB = 32'h 00000000; // Data to send is 0
        repeat(2) @(posedge clock); /* wait for the next clock cycle */
        start = 1'b0; // set the start signal to 0
        repeat(2) @(posedge clock); /* wait for 2 clock cycles */
        $finish;
    end

    initial begin
        // define the name of the .vcd file that can be viewed by GTKWAVE
        $dumpfile("ramDmaCi.vcd"); 
        
        // dump all signals inside the DUT-component in the .vcd file
        $dumpvars(1, DUT); 
    end
endmodule