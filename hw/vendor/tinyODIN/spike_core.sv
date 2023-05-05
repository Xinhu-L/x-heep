`include "./obi_pkg.sv"
module spike_core 
import obi_pkg::*;
#(
    parameter                   N = 256,
    parameter                   M = 8,
    parameter                   INPUT_RESO = 8,
    parameter type              req_t = logic, // OBI request type
    parameter type              rsp_t = logic  // OBI response type
) (
    input   logic                       CLK,
    input   logic                       RSTN,
    
    //Start Singal from controller
    input   logic                       start_i,
    input   logic                       control_i,

    // TTFS time tick 
    input   logic [INPUT_RESO-1:0]      tick_i,
    input   logic                       next_tick_i,
    
    // OBI BUS Slave interface
    input   req_t                       spikecore_slave_req_i,
    output  rsp_t                       spikecore_slave_resp_o,

    // Spikecore finished
    output  logic                       spikecore_done_o,

    // FIFO output
    input   logic                       FIFO_r_en_i,
    output  logic [M-1:0]               FIFO_r_data_o,
    output  logic                       FIFO_empty_o,

    
    // // LIF signal
    // input  logic                    LIF_busy_i,
    // input  logic                    LIF_done_i,
    // // AERIN signal
    // output logic  [M+1:0]           AER_ADDR_o,
    // output logic                    AER_REQ_o,
    // input  logic                    AER_ACK_i,

    // SPike from AER OUT
    input  logic                    AEROUT_REQ_i,
    input  logic [$clog2(N)-1:0]    out_spike_addr_i
    
    
);

// SRAM slect signal
logic [2:0] SRAM_sel;
logic SRAM_EN;
logic SRAM_WE;
logic [$clog2(N)-3:0] SRAM_ADDR;
logic [31:0] SRAM_WDATA;
logic [31:0] SRAM_RDATA;

// Filter signals
logic [$clog2(N)-3:0] filter_addr;
logic [31:0] filter_spike;
logic filtering;

// FIFO signals
logic FIFO_w_en;
logic FIFO_r_en;
logic FIFO_empty;
logic FIFO_full;
logic [M-1:0] FIFO_w_data;
logic [M-1:0] FIFO_r_data;


assign SRAM_sel = {spikecore_slave_req_i.req, filtering, AEROUT_REQ_i};
always_comb begin : MUX_of_spikeSRAM
    case(SRAM_sel)
    3'b100: begin
        SRAM_EN = spikecore_slave_req_i.req;
        SRAM_WE = spikecore_slave_req_i.we;
        SRAM_ADDR = spikecore_slave_req_i.addr[$clog2(N)-1:0];
        SRAM_WDATA = spikecore_slave_req_i.wdata;
        spikecore_slave_resp_o.rdata = SRAM_RDATA;
    end
    3'b010: begin
        SRAM_EN = filtering;
        SRAM_WE = 1'b0;
        SRAM_ADDR = filter_addr;
        SRAM_WDATA = 32'b0;
        filter_spike = SRAM_RDATA;
    end  
    3'b001: begin
        SRAM_EN = 1'b1;
        SRAM_WE = 1'b1;
        SRAM_ADDR = out_spike_addr_i;
        SRAM_WDATA = tick_i;
    end
    default: begin
        SRAM_EN = SRAM_EN;
        SRAM_WE = SRAM_WE;
        SRAM_ADDR = SRAM_ADDR;
        SRAM_WDATA = SRAM_WDATA;
    end
    endcase
end
// SRAM signal from OBI bus
assign spikecore_slave_resp_o.gnt = spikecore_slave_req_i.req;
assign spikecore_slave_resp_o.rvalid = spikecore_slave_resp_o.gnt;

spike_filter 
#(
    .N(256),
    .M(8),
    .INPUT_RESO(8),
    .req_t(obi_pkg::obi_req_t),
    .resp_t(obi_pkg::obi_req_t)
)spike_filter_i(
    .CLK,
    .RSTN,
    .start_i,

    .filter_spike_i(filter_spike),
    .filter_addr_o(filter_addr),
    .filter_o(filtering),

    .tick_i(tick_i),
    .next_tick_i(next_tick_i),

    .spikecore_done_o(spikecore_done_o),

    .FIFO_w_en_o(FIFO_w_en),
    .FIFO_w_data_o(FIFO_w_data),
    .FIFO_empty_i(FIFO_empty),
    .FIFO_full_i(FIFO_full)
);

spike_FIFO
#(
    .DEPTH(256),
    .M(8)
) spike_FIFO_i (
    .CLK,
    .RSTN,
    .FIFO_w_en_i(FIFO_w_en),
    .FIFO_r_en_i(FIFO_r_en_i),
    .FIFO_full_o(FIFO_full),
    .FIFO_empty_o(FIFO_empty_o),
    .FIFO_w_data_i(FIFO_w_data),
    .FIFO_r_data_o(FIFO_r_data_o)
);

// spike_out
// #(
//     .N(256),
//     .M(8)
// )spike_out_i(
//     .CLK,
//     .RSTN,
//     .start_i(start_i),
//     .LIF_busy_i,
//     .LIF_done_i,
//     .control_i,
//     .FIFO_empty_i(FIFO_empty),
//     .FIFO_r_en_o(FIFO_r_en),
//     .FIFO_r_data_i(FIFO_r_data),
//     .AER_ADDR_o,
//     .AER_REQ_o,
//     .AER_ACK_i

// );

sram_spike 
#(
    .N(256),
    .M(8),
    .INPUT_RESO(8)
)sram_spike_i(
    .CLK,
    .EN(SRAM_EN),
    .WE(SRAM_WE),
    .ADDR(SRAM_ADDR),
    .WDATA(SRAM_WDATA),
    .RDATA(SRAM_RDATA)
);

endmodule

module sram_spike #(
    parameter                   N = 256,
    parameter                   M = 8,
    parameter                   INPUT_RESO = 8
) (
    input  logic                     CLK,
    input  logic                     EN,
    input  logic                     WE,
    input  logic [$clog2(N)-3:0]     ADDR,
    input  logic [31:0]              WDATA,
    output logic [31:0]              RDATA
);
    logic [INPUT_RESO-1:0] spike [0:N-1];
    logic [31:0] temp;
    
    always_ff @( posedge ~CLK ) begin : SRAM_SPIKE
        if (EN && WE) begin
            spike[4*ADDR] <= WDATA[7:0];
            spike[4*ADDR+1] <= WDATA[15:8];
            spike[4*ADDR+2] <= WDATA[23:16];
            spike[4*ADDR+3] <= WDATA[31:24];
        end
        if(EN) begin
            temp <= EN ? {spike[4*ADDR+3],spike[4*ADDR+2],spike[4*ADDR+1],spike[4*ADDR]} : temp;
        end
    end

    assign RDATA = temp;
    
endmodule
