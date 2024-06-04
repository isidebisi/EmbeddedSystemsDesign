module pixelDifferenceCi #( parameter[7:0] customId = 8'h00 )
                  ( input wire        start,
                    input wire [31:0] valueA,
                                      valueB,
                    input wire [7:0]  ciN,
                    output wire       done,
                    output wire [31:0] result );

    wire s_isMyCi = (ciN == customId) ? start : 1'b0;
  
    assign done = s_isMyCi;

    /* PixelDifferenceCi takes 8 pixels as input and produces 4 booleans as output.
     * The pixels come in valueA and valueB. 
     * 
     */

    wire[7:0] pixelA0in = valueA[31:24];
    wire[7:0] pixelA1in = valueA[23:16];
    wire[7:0] pixelA2in = valueA[15:8];
    wire[7:0] pixelA3in = valueA[7:0];
    wire[7:0] pixelB0in = valueB[31:24];
    wire[7:0] pixelB1in = valueB[23:16];
    wire[7:0] pixelB2in = valueB[15:8];
    wire[7:0] pixelB3in = valueB[7:0];

    wire difference0 = pixelA0in != pixelB0in;
    wire difference1 = pixelA1in != pixelB1in;
    wire difference2 = pixelA2in != pixelB2in;
    wire difference3 = pixelA3in != pixelB3in;
    wire[2:0] difference = difference0 + difference1 + difference2 + difference3;

    assign result = s_isMyCi ? {29'b0, difference} : 32'b0;
endmodule