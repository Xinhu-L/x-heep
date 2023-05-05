`include "./obi_pkg.sv"
module tinyODIN_OBI_interface 
import obi_pkg::*;
#(
    parameter type              req_t = logic, // OBI request type
    parameter type              rsp_t = logic  // OBI response type
) (
    // Signals from the OBI bus
    input   req_t               tinyODIN_slave_req_i,
    output  rsp_t               tinyODIN_slave_resp_o,

    // To the spikecore
    output  req_t               spikecore_slave_req_o,
    input   rsp_t               spikecore_slave_resp_i,

    // To the neuroncore
    output  req_t               neuroncore_slave_req_o,
    input   rsp_t               neuroncore_slave_resp_i, 

    // To the synapsecore
    output  req_t               synapsecore_slave_req_o,
    input   rsp_t               synapsecore_slave_resp_i, 

    // To the control
    output  req_t               control_slave_req_o,
    input   rsp_t               control_slave_resp_i
);

// Choose the 22,21 bit of addr as the mux selected
always_comb begin 
    case (tinyODIN_slave_req_i.addr[21:20])
    2'b00: begin
        spikecore_slave_req_o   =   tinyODIN_slave_req_i;
        tinyODIN_slave_resp_o   =   spikecore_slave_resp_i;
        end
    2'b01: begin
        neuroncore_slave_req_o  =   tinyODIN_slave_req_i;
        tinyODIN_slave_resp_o   =   neuroncore_slave_resp_i;
    end
    2'b10: begin
        synapsecore_slave_req_o  =   tinyODIN_slave_req_i;
        tinyODIN_slave_resp_o    =   synapsecore_slave_resp_i;
    end
    2'b11: begin
        control_slave_req_o      =   tinyODIN_slave_req_i;
        tinyODIN_slave_resp_o    =   control_slave_resp_i;
    end
    endcase
    
end
    
endmodule
