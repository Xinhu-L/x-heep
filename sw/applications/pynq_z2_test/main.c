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

    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_res = gpio_init(gpio_params, &gpio);
    
    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);


    gpio_res = gpio_input_enabled(&gpio, 15, true);
    gpio_res = gpio_output_set_enabled(&gpio, 16, true);
    gpio_res = gpio_output_set_enabled(&gpio, 17, true);
    bool bit;

    gpio_write(&gpio, 16, true);

    while(1){
        gpio_read(&gpio, 15, &bit);

        if(bit == true){
            gpio_write(&gpio, 16, false);
            gpio_write(&gpio, 17, true);
        }

    }
    
    printf("---Exit---");
    return EXIT_SUCCESS;
}
