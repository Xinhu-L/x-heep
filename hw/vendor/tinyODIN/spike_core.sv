`include "./obi_pkg.sv"
module spike_core 
import obi_pkg::*;
#(
    parameter                               N = 256,
    parameter type                          req_t = logic, // OBI request type
    parameter type                          rsp_t = logic  // OBI response type
) (
    input   logic                           CLK,
    input   logic                           RSTN,
    
    //Start Singal from controller
    input   logic                           spikecore_working_i,

    // TTFS time tick 
    input   logic   [7:0]                   tick_i,
    input   logic                           next_tick_i,
    
    // OBI BUS Slave interface
    input   req_t                           spikecore_slave_req_i,
    output  rsp_t                           spikecore_slave_resp_o,

    // Spikecore finished
    output  logic                           spikecore_done_o,

    // FIFO output
    input   logic                           spikecore_r_en_i,
    output  logic   [$clog2(N)-1:0]         spikecore_r_data_o,
    output  logic                           spikecore_empty_o,

    // SPike from AER OUT
    input   logic                           spike_pushback_i,
    input   logic   [$clog2(N)-1:0]         spike_pushback_addr_i
    
    
);

// SRAM slect signal
logic [2:0] SRAM_sel;
logic SRAM_EN;
logic [3:0]  SRAM_WE;
logic [$clog2(N)-3:0] SRAM_ADDR;
logic [31:0] SRAM_WDATA;
logic [31:0] SRAM_RDATA;

// Filter signals
logic [$clog2(N)-3:0] filter_addr;
logic [31:0] filter_spike;
logic filter_read_en;

// FIFO signals
logic FIFO_w_en;
logic FIFO_full;
logic [$clog2(N)-1:0] FIFO_w_data;


// Pushback mask
logic [3:0] spike_pushback_we;
always_comb begin
    case(spike_pushback_addr_i[1:0]) 
        2'b00: spike_pushback_we = 4'b0001;
        2'b01: spike_pushback_we = 4'b0010;
        2'b10: spike_pushback_we = 4'b0100;
        2'b11: spike_pushback_we = 4'b1000;
    endcase
end


assign SRAM_sel                     = {spikecore_slave_req_i.req, filter_read_en, spike_pushback_i};
assign spikecore_slave_resp_o.rdata = SRAM_RDATA;
assign filter_spike                 = filter_read_en ? SRAM_RDATA : filter_spike;

always_comb begin : MUX_of_spikeSRAM
    case(SRAM_sel)
    3'b100: begin
        SRAM_EN                         = spikecore_slave_req_i.req;
        SRAM_WE                         = {4{spikecore_slave_req_i.we}};
        SRAM_ADDR                       = spikecore_slave_req_i.addr[$clog2(N)-3:0];
        SRAM_WDATA                      = spikecore_slave_req_i.wdata;
    end
    3'b010: begin
        SRAM_EN                         = filter_read_en;
        SRAM_WE                         = 4'b0;
        SRAM_ADDR                       = filter_addr;
        SRAM_WDATA                      = 32'b0;
    end  
    3'b001: begin
        SRAM_EN                         = spike_pushback_i;
        SRAM_WE                         = spike_pushback_we;
        SRAM_ADDR                       = spike_pushback_addr_i>>2;
        SRAM_WDATA                      = {4{tick_i+1'b1}};
    end
    default: begin
        SRAM_EN                         = 'b0;
        SRAM_WE                         = 'b0;
        SRAM_ADDR                       = 'b0;
        SRAM_WDATA                      = 'b0;
    end
    endcase
end
// SRAM signal from OBI bus
assign spikecore_slave_resp_o.gnt = spikecore_slave_req_i.req;
always_ff @(posedge CLK or negedge RSTN) begin
    if (!RSTN) begin
        spikecore_slave_resp_o.rvalid <= 1'b0;
    end else begin
        spikecore_slave_resp_o.rvalid <= spikecore_slave_resp_o.gnt;
    end

end

spike_filter 
#(
    .N(N),
    .req_t(obi_pkg::obi_req_t),
    .resp_t(obi_pkg::obi_req_t)
)spike_filter_i(
    .CLK,
    .RSTN,
    .spikecore_working_i,

    .filter_spike_i(filter_spike),
    .filter_addr_o(filter_addr),
    .filter_read_en_o(filter_read_en),

    .tick_i(tick_i),
    .next_tick_i(next_tick_i),

    .spikecore_done_o(spikecore_done_o),

    .FIFO_w_en_o(FIFO_w_en),
    .FIFO_w_data_o(FIFO_w_data),
    .FIFO_empty_i(spikecore_empty_o),
    .FIFO_full_i(FIFO_full)
);

spike_FIFO
#(
    .DEPTH(128),
    .N(N)
) spike_FIFO_i (
    .CLK,
    .RSTN,
    .FIFO_w_en_i(FIFO_w_en),
    .FIFO_r_en_i(spikecore_r_en_i),
    .FIFO_full_o(FIFO_full),
    .FIFO_empty_o(spikecore_empty_o),
    .FIFO_w_data_i(FIFO_w_data),
    .FIFO_r_data_o(spikecore_r_data_o)
);

sram_spike 
#(
    .N(N)
)sram_spike_i(
    .CLK(CLK),
    .EN(SRAM_EN),
    .WE(SRAM_WE),
    .ADDR(SRAM_ADDR),
    .WDATA(SRAM_WDATA),
    .RDATA(SRAM_RDATA)
);

endmodule

module sram_spike #(
    parameter                   N = 256
) (
    input  logic                     CLK,
    input  logic                     EN,
    input  logic [3:0]               WE,
    input  logic [$clog2(N)-3:0]     ADDR,
    input  logic [31:0]              WDATA,
    output logic [31:0]              RDATA
);
    logic [31:0] spike [0:N/4-1];
    always_ff @( posedge CLK ) begin : SRAM_SPIKE
        if (EN) begin
            spike[ADDR][7:0]   <= WE[0] ? WDATA[7:0]   : spike[ADDR][7:0];
            spike[ADDR][15:8]  <= WE[1] ? WDATA[15:8]  : spike[ADDR][15:8];
            spike[ADDR][23:16] <= WE[2] ? WDATA[23:16] : spike[ADDR][23:16];
            spike[ADDR][31:24] <= WE[3] ? WDATA[31:24] : spike[ADDR][31:24];
        end
    end
    assign RDATA = EN ? spike[ADDR] : '0;
    
endmodule
