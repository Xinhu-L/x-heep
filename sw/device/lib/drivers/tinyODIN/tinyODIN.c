// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>

#include "tinyODIN.h"
#include "tinyODIN_SRAM.h"  

uint32_t tinyODIN_spike_core_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr) {
  uint32_t val;
  val = mmio_region_read32(tinyODIN->base_addr, TINYODIN_SPIKE_CORE_MUX+read_addr*4);
  return val;
}

void tinyODIN_spike_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data) {
  ptrdiff_t addr;
  addr = (ptrdiff_t)(write_addr*4 + TINYODIN_SPIKE_CORE_MUX);
  mmio_region_write32(tinyODIN->base_addr, addr, write_data);
}

uint32_t tinyODIN_neuron_core_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr) {
  uint32_t val;
  val = mmio_region_read32(tinyODIN->base_addr, TINYODIN_NEURON_CORE_MUX+read_addr*4); 
  return val;
}

void tinyODIN_neuron_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data) {
  ptrdiff_t addr;
  addr = (ptrdiff_t)(write_addr*4 + TINYODIN_NEURON_CORE_MUX);
  mmio_region_write32(tinyODIN->base_addr, addr, write_data);
}

uint32_t tinyODIN_synaptic_core_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr) {
  uint32_t val;
  val = mmio_region_read32(tinyODIN->base_addr, TINYODIN_SYNAPTIC_CORE_MUX+read_addr*4); 
  return val;
}

void tinyODIN_synaptic_core_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data) {
  ptrdiff_t addr;
  addr = (ptrdiff_t)(write_addr*4 + TINYODIN_SYNAPTIC_CORE_MUX);
  mmio_region_write32(tinyODIN->base_addr, addr, write_data);
}

uint32_t tinyODIN_control_read(const tinyODIN_t *tinyODIN, const ptrdiff_t read_addr) {
  uint32_t val;
  val = mmio_region_read32(tinyODIN->base_addr, TINYODIN_CONTROL_MUX+read_addr*4); 
  return val;
}

void tinyODIN_control_write(const tinyODIN_t *tinyODIN, const ptrdiff_t write_addr, uint32_t write_data) {
  ptrdiff_t addr;
  addr = (ptrdiff_t)(write_addr*4 + TINYODIN_CONTROL_MUX);
  mmio_region_write32(tinyODIN->base_addr, addr, write_data);
}

