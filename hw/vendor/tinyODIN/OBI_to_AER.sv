// Copyright 2021 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
// Translates XBAR_PERIPH_BUS to AER interface

module OBI_to_AER
import obi_pkg::*;
#(
  	parameter type         req_t = logic, // OBI request type
  	parameter type         rsp_t = logic,  // OBI response type
  	parameter int unsigned NUM_NEU = 8 // Address width of AERIN
) (
  	input  logic                    clk_i,    // Clock
  	input  logic                    rst_ni,   // Asynchronous reset active low


	input  req_t  odin_slave_req_i,
	output rsp_t odin_slave_resp_o,
	output req_t  odin_master_req_o,
	input  rsp_t odin_master_resp_i
	// Input 10-bit AER -------------------------------
	output  logic [  NUM_NEU+1:0]   AERIN_ADDR,
	output  logic             		AERIN_REQ,
	input   logic 		        	AERIN_ACK
);

assign AERIN_REQ = obi_req_i.req;
assign AERIN_ADDR = obi_req_i.wdata[NUM_NEU+1:0];


assign obi_resp_o.gnt = AEROUT_ACK;
assign obi_resp_o.rvalid = 'b0;
assign obi_resp_o.rdata[NUM_NEU-1:0] = 32'b0;

endmodule
