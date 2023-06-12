module spike_output #(
    parameter   N = 256
) (
    input   logic                           spike_i,
    input   logic   [$clog2(N)-1:0]         count_i,
    input   logic   [7:0]tick_i,
    output  logic                           spike_pushback_o,
    output  logic   [$clog2(N)-1:0]       spike_pushback_addr_o,
    output  logic                           inference_done_o
);

    assign spike_pushback_o = spike_i;
    assign spike_pushback_addr_o = count_i;

    // assign inference_done_o = ((count_i > 8'd245) && spike_i) || (tick_i==8'd1);
    assign inference_done_o = (tick_i==8'd255);

    
endmodule