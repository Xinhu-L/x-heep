module spike_FIFO #(
    parameter                   DEPTH = 256,
    parameter                   M = 8
    
) (
    input  logic    CLK,
    input  logic    RSTN,

    input  logic    FIFO_w_en_i,
    input  logic    FIFO_r_en_i,

    output logic    FIFO_full_o,
    output logic    FIFO_empty_o,

    input  logic [M-1:0] FIFO_w_data_i,
    output logic [M-1:0] FIFO_r_data_o

);

// Pointers of W&R
logic [$clog2(DEPTH)-1:0] w_ptr;
logic [$clog2(DEPTH)-1:0] r_ptr;

// ADDRESS of W&R, first bit used to mark full
logic [$clog2(DEPTH):0]   w_addr;
logic [$clog2(DEPTH):0]   r_addr;

// Full and Empty signal
logic full;
logic empty;

// FIFO regs
logic [M-1:0] FIFO [0:DEPTH-1];

assign w_ptr = w_addr[$clog2(DEPTH)-1:0];
assign r_ptr = r_addr[$clog2(DEPTH)-1:0];

assign full = r_addr==({~w_addr[$clog2(DEPTH)], w_ptr})? 1'b1:1'b0;
assign empty = (r_addr==w_addr)? 1'b1:1'b0;

assign FIFO_full_o  = full;
assign FIFO_empty_o = empty; 

always_ff @( posedge CLK or negedge RSTN ) begin
    if (!RSTN) begin
        w_addr <= 0;
    end
    else if (FIFO_w_en_i&&~full ) begin
        w_addr <= w_addr + 1;
    end
end
always_ff @( posedge CLK or negedge RSTN ) begin 
    if (!RSTN) begin
        r_addr <= 0;
    end
    else if (FIFO_r_en_i&&~empty) begin
        r_addr <= r_addr + 1;
    end
end

always_ff @( posedge CLK or negedge RSTN ) begin 
    if (FIFO_w_en_i&&~full) begin
        FIFO[w_ptr] <= FIFO_w_data_i;
    end
end

always_ff @( posedge CLK or negedge RSTN ) begin 
    if (!RSTN) begin
        FIFO_r_data_o <= 'b0;
    end
    else if (FIFO_r_en_i&&~empty) begin
        FIFO_r_data_o <= FIFO[r_ptr];
    end
end

    
endmodule