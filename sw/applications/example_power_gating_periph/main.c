// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "power_manager.h"

/* Change this value to 0 to disable prints for FPGA and enable them for simulation. */
#define DEFAULT_PRINTF_BEHAVIOR 1

/* By default, printfs are activated for FPGA and disabled for simulation. */
#ifdef TARGET_PYNQ_Z2 
    #define ENABLE_PRINTF DEFAULT_PRINTF_BEHAVIOR
#else 
    #define ENABLE_PRINTF !DEFAULT_PRINTF_BEHAVIOR
#endif

#if ENABLE_PRINTF
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif 

static power_manager_t power_manager;

int main(int argc, char *argv[])
{
    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;

    power_manager_counters_t power_manager_periph_counters;

    // Init peripheral_subsystem's counters
    if (power_gate_counters_init(&power_manager_periph_counters, 30, 30, 30, 30, 30, 30, 0, 0) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail. Check the reset and powergate counters value\n\r");
        return EXIT_FAILURE;
    }

    // Power off peripheral_subsystem domain
    if (power_gate_periph(&power_manager, kOff_e, &power_manager_periph_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }

    // Check that the peripheral_subsystem domain is actually OFF
    while(!periph_power_domain_is_off(&power_manager));

    // Wait some time
    for (int i=0; i<100; i++) asm volatile("nop;");

    // Power on peripheral_subsystem domain
    if (power_gate_periph(&power_manager, kOn_e, &power_manager_periph_counters) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail.\n\r");
        return EXIT_FAILURE;
    }

    /* write something to stdout */
    PRINTF("Success.\n\r");
    return EXIT_SUCCESS;
}
