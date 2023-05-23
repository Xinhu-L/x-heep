`timescale 1ns/1ps

`include "./obi_pkg.sv"
import obi_pkg::*;
module TTFS_tinyODIN_tb();
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
end
always #5 CLK = ~CLK;

// TOP LEVEL SIMULATION
req_t tinyODIN_slave_req;
rsp_t tinyODIN_slave_resp;

// Model
integer model;
logic   [31:0] weight_l1    [0:1873];
logic   [31:0] weight_l2    [0:200];
logic   [31:0] input_spike  [0:35];


initial begin
// Initial neurons
    apply_rst();
    for (int i=0; i<256; ++i) begin
        write_neuroncore(
            .addr(i[7:0]),
            .data(32'h0015_e000),
            .tinyODIN_slave_req_i(tinyODIN_slave_req)); 
        #10 tinyODIN_slave_req.req = 1'b0;
        #10 tinyODIN_slave_req.we  = 1'b0;
    end
// Initial spikecore
    $readmemh("input_spike.txt",input_spike);
    for (int i=0; i<36; ++i) begin
        write_spikecore(
            .addr(i),
            .data(input_spike[i]),
            .tinyODIN_slave_req_i(tinyODIN_slave_req)); 
            #10 tinyODIN_slave_req.req = 1'b0;
            #10 tinyODIN_slave_req.we  = 1'b0;
    end
// Initial synapse
    for (int i=0; i<8192; ++i) begin
        write_synapsecore(
            .addr(i),
            .data(32'h0000_0000),
            .tinyODIN_slave_req_i(tinyODIN_slave_req));
            #10 tinyODIN_slave_req.req = 1'b0;
            #10 tinyODIN_slave_req.we  = 1'b0; 
    end
    $readmemh("model_l1.txt",weight_l1);
    for (int i=0; i<144; ++i) begin
        for (int j=0; j<32; ++j) begin
            if (j<13) begin
                write_synapsecore(
                    .addr(i*32 + j + 18),
                    .data(weight_l1[i*13 + j]),
                    .tinyODIN_slave_req_i(tinyODIN_slave_req));
                    #10 tinyODIN_slave_req.req = 1'b0;
                    #10 tinyODIN_slave_req.we  = 1'b0; 
            end 
        end       
    end

    $readmemh("model_l2.txt",weight_l2);
    for (int i=0; i<100; ++i) begin
        for (int j=0; j<32; ++j) begin
            if (j<2) begin
                write_synapsecore(
                    .addr(i*32 + j +4638),
                    .data(weight_l2[i*2 + j]),
                    .tinyODIN_slave_req_i(tinyODIN_slave_req));
                    #10 tinyODIN_slave_req.req = 1'b0;
                    #10 tinyODIN_slave_req.we  = 1'b0; 
            end
        end       
    end
    write_control(
        .addr(32'h0),
        .data(32'hff64_0400),
        .tinyODIN_slave_req_i(tinyODIN_slave_req));
        #10 tinyODIN_slave_req.req = 1'b0;
        #10 tinyODIN_slave_req.we  = 1'b0; 
end

TTFS_tinyODIN_charge
#(
    .N(N),
    .M(M),
    .INPUT_RESO(INPUT_RESO),
    .req_t(req_t),
    .rsp_t(rsp_t)
) TTFS_tinyODIN_charge (
    .CLK,
    .RSTN,
    .tinyODIN_slave_req_i(tinyODIN_slave_req),
    .tinyODIN_slave_resp_o(tinyODIN_slave_resp)
);

task cancel(
    output req_t tinyODIN_slave_req_i
);
    #10;
    tinyODIN_slave_req_i.req = 1'b0;
    tinyODIN_slave_req_i.we  = 1'b0;
endtask


task write_spikecore(
    input  logic [5:0] addr,
    input  logic [31:0] data,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {24'b0,addr,2'b0};
    tinyODIN_slave_req_i.wdata = data;
endtask

task read_spikecore(
    input  logic [5:0] addr,
    output req_t tinyODIN_slave_req_i
);
    #20; 
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b0;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {24'b0,addr,2'b0};
endtask

task write_neuroncore(
    input  logic [7:0] addr,
    input  logic [31:0] data,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {10'b0,2'b01,10'b0,addr,2'b0};
    tinyODIN_slave_req_i.wdata = data; 
endtask

task read_neuroncore(
    input  logic [7:0] addr,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {10'b0,2'b01,10'b0,addr,2'b0};
endtask

task write_synapsecore(
    input  logic [12:0] addr,
    input  logic [31:0] data,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {10'b0,2'b10,5'b0,addr,2'b0};
    tinyODIN_slave_req_i.wdata = data; 
endtask

task read_synapsecore(
    input  logic [12:0] addr,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {10'b0,2'b10,5'b0,addr,2'b0};
endtask

task write_control(
    input  logic [31:0] addr,
    input  logic [31:0] data,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {10'b0,2'b11,20'b0};
    tinyODIN_slave_req_i.wdata = data; 
endtask

task read_control(
    input  logic [31:0] addr,
    output req_t tinyODIN_slave_req_i
);
    #20;
    tinyODIN_slave_req_i.req  = 1'b1;
    tinyODIN_slave_req_i.we   = 1'b1;
    tinyODIN_slave_req_i.be   = 4'b0000;
    tinyODIN_slave_req_i.addr = {10'b0,2'b11,20'b0};
endtask

task apply_rst();
    #10 RSTN <= 1;
    #10 RSTN <= 0;
    #10 RSTN <= 1;
endtask

endmodule