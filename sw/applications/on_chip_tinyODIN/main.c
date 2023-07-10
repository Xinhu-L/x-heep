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
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  
#include "x-heep.h"

#define T_WRONG 1.0
#define T_CORRECT 2.0
#define LEARNING_RATE 3
#define MINI_BATCH 5

int main(int argc, char *argv[])
{
    gpio_params_t gpio_params;
    gpio_t gpio;
    
    gpio_result_t gpio_res;
    tinyODIN_t tinyODIN;

    tinyODIN.base_addr = mmio_region_from_addr((uintptr_t)TINYODIN_START_ADDRESS);

    gpio_res = gpio_init(gpio_params, &gpio);
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);

    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);

    // Set the ports muxed to GPIO and enable
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_PDM2PCM_PDM_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_PDM2PCM_CLK_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2S_SCK_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2S_WS_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2S_SD_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_CS_0_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_CS_1_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_SCK_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_SD_0_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_SD_1_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_SD_2_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_SPI2_SD_3_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SDA_REG_OFFSET), 1);
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SCL_REG_OFFSET), 1);
    for(int i=0; i<24; i++){
        gpio_res = gpio_input_enabled(&gpio, i+8, true);
    }

    // Initialization the weights of tinyODIN
    tinyODIN_synaptic_init(tinyODIN);
    tinyODIN_synaptic_l1_init(tinyODIN);
    tinyODIN_synaptic_l2_init(tinyODIN);

    // Fetch the weights which are to be updated from tinyODIN
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

    // Turn the weights to fix-points
    int weight_l2_float[100][10];
    for(int i=0; i<100; ++i){
        for(int j=0; j<10; ++j){
            weight_l2_float[i][j] = weight_l2[i][j] * 100000000;
        }
    }  

    bool gpio_in[24];
    int spike_time_input[145]; // The 145th element is the label
    int write_count, frame_count, batch_count;
    frame_count = 0;
    batch_count = 0;

    while(1){
        // If GPIO_0 is high, exit the app
        gpio_read(&gpio, 8, &gpio_in[0]);
        if(gpio_in[0] == 1){
            break;
        }

        // IF GPIO_1 is high, one transfer is ready to be read
        gpio_read(&gpio, 9, &gpio_in[1]);  
        if(gpio_in[1] == 1){
            gpio_read(&gpio, 16, &gpio_in[8]);
            gpio_read(&gpio, 17, &gpio_in[9]);
            gpio_read(&gpio, 18, &gpio_in[10]);
            gpio_read(&gpio, 19, &gpio_in[11]);
            gpio_read(&gpio, 20, &gpio_in[12]);
            gpio_read(&gpio, 21, &gpio_in[13]);
            gpio_read(&gpio, 22, &gpio_in[14]);
            gpio_read(&gpio, 23, &gpio_in[15]);

            // Read the input spike from GPIO and store the inputs
            int input_spike_one_pixel;
            input_spike_one_pixel = (gpio_in[15]<<7) | (gpio_in[14]<<6) | (gpio_in[13]<<5) | (gpio_in[12]<<4) | (gpio_in[11]<<3) | (gpio_in[10]<<2) | (gpio_in[9]<<1) | (gpio_in[8]<<0);
            spike_time_input[frame_count] = input_spike_one_pixel;

            // Enter a dead loop, wait for GPIO_1 to be low, which marks the next transfer is begin
            while(1){
                gpio_read(&gpio, 9, &gpio_in[1]);  
                if(gpio_in[1] == 0){
                    frame_count = frame_count + 1;
                    break;
                }
            }

            // If the transfer of a frame is finished, then begin inference
            if(frame_count==145){
                frame_count = 0;
                // Write the input spikes received just now to tinyODIN
                tinyODIN_spike_core_write_call(tinyODIN, spike_time_input);

                // Befor every inference, initialize the neurons' state
                tinyODIN_neuron_core_write_call(tinyODIN);

                // Begin Inference
                tinyODIN_control_write(&tinyODIN, 0, 0xff000400);
                int32_t inference_done = 0;
                while(inference_done!=0xff000401) {
                    inference_done = tinyODIN_control_read(&tinyODIN, 0);
                }

                // Fetch the inference result
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

                // Fetch the spike time of hidden layer
                uint8_t spike_time_hidden[100];
                uint32_t spike_time_hidden_temp;
                for(int i=0; i<25; ++i){
                    spike_time_hidden_temp = tinyODIN_spike_core_read(&tinyODIN, i+36);
                    spike_time_hidden[i*4]   = spike_time_hidden_temp     & 0xFF;
                    spike_time_hidden[i*4+1] = spike_time_hidden_temp>>8  & 0xFF;
                    spike_time_hidden[i*4+2] = spike_time_hidden_temp>>16 & 0xFF;
                    spike_time_hidden[i*4+3] = spike_time_hidden_temp>>24 & 0xFF;
                }

                // Calculating the necessary values used in back-prop
                int spike_diff_l2[100][10];
                for(int i=0; i<100; ++i){
                    for(int j=0; j<10; ++j){
                        spike_diff_l2[i][j] = spike_time_output[j] - spike_time_hidden[i];
                    }
                }

                // Fetch the weights of last layer
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

                // Caculate the dt
                int spike_label;
                spike_label =spike_time_input[144];
                int dt_output[10];
                for(int i=0; i<10; ++i){
                    if(i==spike_label){
                        dt_output[i] = 0.1 * 10 * (spike_time_output[i] - T_CORRECT*100) + dt_output[i];
                    }else{
                        dt_output[i] = 0.1 * 10 * (spike_time_output[i] - T_WRONG*100) + dt_output[i];
                    }
                }

                // Calculate the dw
                int dw_l2[100][10];
                for(int i=0; i<100; ++i){
                    for(int j=0; j<10; ++j){
                        if(spike_diff_l2[i][j] <= 0){
                            dw_l2[i][j] = 0;
                        } else {            
                            if(weight_l2[i][j]==0){
                                dw_l2[i][j] = spike_diff_l2[i][j]*(-0.016 * 1000) + dw_l2[i][j];
                            } else {
                                dw_l2[i][j] = spike_diff_l2[i][j]*(-0.048 * 1000) + dw_l2[i][j];
                            }
                        }
                    }
                }
                batch_count = batch_count + 1;

                if(batch_count == 5){
                    batch_count = 0;
                    // Update the weights
                    for(int i=0; i<100; ++i){
                        for(int j=0; j<10; ++j){
                            weight_l2_float[i][j] = weight_l2_float[i][j] - (dt_output[j] * dw_l2[i][j] * LEARNING_RATE * 3 / (batch_count*batch_count));
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

                    // Write the updated weights back to tinyODIN
                    uint32_t write_back_addr;
                    uint32_t write_back_weight;
                    for(int i=0; i<100; ++i){
                        for(int j=0; j<10; ++j){
                            if(weight_l2[i][j] < 0){
                                weight_l2[i][j] = 7 - weight_l2[i][j];
                            }
                        }
                        write_back_addr = i*32 +4638;
                        write_back_weight = concat_uint32(weight_l2[i][1], weight_l2[i][0], 0, 0, 0, 0, 0, 0);
                        tinyODIN_synaptic_core_write(&tinyODIN, write_back_addr, write_back_weight);
                        write_back_addr = i*32 +4639;
                        write_back_weight = concat_uint32(weight_l2[i][9],weight_l2[i][8], weight_l2[i][7], weight_l2[i][6], weight_l2[i][5], weight_l2[i][4], weight_l2[i][3], weight_l2[i][2]);         // // printf("\nweight_writeback_0: %x", weight_writeback_0);
                        tinyODIN_synaptic_core_write(&tinyODIN, write_back_addr, write_back_weight);
                    }
                }
            }
        }
    }

    printf("---Exit---");
    return EXIT_SUCCESS;
}
