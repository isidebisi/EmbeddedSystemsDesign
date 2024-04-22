module ramDmaCi #(parameter [7:0] customId = 8'h00)
(
    input wire          start,
                        clock,
                        reset,
    input wire [31:0]   valueA,
                        valueB,
    input wire [7:0]    ciN,
    output wire         done,
    output wire [31:0]  result
);


// two modes: bypass and dma
// bypass: addressA is read or written to SSRAM if valueA[12:10]==3'b000
// dma: addressA is operated on by the DMA controller if valueA[12:10]!=3'b000


wire cpuEnable = start & (ciN == customId);
wire readWriteIn = valueA[9];
wire [2:0] mode = valueA[12:10];
wire [3:0] dmaControl = {mode, readWriteIn};
wire bypassEnable = cpuEnable & (mode == 3'b000);
wire dmaEnable = cpuEnable & (mode ~= 3'b000);



reg readOutputReady;


wire [8:0] addressA, addressB;
wire writeEnableA, writeEnableB;
wire [31:0] dataInA, dataInB, dataOutA, dataOutB;


dualPortSSRAM #(.bitwidth(32), .nrOfEntries(256), .readAfterWrite(0)) ramDmaCi
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

always @(posedge clock or posedge reset) begin
    if (reset) begin
        readOutputReady <= 0;
    end else begin
        readOutputReady <= cpuEnable ? ~readOutputReady : 1'b0;
    end
end



assign addressA = valueA[8:0];
assign writeEnableA = valueA[9] ? bypassEnable : 1'b0;
assign dataInA = valueB;
assign result = dataOutA ? done : 32'h00000000;
assign done =  1'b1 ? (bypassEnable & writeEnableA) :
                1'b1 ? (bypassEnable & ~writeEnableA & readOutputReady) : 1'b0;



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
            WSTATREG       =   4'b1011;


reg [31:0] busStartAddress, memStartAdress, blockSize, burstSize, statReg;
wire [31:0] resultDMA;

// block to handle all the control signals from CPU

always @(posedge clock or posedge reset) begin
    if (reset) begin
        busStartAddress <= 32'h00000000;
        memStartAdress <= 32'h00000000;
        blockSize <= 32'h00000000;
        burstSize <= 32'h00000000;
        statReg <= 32'h00000000;

    end else if (dmaEnable) begin
        case (dmaControl)
            RBUSSTADDR: begin
                resultDMA <= busStartAddress;
            end
            WBUSSTADDR: begin
                busStartAddress <= valueB;
                resultDMA <= valueB;
            end
            RMEMSTADDR: begin
                resultDMA <= memStartAdress;
            end
            WMEMSTADDR: begin
                memStartAdress <= valueB;
                resultDMA <= valueB;
            end
            RBLOCKSIZE: begin
                resultDMA <= blockSize;
            end
            WBLOCKSIZE: begin
                blockSize <= valueB;
                resultDMA <= valueB;
            end
            RBURSTSIZE: begin
                resultDMA <= burstSize;
            end
            WBURSTSIZE: begin
                burstSize <= valueB;
                resultDMA <= valueB;
            end
            RSTATREG: begin
                resultDMA <= statReg;
            end
            WSTATREG: begin
                statReg <= valueB;
                resultDMA <= valueB;
            end
            default: begin
                resultDMA <= 32'h00000000;
            end
            
        endcase
    end
end


// values that are not yet used

assign addressB <= 9'b000000000;
assign dataInB <= 32'h00000000;
assign writeEnableB <= 1'b0;

assign result <= 32'h00000000;

    

endmodule