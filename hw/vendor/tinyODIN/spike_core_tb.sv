`timescale 1ns/1ps

`include "./obi_pkg.sv"
import obi_pkg::*;
module spike_core_tb();
parameter N = 256;
parameter M = 8;
parameter INPUT_RESO = 8;
parameter type req_t = obi_pkg::obi_req_t;
parameter type rsp_t = obi_pkg::obi_resp_t;


bit CLK;
bit RSTN;
bit RST;
assign RST = ~RSTN;

initial begin
    CLK = 1'b0;
    apply_rst();
end
always #5 CLK = ~CLK;

// Signals of Top-Level Module
logic start;
logic control;

logic [INPUT_RESO-1:0] tick;
logic                  next_tick;

req_t spikecore_slave_req;
rsp_t spikecore_slave_resp;
logic spikecore_done;

logic LIF_busy;
logic LIF_done;

logic [M+1:0] AER_ADDR;
logic AER_REQ;
logic AER_ACK;

logic AEROUT_REQ;
logic out_spike_addr;



// MUX

logic [$clog2(N)-3:0]   ADDR_i;

// Testbench of spikeSRAM
logic EN;
logic WE;
logic [$clog2(N)-3:0]   ADDR;
logic [31:0]            WDATA;
logic [31:0]            RDATA;

// Test signal of fitler
logic [31:0]            filter_spike;
logic [$clog2(N)-3:0]   filter_addr;
logic                   filtering;

// Test signal of FIFO
logic                   FIFO_w_en;
logic                   FIFO_r_en;
logic                   FIFO_full;
logic                   FIFO_empty;
logic [M-1:0]           FIFO_w_data;
logic [M-1:0]           FIFO_r_data;

// TOP LEVEL SIMULATION
req_t tinyODIN_slave_req;
rsp_t tinyODIN_slave_resp;

initial begin
    #100
        AEROUT_REQ = 1'b0;
        write_weight_spikecore(
            .addr(6'd0),
            .data(32'hfffe_ffaf),
            .tinyODIN_slave_req_i(tinyODIN_slave_req));


end

TTFS_tinyODIN
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO),
    .req_t(req_t),
    .rsp_t(rsp_t)
) TTFS_tinyODIN (
    .CLK,
    .RST,
    .tinyODIN_slave_req_i(tinyODIN_slave_req),
    .tinyODIN_slave_resp_o(tinyODIN_slave_resp)
);


// spike_core
// #(
//     .N(N),
//     .M(M),
//     .INPUT_RESO(INPUT_RESO),
//     .req_t(req_t),
//     .rsp_t(rsp_t)
// )spike_core_top(
//     .CLK,
//     .RSTN,
//     .start_i(start),
//     .control_i(control),
//     .tick_i(tick),
//     .next_tick_i(next_tick),
//     .spikecore_slave_req_i(spikecore_slave_req),
//     .spikecore_slave_resp_o(spikecore_slave_resp),
//     .spikecore_done_o(spikecore_done),

//     .FIFO_r_en_i(FIFO_r_en),
//     .FIFO_r_data_o(FIFO_r_data),
//     .FIFO_empty_o(FIFO_empty),

//     // .LIF_busy_i(LIF_busy),
//     // .LIF_done_i(LIF_done),
//     // .AER_ADDR_o(AER_ADDR),
//     // .AER_REQ_o(AER_REQ),
//     // .AER_ACK_i(AER_ACK),
//     .AEROUT_REQ_i(AEROUT_REQ),
//     .out_spike_addr_i(out_spike_addr)
// );


// sram_spike sram_spike_tb (
//     .CLK,
//     .EN(filtering|EN),
//     .WE,
//     .ADDR(ADDR_i),
//     .WDATA,
//     .RDATA
// );

// spike_filter 
// #(
//     .req_t(req_t),
//     .resp_t(rsp_t)
// )spike_filter_tb(
//     .CLK,
//     .RSTN,
//     .start_i(start),
//     .filter_spike_i(RDATA),
//     .filter_addr_o(filter_addr),
//     .filter_o(filtering),
//     .tick_i(tick),
//     .FIFO_w_en_o(FIFO_w_en),
//     .FIFO_w_data_o(FIFO_w_data),
//     .FIFO_empty_i(FIFO_empty),
//     .FIFO_full_i(FIFO_full)
// );

// spike_FIFO
// #(
//     .DEPTH(256),
//     .M(8)
// ) spike_FIFO_tb (
//     .CLK,
//     .RSTN,
//     .FIFO_w_en_i(FIFO_w_en),
//     .FIFO_r_en_i(FIFO_r_en),
//     .FIFO_full_o(FIFO_full),
//     .FIFO_empty_o(FIFO_empty),
//     .FIFO_w_data_i(FIFO_w_data),
//     .FIFO_r_data_o(FIFO_r_data)
// );

// spike_out
// #(
//     .N(256),
//     .M(8)
// ) spike_out_tb(
//     .CLK,
//     .RSTN,
//     .start_i(start),
//     .LIF_busy_i(LIF_busy),
//     .LIF_done_i(LIF_done),
//     .control_i(control),
//     .FIFO_empty_i(FIFO_empty),
//     .FIFO_r_en_o(FIFO_r_en),
//     .FIFO_r_data_i(FIFO_r_data),
    
//     .AER_ADDR_o(AER_ADDR),
//     .AER_REQ_o(AER_REQ),
//     .AER_ACK_i(AER_ACK)

// );

task write_weight_spikecore(
    input  logic [5:0] addr,
    input  logic [31:0] data,
    output req_t tinyODIN_slave_req_i
);
    #5 
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {26'b0,addr};
    tinyODIN_slave_req_i.wdata = data; 

    #5
    tinyODIN_slave_req_i.req  = 1'b1;
endtask

task read_weight_spikecore(
    input  logic [5:0] addr,
    input  logic [31:0] data,
    output req_t tinyODIN_slave_req_i
);
    #5 
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b0;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {26'b0,addr};
    tinyODIN_slave_req_i.wdata = data; 
    #5 
    tinyODIN_slave_req_i.req  = 1'b0;
endtask

task apply_rst();
    #10 RSTN <= 1;
    #10 RSTN <= 0;
    #10 RSTN <= 1;
endtask

endmodule