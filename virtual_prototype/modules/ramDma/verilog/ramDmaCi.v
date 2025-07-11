module ramDmaCi #(parameter [7:0] customId = 8'h00)
(
    input wire          start,
                        clock,
                        reset,
                        in_busGranted,
                        in_busError,
                        in_busBusy,
                        in_busEndTransaction,
                        in_busDataValid,
    input wire [31:0]   valueA,
                        valueB,
                        in_busAdressData,
    input wire [7:0]    ciN,
    output wire         done,
    output reg          reg_outBusRequest,
                        reg_outBusBeginTransaction,
                        reg_outBusEndTransaction,
                        reg_outBusReadWrite,
                        reg_outBusDataValid,
                        reg_outBusBusy,
    output reg[3:0]     reg_outBusByteEnable,
    output wire [31:0]  result,
    output wire [31:0]   outBusAddressData,
    output reg [7:0]    reg_outBusBurstSize
);


// two modes: bypass and dma
// bypass: addressA is read or written to SSRAM if valueA[12:10]==3'b000
// dma: addressA is operated on by the DMA controller if valueA[12:10]!=3'b000

//control variables
wire cpuEnable = start & (ciN == customId);
wire readWriteIn = valueA[9];
wire [2:0] mode = valueA[12:10];
wire [3:0] dmaControl = {mode, readWriteIn};
reg bypassEnable, dmaEnable;


reg readOutputReady;



// bypass variables for direct SSRAM access
wire [8:0] addressA;
wire writeEnableA;
wire [31:0] dataInA, dataOutA;

// DMA variables for cpu DMA transactions
reg [31:0] busStartAddress, memStartAdress, blockSize, burstSize, statReg, ctrlReg;
reg [31:0] resultDMA;
reg dmaDone;

// DMA variables for bus DMA transactions and final state machine
reg writeEnableB;
wire [8:0] addressB;
reg [31:0] dataInB, reg_outBusAddressNoData;
wire [31:0] dataOutB;

reg[7:0] burstCount;

reg [2:0] state;
reg [1:0] readOrWrite; // 10 = write, 01 = read, else none
reg [8:0] currBurstSize, burstNumber, currentMemAddress;


// Instanciation of SSRAM
dualPortSSRAM #(.bitwidth(32), .nrOfEntries(512), .readAfterWrite(0)) ramDmaCi
(
    .clockA(clock),
    .clockB(~clock),
    .writeEnableA(writeEnableA),
    .writeEnableB(writeEnableB),
    .addressA(addressA),
    .addressB(addressB),
    .dataInA(dataInA),
    .dataInB(dataInB),
    .dataOutA(dataOutA),
    .dataOutB(dataOutB));



/*
 *
 * BYPASS MODE, exercise 2.2
 *
 */

// always block for all enable and ready and signals
always @(posedge clock or posedge reset) begin
    if (reset) begin
        readOutputReady <= 0;
        bypassEnable = 1'b0;
        dmaEnable = 1'b0;
    end else if (clock) begin
        readOutputReady <= bypassEnable ? ~readOutputReady : 1'b0;

        if (cpuEnable & (mode == 3'b000)) begin
            bypassEnable = 1'b1;
        end
        
        if (cpuEnable & (mode != 3'b000)) begin
            dmaEnable = 1'b1;
        end

        if (done) begin
            bypassEnable = 1'b0;
            dmaEnable = 1'b0;
        end
    end
end



assign addressA = valueA[8:0];
assign writeEnableA = bypassEnable ? valueA[9] : 1'b0;
assign dataInA = valueB;
assign result = (bypassEnable & done) ? dataOutA : (dmaEnable & done) ? resultDMA : 32'h00000000;
assign done =  (bypassEnable & writeEnableA) ? 1'b1 :
                (bypassEnable & ~writeEnableA & readOutputReady) ? 1'b1:
                dmaEnable ? dmaDone : 1'b0;



/*
 *
 * DMA MODE, exercise 2.3
 *
 */




parameter   RBUSSTADDR     =   4'b0010,
            WBUSSTADDR     =   4'b0011,
            RMEMSTADDR     =   4'b0100,
            WMEMSTADDR     =   4'b0101,
            RBLOCKSIZE     =   4'b0110,
            WBLOCKSIZE     =   4'b0111,
            RBURSTSIZE     =   4'b1000,
            WBURSTSIZE     =   4'b1001,
            RSTATREG       =   4'b1010,
            WCTRLREG       =   4'b1011;



// block to handle all the control signals from CPU

always @(posedge clock or posedge reset) begin
    if (reset) begin
        busStartAddress <= 32'h00000000;
        memStartAdress <= 32'h00000000;
        blockSize <= 32'h00000000;
        burstSize <= 32'h00000000;
        ctrlReg <= 32'h00000000;
        dmaDone <= 1'b0;
    end else if (clock) begin
        ctrlReg[1:0] <= (statReg[0] == 1'b1) ? 2'b00 : ctrlReg[1:0];
        dmaDone <= 1'b0;
        resultDMA <= 32'h00000000;

        if (dmaEnable) begin

            case (dmaControl)
                RBUSSTADDR: begin
                    resultDMA <= busStartAddress;
                    dmaDone <= 1'b1;
                end
                WBUSSTADDR: begin
                    busStartAddress <= valueB;
                    resultDMA <= valueB;
                    dmaDone <= 1'b1;
                end
                RMEMSTADDR: begin
                    resultDMA <= memStartAdress;
                    dmaDone <= 1'b1;
                end
                WMEMSTADDR: begin
                    memStartAdress <= valueB;
                    resultDMA <= valueB;
                    dmaDone <= 1'b1;
                end
                RBLOCKSIZE: begin
                    resultDMA <= blockSize;
                    dmaDone <= 1'b1;
                end
                WBLOCKSIZE: begin
                    blockSize <= valueB;
                    resultDMA <= valueB;
                    dmaDone <= 1'b1;
                end
                RBURSTSIZE: begin
                    resultDMA <= burstSize;
                    dmaDone <= 1'b1;
                end
                WBURSTSIZE: begin
                    burstSize <= valueB;
                    resultDMA <= valueB;
                    dmaDone <= 1'b1;
                end
                RSTATREG: begin
                    resultDMA <= statReg;
                    dmaDone <= 1'b1;
                end
                WCTRLREG: begin
                    ctrlReg <= valueB;
                    resultDMA <= valueB;
                    dmaDone <= 1'b1;
                end
                default: begin
                    resultDMA <= 32'h00000000;
                    dmaDone <= 1'b0;
                end
                
            endcase
        end
    end
end


// values that are not yet used

    
// Bus Interactions


parameter   IDLE                =   3'b000,
            REQUEST             =   3'b001,
            BEGIN_TRANSACTION   =   3'b010,
            BURST               =   3'b011,
            ERROR               =   3'b100,
            END_TRANSACTION     =   3'b101;

// Final state machine


always @(posedge clock or posedge reset) begin
    if (reset) begin
        state <= IDLE;
        reg_outBusRequest <= 1'b0;
        reg_outBusByteEnable <= 4'h0;
        reg_outBusBeginTransaction <= 1'b0;
        reg_outBusEndTransaction <= 1'b0;
        reg_outBusReadWrite <= 1'b0;
        reg_outBusDataValid <= 1'b0;
        reg_outBusBusy <= 1'b0;
        reg_outBusAddressNoData <= 32'h00000000;
        reg_outBusBurstSize <= 8'h00;
        burstCount <= 8'h00;
        readOrWrite <= 2'b00;
        burstNumber <= 9'h001;
        currentMemAddress <= 9'h000;
        statReg <= 32'h00000000;
        writeEnableB <= 1'b0;
        dataInB <= 32'h00000000;
        currBurstSize <= 8'h00;

    end else if (clock) begin
        reg_outBusByteEnable <= 4'h0;
        reg_outBusBusy <= 1'b0;
        reg_outBusAddressNoData <= 32'h00000000;

        case (state)
            IDLE: begin
                burstCount <= 8'h00;
                reg_outBusEndTransaction <= 1'b0;
                reg_outBusDataValid <= 1'b0;
                if (ctrlReg[1:0] == 2'b10 | ctrlReg[1:0] == 2'b01 | statReg[0]) begin
                    state <= REQUEST;
                    readOrWrite <= statReg[0] ? readOrWrite : ctrlReg[1:0];
                    statReg[0] <= 1'b1; //busy
                    statReg[1] <= 1'b0; //no error
                    currentMemAddress <= memStartAdress[8:0] + (burstSize+1)*(burstNumber-1);
                end
            end
            
            REQUEST: begin
                reg_outBusRequest <= 1'b1;

                //If needed, adapt burstSize to blockSize
                if ((burstSize+1) * burstNumber > blockSize) begin 
                currBurstSize <= blockSize - ((burstSize+1) * (burstNumber-1)) -1;
                end else begin
                    currBurstSize <= burstSize[8:0];    
                end

                if (in_busGranted) begin
                    
                    reg_outBusRequest <= 1'b0;
                    reg_outBusBeginTransaction <= 1'b1;
                    reg_outBusBurstSize <= currBurstSize;
                    reg_outBusAddressNoData <= busStartAddress;
                    reg_outBusReadWrite <=  (readOrWrite == 2'b10) ? 1'b0 :
                                            (readOrWrite == 2'b01) ? 1'b1 : 1'b0;
                    reg_outBusByteEnable <= 4'hF;
                    writeEnableB <= 1'b0;
                    state <= BURST;
                    if (readOrWrite == 2'b10 & ~in_busBusy) begin
                        burstCount <= burstCount + 1;
                    end
                end
            end

            BURST: begin
                reg_outBusBeginTransaction <= 1'b0;
                reg_outBusBurstSize <= 8'h00;
                reg_outBusReadWrite <= 1'b0;
                reg_outBusByteEnable <= 4'h0;
                reg_outBusAddressNoData <= 32'h00000000;

                // ***** WRITE ******
                if (readOrWrite == 2'b10) begin
                    
                    //check if slave busy
                    if (in_busBusy) begin   //slave not busy
                        burstCount <= burstCount;
                    end else begin
                        burstCount <= burstCount + 1;
                        reg_outBusDataValid <= 1'b1;
                    end
                    
                    if (in_busError) begin
                        state <= ERROR;
                        reg_outBusEndTransaction <= 1'b1;
                        burstCount <= 8'h00;
                        reg_outBusDataValid <= 1'b0;
                    end else if (burstCount > currBurstSize+1 & ~in_busBusy) begin
                        state <= END_TRANSACTION;
                        burstCount <= 8'h00;
                        reg_outBusDataValid <= 1'b0;
                        reg_outBusEndTransaction <= 1'b1;
                        //determine if next burst is needed or block finished
                        if (burstNumber*(burstSize+1) >= blockSize) begin
                            burstNumber <= 9'h001;
                            statReg <= 32'h00000000;
                            
                        end else begin
                            burstNumber <= burstNumber+1;
                        end
                    end
                    

                // ***** READ *****
                end else if (readOrWrite == 2'b01) begin
                    
                    reg_outBusDataValid <= 1'b0;
                    reg_outBusAddressNoData <= 32'h00000000;
                    writeEnableB <= 1'b1;
                    dataInB <= in_busAdressData;

                    if (in_busDataValid) begin 
                        burstCount <= burstCount + 1;
                    end else begin
                        writeEnableB <= 1'b0;
                        burstCount <= burstCount;
                    end

                    if (in_busError) begin
                        writeEnableB <= 1'b0;
                        state <= ERROR;
                        reg_outBusEndTransaction <= 1'b1;
                        burstCount <= 8'h00;
                    end else if (in_busEndTransaction) begin
                        writeEnableB <= 1'b0;
                        state <= IDLE;
                        burstCount <= 8'h00;
                        reg_outBusAddressNoData <= 32'h00000000;
                        //determine if next burst is needed or block finished
                        if (burstNumber*(burstSize+1) >= blockSize) begin
                            burstNumber <= 9'h001;
                            statReg <= 32'h00000000;
                        end else begin
                            burstNumber <= burstNumber+1;
                        end
                    end
                end
            end
            
            ERROR: begin
                statReg <= 32'h00000002;    //finished but with error
                reg_outBusAddressNoData <= 32'h00000000;
                state <= IDLE;
                burstCount <= 8'h00;
                reg_outBusDataValid <= 1'b0;
                state <= IDLE;
                reg_outBusRequest <= 1'b0;
                reg_outBusByteEnable <= 4'h0;
                reg_outBusBeginTransaction <= 1'b0;
                reg_outBusEndTransaction <= 1'b0;
                reg_outBusReadWrite <= 1'b0;
                reg_outBusBusy <= 1'b0;
                reg_outBusBurstSize <= 8'h00;
                readOrWrite <= 2'b00;
                burstNumber <= 9'h001;
                writeEnableB <= 1'b0;
                dataInB <= 32'h00000000;
            end

            END_TRANSACTION: begin
                state <= IDLE;
                reg_outBusEndTransaction <= 1'b0;
                reg_outBusDataValid <= 1'b0;
                reg_outBusAddressNoData <= 32'h00000000;
                burstCount <= 8'h00;
            end

            default: begin
                state <= IDLE;
            end
        endcase
    end
end


//switch between adress and data output
assign outBusAddressData = reg_outBusDataValid ? dataOutB : reg_outBusAddressNoData;
assign addressB = currentMemAddress + burstCount;

endmodule