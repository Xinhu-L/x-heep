#include <stdio.h>
#include <stdlib.h>

#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "core_v_mini_mcu.h"
#include "tinyODIN.h"
#include "tinyODIN_SRAM.h"  

int main(int argc, char *argv[])
{
    plic_result_t plic_res;
    plic_res = plic_Init();
    if (plic_res != kPlicOk) {
        printf("Init PLIC failed\n;");
        return -1;
    }
    plic_res = plic_irq_set_priority(INTR_ODIN_FINISHED, 1);
    if (plic_res != kPlicOk) {
        printf("Failed\n;");
        return -1;
    }

    plic_res = plic_irq_set_enabled(INTR_ODIN_FINISHED, kPlicToggleEnabled);
    if (plic_res != kPlicOk) {
        printf("Failed\n;");
        return -1;
    }

    uint32_t plic_intr_flag;
    plic_intr_flag = 0;

    printf("Initializing base address of tinyODIN...\n");
    tinyODIN_t tinyODIN;
    tinyODIN.base_addr = mmio_region_from_addr((uintptr_t)TINYODIN_START_ADDRESS);

    int write_addr;
    uint32_t write_data;
    // Initialize the spike core
    write_addr = 0x00000001;
    write_data = 0x00000001;
    tinyODIN_spike_core_write(&tinyODIN,write_addr,write_data);
    write_addr = 0x00000006;
    write_data = 0xBAB49E00;
    tinyODIN_spike_core_write(&tinyODIN,write_addr,write_data);
    write_addr = 0x00000022;
    write_data = 0x00ACE9CF;
    tinyODIN_spike_core_write(&tinyODIN,write_addr,write_data);

    // Initialize the neuron core
    for(ptrdiff_t i=0x00000000; i<0x00000100; i++){
        write_addr = i;
        write_data = 0x0015e000;
        tinyODIN_neuron_core_write(&tinyODIN,write_addr,write_data);
    }

    // Initialize the synaptic core
    write_addr = 0x00000012;
    write_data = 0x1ceeef40;
    tinyODIN_synaptic_core_write(&tinyODIN,write_addr,write_data);
    write_addr = 0x0000121E;
    write_data = 0x1d000000;
    tinyODIN_synaptic_core_write(&tinyODIN,write_addr,write_data);
    // Begin Inference
    write_addr = 0x00000000;
    write_data = 0xff640400;
    tinyODIN_control_write(&tinyODIN,write_addr,write_data);


    while(plic_intr_flag==0) {
        wait_for_interrupt();
        volatile int i=0;
        i++;
    }
    return EXIT_SUCCESS;
}

