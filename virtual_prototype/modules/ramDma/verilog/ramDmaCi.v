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


wire cpuEnable <= start & (ciN == customId);

wire bypassEnable <= cpuEnable & (valueA[12:10] == 3'b000);
wire dmaEnable <= cpuEnable & (valueA[12:10] ~= 3'b000);


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



always @(posedge clock or posedge reset) begin
    if (reset) begin
        readOutputReady <= 0;
    end else begin
        readOutputReady <= cpuEnable ? ~readOutputReady : 1'b0;
    end
end



assign addressA <= valueA[8:0];
assign writeEnableA <= valueA[9] ? bypassEnable : 1'b0;
assign dataInA <= valueB;
assign result <= dataOutA ? done : 32'h00000000;
assign done <=  1'b1 ? (bypassEnable & writeEnableA) :
                1'b1 ? (bypassEnable & ~writeEnableA & readOutputReady) : 1'b0;



// values that are not yet used

assign addressB <= 9'b000000000;
assign dataInB <= 32'h00000000;
assign writeEnableB <= 1'b0;

assign result <= 32'h00000000;

    

endmodule