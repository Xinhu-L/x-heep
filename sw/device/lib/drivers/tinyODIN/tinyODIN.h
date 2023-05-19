// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_tinyODIN_H_
#define _DRIVERS_tinyODIN_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for tinyODIN.
 *
 */
typedef struct tinyODIN {
  /**
   * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t base_addr;
} tinyODIN_t;

uint32_t tinyODIN_spike_core_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr);


void tinyODIN_spike_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data);


uint32_t tinyODIN_neuron_core_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr);


void tinyODIN_spike_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data);


uint32_t tinyODIN_synaptic_core_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr);


void tinyODIN_spike_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data);


uint32_t tinyODIN_control_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr);


void tinyODIN_spike_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_tinyODIN_H_
