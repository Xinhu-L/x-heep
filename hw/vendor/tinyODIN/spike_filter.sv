`include "./obi_pkg.sv"
module spike_filter 
import obi_pkg::*;
#(
    parameter                   N = 256,
    parameter type              req_t = logic, // OBI request type
    parameter type              resp_t = logic  // OBI response type
) (
    input  logic                            CLK,
    input  logic                            RSTN,

    // Start filter
    input  logic                            start_i,

    // Intereact with Spike Core
    input  logic    [31:0]                  filter_spike_i, 
    output logic    [$clog2(N)-3:0]         filter_addr_o,
    output logic                            filter_o,                                                    

    // From Tick generator
    input  logic    [7:0]                   tick_i,
    input  logic                            next_tick_i,

    // Finished
    output logic                            spikecore_done_o,

    // Inform FIFO
    output logic                            FIFO_w_en_o,
    output logic    [$clog2(N)-1:0]         FIFO_w_data_o,
    input  logic                            FIFO_empty_i,
    //To-Do: When full, stop writting
    input  logic                            FIFO_full_i
);

logic [$clog2(N)-3:0]  filter_addr;



logic [7:0] input_spike [0:3];
logic [3:0] match;
logic match_found;
assign input_spike[3] = filter_spike_i[31:24];
assign input_spike[2] = filter_spike_i[23:16];
assign input_spike[1] = filter_spike_i[15:8];
assign input_spike[0] = filter_spike_i[7:0];
assign match[0] = (tick_i==input_spike[0]); 
assign match[1] = (tick_i==input_spike[1]); 
assign match[2] = (tick_i==input_spike[2]); 
assign match[3] = (tick_i==input_spike[3]); 
assign match_found = |match;
logic [$clog2(N)-1:0]          FIFO_w_data;


logic [($clog2(N/(32/8)))-1:0] fetch_counter;
logic [$clog2(32/8)-1:0]       check_counter;

enum logic[2:0] { 
    IDLE    = 3'd0,
    FETCH   = 3'd1,
    CHECK   = 3'd2,
    PUSH    = 3'd3,
    DONE    = 3'd4
} state, next_state;

always_ff @( posedge CLK or negedge RSTN ) begin : state_jump
    if(!RSTN) begin
        state <= IDLE;
    end
    else begin
        state <= next_state;
    end
end

always_comb begin : state_jump_condition
    case(state)
    IDLE:begin
        if(start_i==1'b1) begin
            next_state = FETCH;
        end
        else begin
            next_state = IDLE;
        end
    end
    FETCH:begin
        if (match_found==1'b1) begin
            next_state = CHECK;
        end
        else if (fetch_counter==6'h3f) begin
            next_state = DONE;
        end
        else begin
            next_state = FETCH;
        end    

    end
    CHECK:begin

        if (input_spike[check_counter]==tick_i) begin
            next_state = PUSH;
        end
        else if(check_counter==2'b11) begin
            next_state = FETCH;
        end
        else begin
            next_state = CHECK;
        end
    end
    PUSH:begin
        if (fetch_counter==6'h0) begin
            next_state = DONE;
        end
        else if(check_counter==2'b00) begin
            next_state = FETCH;
        end
        else begin
            next_state = CHECK;
        end
    end
    DONE:begin
        if(next_tick_i==2'b01) begin
            next_state = IDLE;
        end
        else begin
            next_state = DONE;
        end
    end
    default begin
        next_state = state;        
    end
    
    endcase
end


always_comb begin 
    if(!RSTN) begin
        filter_o        = 'b0;
        FIFO_w_en_o     = 'b0;  
        spikecore_done_o= 'b0; 
    end
    else if (state==FETCH) begin
        filter_o        = 'b1;
        FIFO_w_en_o     = 'b0;
        spikecore_done_o= 'b0;
    end
    else if (state==CHECK) begin
        filter_o        = 'b0;
        FIFO_w_en_o     = 'b0;
        spikecore_done_o= 'b0;
    end
    else if (state==PUSH) begin
        filter_o        = 'b0;
        FIFO_w_en_o     = 'b1;
        spikecore_done_o= 'b0;
    end
    else if (state==DONE) begin
        filter_o        = 'b0;
        FIFO_w_en_o     = 'b0;
        spikecore_done_o= 'b1;
    end
    else begin
        filter_o        = 'b0;
        FIFO_w_en_o     = 'b0;
        spikecore_done_o= 'b0;
    end
end

always_ff @( posedge CLK or negedge RSTN ) begin 
    if(!RSTN)begin
        fetch_counter <= 'b0;
        check_counter <= 'b0;
        filter_addr   <= 'b0;
        FIFO_w_data_o <= 'b0;
    end
    else if (state==FETCH) begin
        fetch_counter <= fetch_counter + 1'b1;
        check_counter <= 0;
        filter_addr   <= filter_addr + 1'b1;
        FIFO_w_data_o <= 'b0;
    end
    else if (state==CHECK) begin
        fetch_counter <= fetch_counter;
        check_counter <= check_counter + 1'b1;
        filter_addr   <= filter_addr;
        FIFO_w_data_o <= FIFO_w_data;

    end
    else if (state==PUSH) begin
        fetch_counter <= fetch_counter;
        check_counter <= check_counter;
        filter_addr   <= filter_addr;
        FIFO_w_data_o <= FIFO_w_data_o;
    end
    else if (state==DONE) begin
        fetch_counter <= fetch_counter;
        check_counter <= check_counter;
        filter_addr   <= filter_addr;
        FIFO_w_data_o <= FIFO_w_data_o;
    end
    else begin
        fetch_counter <= fetch_counter;
        check_counter <= check_counter;
        filter_addr   <= filter_addr;
        FIFO_w_data_o <= FIFO_w_data_o;
    end
    
end
//To-Do: Optimize the following
assign FIFO_w_data = (filter_addr_o-1)*4 + check_counter;
assign filter_addr_o = filter_addr;
endmodule

