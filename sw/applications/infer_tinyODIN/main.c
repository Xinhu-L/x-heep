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

#define T_WRONG 1.0
#define T_CORRECT 2.0
#define LEARNING_RATE 3

int main(int argc, char *argv[])
{
    tinyODIN_result_t plic_res;
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

    printf("Initializing base address of tinyODIN...");
    tinyODIN_t tinyODIN;
    tinyODIN.base_addr = mmio_region_from_addr((uintptr_t)TINYODIN_START_ADDRESS);
    printf("Finished :)\n");
    

    printf("Set the input spike and label...");
    uint32_t spike_time_input[144] = {  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
                                        0x45, 0x4B, 0x61, 0xFF, 0x5B, 0x5B, 0x5A, 0x52, 
                                        0xFF, 0x64, 0x5F, 0x5C, 0x2E, 0x39, 0x5F, 0xFF, 
                                        0x26, 0x26, 0x27, 0x27, 0xFF, 0x60, 0x36, 0x24, 
                                        0x5F, 0x60, 0x64, 0xFF, 0x4E, 0x51, 0x54, 0x59, 
                                        0xFF, 0x61, 0x34, 0x2A, 0xFF, 0xFF, 0xFF, 0xFF, 
                                        0x4D, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x21, 
                                        0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0x5F, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0x61, 0x37, 0xFF, 0xFF, 0xFF, 0xFF, 
                                        0x27, 0x4E, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 
                                        0xFF, 0xFF, 0xFF, 0xFF, 0x3B, 0x29, 0x59, 0xFF, 
                                        0xFF, 0xFF, 0xFF, 0x62, 0xFF, 0xFF, 0xFF, 0xFF, 
                                        0x59, 0x28, 0x39, 0x62, 0xFF, 0xFF, 0xFF, 0xFF, 
                                        0x64, 0xFF, 0xFF, 0xFF, 0x64, 0x4A, 0x1D, 0x4D, 
                                        0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0xFF, 0xFF, 0xFF, 
                                        0xFF, 0x53, 0x16, 0x30, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t spike_label = 7;
    printf("Finished :)\n");

    printf("Initialize input...");
    tinyODIN_spike_core_write_call(tinyODIN, spike_time_input);
    printf("Finished :)\n");

    printf("Initialize neuron state...");
    int neuron_addr;
    uint32_t neuron_data;
    for(ptrdiff_t i=0x00000000; i<0x00000101; i++){
        neuron_addr = i;
        neuron_data = 0x0015e000;
        tinyODIN_neuron_core_write(&tinyODIN,neuron_addr,neuron_data);
    }
    printf("Finished :)\n");

    printf("Initialize weights...");
    tinyODIN_synaptic_init(tinyODIN);
    tinyODIN_synaptic_l1_init(tinyODIN);
    tinyODIN_synaptic_l2_init(tinyODIN);
    printf("Finished :)\n");

    printf("Begin inference...\n");
    tinyODIN_control_write(&tinyODIN, 0, 0xff000400);

    printf("Inferencing...");
    uint32_t inference_done;
    while(inference_done!=0xff000401) {
        inference_done = tinyODIN_control_read(&tinyODIN, 0);
    }
    printf("Finished :)\n");

    printf("Fetch the output spike...");
    uint8_t spike_time_output[10] = {34, 34, 33, 34, 34, 34, 34, 33, 34, 34};
    uint32_t spike_time_output_temp;

    // spike_time_output_temp = tinyODIN_spike_core_read(&tinyODIN, 61);
    // spike_time_output[0] = spike_time_output_temp>>16 & 0xFF;
    // spike_time_output[1] = spike_time_output_temp>>24 & 0xFF;
    // spike_time_output_temp = tinyODIN_spike_core_read(&tinyODIN, 62);
    // spike_time_output[2] = spike_time_output_temp     & 0xFF;
    // spike_time_output[3] = spike_time_output_temp>>8  & 0xFF;
    // spike_time_output[4] = spike_time_output_temp>>16 & 0xFF;
    // spike_time_output[5] = spike_time_output_temp>>24 & 0xFF;
    // spike_time_output_temp = tinyODIN_spike_core_read(&tinyODIN, 63);
    // spike_time_output[6] = spike_time_output_temp     & 0xFF;
    // spike_time_output[7] = spike_time_output_temp>>8  & 0xFF;
    // spike_time_output[8] = spike_time_output_temp>>16 & 0xFF;
    // spike_time_output[9] = spike_time_output_temp>>24 & 0xFF;
    printf("The spike_time_output are: ");
    for(int i=0; i<10; ++i){
        printf("%d ", spike_time_output[i]);
    }
    printf("Finished :)\n");

    printf("Fetch the hidden spike...");
    uint8_t spike_time_hidden[100] = {
        36, 28, 29, 37, 26, 26, 28, 35, 28,
         51, 25, 41, 38, 29, 41, 28, 33, 39,
         38, 27, 29, 40, 36, 32, 36, 33, 49,
         33, 29, 35, 29, 26, 29, 28, 37, 25,
         32, 29, 31, 34, 30, 47, 33, 27, 29,
         24, 34, 24, 34, 39, 33, 28, 35, 28,
         26, 44, 31, 39, 37, 34, 31, 35, 23,
         30, 31, 27, 24, 26, 36, 42, 38, 31,
         29, 55, 32, 31, 24, 30, 29, 30, 29,
         31, 31, 31, 38, 37, 30, 36, 26, 29,
         35, 31, 40, 40, 29, 34, 33, 31, 28,
         33
    };
    uint32_t spike_time_hidden_temp;
    // for(int i=0; i<25; ++i){
    //     spike_time_hidden_temp = tinyODIN_spike_core_read(&tinyODIN, i+36);
    //     spike_time_hidden[i*4]   = spike_time_hidden_temp     & 0xFF;
    //     spike_time_hidden[i*4+1] = spike_time_hidden_temp>>8  & 0xFF;
    //     spike_time_hidden[i*4+2] = spike_time_hidden_temp>>16 & 0xFF;
    //     spike_time_hidden[i*4+3] = spike_time_hidden_temp>>24 & 0xFF;
    // }
    printf("The spike_time_hidden are: ");
    for(int i=0; i<100; ++i){
        printf("%d ", spike_time_hidden[i]);
    }
    printf("Finished :)\n");

    
    printf("Calculating spike diff of layer2...");
    int spike_diff_l2[100][10];
    for(int i=0; i<100; ++i){
        for(int j=0; j<10; ++j){
            spike_diff_l2[i][j] = spike_time_output[j] - spike_time_hidden[i];
        }
    }

    printf("The spike diff of l2 are:");
    for(int i=0; i<10; ++i){
        printf("%d ", spike_diff_l2[1][i]);
    }
    printf("Finished :)\n");

    printf("Fetch weights of layer 2 from tinyODIN...");
    int weight_l2[100][10];
    int fetch_weight_l2_addr;
    int weight_l2_temp;
    for(int i=0; i<100; ++i){
        fetch_weight_l2_addr = i*32 + 4638;
        weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
        weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
        weight_l2[i][0] = weight_l2_temp>>24  & 0xF;
        weight_l2[i][1] = weight_l2_temp>>28  & 0xF;
        fetch_weight_l2_addr = i*32 + 4639;
        weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
        weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
        weight_l2[i][2] = weight_l2_temp      & 0xF;
        weight_l2[i][3] = weight_l2_temp>>4   & 0xF;
        weight_l2[i][4] = weight_l2_temp>>8   & 0xF;
        weight_l2[i][5] = weight_l2_temp>>12  & 0xF;
        weight_l2[i][6] = weight_l2_temp>>16  & 0xF;
        weight_l2[i][7] = weight_l2_temp>>20  & 0xF;
        weight_l2[i][8] = weight_l2_temp>>24  & 0xF;
        weight_l2[i][9] = weight_l2_temp>>28  & 0xF;
        for(int j=0; j<10; ++j){
            if(weight_l2[i][j]>7){
                weight_l2[i][j] = 7 - weight_l2[i][j];
            }
        }
    }
    printf("Finished :)\n");

    printf("The expanded weight are");
    int weight_l2_float[100][10];
    for(int i=0; i<100; ++i){
        for(int j=0; j<10; ++j){
            weight_l2_float[i][j] = weight_l2[i][j] * 100000000;
        }
    }  
    for(int i=0; i<10; ++i){
        printf("%d ", weight_l2_float[1][i]);
    }
    printf("Finished :)\n");      


    printf("Calculating dLoss/dt of output layer...");
    int dt_output[10];
    for(int i=0; i<10; ++i){
        if(i==spike_label){
            dt_output[i] = 0.1 * 10 * (spike_time_output[i] - T_CORRECT*100);
        }else{
            dt_output[i] = 0.1 * 10 * (spike_time_output[i] - T_WRONG*100);
        }
    }
    printf("The dt_l2 of l2 are:");
    for(int i=0; i<10; ++i){
        printf("%d ", dt_output[i]);
    }
    printf("Finished :)\n");

    printf("Calculating dw_l2...");
    int dw_l2[100][10];
    for(int i=0; i<100; ++i){
        for(int j=0; j<10; ++j){
            if(spike_diff_l2[i][j] <= 0){
                dw_l2[i][j] = 0;
            } else {            
            if(weight_l2[i][j]==0){
                dw_l2[i][j] = spike_diff_l2[i][j]*(-0.016 * 1000);
            } else {
                dw_l2[i][j] = spike_diff_l2[i][j]*(-0.048 * 1000);
            }}

        }
    }
    printf("The dw_l2 of l2 are:");
    for(int i=0; i<10; ++i){
        printf("%d ", dw_l2[1][i]);
    }
    printf("Finished :)\n");



    printf("Updating weights...");
    for(int i=0; i<100; ++i){
        for(int j=0; j<10; ++j){
            weight_l2_float[i][j] = weight_l2_float[i][j] - (dt_output[j] * dw_l2[i][j] * LEARNING_RATE * 3);
            if(weight_l2_float[i][j] >= 0){
                weight_l2[i][j] = (weight_l2_float[i][j] + 50000000) / 100000000;
            }else{
                weight_l2[i][j] = (weight_l2_float[i][j] - 50000000) / 100000000;
            }
            if(weight_l2[i][j]>7){
                weight_l2[i][j] = 7;
            }else if(weight_l2[i][j]<-8){
                weight_l2[i][j] = -8;
            }
        }
    }
    printf("The grad are:");
    for(int i=0; i<10; ++i){
        printf(" %d ", dt_output[i] * dw_l2[1][i] * LEARNING_RATE * 3);
    }
    printf("\nThe weight_float are:");
    for(int i=0; i<10; ++i){
        printf(" %d ", weight_l2_float[1][i]);
    }
    printf("\nThe weight are:");
    for(int i=0; i<10; ++i){
        printf(" %d ", weight_l2[1][i]);
    }
    printf("Finished :)\n");

    // printf("Writing back the updated weights...");
    // uint32_t write_back_addr;
    // uint32_t write_back_weight;
    // for(int i=0; i<100; ++i){
    //     for(int j=0; j<10; ++j){
    //         if(weight_l2[i][j] < 0){
    //             weight_l2[i][j] = 7 - weight_l2[i][j];
    //         }
    //     }
    //     write_back_addr = i*32 +4638;
    //     write_back_weight = concat_uint32(weight_l2[i][1], weight_l2[i][0], 0, 0, 0, 0, 0, 0);
    //     tinyODIN_synaptic_core_write(&tinyODIN, write_back_addr, write_back_weight);
    //     write_back_addr = i*32 +4639;
    //     write_back_weight = concat_uint32(weight_l2[i][9],weight_l2[i][8], weight_l2[i][7], weight_l2[i][6], weight_l2[i][5], weight_l2[i][4], weight_l2[i][3], weight_l2[i][2]);         // // printf("\nweight_writeback_0: %x", weight_writeback_0);
    //     tinyODIN_synaptic_core_write(&tinyODIN, write_back_addr, write_back_weight);
    // }
    // printf("Finished :)\n");

    // printf("Checking the updated weights...");
    // uint32_t check_weight[10];
    // fetch_weight_l2_addr = 4638;
    // weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
    // weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
    // check_weight[0] = weight_l2_temp>>24  & 0xF;
    // check_weight[1] = weight_l2_temp>>28  & 0xF;
    // fetch_weight_l2_addr = 4639;
    // weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
    // weight_l2_temp = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr);
    // check_weight[2] = weight_l2_temp      & 0xF;
    // check_weight[3] = weight_l2_temp>>4   & 0xF;
    // check_weight[4] = weight_l2_temp>>8   & 0xF;
    // check_weight[5] = weight_l2_temp>>12  & 0xF;
    // check_weight[6] = weight_l2_temp>>16  & 0xF;
    // check_weight[7] = weight_l2_temp>>20  & 0xF;
    // check_weight[8] = weight_l2_temp>>24  & 0xF;
    // check_weight[9] = weight_l2_temp>>28  & 0xF;
    // printf("The written-back weights are: ");
    // for(int i=0; i<10; ++i){
    //     printf("%d ", check_weight[i]);
    // }
    // printf("Finished :)\n");

    printf("---Exit---");
    return EXIT_SUCCESS;
}
