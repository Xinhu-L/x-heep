// Copyright (C) 2019-2022, Université catholique de Louvain (UCLouvain, Belgium), University of Zürich (UZH, Switzerland),
//         Katholieke Universiteit Leuven (KU Leuven, Belgium), and Delft University of Technology (TU Delft, Netherlands).
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// Licensed under the Solderpad Hardware License v 2.1 (the “License”); you may not use this file except in compliance
// with the License, or, at your option, the Apache License version 2.0. You may obtain a copy of the License at
// https://solderpad.org/licenses/SHL-2.1/
//
// Unless required by applicable law or agreed to in writing, any work distributed under the License is distributed on
// an “AS IS” BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
//------------------------------------------------------------------------------
//
// "synaptic_core.v" - File containing the time-multiplexed synaptic array based on 4-bit-weight fixed synapses
//                     (as opposed to ODIN, which had 3-bit-weight synapses with SDSP online learning)
// 
// Project: tinyODIN - A low-cost digital spiking neuromorphic processor adapted from ODIN.
//
// Author:  C. Frenkel, Delft University of Technology
//
// Cite/paper: C. Frenkel, M. Lefebvre, J.-D. Legat and D. Bol, "A 0.086-mm² 12.7-pJ/SOP 64k-Synapse 256-Neuron Online-Learning
//             Digital Spiking Neuromorphic Processor in 28-nm CMOS," IEEE Transactions on Biomedical Circuits and Systems,
//             vol. 13, no. 1, pp. 145-158, 2019.
//
//------------------------------------------------------------------------------


module synaptic_core #(
    parameter N = 256,
    parameter M = 8,
    parameter type              req_t = logic, // OBI request type
    parameter type              rsp_t = logic  // OBI response type
)(
    
    // Global inputs ------------------------------------------
    input   logic                   CLK,
    input   logic                   RST,
    
    // Inputs from controller ---------------------------------
    input   logic                   neuron_event_i,
    input   logic [   M-1:0]        neuron_idx_i,
    input   logic [   M-1:0]        count_i,
    
    // Outputs ------------------------------------------------
    output  logic [   31:0]         synapse_data_o,
    
    // Form OBI BUS -------------------------------------------
    input   req_t                    synapsecore_slave_req_i,
    output  rsp_t                    synapsecore_slave_resp_o
);

    // SRAM control signal defined here
    logic           synarray_cs,synarray_we;
    logic [12:0]    synarray_addr;
    logic [31:0]    synarray_wdata,synarray_rdata; 
    logic           synarray_valid;
       
    assign synarray_cs      = neuron_event_i || synapsecore_slave_req_i.req;
    assign synarray_we      = synapsecore_slave_req_i.we;
    assign synarray_addr    = neuron_event_i ? {neuron_idx_i,count_i[7:3]} : synapsecore_slave_req_i.req ? synapsecore_slave_req_i.addr : 'b0;
    assign synapse_data_o   = synarray_rdata;
    // OBI interface

    always_ff @(posedge CLK or posedge RST) begin
        if (RST) begin
        synarray_valid <= '0;
        end else begin
        synarray_valid <= synapsecore_slave_resp_o.gnt;
        end
      end
  
      assign synapsecore_slave_resp_o.gnt = synapsecore_slave_req_i.req;
      assign synapsecore_slave_resp_o.rvalid = synarray_valid;
    
    genvar i;
    // Updated or configured weights to be written to the synaptic memory

    // generate
    //     for (i=0; i<4; i=i+1) begin
    //         assign synarray_wdata[(i<<3)+7:(i<<3)] = (i == CTRL_SPI_ADDR[14:13])
    //                                                ? ((CTRL_PROG_DATA[M-1:0] & ~CTRL_PROG_DATA[2*M-1:M]) | (SYNARRAY_RDATA[(i<<3)+7:(i<<3)] & CTRL_PROG_DATA[2*M-1:M]))
    //                                                : SYNARRAY_RDATA[(i<<3)+7:(i<<3)];
    //     end
    // endgenerate
    
    
    // Synaptic memory wrapper

    SRAM_8192x32_wrapper synarray_0 (
        
        // Global inputs
        .CK         (CLK),
	
		// Control and data inputs
		.CS         (synarray_cs),
		.WE         (synarray_we),
		.A			(synarray_addr),
		.D			(synarray_wdata),
		
		// Data output
		.Q			(synarray_rdata)
    );


endmodule




module SRAM_8192x32_wrapper (

    // Global inputs
    input         CK,                       // Clock (synchronous read/write)

    // Control and data inputs
    input         CS,                       // Chip select
    input         WE,                       // Write enable
    input  [12:0] A,                        // Address bus 
    input  [31:0] D,                        // Data input bus (write)

    // Data output
    output [31:0] Q                         // Data output bus (read)   
);


    /*
     *  Simple behavioral code for simulation, to be replaced by a 8192-word 32-bit SRAM macro 
     *  or Block RAM (BRAM) memory with the same format for FPGA implementations.
     */      
        reg [31:0] SRAM[8191:0];
        reg [31:0] Qr;

        always @(posedge CK) begin
            Qr <= CS ? SRAM[A] : Qr;
            if (CS & WE) SRAM[A] <= D;
        end
        assign Q = Qr;
    
endmodule
