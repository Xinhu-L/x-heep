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

#define GPIO_INTR  GPIO_INTR_16

int main(int argc, char *argv[])
{
    plic_result_t plic_res;
    gpio_params_t gpio_params, gpio_ap_params;
    gpio_t gpio, gpio_ao;
    
    gpio_result_t gpio_res;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_AO_START_ADDRESS);
    gpio_res = gpio_init(gpio_ap_params, &gpio_ao);
    if (gpio_res != kGpioOk) {
        printf("Failed\n;");
        return -1;
    }
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_res = gpio_init(gpio_params, &gpio);
    if (gpio_res != kGpioOk) {
        printf("Failed\n;");
        return -1;
    }


    for(int i=0; i<4; i++){
        gpio_res = gpio_output_set_enabled(&gpio, i+12, true);
        if (gpio_res != kGpioOk) {
            printf("Failed\n;");
            return -1;
        }
    }

    for(int i=0; i<4; i++){
        gpio_res = gpio_input_enabled(&gpio, i+8, true);
        if (gpio_res != kGpioOk) {
            printf("Failed\n;");
            return -1;
        }
    }

    bool bit_0, bit_1, bit_2, bit_3;
    gpio_write(&gpio, 12, false);
    gpio_write(&gpio, 13, true);
    gpio_write(&gpio, 14, false);
    gpio_write(&gpio, 15, true);
    gpio_read(&gpio, 8, &bit_0);
    gpio_read(&gpio, 9, &bit_1);
    gpio_read(&gpio, 10, &bit_2);
    gpio_read(&gpio, 11, &bit_3);

    bool bit_7, bit_6, bit_5, bit_4;
    gpio_write(&gpio, 12, false);
    gpio_write(&gpio, 13, false);
    gpio_write(&gpio, 14, false);
    gpio_write(&gpio, 15, true);
    gpio_read(&gpio, 8, &bit_4);
    gpio_read(&gpio, 9, &bit_5);
    gpio_read(&gpio, 10, &bit_6);
    gpio_read(&gpio, 11, &bit_7);

    tinyODIN_t tinyODIN;
    tinyODIN.base_addr = mmio_region_from_addr((uintptr_t)TINYODIN_START_ADDRESS);
    printf("Finished :)\n");

    int input_spike_check, input_spike;
    input_spike = (bit_7<<7) + (bit_6<<6) + (bit_5<<5) + (bit_4<<4) + (bit_3<<3) + (bit_2<<2) + (bit_1<<1) + (bit_0<<0);
    gpio_write(&gpio, 12, true);
    gpio_write(&gpio, 13, false);
    gpio_write(&gpio, 14, false);
    gpio_write(&gpio, 15, false);
    gpio_read(&gpio, 8, &bit_0);
    gpio_read(&gpio, 9, &bit_1);
    gpio_read(&gpio, 10, &bit_2);
    gpio_read(&gpio, 11, &bit_3);
    gpio_write(&gpio, 12, true);
    gpio_write(&gpio, 13, false);
    gpio_write(&gpio, 14, true);
    gpio_write(&gpio, 15, true);
    gpio_read(&gpio, 8, &bit_4);
    gpio_read(&gpio, 9, &bit_5);
    gpio_read(&gpio, 10, &bit_6);
    gpio_read(&gpio, 11, &bit_7);
    input_spike = input_spike<<8 + (bit_7<<7) + (bit_6<<6) + (bit_5<<5) + (bit_4<<4) + (bit_3<<3) + (bit_2<<2) + (bit_1<<1) + (bit_0<<0); 
    printf("Input spike is %x", input_spike);
    tinyODIN_spike_core_write(&tinyODIN, 0, input_spike);
    input_spike_check = tinyODIN_spike_core_read(&tinyODIN, 0);
    printf("Read-back spike is %x", input_spike_check);
    printf("---Exit---");
    return EXIT_SUCCESS;
}
