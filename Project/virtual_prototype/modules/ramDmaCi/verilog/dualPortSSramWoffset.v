module dualPortSSRAMWoffset #( parameter bitwidth = 32,
                        parameter nrOfEntries = 640)
                      ( input wire                             clockA, clockB,
                                                               writeEnableA, writeEnableB,
                        input wire [1:0]                                       offsetA,
                        input wire [$clog2(nrOfEntries)-1 : 0] addressA, addressB,
                        input wire [bitwidth-1 : 0]            dataInA, dataInB,
                        output reg [bitwidth-1 : 0]            dataOutA, dataOutB);
  

wire [$clog2(nrOfEntries)+1 : 0]    addressAWoffset0, addressAWoffset1, addressBWoffset0, addressBWoffset1;
wire [bitwidth-1 : 0]               dataOutA0, dataOutA1, dataOutB0, dataOutB1;

wire [$clog2(nrOfEntries)+1 : 0] longAddressB = {addressB, 2'b00};

assign addressAWoffset0 = (offsetA == 2'b00) ? {addressA, 2'b00} : {addressA, 2'b01};
assign addressAWoffset1 = (offsetA == 2'b00 || offsetA == 2'b01) ? {addressA, 2'b00} : {addressA, 2'b01};
assign addressBWoffset2 = (offsetA == 2'b11) ? {addressA, 2'b01} : {addressA, 2'b00};
assign addressBWoffset3 = {addressA, 2'b00};

assign dataOutA0 =  (offsetA == 2'b00) ? {dataOutA0, dataOutA1, dataOutA2, dataOutA3} :
                    (offsetA == 2'b01) ? {dataOutA1, dataOutA2, dataOutA3, dataOutA0} :
                    (offsetA == 2'b10) ? {dataOutA2, dataOutA3, dataOutA0, dataOutA1} :
                    {dataOutA3, dataOutA0, dataOutA1, dataOutA2};

/* For reference this case statement is left here to see what the above assigns are duing.
 * It is a bit easier to read for a human.
always @*
begin
    case(offsetA)
        2'b00: begin
            addressAWoffset0 = {addressA, 2'b00};
            addressAWoffset1 = {addressA, 2'b00};
            addressBWoffset0 = {addressB, 2'b00};
            addressBWoffset1 = {addressB, 2'b00};
            dataOutA = {dataOutA0, dataOutA1, dataOutA2, dataOutA3};
        end
        2'b01: begin
            addressAWoffset0 = {addressA, 2'b01};
            addressAWoffset1 = {addressA, 2'b00};
            addressBWoffset0 = {addressB, 2'b00};
            addressBWoffset1 = {addressB, 2'b00};
            dataOutA = {dataOutA1, dataOutA2, dataOutA3, dataOutA0};
        end
        2'b10: begin
            addressAWoffset0 = {addressA, 2'b01};
            addressAWoffset1 = {addressA, 2'b01};
            addressBWoffset0 = {addressB, 2'b00};
            addressBWoffset1 = {addressB, 2'b00};
            dataOutA = {dataOutA2, dataOutA3, dataOutA0, dataOutA1};
        end
        2'b11: begin
            addressAWoffset0 = {addressA, 2'b01};
            addressAWoffset1 = {addressA, 2'b01};
            addressBWoffset0 = {addressB, 2'b01};
            addressBWoffset1 = {addressB, 2'b00};
            dataOutA = {dataOutA3, dataOutA0, dataOutA1, dataOutA2};
        end
    endcase
end
*/

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAM0
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressAWoffset0), .addressB(longAddressB),
     .dataInA(dataInA), .dataInB(dataInB),
     .dataOutA(dataOutA0), .dataOutB(dataOutB0));

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAM1
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressAWoffset1), .addressB(longAddressB),
     .dataInA(dataInA), .dataInB(dataInB),
     .dataOutA(dataOutA1), .dataOutB(dataOutB1));

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAM2
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressBWoffset0), .addressB(longAddressB),
     .dataInA(dataInA), .dataInB(dataInB),
     .dataOutA(dataOutA2), .dataOutB(dataOutB0));

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAM3
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressBWoffset1), .addressB(longAddressB),
     .dataInA(dataInA), .dataInB(dataInB),
     .dataOutA(dataOutA3), .dataOutB(dataOutB1));

endmodule