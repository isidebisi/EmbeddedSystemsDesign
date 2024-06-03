module dualPortSSRAM #( parameter bitwidth = 8,
                        parameter nrOfEntries = 512)
                      ( input wire                             clockA, clockB,
                                                               writeEnableA, writeEnableB,
                        input wire[1:0]                        offsetA,                                       
                        input wire [$clog2(nrOfEntries)-1 : 0] addressA, addressB,
                        input wire [bitwidth-1 : 0]            dataInA, dataInB,
                        output reg [bitwidth-1 : 0]            dataOutA, dataOutB);
  
  reg [bitwidth-1 : 0] memoryContent [nrOfEntries-1 : 0];
  
always @(posedge clockA)
begin
  if (writeEnableA == 1'b1) memoryContent[addressA] = dataInA;
  
  case (offsetA)
    2'b00: dataOutA = memoryContent[addressA];
    2'b01: dataOutA = {memoryContent[addressA][24:8], memoryContent[addressA+1][31:24]};
    2'b10: dataOutA = {memoryContent[addressA][16:0], memoryContent[addressA+1][31:16]};
    2'b11: dataOutA = {memoryContent[addressA][8:0], memoryContent[addressA+1][31:8]};
  endcase
end

  always @(posedge clockB)
    begin
      if (writeEnableB == 1'b1) memoryContent[addressB] = dataInB;
      dataOutB = memoryContent[addressB];
    end

endmodule

