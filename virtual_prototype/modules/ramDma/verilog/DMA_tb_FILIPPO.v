`timescale 1ps/1ps // set the time-units for simulation

`define WAITHALFCYCLE #5;
`define WAITCYCLE #10;
`define WAIT2CYCLES repeat(2) @(posedge clock);

`define DISPLAY_DMA_REGISTERS \
    $display("[DMA_SETUP] bus_start_address: \t%0d", DUT.DMA.bus_start_address_out);\
    $display("            mem_start_address: \t%0d", DUT.memory_start_address);\
    $display("            block_size: \t%0d", DUT.block_size);\
    $display("            burst_size: \t%0d", DUT.burst_size);\
    $display("            control_register: \t%0b   %0b", DUT.control_register[1], DUT.control_register[0]);\
    $display("            status_register: \t%0b   %0b", DUT.status_register[1], DUT.status_register[0]);

module DMATestBench;

    /// Testbench signals
    reg start;
    reg clock;
    reg reset;
    reg [31:0] valueA = 0;
    reg [31:0] valueB = 0;
    reg [7:0] ciN;
    wire done;
    wire [31:0] result;

    reg busGrants = 0;
    reg [31:0] busIn_address_data = 0;
    reg busIn_end_transaction = 0;
    reg busIn_data_valid = 0;
    reg busIn_busy = 0;
    reg busIn_error = 0;

    reg [7:0] burst_size = 0;
    reg [8:0] memory_start_address = 0;
    reg [9:0] block_size = 0;

    /// Instantiate the DUT
    ramDmaCi #(.customId(8'h0B)) DUT (
        .start(start), 
        .clock(clock), 
        .reset(reset), 
        .valueA(valueA), 
        .valueB(valueB),
        .ciN(ciN),
        .done(done), 
        .result(result),
        .busIn_grants(busGrants),
        .busIn_address_data(busIn_address_data),
        .busIn_end_transaction(busIn_end_transaction),
        .busIn_data_valid(busIn_data_valid),
        .busIn_busy(busIn_busy),
        .busIn_error(busIn_error)
    );

    /// Testbench functions

    //* Set the bus start address
    task set_bus_start_address;
        input [31:0] new_address;
        begin
            valueA = {19'b0, 3'b001, 1'b1, 9'b0};
            valueB = new_address;
            `WAITHALFCYCLE
            start = 1'b1;
            `WAITCYCLE;
            start = 1'b0;
            $display("[BUS_START] Setting bus_start_address to %0d", new_address);
        end
    endtask

    //* Read the bus start address
    task read_bus_start_address;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b001, 1'b0, 9'b0};
            `WAIT2CYCLES;
            start = 1'b0;
            $display("[BUS_START] Reading bus_start_address via resTemp = %0d", DUT.result);
        end
    endtask

    //* Set the memory start address
    task set_memory_start_address;
        input [8:0] new_address;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b010, 1'b1, 9'b0};
            valueB = new_address;
            @(posedge clock);
            start = 1'b0;
            $display("[MEMORY_START] Setting memory_start_address to %0d", new_address);
            memory_start_address = new_address;
        end
    endtask

    //* Read the memory start address
    task read_memory_start_address;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b010, 1'b0, 9'b0};
            `WAIT2CYCLES;
            start = 1'b0;
            $display("[MEMORY_START] Reading memory_start_address via resTemp = %0d", DUT.result);
        end
    endtask

    //* Set the block size
    task set_block_size;
        input [9:0] new_block_size;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b011, 1'b1, 9'b0};
            valueB = new_block_size;
            @(posedge clock);
            start = 1'b0;
            $display("[BLOCK_SIZE] Setting block_size to %0d", new_block_size);
            block_size = new_block_size;
        end
    endtask

    //* Read the block size
    task read_block_size;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b011, 1'b0, 9'b0};
            `WAIT2CYCLES;
            start = 1'b0;
            $display("[BLOCK_SIZE] Reading block_size via resTemp = %0d", DUT.result);
        end
    endtask

    //* Set the burst size
    task set_burst_size;
        input [7:0] new_burst_size;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b100, 1'b1, 9'b0};
            valueB = new_burst_size;
            @(posedge clock);
            start = 1'b0;
            $display("[BURST_SIZE] Setting burst_size to %0d", new_burst_size);
            burst_size = new_burst_size;
        end
    endtask

    //* Read the burst size
    task read_burst_size;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b100, 1'b0, 9'b0};
            `WAIT2CYCLES;
            start = 1'b0;
            $display("[BURST_SIZE] Reading burst_size via resTemp = %0d", DUT.result);
        end
    endtask

    //* Set the control register
    task set_control_register;
        input [1:0] new_control_register;
        begin
            start = 1'b1;   
            valueA = {19'b0, 3'b101, 1'b1, 9'b0};
            valueB = new_control_register;
            @(posedge clock);
            start = 1'b0;
            $display("[CTRL_REG] Setting control_register to [%0b %0b]", new_control_register[1], new_control_register[0]);
        end
    endtask

    //* Read the control register
    task read_status_register;
        begin
            start = 1'b1;
            valueA = {19'b0, 3'b101, 1'b0, 9'b0};
            `WAIT2CYCLES;
            start = 1'b0;
            $display("[STAT_REG] Reading status_register via resTemp = [%0b %0b]", DUT.result[1], DUT.result[0]);
        end
    endtask


    
    /// TestBench behavior

    //? Generate the clock signal
    initial begin
        clock = 1'b1;
        forever #5 clock = ~clock; 
    end

    initial begin
        $dumpfile("dma_tb.vcd");
        $dumpvars(1, DUT);
        $dumpvars(1, DUT.SSRAM);
        $dumpvars(1, DUT.DMA);

        /// Set the initial values and reset the DUT
        $display("\n[LOG] Resetting the DUT");
        ciN = 8'h0B;
        start = 1'b0;
        reset = 1'b1;
        `WAITCYCLE;
        reset = 1'b0;
        `WAITCYCLE;
        $display("[LOG] DUT reset complete at %0tps", $time);

        //* Wait 2 clock cycles
        `WAIT2CYCLES;

        // //* Perform some write operation from the CPU to the SSRAM
        // // Eneble write operation
        // valueA[9] = 1;

        // repeat(5) begin
        //     start = 1'b1; 
        //     valueA = valueA + 1;
        //     valueB[7:0] = $random;
        //     `WAITCYCLE;
        //     start = 1'b0; 
        //     `WAITCYCLE;
        //     $display("\n[W_CPU] Write value %0d to address %0d", valueB, valueA[8:0]);
        // end

        // //* Wait 2 clock cycles
        // `WAIT2CYCLES;
        // $display("\n");

        // //* Perform some read operation from the CPU to the SSRAM
        // // Disable write operation
        // valueA = 0;

        // repeat(5) begin
        //     start = 1'b1; 
        //     valueA = valueA + 1;
        //     #20;
        //     start = 1'b0;             
        //     $display("[R_CPU] Read value %0d from address %0d", result, valueA[8:0]);
        //     `WAITCYCLE;
        // end

        //* Wait 1.5 clock cycles
        `WAITCYCLE;
        `WAITHALFCYCLE;

        /// Setup the DMA controller
        $display("\n[DMA_SETUP] Setting up the DMA controller\n");
        
        set_bus_start_address(32'd5);
        `WAIT2CYCLES;
        read_bus_start_address();
        `WAIT2CYCLES;

        $display("\n");

        set_memory_start_address(9'd10);
        `WAIT2CYCLES;
        read_memory_start_address();
        `WAIT2CYCLES;   

        $display("\n");

        set_block_size(10'd20);
        `WAIT2CYCLES;
        read_block_size();
        `WAIT2CYCLES;

        $display("\n");

        set_burst_size(8'd10);
        `WAIT2CYCLES;
        read_burst_size();
        `WAIT2CYCLES;

        $display("\n");

        set_control_register(2'b11);
        `WAIT2CYCLES;
        valueA = 0;
        
        
        // read_status_register();
        // `WAIT2CYCLES;

        // $display("\n");

        // `DISPLAY_DMA_REGISTERS;

        // $display("\n");

        // $display("[DMA_ERROR] Test an error case. Normally the DMA controller should not respond to this operation");
        // $display("            Setting valueA[12:10] = 3'b111 and valueA[9] = 1'b1 and valueB = 32'd17n");
        // valueA = 0;
        // valueA[12:10] = 3'b111;
        // valueA[9] = 1;
        // valueB = 32'd17;
        
        
        /// Burst transaction
        `WAIT2CYCLES;
        $display("\n");

        $display("Begin the transaction");
        busGrants = 1;
        @(posedge clock);
        busGrants = 0;
        `WAITHALFCYCLE

        //* Sending the data
        busIn_data_valid = 1;
        repeat(burst_size >> 1) begin
            busIn_address_data = busIn_address_data + 1;
            `WAITCYCLE;
        end
        busIn_data_valid = 0;
        `WAITCYCLE;
        busIn_data_valid = 1;
        repeat(burst_size >> 1) begin
            busIn_address_data = busIn_address_data + 1;
            `WAITCYCLE;
        end
        busIn_address_data = 0;
        busIn_data_valid = 0;
        busIn_end_transaction = 1;
        `WAITCYCLE;
        busIn_end_transaction = 0;
        `WAITCYCLE;

        `WAIT2CYCLES;
        `WAITHALFCYCLE
        busIn_data_valid = 1;
        repeat(burst_size) begin
            busIn_address_data = busIn_address_data + 1;
            `WAITCYCLE;
        end
        busIn_data_valid = 0;
        busIn_end_transaction = 1;
        `WAITCYCLE;
        busIn_end_transaction = 0;


        //* Check if the burst transaction was successful
        // Disable write operation
        valueA = memory_start_address;

        repeat(block_size) begin
            start = 1'b1; 
            #20;
            start = 1'b0;             
            $display("[R_CPU] Read value %0d from address %0d", result, valueA[8:0]);
            `WAITCYCLE;
            valueA = valueA + 1;
            `WAITCYCLE;
        end


        
        `DISPLAY_DMA_REGISTERS;


        
        //* End the simulation
        $display("\n");
        $finish;
    end

endmodule
