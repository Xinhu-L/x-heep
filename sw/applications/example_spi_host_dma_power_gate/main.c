// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "soc_ctrl.h"
#include "spi_host.h"
#include "dma.h"
#include "fast_intr_ctrl.h"
#include "power_manager.h"

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif

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

// Type of data frome the SPI. For types different than words the SPI data is requested in separate transactions
// word(0), half-word(1), byte(2,3)
#define SPI_DATA_TYPE 0

// Number of elements to copy
#define COPY_DATA_NUM 16

#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

int8_t dma_intr_flag;
int8_t core_sleep_flag;
spi_host_t spi_host;

static power_manager_t power_manager;

void dma_intr_handler_trans_done(void)
{
    PRINTF("Non-weak implementation of a DMA interrupt\n\r");
    dma_intr_flag = 1;
}

// Reserve memory array
#if SPI_DATA_TYPE == 0
    uint32_t flash_data[COPY_DATA_NUM] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};
    uint32_t copy_data[COPY_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };
#elif SPI_DATA_TYPE == 1
    uint16_t flash_data[COPY_DATA_NUM] __attribute__ ((aligned (2))) = {0x7654,0xfedc,0x579a,0x657d,0x758e,0x0123,0xfedb,0x89ab,0x6798,0xff82,0x763b,0x6875,0x09ac,0x666b,0x4455,0x0000};
    uint16_t copy_data[COPY_DATA_NUM] __attribute__ ((aligned (2)))  = { 0 };
#else
    uint8_t flash_data[COPY_DATA_NUM] = {0x76,0xfe,0x57,0x65,0x75,0x01,0xfe,0x89,0x67,0xff,0x76,0x68,0x09,0x66,0x44,0x00};
    uint8_t copy_data[COPY_DATA_NUM] = { 0 };
#endif

int main(int argc, char *argv[])
{

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
 
#ifdef USE_SPI_FLASH
   if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO )
    {
        PRINTF("This application cannot work with the memory mapped SPI FLASH module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    }
#endif

    #ifndef USE_SPI_FLASH
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;
    power_manager_counters_t power_manager_cpu_counters;
    // Init cpu_subsystem's counters
    if (power_gate_counters_init(&power_manager_cpu_counters, 300, 300, 300, 300, 300, 300, 0, 0) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail. Check the reset and powergate counters value\n\r");
        return EXIT_FAILURE;
    }

    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    
    // Set mie.MEIE bit to one to enable machine-level fast dma interrupt
    const uint32_t mask = 1 << 19;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    #ifdef USE_SPI_FLASH
        // Select SPI host as SPI output
        soc_ctrl_select_spi_host(&soc_ctrl);
    #endif

    // Enable SPI host device
    spi_set_enable(&spi_host, true);
    // Enable SPI output
    spi_output_enable(&spi_host, true);

    // SPI and SPI_FLASH are the same IP so same register map
    uint32_t *fifo_ptr_rx = spi_host.base_addr.base + SPI_HOST_RXDATA_REG_OFFSET;

    core_sleep_flag = 0;

    // -- DMA CONFIGURATION --

    dma_init(NULL);

    #ifndef USE_SPI_FLASH
        uint8_t slot =  DMA_TRIG_SLOT_SPI_RX ; // The DMA will wait for the SPI RX FIFO valid signal
    #else
        uint8_t slot =  DMA_TRIG_SLOT_SPI_FLASH_RX ; // The DMA will wait for the SPI FLASH RX FIFO valid signal
    #endif

    static dma_target_t tgt_src = {
        .size_du = COPY_DATA_NUM,
        .inc_du = 0,
        .type = SPI_DATA_TYPE,
    };
    tgt_src.ptr = fifo_ptr_rx;
    tgt_src.trig = slot;

    static dma_target_t tgt_dst = {
        .ptr = copy_data,
        .inc_du = 1,
        .type = SPI_DATA_TYPE,
        .trig = DMA_TRIG_MEMORY,
    };

    static dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_INTR,
    };

    dma_config_flags_t res;

    res = dma_validate_transaction(&trans ,DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("trans: %u \n\r", res );
    res = dma_load_transaction(&trans);
    PRINTF(" load: %u \n\r", res );



    // Configure SPI clock
    // SPI clk freq = 1/2 core clk freq when clk_div = 0
    // SPI_CLK = CORE_CLK/(2 + 2 * CLK_DIV) <= CLK_MAX => CLK_DIV > (CORE_CLK/CLK_MAX - 2)/2
    uint16_t clk_div = 0;
    if(FLASH_CLK_MAX_HZ < core_clk/2){
        clk_div = (core_clk/(FLASH_CLK_MAX_HZ) - 2)/2; // The value is truncated
        if (core_clk/(2 + 2 * clk_div) > FLASH_CLK_MAX_HZ) clk_div += 1; // Adjust if the truncation was not 0
    }
    // SPI Configuration
    // Configure chip 0 (flash memory)
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = clk_div,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0
    });
    spi_set_configopts(&spi_host, 0, chip_cfg);
    spi_set_csid(&spi_host, 0);

    // Reset
    const uint32_t reset_cmd = 0xFFFFFFFF;
    spi_write_word(&spi_host, reset_cmd);
    const uint32_t cmd_reset = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_reset);
    spi_wait_for_ready(&spi_host);

    // Power up flash
    const uint32_t powerup_byte_cmd = 0xab;
    spi_write_word(&spi_host, powerup_byte_cmd);
    const uint32_t cmd_powerup = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_powerup);
    spi_wait_for_ready(&spi_host);

    // Load command FIFO with read command (1 Byte at single speed)
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });

    uint32_t read_byte_cmd;
    read_byte_cmd = ((REVERT_24b_ADDR(flash_data) << 8) | 0x03); // The address bytes sent through the SPI to the Flash are in reverse order

    dma_intr_flag = 0;
    dma_launch(&trans);
    PRINTF("Launched\n\r");

    #if SPI_DATA_TYPE == 0
        const uint32_t cmd_read_rx = spi_create_command((spi_command_t){ // Single transaction
            .len        = COPY_DATA_NUM*sizeof(*copy_data) - 1, // In bytes - 1
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        spi_write_word(&spi_host, read_byte_cmd); // Fill TX FIFO with TX data (read command + 3B address)
        spi_wait_for_ready(&spi_host); // Wait for readiness to process commands
        spi_set_command(&spi_host, cmd_read); // Send read command to the external device through SPI
        spi_wait_for_ready(&spi_host);
        spi_set_command(&spi_host, cmd_read_rx); // Receive data in RX
        spi_wait_for_ready(&spi_host);
    #else
        const uint32_t cmd_read_rx = spi_create_command((spi_command_t){ // Multiple transactions of the data type
            .len        = (sizeof(*copy_data) - 1),
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        for (int i = 0; i<COPY_DATA_NUM; i++) { // Multiple 16-bit transactions
            // Request the same data multiple times
            spi_write_word(&spi_host, read_byte_cmd); // Fill TX FIFO with TX data (read command + 3B address)
            spi_wait_for_ready(&spi_host); // Wait for readiness to process commands
            spi_set_command(&spi_host, cmd_read); // Send read command to the external device through SPI
            spi_wait_for_ready(&spi_host);
            spi_set_command(&spi_host, cmd_read_rx); // Receive data in RX
            spi_wait_for_ready(&spi_host);
        }
    #endif

    // Power gate core and wait for fast DMA interrupt
    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if(dma_intr_flag == 0) {
        if (power_gate_core(&power_manager, kDma_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
        {
            PRINTF("Error: power manager fail.\n\r");
            return EXIT_FAILURE;
        }
        core_sleep_flag = 1;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    if(core_sleep_flag == 1) PRINTF("Woke up from sleep!\n\r");

    // Wait for DMA interrupt
    if( trans.end == DMA_TRANS_END_POLLING ){
        PRINTF("Waiting for DMA DONE...\n\r");
        while( ! dma_is_ready() ){};
    } else{
        PRINTF("Waiting for the DMA interrupt...\n\r");
        while(dma_intr_flag == 0) {
            wait_for_interrupt();
        }
    }
    PRINTF("triggered!\n\r");

    // Power down flash
    const uint32_t powerdown_byte_cmd = 0xb9;
    spi_write_word(&spi_host, powerdown_byte_cmd);
    const uint32_t cmd_powerdown = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_powerdown);
    spi_wait_for_ready(&spi_host);

    // The data is already in memory -- Check results
    PRINTF("flash vs ram...\n\r");

    uint32_t errors = 0;
    uint32_t count = 0;
    #if SPI_DATA_TYPE == 0
        for (int i = 0; i<COPY_DATA_NUM; i++) {
            if(flash_data[i] != copy_data[i]) {
                PRINTF("@%08x-@%08x : %02x != %02x\n\r" , &flash_data[i] , &copy_data[i], flash_data[i], copy_data[i]);
                errors++;
            }
            count++;
        }
    #else
        for (int i = 0; i<COPY_DATA_NUM; i++) {
            if(flash_data[0] != copy_data[i]) {
                PRINTF("@%08x-@%08x : %02x != %02x\n\r" , &flash_data[0] , &copy_data[i], flash_data[0], copy_data[i]);
                errors++;
            }
            count++;
        }
    #endif

    if (errors == 0) {
        PRINTF("success! (bytes checked: %d)\n\r", count*sizeof(*copy_data));
    } else {
        PRINTF("failure, %d errors! (Out of %d)\n\r", errors, count);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
