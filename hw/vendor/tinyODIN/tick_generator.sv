module tick_generator
import obi_pkg::*;
#(
    parameter N = 256,
    parameter type         req_t = logic, // OBI request type
    parameter type         rsp_t = logic  // OBI response type

) (
    input   logic                       CLK,
    input   logic                       RSTN,

    input   logic                       spikecore_done_i,
    input   logic                       ODIN_done_i,

    input   logic                       inference_done_i,

    output  logic   [7:0]    tick_o,
    output  logic                       next_tick_o
    
);

logic   [7:0]   tick;

assign tick_o           = tick;
assign next_tick_o      = spikecore_done_i && ODIN_done_i;


always_ff @( posedge CLK or negedge RSTN ) begin 
    if(!RSTN)begin
        tick        <= 8'd0;
    end
    else if (inference_done_i) begin
        tick        <= 8'd0;
    end
    else if (spikecore_done_i && ODIN_done_i) begin
        tick        <= tick + 'b1;
    end 
    else begin
        tick        <= tick;
    end
end

    
endmodule