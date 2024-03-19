module profileCi #( parameter[7:0] customId = 8'h00) 
                (input wire start,
                            clock,
                            reset,
                            stall,
                            busIdle,
                input wire [31:0] valueA, valueB,
                input wire [7:0] cIn,
                output wire done,
                output wire [31:0] result);

wire [31:0] s_value0, s_value1, s_value2, s_value3;
reg r_enable0, r_enable1, r_enable2, r_enable3;
wire s_reset0, s_reset1, s_reset2, s_reset3;
wire enableGeneral;

counter #(.WIDTH(32)) Counter0 
    ( .reset(s_reset0),
      .clock(clock),
      .enable(r_enable0),
      .direction(1'b1),
      .counterValue(s_value0));

counter #(.WIDTH(32)) Counter1 
        ( .reset(s_reset1),
            .clock(clock),
            .enable(r_enable1 && stall),
            .direction(1'b1),
            .counterValue(s_value1));

counter #(.WIDTH(32)) Counter2 
        ( .reset(s_reset2),
            .clock(clock),
            .enable(r_enable2 && busIdle),
            .direction(1'b1),
            .counterValue(s_value2));

counter #(.WIDTH(32)) Counter3 
        ( .reset(s_reset3),
            .clock(clock),
            .enable(r_enable3),
            .direction(1'b1),
            .counterValue(s_value3));


always @(posedge clock or posedge reset) begin
    if (reset) begin
        r_enable0 <= 1'b0;
        r_enable1 <= 1'b0;
        r_enable2 <= 1'b0;
        r_enable3 <= 1'b0;
    end else begin
        r_enable0 <=    enableGeneral ? 
                        (valueB[4] == 1'b1) ? 1'b0 :
                        (valueB[0] == 1'b1) ? 1'b1 : r_enable0 : r_enable0;

        r_enable1 <=    enableGeneral ? 
                        (valueB[5] == 1'b1) ? 1'b0 :
                        (valueB[1] == 1'b1) ? 1'b1 : r_enable1 : r_enable1;

        r_enable2 <=    enableGeneral ?
                        (valueB[6] == 1'b1) ? 1'b0 :
                        (valueB[2] == 1'b1) ? 1'b1 : r_enable2 : r_enable2;

        r_enable3 <=    enableGeneral ?
                        (valueB[7] == 1'b1) ? 1'b0 :
                        (valueB[3] == 1'b1) ? 1'b1 : r_enable3 : r_enable3;
    end
end


assign enableGeneral = (cIn == customId && start == 1'b1);

assign done = enableGeneral;

assign result = (enableGeneral) ? 
                (valueA[1:0] == 2'd0) ? s_value0 :
                (valueA[1:0] == 2'd1) ? s_value1 :
                (valueA[1:0] == 2'd2) ? s_value2 :
                (valueA[1:0] == 2'd3) ? s_value3 : 32'h00000000: 32'h00000000;

assign s_reset0 = (reset == 1'b1) || (enableGeneral && valueB[8] == 1'b1);
assign s_reset1 = (reset == 1'b1) || (enableGeneral && valueB[9] == 1'b1);
assign s_reset2 = (reset == 1'b1) || (enableGeneral && valueB[10] == 1'b1);
assign s_reset3 = (reset == 1'b1) || (enableGeneral && valueB[11] == 1'b1);


endmodule