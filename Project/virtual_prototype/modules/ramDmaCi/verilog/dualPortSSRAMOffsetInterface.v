module dualPortSSRAMOffsetInterface #( parameter bitwidth = 32,
                        parameter nrOfEntries = 640)
                      ( input wire                             clockA, clockB,
                                                               writeEnableA, writeEnableB,
                        input wire [1:0]                                       offsetA,
                        input wire [$clog2(nrOfEntries)-1 : 0] addressA, addressB,
                        input wire [bitwidth-1 : 0]            dataInA, dataInB,
                        output wire [bitwidth-1 : 0]            dataOutA, dataOutB);
  


wire [$clog2(nrOfEntries)-1 : 0]    addressAWoffset0, addressAWoffset1, addressAWoffset2, addressAWoffset3;
wire [bitwidth/4-1 : 0] dataOutA0, dataOutA1, dataOutA2, dataOutA3,
                        dataOutB0, dataOutB1, dataOutB2, dataOutB3,
                        dataInA0, dataInA1, dataInA2, dataInA3,
                        dataInB0, dataInB1, dataInB2, dataInB3;


assign dataInB0 = dataInB[bitwidth-1:bitwidth/4*3];
assign dataInB1 = dataInB[bitwidth/4*3-1:bitwidth/2];
assign dataInB2 = dataInB[bitwidth/2-1:bitwidth/4];
assign dataInB3 = dataInB[bitwidth/4-1:0];

assign dataOutB = {dataOutB0, dataOutB1, dataOutB2, dataOutB3};


assign dataInA0 =   (offsetA == 2'b00) ? dataInA[bitwidth-1:bitwidth/4*3] :
                    (offsetA == 2'b01) ? dataInA[bitwidth/4-1:0] :
                    (offsetA == 2'b10) ? dataInA[bitwidth/2-1:bitwidth/4] :
                                         dataInA[bitwidth/4*3-1:bitwidth/2];

assign dataInA1 =   (offsetA == 2'b00) ? dataInA[bitwidth/4*3-1:bitwidth/2] :
                    (offsetA == 2'b01) ? dataInA[bitwidth-1:bitwidth/4*3] :
                    (offsetA == 2'b10) ? dataInA[bitwidth/4-1:0] :
                                         dataInA[bitwidth/2-1:bitwidth/4];

assign dataInA2 =   (offsetA == 2'b00) ? dataInA[bitwidth/2-1:bitwidth/4] :
                    (offsetA == 2'b01) ? dataInA[bitwidth/4*3-1:bitwidth/2] :
                    (offsetA == 2'b10) ? dataInA[bitwidth-1:bitwidth/4*3] :
                                         dataInA[bitwidth/4-1:0];

assign dataInA3 =   (offsetA == 2'b00) ? dataInA[bitwidth/4-1:0] :
                    (offsetA == 2'b01) ? dataInA[bitwidth/2-1:bitwidth/4] :
                    (offsetA == 2'b10) ? dataInA[bitwidth/4*3-1:bitwidth/2] :
                                         dataInA[bitwidth-1:bitwidth/4*3];

assign addressAWoffset0 = (offsetA == 2'b00) ? addressA : addressA + 1;
assign addressAWoffset1 = (offsetA == 2'b00 || offsetA == 2'b01) ? addressA : addressA + 1;
assign addressAWoffset2 = (offsetA == 2'b11) ? addressA + 1 : addressA;
assign addressAWoffset3 = addressA;

assign dataOutA =   (offsetA == 2'b00) ? {dataOutA0, dataOutA1, dataOutA2, dataOutA3} :
                    (offsetA == 2'b01) ? {dataOutA1, dataOutA2, dataOutA3, dataOutA0} :
                    (offsetA == 2'b10) ? {dataOutA2, dataOutA3, dataOutA0, dataOutA1} :
                    {dataOutA3, dataOutA0, dataOutA1, dataOutA2};



dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAMByte0
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressAWoffset0), .addressB(addressB),
     .dataInA(dataInA0), .dataInB(dataInB0),
     .dataOutA(dataOutA0), .dataOutB(dataOutB0));

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAMByte1
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressAWoffset1), .addressB(addressB),
     .dataInA(dataInA1), .dataInB(dataInB1),
     .dataOutA(dataOutA1), .dataOutB(dataOutB1));

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAMByte2
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressAWoffset2), .addressB(addressB),
     .dataInA(dataInA2), .dataInB(dataInB2),
     .dataOutA(dataOutA2), .dataOutB(dataOutB2));

dualPortSSRAM #(.bitwidth(bitwidth/4), .nrOfEntries(nrOfEntries)) dualPortSSRAMByte3
    (.clockA(clockA), .clockB(clockB),
     .writeEnableA(writeEnableA), .writeEnableB(writeEnableB),
     .addressA(addressAWoffset3), .addressB(addressB),
     .dataInA(dataInA3), .dataInB(dataInB3),
     .dataOutA(dataOutA3), .dataOutB(dataOutB3));

endmodule