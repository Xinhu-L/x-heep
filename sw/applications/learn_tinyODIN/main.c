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
#include "math.h"

#define TRUE_LEBEL 7  
#define T_CORECT 150
#define T_WRONG 100

#define LEARNING_RATE 1

int main(int argc, char *argv[])
{

    printf("Initializing base address of tinyODIN...");
    tinyODIN_t tinyODIN;
    tinyODIN.base_addr = mmio_region_from_addr((uintptr_t)TINYODIN_START_ADDRESS);
    printf("Finished :)\n");

    printf("Set the input spike...");
    uint32_t spike_time_input[144] = {   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
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
    printf("Finished :)\n");

    printf("Initialize input...");
    tinyODIN_spike_core_write_call(tinyODIN);
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
    tinyODIN_synaptic_l1_init_1(tinyODIN);
    tinyODIN_synaptic_l1_init_2(tinyODIN);
    tinyODIN_synaptic_l1_init_3(tinyODIN);
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
    uint8_t spike_time_output[10]; 
    uint32_t spike_time_output_temp;
    spike_time_output_temp = tinyODIN_spike_core_read(&tinyODIN, 61);
    spike_time_output[0] = spike_time_output_temp>>16 & 0xFF;
    spike_time_output[1] = spike_time_output_temp>>24 & 0xFF;
    spike_time_output_temp = tinyODIN_spike_core_read(&tinyODIN, 62);
    spike_time_output[2] = spike_time_output_temp     & 0xFF;
    spike_time_output[3] = spike_time_output_temp>>8  & 0xFF;
    spike_time_output[4] = spike_time_output_temp>>16 & 0xFF;
    spike_time_output[5] = spike_time_output_temp>>24 & 0xFF;
    spike_time_output_temp = tinyODIN_spike_core_read(&tinyODIN, 63);
    spike_time_output[6] = spike_time_output_temp     & 0xFF;
    spike_time_output[7] = spike_time_output_temp>>8  & 0xFF;
    spike_time_output[8] = spike_time_output_temp>>16 & 0xFF;
    spike_time_output[9] = spike_time_output_temp>>24 & 0xFF;
    // printf("The spike_time_output are: ");
    // for(int i=0; i<10; ++i){
    //     printf("%d,", spike_time_output[i]);
    // }
    printf("Finished :)\n");

    printf("Fetch the hidden spike...");
    uint8_t spike_time_hidden[100];
    uint32_t spike_time_hidden_temp;
    for(int i=0; i<25; ++i){
        spike_time_hidden_temp = tinyODIN_spike_core_read(&tinyODIN, i+36);
        spike_time_hidden[i*4]   = spike_time_hidden_temp     & 0xFF;
        spike_time_hidden[i*4+1] = spike_time_hidden_temp>>8  & 0xFF;
        spike_time_hidden[i*4+2] = spike_time_hidden_temp>>16 & 0xFF;
        spike_time_hidden[i*4+3] = spike_time_hidden_temp>>24 & 0xFF;
    }
    // printf("The spike_time_hidden are: %d, %d ", spike_time_hidden[0], spike_time_hidden[1]);
    printf("Finished :)\n");

    printf("Calculating spike diff of layer2...");
    int spike_diff_l2_1_50[10][10];
    for(int i=0; i<50; ++i){
        for(int j=0; j<10; ++j){
            spike_diff_l2_1_50[i][j] = spike_time_output[j] - spike_time_hidden[i];
            printf("spike_diff_l2_1_50: %d\n,", spike_diff_l2_1_50[i][j]);
        }
    }
    // int spike_diff_l2_11_20[10][10];
    // for(int i=0; i<10; ++i){
    //     for(int j=0; j<10; ++j){
    //         spike_diff_l2_11_20[i][j] = spike_time_output[j] - spike_time_hidden[i+10];
    //         // printf("spike_diff_l2_11_20: %d\n,", spike_diff_l2_11_20[i][j]);
    //     }
    // }
    int spike_diff_l2_50_100[50][10];
    for(int i=0; i<50; ++i){
        for(int j=0; j<10; ++j){
            spike_diff_l2_50_100[i][j] = spike_time_output[j] - spike_time_hidden[i+50];
            printf("spike_diff_l2_50_100: %d\n,", spike_diff_l2_50_100[i][j]);
        }
    }
    printf("The spike diff of l2 are:");
    for(int i=0; i<5; ++i){
        printf("spike_diff_l2_1_10: %d\n,", spike_diff_l2_1_50[0][i]);
        printf("spike_diff_l2_50_100: %d\n,", spike_diff_l2_50_100[0][i]);
    }
    float a;
    a=0.1;
    
    printf("Finished :)\n");

    // printf("Calculating spike diff of layer1...");
    // int spike_diff_l1_1_6[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_1_6[i][j] = spike_time_hidden[j] - spike_time_input[i+0];
    //     }
    // }
    // int spike_diff_l1_7_12[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_7_12[i][j] = spike_time_hidden[j] - spike_time_input[i+6];
    //     }
    // }
    // int spike_diff_l1_13_18[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_13_18[i][j] = spike_time_hidden[j] - spike_time_input[i+12];
    //     }
    // }
    // int spike_diff_l1_19_24[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_19_24[i][j] = spike_time_hidden[j] - spike_time_input[i+18];
    //     }
    // }
    // int spike_diff_l1_25_30[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_25_30[i][j] = spike_time_hidden[j] - spike_time_input[i+24];
    //     }
    // }
    // int spike_diff_l1_31_36[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_31_36[i][j] = spike_time_hidden[j] - spike_time_input[i+30];
    //     }
    // }
    // int spike_diff_l1_37_42[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_37_42[i][j] = spike_time_hidden[j] - spike_time_input[i+36];
    //     }
    // }
    // int spike_diff_l1_43_48[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_43_48[i][j] = spike_time_hidden[j] - spike_time_input[i+42];
    //     }
    // }
    // int spike_diff_l1_49_54[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_49_54[i][j] = spike_time_hidden[j] - spike_time_input[i+48];
    //     }
    // }
    // int spike_diff_l1_55_60[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_55_60[i][j] = spike_time_hidden[j] - spike_time_input[i+54];
    //     }
    // }
    // int spike_diff_l1_61_66[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_61_66[i][j] = spike_time_hidden[j] - spike_time_input[i+60];
    //     }
    // }
    // int spike_diff_l1_67_72[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_67_72[i][j] = spike_time_hidden[j] - spike_time_input[i+66];
    //     }
    // }
    // int spike_diff_l1_73_78[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_73_78[i][j] = spike_time_hidden[j] - spike_time_input[i+72];
    //     }
    // }
    // int spike_diff_l1_79_84[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_79_84[i][j] = spike_time_hidden[j] - spike_time_input[i+78];
    //     }
    // }
    // int spike_diff_l1_85_90[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_85_90[i][j] = spike_time_hidden[j] - spike_time_input[i+84];
    //     }
    // }
    // int spike_diff_l1_91_96[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_91_96[i][j] = spike_time_hidden[j] - spike_time_input[i+90];
    //     }
    // }
    // int spike_diff_l1_97_102[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_97_102[i][j] = spike_time_hidden[j] - spike_time_input[i+96];
    //     }
    // }
    // int spike_diff_l1_103_108[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_103_108[i][j] = spike_time_hidden[j] - spike_time_input[i+102];
    //     }
    // }
    // int spike_diff_l1_109_114[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_109_114[i][j] = spike_time_hidden[j] - spike_time_input[i+108];
    //     }
    // }
    // int spike_diff_l1_115_120[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_115_120[i][j] = spike_time_hidden[j] - spike_time_input[i+114];
    //     }
    // }
    // int spike_diff_l1_121_126[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_121_126[i][j] = spike_time_hidden[j] - spike_time_input[i+120];
    //     }
    // }
    // int spike_diff_l1_127_132[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_127_132[i][j] = spike_time_hidden[j] - spike_time_input[i+126];
    //     }
    // }
    // int spike_diff_l1_133_138[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_133_138[i][j] = spike_time_hidden[j] - spike_time_input[i+132];
    //     }
    // }
    // int spike_diff_l1_139_144[6][100];
    // for(int i=0; i<6; ++i){
    //     for(int j=0; j<100; ++j){
    //         spike_diff_l1_139_144[i][j] = spike_time_hidden[j] - spike_time_input[i+138];
    //     }
    // }
    // printf("The spike diff of the l1 are:");
    // for(int i=0; i<10; ++i){
    //     printf("spike_diff_l1_1_6: %d\n,", spike_diff_l1_1_6[0][i]);
    //     printf("spike_diff_l1_7_12: %d\n,", spike_diff_l1_7_12[0][i]);
    // }
    // printf("Finished :)\n");
    

    // printf("Calculating Loss...");
    // int target[10];
    // for(int i=0; i<10; ++i){
    //     if(i==TRUE_LEBEL){
    //         target[i] = T_CORECT;
    //     } else {
    //         target[i] = T_WRONG;
    //     }
    // }
    // int loss[10];
    // for(int i=0; i<10; ++i){
    //     loss[i] = (spike_time_output[i] - target[i])*(spike_time_output[i] - target[i]);
    // }
    // printf("Finished :)\n");

    /*******************************************************Back Prop***********************************************************/

    // printf("Fetch weights of layer 2 from tinyODIN...");
    // int weight_l2_1_50[50][10];
    // int fetch_weight_l2_addr_0, fetch_weight_l2_addr_1;
    // int weight_l2_temp_0, weight_l2_temp_1;
    // for(int i=0; i<50; ++i){
    //     fetch_weight_l2_addr_0 = i*32 + 4638;
    //     weight_l2_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr_0);
    //     fetch_weight_l2_addr_1 = i*32 + 4639;
    //     weight_l2_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr_1);
    //     weight_l2_1_50[i][0] = weight_l2_temp_0>>24  & 0xF;
    //     weight_l2_1_50[i][1] = weight_l2_temp_0>>28  & 0xF;
    //     weight_l2_1_50[i][2] = weight_l2_temp_1      & 0xF;
    //     weight_l2_1_50[i][3] = weight_l2_temp_1>>4   & 0xF;
    //     weight_l2_1_50[i][4] = weight_l2_temp_1>>8   & 0xF;
    //     weight_l2_1_50[i][5] = weight_l2_temp_1>>12  & 0xF;
    //     weight_l2_1_50[i][6] = weight_l2_temp_1>>16  & 0xF;
    //     weight_l2_1_50[i][7] = weight_l2_temp_1>>20  & 0xF;
    //     weight_l2_1_50[i][8] = weight_l2_temp_1>>24  & 0xF;
    //     weight_l2_1_50[i][9] = weight_l2_temp_1>>28  & 0xF;
    //     for(int j=0; j<10; ++j){
    //         if(weight_l2_1_50[i][j]>7){
    //             weight_l2_1_50[i][j] = 7 - weight_l2_1_50[i][j];
    //         }
    //     }
    // }

    // int weight_l2_50_100[50][10];
    // for(int i=0; i<50; ++i){
    //     fetch_weight_l2_addr_0 = i*32 + 4638 + 50*32;
    //     weight_l2_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr_0);
    //     fetch_weight_l2_addr_1 = i*32 + 4639 + 50*32;
    //     weight_l2_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l2_addr_1);
    //     weight_l2_50_100[i][0] = weight_l2_temp_0>>24  & 0xF;
    //     weight_l2_50_100[i][1] = weight_l2_temp_0>>28  & 0xF;
    //     weight_l2_50_100[i][2] = weight_l2_temp_1      & 0xF;
    //     weight_l2_50_100[i][3] = weight_l2_temp_1>>4   & 0xF;
    //     weight_l2_50_100[i][4] = weight_l2_temp_1>>8   & 0xF;
    //     weight_l2_50_100[i][5] = weight_l2_temp_1>>12  & 0xF;
    //     weight_l2_50_100[i][6] = weight_l2_temp_1>>16  & 0xF;
    //     weight_l2_50_100[i][7] = weight_l2_temp_1>>20  & 0xF;
    //     weight_l2_50_100[i][8] = weight_l2_temp_1>>24  & 0xF;
    //     weight_l2_50_100[i][9] = weight_l2_temp_1>>28  & 0xF;
    //     for(int j=0; j<10; ++j){
    //         if(weight_l2_50_100[i][j]>7){
    //             weight_l2_50_100[i][j] = 7 - weight_l2_50_100[i][j];
    //         }
    //     }
    // }
    // printf("Finished :)\n");

//
    // printf("Fetch weights of layer 1 from tinyODIN...");
    // int fetch_weight_l1_addr_0, fetch_weight_l1_addr_1;
    // int weight_l1_temp_0, weight_l1_temp_1;

    // int weight_l1_1_6[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 0  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_1_6[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_1_6[i][j]>7){
    //             weight_l1_1_6[i][j] = 7 - weight_l1_1_6[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*0 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_1_6[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_1_6[i][j*8+4+k]>7){
    //             weight_l1_1_6[i][j*8+4+k] = 7 - weight_l1_1_6[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // printf("weight_l1_1_6:");
    // for(int i=0; i<6; ++i){
    //     printf("%d, %d, %d\n",weight_l1_1_6[i][0], weight_l1_1_6[i][1], weight_l1_1_6[i][2]);
    // }
    // int weight_l1_7_12[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 6  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_7_12[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_7_12[i][j]>7){
    //             weight_l1_7_12[i][j] = 7 - weight_l1_7_12[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*6 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_7_12[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_7_12[i][j*8+4+k]>7){
    //             weight_l1_7_12[i][j*8+4+k] = 7 - weight_l1_7_12[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_13_18[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 12  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_13_18[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_13_18[i][j]>7){
    //             weight_l1_13_18[i][j] = 7 - weight_l1_13_18[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*12 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_13_18[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_13_18[i][j*8+4+k]>7){
    //             weight_l1_13_18[i][j*8+4+k] = 7 - weight_l1_13_18[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_19_24[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 18  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_19_24[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_19_24[i][j]>7){
    //             weight_l1_19_24[i][j] = 7 - weight_l1_19_24[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*18 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_19_24[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_19_24[i][j*8+4+k]>7){
    //             weight_l1_19_24[i][j*8+4+k] = 7 - weight_l1_19_24[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_25_30[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 24  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_25_30[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_25_30[i][j]>7){
    //             weight_l1_25_30[i][j] = 7 - weight_l1_25_30[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*24 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_25_30[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_25_30[i][j*8+4+k]>7){
    //             weight_l1_25_30[i][j*8+4+k] = 7 - weight_l1_25_30[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_31_36[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 30  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_31_36[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_31_36[i][j]>7){
    //             weight_l1_31_36[i][j] = 7 - weight_l1_31_36[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*30 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_31_36[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_31_36[i][j*8+4+k]>7){
    //             weight_l1_31_36[i][j*8+4+k] = 7 - weight_l1_31_36[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_37_42[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 36  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_37_42[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_37_42[i][j]>7){
    //             weight_l1_37_42[i][j] = 7 - weight_l1_37_42[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*36 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_37_42[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_37_42[i][j*8+4+k]>7){
    //             weight_l1_37_42[i][j*8+4+k] = 7 - weight_l1_37_42[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_43_48[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 42  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_43_48[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_43_48[i][j]>7){
    //             weight_l1_43_48[i][j] = 7 - weight_l1_43_48[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*42 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_43_48[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_43_48[i][j*8+4+k]>7){
    //             weight_l1_43_48[i][j*8+4+k] = 7 - weight_l1_43_48[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_49_54[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 48  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_49_54[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_49_54[i][j]>7){
    //             weight_l1_49_54[i][j] = 7 - weight_l1_49_54[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*48 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_49_54[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_49_54[i][j*8+4+k]>7){
    //             weight_l1_49_54[i][j*8+4+k] = 7 - weight_l1_49_54[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_55_60[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 54  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_55_60[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_55_60[i][j]>7){
    //             weight_l1_55_60[i][j] = 7 - weight_l1_55_60[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*54 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_55_60[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_55_60[i][j*8+4+k]>7){
    //             weight_l1_55_60[i][j*8+4+k] = 7 - weight_l1_55_60[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_61_66[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 60  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_61_66[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_61_66[i][j]>7){
    //             weight_l1_61_66[i][j] = 7 - weight_l1_61_66[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*60 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_61_66[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_61_66[i][j*8+4+k]>7){
    //             weight_l1_61_66[i][j*8+4+k] = 7 - weight_l1_61_66[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_67_72[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 66  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_67_72[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_67_72[i][j]>7){
    //             weight_l1_67_72[i][j] = 7 - weight_l1_67_72[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*66 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_67_72[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_67_72[i][j*8+4+k]>7){
    //             weight_l1_67_72[i][j*8+4+k] = 7 - weight_l1_67_72[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_73_78[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 72  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_73_78[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_73_78[i][j]>7){
    //             weight_l1_73_78[i][j] = 7 - weight_l1_73_78[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*72 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_73_78[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_73_78[i][j*8+4+k]>7){
    //             weight_l1_73_78[i][j*8+4+k] = 7 - weight_l1_73_78[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_79_84[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 78  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_79_84[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_79_84[i][j]>7){
    //             weight_l1_79_84[i][j] = 7 - weight_l1_79_84[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*78 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_79_84[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_79_84[i][j*8+4+k]>7){
    //             weight_l1_79_84[i][j*8+4+k] = 7 - weight_l1_79_84[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_85_90[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 84  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_85_90[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_85_90[i][j]>7){
    //             weight_l1_85_90[i][j] = 7 - weight_l1_85_90[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*84 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_85_90[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_85_90[i][j*8+4+k]>7){
    //             weight_l1_85_90[i][j*8+4+k] = 7 - weight_l1_85_90[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_91_96[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 90  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_91_96[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_91_96[i][j]>7){
    //             weight_l1_91_96[i][j] = 7 - weight_l1_91_96[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*90 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_91_96[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_91_96[i][j*8+4+k]>7){
    //             weight_l1_91_96[i][j*8+4+k] = 7 - weight_l1_91_96[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_97_102[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 96  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_97_102[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_97_102[i][j]>7){
    //             weight_l1_97_102[i][j] = 7 - weight_l1_97_102[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*96 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_97_102[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_97_102[i][j*8+4+k]>7){
    //             weight_l1_97_102[i][j*8+4+k] = 7 - weight_l1_97_102[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_103_108[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 102  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_103_108[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_103_108[i][j]>7){
    //             weight_l1_103_108[i][j] = 7 - weight_l1_103_108[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*102 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_103_108[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_103_108[i][j*8+4+k]>7){
    //             weight_l1_103_108[i][j*8+4+k] = 7 - weight_l1_103_108[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_109_114[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 108  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_109_114[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_109_114[i][j]>7){
    //             weight_l1_109_114[i][j] = 7 - weight_l1_109_114[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*108 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_109_114[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_109_114[i][j*8+4+k]>7){
    //             weight_l1_109_114[i][j*8+4+k] = 7 - weight_l1_109_114[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_115_120[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 114  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_115_120[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_115_120[i][j]>7){
    //             weight_l1_115_120[i][j] = 7 - weight_l1_115_120[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*114 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_115_120[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_115_120[i][j*8+4+k]>7){
    //             weight_l1_115_120[i][j*8+4+k] = 7 - weight_l1_115_120[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_121_126[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 120  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_121_126[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_121_126[i][j]>7){
    //             weight_l1_121_126[i][j] = 7 - weight_l1_121_126[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*120 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_121_126[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_121_126[i][j*8+4+k]>7){
    //             weight_l1_121_126[i][j*8+4+k] = 7 - weight_l1_121_126[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_127_132[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 126  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_127_132[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_127_132[i][j]>7){
    //             weight_l1_127_132[i][j] = 7 - weight_l1_127_132[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*126 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_127_132[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_127_132[i][j*8+4+k]>7){
    //             weight_l1_127_132[i][j*8+4+k] = 7 - weight_l1_127_132[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_133_138[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 132  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_133_138[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_133_138[i][j]>7){
    //             weight_l1_133_138[i][j] = 7 - weight_l1_133_138[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*132 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_133_138[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_133_138[i][j*8+4+k]>7){
    //             weight_l1_133_138[i][j*8+4+k] = 7 - weight_l1_133_138[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // int weight_l1_139_144[6][100];
    // for(int i=0; i<6; ++i){
    //     fetch_weight_l1_addr_0 = i*32 + 32* 138  + 18;
    //     weight_l1_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_0);
    //     for(int j=0; j<4; ++j){
    //         weight_l1_139_144[i][j] = weight_l1_temp_0 >> (4*j+16) & 0xF;
    //         if(weight_l1_139_144[i][j]>7){
    //             weight_l1_139_144[i][j] = 7 - weight_l1_139_144[i][j];
    //         }
    //     }
    //     for(int j=0; j<12; ++j){
    //         fetch_weight_l1_addr_1 = i*32 + 32*138 + 19;
    //         weight_l1_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, fetch_weight_l1_addr_1);
    //         for(int k=0; k<8; ++k){
    //             weight_l1_139_144[i][j*8+4+k] = weight_l1_temp_0 >> (4*k) & 0xF;
    //             if(weight_l1_139_144[i][j*8+4+k]>7){
    //             weight_l1_139_144[i][j*8+4+k] = 7 - weight_l1_139_144[i][j*8+4+k];
    //         }
    //         }
    //     }
    // }
    // printf("Finished :)\n");
//

    // printf("Calculating dw_l2...");
    // int dw_l2[10][10];
    // for(int i=0; i<100; ++i){
    //     for(int j=0; j<10; ++j){
    //         if(weight_l2_1_50[i][j]==0){
    //             // dw_l2[i][j] = output_minus_input[i][j]*(-16);
    //         } else {
    //             // dw_l2[i][j] = output_minus_input[i][j]*(-48);
    //         }
    //     }
    // }
    // // printf("The dw_l2 of the first neuron are:");
    // for(int i=0; i<10; ++i){
    //     // printf("%d,", dw_l2[0][i]);
    // }
    // printf("Finished :)\n");

    // printf("Updating weights of layer2...");
    // for(int i=0; i<50; ++i){
    //     for(int j=0; j<10; ++j){
    //     weight_l2_1_50[i][j] = weight_l2_1_50[i][j] + (dw_l2[i][j] * LEARNING_RATE)/1000;
    //     if (weight_l2_1_50[i][j] > 7){
    //         weight_l2_1_50[i][j] = 7;
    //     } else if (weight_l2_1_50[i][j] < -8) {
    //         weight_l2_1_50[i][j] = -8;
    //     }
    //     if (weight_l2_1_50[i][j] < 0){
    //         weight_l2_1_50[i][j] = 7 - weight_l2_1_50[i][j];
    //     }
    //     }
    // }
    // // printf("The updated weights of the first neuron are:");
    // for(int i=0; i<10; ++i){
    //     // printf("%d,", weight_l2_1_50[0][i]);
    // }
    // printf("Finished :)\n");

    // printf("Writing back updated weights...");
    // uint32_t weight_writeback_0, weight_writeback_1;
    // int weight_writeback_addr_0, weight_writeback_addr_1;
    // weight_writeback_0 = 0;
    // weight_writeback_0 = weight_l2_1_50[0][1] << 28;  
    // weight_writeback_0 |= (weight_l2_1_50[0][0] & 0xF) << 24;
    // weight_writeback_addr_0 = 0*32 + 0 +4638;
    // tinyODIN_synaptic_core_write(&tinyODIN, weight_writeback_addr_0, weight_writeback_0);
    // uint32_t test;
    // test = tinyODIN_synaptic_core_read(&tinyODIN, weight_writeback_addr_0);
    //  printf("%x\n",test);

    // weight_writeback_0 = concat_uint32(weight_l2_1_50[0][1], weight_l2_1_50[0][0], 0, 0, 0, 0, 0, 0);
    // weight_writeback_1 = concat_uint32(weight_l2_1_50[0][1],weight_l2_1_50[0][1], weight_l2_1_50[0][1], weight_l2_1_50[0][1], weight_l[0][1], weight_l2_1_50[0][1], weight_l2_1_50[0][1], weight_l2_1_50[0][1]);         // // printf("\nweight_writeback_0: %x", weight_writeback_0);
    // for(int i=0; i<100; ++i){
        // weight_writeback_addr_0 = i*32 + 0 +4638;
        // weight_writeback_addr_1 = i*32 + 1 +4638;
        // weight_writeback_0 = concat_uint32(weight_l2_1_50[i][1], weight_l2_1_50[i][0], 0, 0, 0, 0, 0, 0);
        // // printf("\nweight_writeback_0: %x", weight_writeback_0);
        // tinyODIN_synaptic_core_write(&tinyODIN, weight_writeback_addr_0, weight_writeback_0);
        // weight_writeback_1 = concat_uint32(weight_l2_1_50[i][9], weight_l2_1_50[i][8], weight_l2_1_50[i][7], weight_l2_1_50[i][6], weight_l2_1_50[i][5], weight_l2_1_50[i][4], weight_l2_1_50[i][3], weight_l2_1_50[i][2]); 
        // // printf("\nweight_writeback_1: %x", weight_writeback_1);
        // tinyODIN_synaptic_core_write(&tinyODIN, weight_writeback_addr_1, weight_writeback_1);
    // } 
    // printf("Finished :)\n");



    // printf("Check updated weights...");
    //     weight_l2_temp_0 = tinyODIN_synaptic_core_read(&tinyODIN, weight_writeback_addr_0);
    //     weight_l2_temp_1 = tinyODIN_synaptic_core_read(&tinyODIN, weight_writeback_addr_1);
    //     weight_l2_1_50[i][0] = weight_l2_temp_0>>24  & 0xF;
    //     weight_l2_1_50[i][1] = weight_l2_temp_0>>28  & 0xF;
    //     weight_l2_1_50[i][2] = weight_l2_temp_1      & 0xF;
    //     weight_l2_1_50[i][3] = weight_l2_temp_1>>4   & 0xF;
    //     weight_l2_1_50[i][4] = weight_l2_temp_1>>8   & 0xF;
    //     weight_l2_1_50[i][5] = weight_l2_temp_1>>12  & 0xF;
    //     weight_l2_1_50[i][6] = weight_l2_temp_1>>16  & 0xF;
    //     weight_l2_1_50[i][7] = weight_l2_temp_1>>20  & 0xF;
    //     weight_l2_1_50[i][8] = weight_l2_temp_1>>24  & 0xF;
    //     weight_l2_1_50[i][9] = weight_l2_temp_1>>28  & 0xF;
    //     for(int i; i<10; ++i){
    //         if(weight_l2_1_50[i]>7){
    //             weight_l2_1_50[i] = 7 - weight_l2_1_50[i];
    //         }
    //     }
    // printf("\nThe updated weights of neuron %d are", neuron_idx);
    // for(int i; i<10; ++i){
    //     printf("%d,", weight_l2_1_50[i]);
    // }
    // printf("Finished :)\n");



    printf("---Exit---");
    return EXIT_SUCCESS;
}

