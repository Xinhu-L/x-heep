module tinyODIN_OBI_wrapper
import obi_pkg::*; 
#(
    parameter N = 256,
	parameter M = 8
  	parameter type         req_t = obi_pkg::obi_req_t, // OBI request type
  	parameter type         rsp_t = obi_pkg::obi_resp_t,  // OBI response type
) (
    input  wire           CLK,
    input  wire           RST,
    
    // SPI slave        -------------------------------
    input  wire           SCK,
    input  wire           MOSI,
    output wire           MISO,

    // OBI     --------------------------------
	input  req_t    odin_slave_req_i,
	output rsp_t    odin_slave_resp_o,
	output req_t    odin_master_req_o,
	input  rsp_t    odin_master_resp_i,

    output wire           SCHED_FULL
);


tinyODIN
#(
    .N(256),
    .M(8),
    .req_t(req_t),
    .rsp_t(rsp_t)
) tinyODIN_i(
    .CLK,
    .RST,

    .SCK,
    .MOSI,
    .MISO,

    .AERIN_ADDR(),
    .AERIN_REQ(),
    .AERIN_ACK(),

    .AEROUT_ADDR(),
    .AEROUT_REQ(),
    .AEROUT_ACK(),
    
    .SCHED_FULL()
)

OBI_to_AER
#(

) obi_to_aer_i(
    
)

    
endmodule