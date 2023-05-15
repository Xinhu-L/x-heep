module spike_output #(
    parameter   M = 8
) (
    input   logic                   spike_i,
    input   logic   [M-1:0]         neuron_idx_i,
    output  logic                   spike_pushback_o,
    output  logic   [M-1:0]         spike_pushback_addr_o,
    output  logic                   inference_done_o
);

    assign spike_pushback_o = spike_i;
    assign spike_pushback_addr_o = neuron_idx_i;

    assign inference_done = &neuron_idx_i[7:4] && (neuron_idx_i[3:0] > 4'd6);

    
endmodule