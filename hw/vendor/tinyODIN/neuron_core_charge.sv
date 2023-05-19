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
// "neuron_core.v" - File containing the time-multiplexed neuron array based on 12-bit leaky integrate-and-fire (LIF) neurons
//                   (as opposed to ODIN, which both 8-bit LIF and custom phenomenological Izhikevich neuron models)
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

`include "./obi_pkg.sv"

module neuron_core_charge 
import obi_pkg::*;
#(
    parameter N = 256,
    parameter M = 8,
    parameter INPUT_RESO = 8,
    parameter type              req_t = logic, // OBI request type
    parameter type              rsp_t = logic  // OBI response type

)(
    
    // Global inputs ------------------------------------------
    input   logic                       CLK,
    input   logic                       RSTN,
    
    // Synaptic inputs ----------------------------------------
    input   logic [         31:0]       synapse_data_i,
    
    // Inputs from controller ---------------------------------
    input   logic                       neuron_event_i,
    input   logic                       neuron_write_i,
    input   logic                       neuron_tref_i,
    input   logic [   M-1:0]            neuron_idx_i,
    input   logic [   M-1:0]            count_i,
    
    // Outputs ------------------------------------------------
    output  logic [         31:0]       neuron_state_o,
    output  logic                       neuron_spike_o,

    // Input from the OBI bus
    input   req_t                       neuroncore_slave_req_i,
    output  rsp_t                       neuroncore_slave_resp_o,

    // Tick  --------------------------------------------------
    input   logic [  INPUT_RESO-1:0]    tick_o
);
    
    // Chip select
    logic           req_req;
    logic           req_we;

    // Input Weight select
    logic [    7:0] syn_weight;
    logic [   31:0] syn_weight_int;
    
    logic [   11:0] LIF_neuron_next_state;
    logic [   31:0] neuron_data_int, neuron_data;
    logic           LIF_neuron_event_out;

    // Neuron memory wrapper
    logic           neurarray_cs,neurarray_we;
    logic [7:0]     neurarray_addr;
    logic [31:0]    neurarray_wdata,neurarray_rdata; 

    assign syn_weight_int  = synapse_data_i >> ({3'b0,count_i[1:0]} << 3);
    assign syn_weight      = syn_weight_int[7:0];
    
    assign neurarray_cs      = neuroncore_slave_req_i.req ? (neuron_event_i || neuroncore_slave_req_i.req) : (neuron_event_i || req_req);
    assign neurarray_we      = neuroncore_slave_req_i.we ? (neuron_write_i || neuroncore_slave_req_i.we) : (neuron_write_i || req_we);
    assign neurarray_addr    = neuron_event_i ? count_i : neuroncore_slave_req_i.req ? neuroncore_slave_req_i.addr[7:0] : 'b0;
    assign neuron_state_o   = neurarray_rdata;
    assign neurarray_wdata = neuroncore_slave_req_i.req ? neuroncore_slave_req_i.wdata : neuron_data_int;

    //
    always_ff @( posedge CLK or negedge RSTN ) begin 
        if (RSTN) begin
            req_req <= 'b0;
            req_we  <= 'b0;
        end
        
    end

    // OBI interface
    assign neuroncore_slave_resp_o.gnt = neuroncore_slave_req_i.req;
    always_ff @(posedge CLK or negedge RSTN) begin
        if (!RSTN) begin
            neuroncore_slave_resp_o.rvalid <= 1'b0;
        end else begin
            neuroncore_slave_resp_o.rvalid <= neuroncore_slave_resp_o.gnt;
        end
      end
  

      assign neuron_data_int =  {neuron_state_o[31] ? 1'b1 : LIF_neuron_event_out, neuron_state_o[30:12], LIF_neuron_next_state};


    // Neuron update logic for leaky integrate-and-fire (LIF) model

    assign neuron_spike_o = neuron_state_o[31] ? 1'b0 : ((neurarray_cs && neurarray_we) ? LIF_neuron_event_out : 1'b0);
    lif_neuron_charge lif_neuron_charge_i ( 
        .param_enable(                           neuron_state_o[31   ]),
        .param_leak_str(                         neuron_state_o[30:24]),
        .param_thr(                              neuron_state_o[23:12]),
        
        .state_core(                             neuron_state_o[11: 0]),
        .state_core_next(                 LIF_neuron_next_state[11: 0]),
        
        .syn_weight(syn_weight),
        .syn_event(neuron_event_i),
        .time_ref(neuron_tref_i),
        
        .spike_out(LIF_neuron_event_out) 
    );


    // Neuron output spike events

    SRAM_256x32_wrapper neurarray_0 (       
        
        // Global inputs
        .CK         (CLK),
    
        // Control and data inputs
        .CS         (neurarray_cs),
        .WE         (neurarray_we),
        .A          (neurarray_addr),
        .D          (neurarray_wdata),
        
        // Data output
        .Q          (neurarray_rdata)
    );
    

endmodule




module SRAM_256x32_wrapper (

    // Global inputs
    input          CK,                       // Clock (synchronous read/write)

    // Control and data inputs
    input          CS,                       // Chip select
    input          WE,                       // Write enable
    input  [  7:0] A,                        // Address bus 
    input  [ 31:0] D,                        // Data input bus (write)

    // Data output
    output [ 31:0] Q                         // Data output bus (read)   
);


    /*
     *  Simple behavioral code for simulation, to be replaced by a 256-word 32-bit SRAM macro 
     *  or Block RAM (BRAM) memory with the same format for FPGA implementations.
     */      
        reg [31:0] SRAM[255:0];
        reg [31:0] Qr;
        always @(posedge CK) begin
            Qr <= CS ? SRAM[A] : Qr;
            if (CS & WE) SRAM[A] <= D;
        end
        assign Q = Qr;


endmodule
