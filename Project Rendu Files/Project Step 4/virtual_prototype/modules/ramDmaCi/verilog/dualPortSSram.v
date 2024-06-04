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
  case (offsetA)
    2'b00: begin
      if (writeEnableA == 1'b1) memoryContent[addressA] = dataInA;
      dataOutA = memoryContent[addressA];
    end
    2'b01: begin
      if (writeEnableA == 1'b1) begin
        memoryContent[addressA] = {memoryContent[addressA][31:24], dataInA[23:0]};
        memoryContent[addressA+1] = {dataInA[31:24], memoryContent[addressA+1][23:0]};
      end
      dataOutA = {memoryContent[addressA][23:0], memoryContent[addressA+1][31:24]};
    end
    2'b10: begin
      if (writeEnableA == 1'b1) begin
        memoryContent[addressA] = {memoryContent[addressA][31:16], dataInA[15:0]};
        memoryContent[addressA+1] = {dataInA[31:16], memoryContent[addressA+1][15:0]};
      end
      dataOutA = {memoryContent[addressA][15:0], memoryContent[addressA+1][31:16]};
    end
    2'b11: begin
      if (writeEnableA == 1'b1) begin
        memoryContent[addressA] = {memoryContent[addressA][31:8], dataInA[7:0]};
        memoryContent[addressA+1] = {dataInA[31:8], memoryContent[addressA+1][7:0]};
      end
      dataOutA = {memoryContent[addressA][7:0], memoryContent[addressA+1][31:8]};
    end
  endcase
end

  always @(posedge clockB)
    begin
      if (writeEnableB == 1'b1) memoryContent[addressB] = dataInB;
      dataOutB = memoryContent[addressB];
    end

endmodule

