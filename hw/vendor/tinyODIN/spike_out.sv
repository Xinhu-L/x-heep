// TODO: output spike pushed back to spike core
module spike_out #(
    parameter                   N = 256,
    parameter                   M = 8
) (
    input logic                     CLK,
    input logic                     RSTN,

    // Start signal
    input logic                     start_i,

    // LIF busy model
    input logic                     LIF_busy_i,
    input logic                     LIF_done_i,
    // Global configuration signal
    input logic                     control_i,

    // FIFO signal
    input  logic                    FIFO_empty_i,
    output logic                    FIFO_r_en_o,
    input  logic  [M-1:0]           FIFO_r_data_i,

    output logic  [M+1:0]           AER_ADDR_o,
    output logic                    AER_REQ_o,
    input  logic                    AER_ACK_i
);

enum logic[1:0] { 
    IDLE = 2'b00,
    READ = 2'b01,
    AER  = 2'b10
} state, next_state;

always_ff @( posedge CLK or negedge RSTN ) begin : state_jump
    if(!RSTN) begin
        state <= IDLE;
    end
    else begin
        state <= next_state;
    end
end

always_comb begin : jump_condition
    case(state)
    IDLE: begin
        if(start_i==1'b1&&(LIF_busy_i==0)&&(FIFO_empty_i==0))begin
            next_state = READ;
        end
        else begin
            next_state = IDLE;
        end
    end
    READ: begin
        next_state = AER;
    end
    AER: begin
        if(LIF_done_i==1'b1)begin
            next_state = IDLE;
        end
        else begin
            next_state = AER;
        end
    end
    default begin
        next_state = state;        
    end
    endcase
end

always_comb begin 
    if(!RSTN) begin
        FIFO_r_en_o         = 'b0;
        AER_ADDR_o          = 'b0;
        AER_REQ_o           = 'b0;
    end
    else if (state==IDLE) begin
        FIFO_r_en_o         = 'b0;
        AER_REQ_o           = 'b0;
    end
    else if (state==READ) begin
        FIFO_r_en_o         = 'b1;
    end
    else if (state==AER) begin
        FIFO_r_en_o         = 'b0;
        // To-Do: Add the MSB 2bit as configuration bits
        AER_ADDR_o[M-1:0]   = FIFO_r_data_i;
        AER_REQ_o           = 'b1;
    end
    
end
    
endmodule