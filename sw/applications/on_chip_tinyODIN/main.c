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
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"

int main(int argc, char *argv[])
{
    gpio_params_t gpio_params;
    gpio_t gpio;
    
    gpio_result_t gpio_res;
    tinyODIN_t tinyODIN;

    tinyODIN.base_addr = mmio_region_from_addr((uintptr_t)TINYODIN_START_ADDRESS);
    
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_res = gpio_init(gpio_params, &gpio);

    for(int i=0; i<10; i++){
        gpio_res = gpio_input_enabled(&gpio, i+8, true);
    }

    tinyODIN_synaptic_init(tinyODIN);
    tinyODIN_synaptic_l1_init(tinyODIN);
    tinyODIN_synaptic_l2_init(tinyODIN);

    bool bit_7, bit_6, bit_5, bit_4, bit_0, bit_1, bit_2, bit_3;
    bool one_pixel, exit;
    int input_spike_one_pixel;
    int input_spike[144];
    int frame_count = 0;
    while(1){

        gpio_read(&gpio, 17, &exit);
        if(exit == 1){
            break;
        }
        gpio_read(&gpio, 16, &one_pixel);
        if(one_pixel == 1){
            printf("One pixel coming...");
            gpio_read(&gpio, 8, &bit_0);
            gpio_read(&gpio, 9, &bit_1);
            gpio_read(&gpio, 10, &bit_2);
            gpio_read(&gpio, 11, &bit_3);
            gpio_read(&gpio, 12, &bit_4);
            gpio_read(&gpio, 13, &bit_5);
            gpio_read(&gpio, 14, &bit_6);
            gpio_read(&gpio, 15, &bit_7);
            input_spike_one_pixel = (bit_7<<7) + (bit_6<<6) + (bit_5<<5) + (bit_4<<4) + (bit_3<<3) + (bit_2<<2) + (bit_1<<1) + (bit_0<<0);
            input_spike[frame_count] = input_spike_one_pixel;
            frame_count = frame_count + 1;
            if(frame_count==144){
                for(ptrdiff_t i=0x00000000; i<0x00000101; i++){
                    neuron_addr = i;
                    neuron_data = 0x0015e000;
                    tinyODIN_neuron_core_write(&tinyODIN,neuron_addr,neuron_data);
                }
                tinyODIN_spike_core_write_call(tinyODIN, input_spike);
                printf("Inferencing...");
                uint32_t inference_done;
                while(inference_done!=0xff000401) {
                    inference_done = tinyODIN_control_read(&tinyODIN, 0);
                }
                printf("Finished :)\n");
            }
        }

    }

    printf("---Exit---");
    return EXIT_SUCCESS;
}
