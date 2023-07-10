
module pre_load_mem #(
    parameter type obi_req_t = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,
    
    output obi_req_t pre_load_mem_master_req_o,
    input  obi_resp_t pre_load_mem_master_resp_i,

    output logic pre_load_finished_o
);

    (*rom_systle = "block"*)logic [7:0] boot_code [0:100000];
    logic [31:0] boot_count;
    logic [7:0] boot_code_temp [0:3];

    initial begin
        $readmemh("app.mem", boot_code);
    end

    logic           pre_load_mem_req;
    logic           pre_load_mem_we;
    logic [3:0]     pre_load_mem_be;
    logic [31:0]    pre_load_mem_addr;
    logic [31:0]    pre_load_mem_wdata;
    logic           pre_load_mem_gnt;
    logic           pre_load_mem_rvalid;
    logic [31:0]    pre_load_mem_rdata;

    logic           pre_load_finished;
    logic [31:0]    addr_count;
    logic [2:0]     load_count;

    assign pre_load_mem_master_req_o.req = pre_load_mem_req;
    assign pre_load_mem_master_req_o.we = pre_load_mem_we;
    assign pre_load_mem_master_req_o.be = pre_load_mem_be;
    assign pre_load_mem_master_req_o.addr = pre_load_mem_addr;
    assign pre_load_mem_master_req_o.wdata = pre_load_mem_wdata;

    assign pre_load_mem_gnt = pre_load_mem_master_resp_i.gnt;
    assign pre_load_mem_rvalid = pre_load_mem_master_resp_i.rvalid;
    assign pre_load_mem_rdata = pre_load_mem_master_resp_i.rdata;

    assign pre_load_finished_o = pre_load_finished;

    enum logic[1:0] {
        IDLE,
        LOAD,
        WRITE,
        DONE
    } state, next_state;

    always_ff @(posedge clk_i or negedge rst_ni) begin
        if(!rst_ni) begin 
            state <= IDLE;
        end else begin
            state <= next_state;
        end
    end

    always_comb begin
    
        unique case(state)
        IDLE: begin
            if (pre_load_finished)
                next_state = IDLE;
            else 
                next_state = LOAD;
        end
        LOAD: begin
            if (load_count==2'd3)
                next_state = WRITE;
            else if(addr_count == 32'h00018000)
                next_state = DONE;
            else
                next_state = LOAD;
            
        end
        WRITE: begin  
            if (pre_load_finished)
                next_state = IDLE;
            else if(addr_count == 32'h00018000)
                next_state = DONE;
            else
                next_state = LOAD;
        end
        DONE: begin
            next_state = DONE;
        end
        endcase
    end

    always_ff @(posedge clk_i or negedge rst_ni) begin
        if (!rst_ni) begin
            addr_count <= 0;
            load_count <= 0;
        end else if(state == LOAD) begin
            load_count <= load_count +2'b1;
        end
        else if(state == WRITE) begin
            addr_count <= addr_count + 32'd4;
        end
    end

    always_ff @(posedge clk_i) begin
        boot_code_temp[load_count] <= boot_code[addr_count+load_count];
    end

    always_comb begin
        unique case(state)
        IDLE: begin 
            pre_load_mem_req = 1'b0;
            pre_load_mem_we = 1'b0;
            pre_load_mem_be = 4'b0;
            pre_load_mem_addr = 32'b0;
            pre_load_mem_wdata = 32'b0;
            pre_load_finished = 1'b0;
        end
        LOAD:begin
            pre_load_mem_req = 1'b0;
            pre_load_mem_we = 1'b0;
            pre_load_mem_be = 4'b0;
            pre_load_mem_addr = 32'b0;
            pre_load_mem_wdata = 32'b0;
            pre_load_finished = 1'b0;
        end
        WRITE: begin  
            pre_load_mem_req = 1'b1;
            pre_load_mem_we = 1'b1;
            pre_load_mem_be = 4'b1111;
            pre_load_mem_addr = addr_count;
            pre_load_mem_wdata = {boot_code_temp[3], boot_code_temp[2], boot_code_temp[1], boot_code_temp[0]};
            pre_load_finished = 1'b0;
        end
        DONE: begin
            pre_load_mem_req = 1'b0;
            pre_load_mem_we = 1'b0;
            pre_load_mem_be = 4'b0;
            pre_load_mem_addr = 32'b0;
            pre_load_mem_wdata = 32'b0;
            pre_load_finished = 1'b1;
        end
        endcase
    end
    
endmodule