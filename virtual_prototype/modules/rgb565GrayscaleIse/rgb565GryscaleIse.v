module rgb565GrayscaleIse #(parameter [7:0] customInstructionId = 8'd0)
(
    input wire start,
    input wire [31:0] valueA,
    input wire [7:0] iseId,
    output wire done,
    output wire [31:0] result
);


    wire enableGeneral;
    wire [31:0] red, green, blue;





    assign red = {27'b0, valueA[15:11]};
    assign green = {26'b0, valueA[10:5]};
    assign blue = {27'b0, valueA[4:0]};

    assign enableGeneral = (iseId == customInstructionId && start == 1'b1);

endmodule


