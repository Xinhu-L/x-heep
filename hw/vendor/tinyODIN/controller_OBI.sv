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
// "controller.v" - Controller module
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
module controller
import obi_pkg::*;
#(
    parameter N = 256,
    parameter M = 8,
    parameter INPUT_RESO = 8,
    parameter type              req_t = logic, // OBI request type
    parameter type              rsp_t = logic  // OBI response type
)(    

    // Global inputs ------------------------------------------
    input  logic            CLK,
    input  logic            RST,
    
    // Inputs from AER ----------------------------------------
    // input  logic   [M+1:0]  AERIN_ADDR,
    // input  logic            AERIN_REQ,
    // output logic            AERIN_ACK,
    
    // Read FIFO ----------------------------------------------
    output logic            FIFO_r_en_o,
    input  logic [   M-1:0] FIFO_r_data_i,      
    input  logic            FIFO_empty_i,      

    // Spikecore finished -------------------------------------
    input logic             spikecore_done_i,
    
    // Control interface for readback -------------------------
    input  req_t            control_slave_req_i,
    output rsp_t            control_slave_resp_o,

    // Output control signal
    output logic            start_o,

    output logic            open_loop_o,
    output logic            aer_src_ctrl_neuron_o,
    output logic [   M-1:0] max_neuron_o,
    output logic [   M-1:0] count_o,
    
    // Done output
    output logic            ODIN_done_o,

    // Output to neuroncore
    output logic [   M-1:0] neuron_idx_o,
    output logic            neuron_write_o,
    output logic            neuron_event_o,
    output logic            neuron_tref_o,
    
    // Input from tick-gen
    input  logic            next_tick_i
    
    
);
    
	//----------------------------------------------------------------------------------
	//	PARAMETERS 
	//----------------------------------------------------------------------------------

	// FSM states 
    logic   [3:0]                       state,next_state;
	localparam WAIT                     = 4'd0; 
    localparam WAIT_SPIKE               = 4'd1;
    localparam READ_FIFO                = 4'd2;
    localparam COMPUTE                  = 4'd3;
    localparam NEURON_EVENT_READ        = 4'd4;
    localparam NEURON_EVENT_WRITE       = 4'd5;
    localparam TREF                     = 4'd6;
    localparam WAIT_NEXT_TICK           = 4'd7;
    // TODO: neuron event and refevent



	//----------------------------------------------------------------------------------
	//	REGS & WIRES
	//----------------------------------------------------------------------------------
    
    logic   [31:0]          config_reg;

    logic   [ 8:0]          count;


	//----------------------------------------------------------------------------------
	//	EVENT TYPE DECODING 
	//----------------------------------------------------------------------------------

    assign start            = config_reg[10];
    assign neur_event       = config_reg[9];
    assign tref_event       = config_reg[8];
    assign event_addr       = config_reg[7:0];

	//----------------------------------------------------------------------------------
	//	Config register
	//----------------------------------------------------------------------------------
    
    always_ff @(posedge CLK, posedge RST) begin : blockName
        if (!RST) begin
            control_slave_resp_o.rdata  <=  'b0;
            control_slave_resp_o.gnt    <=  'b0;
            control_slave_resp_o.rvalid <=  'b0;
            config_reg                  <=  'b0; 
        end
        else if(control_slave_req_i.req && control_slave_req_i.we) begin
            control_slave_resp_o.gnt    <=  1'b1;
            control_slave_resp_o.rdata  <=  'b0;
            control_slave_resp_o.rvalid <=  1'b0;
            config_reg                  <=  control_slave_req_i.wdata;
        end
        else if (control_slave_req_i.req && !control_slave_req_i.we) begin
            control_slave_resp_o.gnt    <=  1'b1;
            control_slave_resp_o.rdata  <=  config_reg;
            control_slave_resp_o.rvalid <=  1'b1;
        end
        else begin
            control_slave_resp_o.rdata  <=  'b0;
            control_slave_resp_o.gnt    <=  'b0;
            control_slave_resp_o.rvalid <=  'b0; 
        end
    end
    assign max_neuron_o         =       config_reg[31:24];
    assign open_loop_o          =       config_reg[23];  
    assign aer_src_ctrl_neuron_o=       config_reg[22];  

	//----------------------------------------------------------------------------------
	//	CONTROL FSM
	//----------------------------------------------------------------------------------
    
    // State register
	always_ff @(posedge CLK or posedge RST)
	begin
		if   (RST) state <= WAIT;
		else       state <= next_state;
	end
    
	// Next state logic
	always_comb begin 
        case(state)
        WAIT:	
                if(start)                               next_state = WAIT_SPIKE;
                else                                    next_state = WAIT;
        WAIT_SPIKE:
                if(spikecore_done_i)                  next_state = READ_FIFO;
                else                                    next_state = WAIT_SPIKE;
        READ_FIFO:
                if(FIFO_empty_i)                        next_state = WAIT_NEXT_TICK;
                else                                    next_state = NEURON_EVENT_READ;
        NEURON_EVENT_READ:
                                                        next_state = NEURON_EVENT_WRITE;
        NEURON_EVENT_WRITE:
                if(count == max_neuron_o)               next_state = READ_FIFO;
                else                                    next_state = NEURON_EVENT_READ;
        TREF:
                if(count == max_neuron_o)               next_state = WAIT_NEXT_TICK;
                else                                    next_state = TREF;
        WAIT_NEXT_TICK:
                if(next_tick_i)                         next_state = WAIT_SPIKE;
                else                                    next_state = WAIT;
        default:                                        next_state = state;
		endcase 
    end
		
    // Time-multiplexed neuron counter
	always @(posedge CLK, posedge RST)
		if      (RST)                                count <= 8'd0;
        else if (state == WAIT || count == 8'd63)    count <= 8'd0;
		else if (state == NEURON_EVENT_WRITE || state == TREF)  count <= count + 8'd1;
        else                                         count <= count;
      

    // Output logic      
    always @(*) begin
        if (state == WAIT) begin
            FIFO_r_en_o     = 'b1;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b0;
            neuron_tref_o   = 'b0;
            start_o         = 'b0;
            count_o         = 'b0;
            neuron_write_o  = 'b0;
            ODIN_done_o     = 'b0;
        end
        else if (state == WAIT_SPIKE) begin
            FIFO_r_en_o     = 'b1;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b0;
            neuron_tref_o   = 'b0;
            start_o         = 'b1;
            count_o         = count;
            neuron_write_o  = 'b0;
            ODIN_done_o     = 'b0;
        end
        else if (state == READ_FIFO) begin
            FIFO_r_en_o     = 'b1;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b0;
            neuron_tref_o   = 'b0;
            start_o         = 'b1;
            count_o         = count;
            neuron_write_o  = 'b0;
            ODIN_done_o     = 'b0;
        end
        else if (state == NEURON_EVENT_READ) begin
            FIFO_r_en_o     = 'b0;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b1;
            neuron_tref_o   = 'b0;
            start_o         = 'b1;
            count_o         = count;
            neuron_write_o  = 'b0;
            ODIN_done_o     = 'b0;
        end
        else if (state == NEURON_EVENT_WRITE) begin
            FIFO_r_en_o     = 'b0;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b1;
            neuron_tref_o   = 'b0;
            start_o         = 'b1;
            count_o         = count;
            neuron_write_o  = 'b1;
            ODIN_done_o     = 'b0;
        end
        else if (state == TREF) begin
            FIFO_r_en_o     = 'b0;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b0;
            neuron_tref_o   = 'b1;
            start_o         = 'b1;
            count_o         = count;
            neuron_write_o  = 'b0;
            ODIN_done_o     = 'b0;
        end
        else if (state == WAIT_NEXT_TICK) begin
            FIFO_r_en_o     = 'b0;
            neuron_idx_o    = 'b0;
            neuron_event_o  = 'b0;
            neuron_tref_o   = 'b1;
            start_o         = 'b1;
            count_o         = count;
            neuron_write_o  = 'b0;
            ODIN_done_o     = 'b1;
        end
        else begin
            FIFO_r_en_o     = FIFO_r_en_o;
            neuron_idx_o    = neuron_idx_o;
            neuron_event_o  = neuron_event_o;
            neuron_tref_o   = neuron_tref_o;
            start_o         = start_o;  
            count_o         = count;
            neuron_write_o  = neuron_write_o;
            ODIN_done_o     = ODIN_done_o;
        end
            
    end
endmodule

