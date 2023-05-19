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

assign tinyODIN_slave_resp_o.rvalid = spikecore_slave_resp_i.rvalid || neuroncore_slave_resp_i.rvalid || synapsecore_slave_resp_i.rvalid || control_slave_resp_i.rvalid;
// Choose the 22,21 bit of addr as the mux selected
always_comb begin 
    case (tinyODIN_slave_req_i.addr[21:20])
    2'b00: begin
        spikecore_slave_req_o.req       =   tinyODIN_slave_req_i.req;
        spikecore_slave_req_o.we        =   tinyODIN_slave_req_i.we;
        spikecore_slave_req_o.be        =   tinyODIN_slave_req_i.be;
        spikecore_slave_req_o.addr      =   tinyODIN_slave_req_i.addr>>2;
        spikecore_slave_req_o.wdata    =   tinyODIN_slave_req_i.wdata;
        neuroncore_slave_req_o  =   'b0;
        synapsecore_slave_req_o =   'b0;
        control_slave_req_o     =   'b0;
        tinyODIN_slave_resp_o.gnt       =   spikecore_slave_resp_i.gnt;
        tinyODIN_slave_resp_o.rdata     =   spikecore_slave_resp_i.rdata;       
        end
    2'b01: begin
        spikecore_slave_req_o   =   'b0;
        neuroncore_slave_req_o.req      =   tinyODIN_slave_req_i.req;
        neuroncore_slave_req_o.we       =   tinyODIN_slave_req_i.we;
        neuroncore_slave_req_o.be       =   tinyODIN_slave_req_i.be;
        neuroncore_slave_req_o.addr     =   tinyODIN_slave_req_i.addr>>2;
        neuroncore_slave_req_o.wdata    =   tinyODIN_slave_req_i.wdata;
        synapsecore_slave_req_o =   'b0;
        control_slave_req_o     =   'b0;
        tinyODIN_slave_resp_o.gnt       =   neuroncore_slave_resp_i.gnt;
        tinyODIN_slave_resp_o.rdata     =   neuroncore_slave_resp_i.rdata;
    end
    2'b10: begin
        spikecore_slave_req_o    =   'b0;
        neuroncore_slave_req_o   =   'b0;
        synapsecore_slave_req_o.req     =   tinyODIN_slave_req_i.req;
        synapsecore_slave_req_o.we      =   tinyODIN_slave_req_i.we;
        synapsecore_slave_req_o.be      =   tinyODIN_slave_req_i.be;
        synapsecore_slave_req_o.addr    =   tinyODIN_slave_req_i.addr>>2;
        synapsecore_slave_req_o.wdata   =   tinyODIN_slave_req_i.wdata;
        control_slave_req_o      =   'b0;
        tinyODIN_slave_resp_o.gnt       =   synapsecore_slave_resp_i.gnt;
        tinyODIN_slave_resp_o.rdata     =   synapsecore_slave_resp_i.rdata;
    end
    2'b11: begin
        spikecore_slave_req_o    =   'b0;
        neuroncore_slave_req_o   =   'b0;
        synapsecore_slave_req_o  =   'b0;
        control_slave_req_o      =   tinyODIN_slave_req_i;
        tinyODIN_slave_resp_o.gnt       =   control_slave_resp_i.gnt;
        tinyODIN_slave_resp_o.rdata     =   synapsecore_slave_resp_i.rdata;
    end
    endcase
    
end
    
endmodule
