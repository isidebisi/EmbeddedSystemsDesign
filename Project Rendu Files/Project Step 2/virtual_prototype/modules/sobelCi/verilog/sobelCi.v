module sobelCi #( parameter[7:0] customId = 8'h00 )
                  ( input wire        start,
                                      clock,
                    input wire [31:0] valueA,
                                      valueB,
                    input wire [7:0]  ciN,
                    output wire       done,
                    output wire [31:0] result );

    wire s_isMyCi = (ciN == customId) ? start : 1'b0;
  
    assign done = s_isMyCi;

    /* SobelCi takes 8 pixels (8 neighbours of a pixel) as input and produces a single pixel as output.
     * The pixels come in valueA and valueB row wise from top left pixel to bottom right pixel. 
     * Middle pixel is not needed for sobel algorithm as it is always multiplied by 0.
     *
     */

    wire[7:0] pixel0in = valueA[31:24];
    wire[7:0] pixel1in = valueA[23:16];
    wire[7:0] pixel2in = valueA[15:8];
    wire[7:0] pixel3in = valueA[7:0];
    wire[7:0] pixel4in = valueB[31:24];
    wire[7:0] pixel5in = valueB[23:16];
    wire[7:0] pixel6in = valueB[15:8];
    wire[7:0] pixel7in = valueB[7:0];


    wire[16:0] dX, dXplus, dXminus, dY, dYplus, dYminus;

    // Sobel operator
    assign dXplus = pixel2in + pixel4in + pixel4in + pixel7in;
    assign dXminus = pixel0in +  pixel3in + pixel3in + pixel5in;

    //take absolute value of difference
    assign dX = (dXplus > dXminus) ? dXplus - dXminus : dXminus - dXplus;

    assign dYplus = pixel0in + pixel1in + pixel1in + pixel2in;
    assign dYminus = pixel5in + pixel6in + pixel6in + pixel7in;

    //take absolute value of difference
    assign dY = (dYplus > dYminus) ? dYplus - dYminus : dYminus - dYplus;


    assign result = s_isMyCi ? {16'b0, dX + dY} : 32'b0;
endmodule