module charger #(
    parameter   N = 256,
    parameter   M = 8,
    parameter   INPUT_RESO = 8
) (
    input   logic                   CLK,
    input   logic                   RSTN,

    input   logic   [5:0]           count_i,
    input   logic   [31:0]          synapse_data_i,
    input   logic   [4:0]           charge_count_i,
    input   logic                   chagre_enable_i,
    input   logic                   event_tref_i,

    output  logic   [31:0]          synapse_charge_o        
);

logic signed [7:0] charge [0:255];
logic signed [7:0] weight [0:7];

// assign weight[0] = synapse_data_i[3] ? {synapse_data_i[3],{4'b1111,~synapse_data_i[2:0]}+1'b1} : {synapse_data_i[3],4'b0000,synapse_data_i[2:0]};
// assign weight[1] = synapse_data_i[7] ? {synapse_data_i[7],{4'b1111,~synapse_data_i[6:4]}+1'b1} : {synapse_data_i[7],4'b0000,synapse_data_i[6:4]};
// assign weight[2] = synapse_data_i[11] ? {synapse_data_i[11],{4'b1111,~synapse_data_i[10:8]}+1'b1} : {synapse_data_i[11],4'b0000,synapse_data_i[10:8]};
// assign weight[3] = synapse_data_i[15] ? {synapse_data_i[15],{4'b1111,~synapse_data_i[14:12]}+1'b1} : {synapse_data_i[15],4'b0000,synapse_data_i[14:12]};
// assign weight[4] = synapse_data_i[19] ? {synapse_data_i[19],{4'b1111,~synapse_data_i[18:16]}+1'b1} : {synapse_data_i[19],4'b0000,synapse_data_i[18:16]};
// assign weight[5] = synapse_data_i[23] ? {synapse_data_i[23],{4'b1111,~synapse_data_i[22:20]}+1'b1} : {synapse_data_i[23],4'b0000,synapse_data_i[22:20]};
// assign weight[6] = synapse_data_i[27] ? {synapse_data_i[27],{4'b1111,~synapse_data_i[26:24]}+1'b1} : {synapse_data_i[27],4'b0000,synapse_data_i[26:24]};
// assign weight[7] = synapse_data_i[31] ? {synapse_data_i[31],{4'b1111,~synapse_data_i[30:28]}+1'b1} : {synapse_data_i[31],4'b0000,synapse_data_i[30:28]};

assign weight[0] = synapse_data_i[3] ? {4'b1111, synapse_data_i[3:0]} : synapse_data_i[3:0];
assign weight[1] = synapse_data_i[7] ? {4'b1111, synapse_data_i[7:4]} : synapse_data_i[7:4];
assign weight[2] = synapse_data_i[11] ? {4'b1111, synapse_data_i[11:8]} : synapse_data_i[11:8];
assign weight[3] = synapse_data_i[15] ? {4'b1111, synapse_data_i[15:12]} : synapse_data_i[15:12];
assign weight[4] = synapse_data_i[19] ? {4'b1111, synapse_data_i[19:16]} : synapse_data_i[19:16];
assign weight[5] = synapse_data_i[23] ? {4'b1111, synapse_data_i[23:20]} : synapse_data_i[23:20];
assign weight[6] = synapse_data_i[27] ? {4'b1111, synapse_data_i[27:24]} : synapse_data_i[27:24];
assign weight[7] = synapse_data_i[31] ? {4'b1111, synapse_data_i[31:28]} : synapse_data_i[31:28];


always_ff @( posedge CLK or negedge RSTN ) begin 
    if (!RSTN) begin
        for (int i=0; i<256; ++i) begin
            charge[i] <= 'b0;
        end
    end 
    else if(chagre_enable_i) begin
        charge[charge_count_i*8+0] <= charge[charge_count_i*8+0] + weight[0];
        charge[charge_count_i*8+1] <= charge[charge_count_i*8+1] + weight[1];
        charge[charge_count_i*8+2] <= charge[charge_count_i*8+2] + weight[2];
        charge[charge_count_i*8+3] <= charge[charge_count_i*8+3] + weight[3];
        charge[charge_count_i*8+4] <= charge[charge_count_i*8+4] + weight[4];
        charge[charge_count_i*8+5] <= charge[charge_count_i*8+5] + weight[5];
        charge[charge_count_i*8+6] <= charge[charge_count_i*8+6] + weight[6];
        charge[charge_count_i*8+7] <= charge[charge_count_i*8+7] + weight[7];
    end
    // else if (event_tref_i) begin
    //     charge[charge_count_i*8+0] <= {{{7'h00, charge[charge_count_i*8+0]} << 3'd7} - charge[charge_count_i*8+0]}[14:7];
    //     charge[charge_count_i*8+1] <= {{{7'h00, charge[charge_count_i*8+1]} << 3'd7} - charge[charge_count_i*8+1]}[14:7];
    //     charge[charge_count_i*8+2] <= {{{7'h00, charge[charge_count_i*8+2]} << 3'd7} - charge[charge_count_i*8+2]}[14:7];
    //     charge[charge_count_i*8+3] <= {{{7'h00, charge[charge_count_i*8+3]} << 3'd7} - charge[charge_count_i*8+3]}[14:7];
    //     charge[charge_count_i*8+4] <= {{{7'h00, charge[charge_count_i*8+4]} << 3'd7} - charge[charge_count_i*8+4]}[14:7];
    //     charge[charge_count_i*8+5] <= {{{7'h00, charge[charge_count_i*8+5]} << 3'd7} - charge[charge_count_i*8+5]}[14:7];
    //     charge[charge_count_i*8+6] <= {{{7'h00, charge[charge_count_i*8+6]} << 3'd7} - charge[charge_count_i*8+6]}[14:7];
    //     charge[charge_count_i*8+7] <= {{{7'h00, charge[charge_count_i*8+7]} << 3'd7} - charge[charge_count_i*8+7]}[14:7];
    // // end
    // else if (event_tref_i) begin
    //     charge[charge_ref_count_i*8+0] <= charge[charge_ref_count_i*8+0] - (charge[charge_ref_count_i*8+0]>>7);
    //     charge[charge_ref_count_i*8+1] <= charge[charge_ref_count_i*8+1] - (charge[charge_ref_count_i*8+1]>>7);
    //     charge[charge_ref_count_i*8+2] <= charge[charge_ref_count_i*8+2] - (charge[charge_ref_count_i*8+2]>>7);
    //     charge[charge_ref_count_i*8+3] <= charge[charge_ref_count_i*8+3] - (charge[charge_ref_count_i*8+3]>>7);
    //     charge[charge_ref_count_i*8+4] <= charge[charge_ref_count_i*8+4] - (charge[charge_ref_count_i*8+4]>>7);
    //     charge[charge_ref_count_i*8+5] <= charge[charge_ref_count_i*8+5] - (charge[charge_ref_count_i*8+5]>>7);
    //     charge[charge_ref_count_i*8+6] <= charge[charge_ref_count_i*8+6] - (charge[charge_ref_count_i*8+6]>>7);
    //     charge[charge_ref_count_i*8+7] <= charge[charge_ref_count_i*8+7] - (charge[charge_ref_count_i*8+7]>>7);
    // end
    
end

assign synapse_charge_o = {charge[count_i*4+3], charge[count_i*4+2], charge[count_i*4+1], charge[count_i*4]};

    
endmodule