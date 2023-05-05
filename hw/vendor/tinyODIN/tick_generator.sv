module tick_generator
import obi_pkg::*;
#(
    parameter N = 256,
    parameter M = 8,
    parameter INPUT_RESO = 8,
    parameter type         req_t = obi_pkg::obi_req_t, // OBI request type
    parameter type         rsp_t = obi_pkg::obi_resp_t  // OBI response type

) (
    input   logic                       CLK,
    input   logic                       RST,

    input   logic                       spikecore_done_i,
    input   logic                       ODIN_done_i,

    output  logic   [INPUT_RESO-1:0]    tick_o,
    output  logic                       next_tick_o
    
);

logic [INPUT_RESO-1:0]  tick;

assign next_tick_o = spikecore_done_i && ODIN_done_i;


always_ff @( posedge CLK or posedge RST ) begin 
    if(RST)begin
        tick <= 'b1;
    end
    else if (spikecore_done_i && ODIN_done_i) begin
        tick        <= tick - 'b1;
    end 
    else begin
        tick        <= tick;
    end
end

    
endmodule