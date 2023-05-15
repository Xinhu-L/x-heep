module TTFS_tinyODIN_charge #(
    parameter                   N = 256,
    parameter                   M = 8,
    parameter                   INPUT_RESO = 8,
    parameter type              req_t = logic, // OBI request type
    parameter type              rsp_t = logic  // OBI response type
) (
    input   logic                               CLK,
    input   logic                               RSTN,

    input  req_t                    tinyODIN_slave_req_i,
    output rsp_t                    tinyODIN_slave_resp_o
);

// TODO:uniform RSTN

// OBI mux interface
req_t                       spikecore_slave_req;
req_t                       neuroncore_slave_req;
req_t                       synapsecore_slave_req;                
req_t                       control_slave_req; 

rsp_t                       spikecore_slave_resp;
rsp_t                       neuroncore_slave_resp;
rsp_t                       synapsecore_slave_resp;
rsp_t                       control_slave_resp;

// logics
logic                       spikecore_done;
logic                       ODIN_done;
logic   [INPUT_RESO-1:0]    tick;
logic                       next_tick;     
logic                       FIFO_r_en;
logic   [M-1:0]             FIFO_r_data;
logic                       FIFO_empty;
logic                       start;
logic                       open_loop;
logic                       aer_src_ctrl_neuron;
logic   [M-1:0]             max_neuron;
logic   [M-1:0]             count;
logic   [M-1:0]             neuron_idx;
logic                       neuron_write;
logic                       neuron_event;
logic                       neuron_tref;
logic   [31:0]              synapse_data;
logic   [31:0]              neuron_state;
logic                       neuron_spike;
logic                       spike_pushback;
logic   [M-1:0]             spike_pushback_addr;
logic                       inference_done;

logic                       charge_enable;
logic   [ 31:0]             synapse_charge;



tinyODIN_OBI_interface
#(
    .req_t(req_t), // OBI request type
    .rsp_t(rsp_t)  // OBI response type
) tinyODIN_OBI_interface_i (
    .tinyODIN_slave_req_i,
    .tinyODIN_slave_resp_o,

    .spikecore_slave_req_o(spikecore_slave_req),
    .spikecore_slave_resp_i(spikecore_slave_resp),

    .neuroncore_slave_req_o(neuroncore_slave_req),
    .neuroncore_slave_resp_i(neuroncore_slave_resp),

    .synapsecore_slave_req_o(synapsecore_slave_req),
    .synapsecore_slave_resp_i(synapsecore_slave_resp),

    .control_slave_req_o(control_slave_req),
    .control_slave_resp_i(control_slave_resp)
);

tick_generator
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO),
    .req_t(req_t),
    .rsp_t(rsp_t)
) tick_generator_i (
    .CLK,
    .RSTN,
    .spikecore_done_i(spikecore_done),
    .ODIN_done_i(ODIN_done),
    .inference_done_i(inference_done),
    .tick_o(tick),
    .next_tick_o(next_tick)
);

spike_core
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO),
    .req_t(req_t),
    .rsp_t(rsp_t)
) spike_core_i (
    .CLK,
    .RSTN,
    .start_i(start),
    .control_i(),

    .tick_i(tick),
    .next_tick_i(next_tick),

    .spikecore_slave_req_i(spikecore_slave_req),
    .spikecore_slave_resp_o(spikecore_slave_resp),
    .spikecore_done_o(spikecore_done),

    .FIFO_r_en_i(FIFO_r_en),
    .FIFO_r_data_o(FIFO_r_data),
    .FIFO_empty_o(FIFO_empty),

    .spike_pushback_i(spike_pushback),
    .spike_pushback_addr_i(spike_pushback_addr)
);


controller_charge
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO),
    .req_t(req_t),
    .rsp_t(rsp_t)
) controller_charge_i (
    .CLK,
    .RSTN,
    
    .control_slave_req_i(control_slave_req),
    .control_slave_resp_o(control_slave_resp),

    .FIFO_r_en_o(FIFO_r_en),
    .FIFO_r_data_i(FIFO_r_data),
    .FIFO_empty_i(FIFO_empty),
    
    .spikecore_done_i(spikecore_done),
    
    .start_o(start),

    .open_loop_o(open_loop),
    .aer_src_ctrl_neuron_o(aer_src_ctrl_neuron),
    .max_neuron_o(max_neuron),
    .count_o(count),

    .ODIN_done_o(ODIN_done),
    
    .neuron_idx_o(neuron_idx),
    .neuron_write_o(neuron_write),
    .neuron_event_o(neuron_event),
    .neuron_tref_o(neuron_tref),

    .next_tick_i(next_tick),

    .charge_enable_o(charge_enable),
    .charge_count_o(),

    .inference_done_i(inference_done)
);

neuron_core_charge
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO),
    .req_t(req_t),
    .rsp_t(rsp_t)
) neuron_core_charge_i
(
    .CLK,
    .RSTN,
    .synapse_data_i(synapse_charge),
    .neuron_event_i(neuron_event),
    .neuron_write_i(neuron_write),
    .neuron_tref_i(neuron_tref),
    .neuron_idx_i(neuron_idx),
    .count_i(count),

    .neuron_state_o(neuron_state),
    .neuron_spike_o(neuron_spike),

    .neuroncore_slave_req_i(neuroncore_slave_req),
    .neuroncore_slave_resp_o(neuroncore_slave_resp),

    .tick_o(tick)

);

synaptic_core
#(
    .N(N),
    .M(M),
    .req_t(req_t),
    .rsp_t(rsp_t)
) synaptic_core_i (
    .CLK,
    .RSTN,

    .neuron_event_i(neuron_event),
    .neuron_idx_i(neuron_idx),
    .count_i(count),
    .charge_enable_i(charge_enable),

    .synapse_data_o(synapse_data),

    .synapsecore_slave_req_i(synapsecore_slave_req),
    .synapsecore_slave_resp_o(synapsecore_slave_resp)
);

spike_output
#(
    .M(M)
) spike_pushback_i (
    .spike_i(neuron_spike),
    .neuron_idx_i(count),
    .spike_pushback_o(spike_pushback),
    .spike_pushback_addr_o(spike_pushback_addr),
    .inference_done_o(inference_done)
);

charger
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO)
) charger_i (
    .CLK,
    .RSTN,
    .count_i(count[7:2]),
    .synapse_data_i(synapse_data),
    .charge_count_i(count[4:0]),
    .charge_ref_count_i(count[7:3]),
    .chagre_enable_i(charge_enable),
    .event_tref_i(neuron_tref),
    .synapse_charge_o(synapse_charge)
);


endmodule