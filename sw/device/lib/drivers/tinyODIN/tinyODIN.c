// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include "tinyODIN.h"
#include "tinyODIN_SRAM.h"  


#define EPSILON 1e-8

typedef struct {
    float *parameters;
    float *gradients;
    float *m;
    float *v;
    float beta1;
    float beta2;
    float learning_rate;
    int num_parameters;
    int t;
} AdamOptimizer;

AdamOptimizer *adam_optimizer_create(float *parameters, float *gradients, float learning_rate, float beta1, float beta2, int num_parameters) {
    AdamOptimizer *optimizer = (AdamOptimizer *)malloc(sizeof(AdamOptimizer));

    optimizer->parameters = parameters;
    optimizer->gradients = gradients;
    optimizer->m = (float *)malloc(num_parameters * sizeof(float));
    optimizer->v = (float *)malloc(num_parameters * sizeof(float));
    optimizer->beta1 = beta1;
    optimizer->beta2 = beta2;
    optimizer->learning_rate = learning_rate;
    optimizer->num_parameters = num_parameters;
    optimizer->t = 0;

    // 初始化 m 和 v
    for (int i = 0; i < num_parameters; i++) {
        optimizer->m[i] = 0.0;
        optimizer->v[i] = 0.0;
    }

    return optimizer;
}

void adam_optimizer_step(AdamOptimizer *optimizer) {
    optimizer->t += 1;
    float lr_t = optimizer->learning_rate * sqrt(1.0 - pow(optimizer->beta2, optimizer->t)) / (1.0 - pow(optimizer->beta1, optimizer->t));

    for (int i = 0; i < optimizer->num_parameters; i++) {
        optimizer->m[i] = optimizer->beta1 * optimizer->m[i] + (1.0 - optimizer->beta1) * optimizer->gradients[i];
        optimizer->v[i] = optimizer->beta2 * optimizer->v[i] + (1.0 - optimizer->beta2) * pow(optimizer->gradients[i], 2);

        float delta = -lr_t * optimizer->m[i] / (sqrt(optimizer->v[i]) + EPSILON);
        optimizer->parameters[i] += delta;
    }
}

void adam_optimizer_destroy(AdamOptimizer *optimizer) {
    free(optimizer->m);
    free(optimizer->v);
    free(optimizer);
}

uint32_t concat_uint32(uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4,
                       uint32_t num5, uint32_t num6, uint32_t num7, uint32_t num8) {
    uint32_t result = 0;

    result |= (num1 & 0x0F) << 28;
    result |= (num2 & 0x0F) << 24;
    result |= (num3 & 0x0F) << 20;
    result |= (num4 & 0x0F) << 16;
    result |= (num5 & 0x0F) << 12;
    result |= (num6 & 0x0F) << 8;
    result |= (num7 & 0x0F) << 4;
    result |= (num8 & 0x0F);

    return result;
}

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

// Write spike

void tinyODIN_spike_core_write_call(const tinyODIN_t tinyODIN, uint32_t input[144]) {
  uint32_t input_spike;
  int input_spike_addr;
    // uint32_t spike_time_input[144] = {  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    //                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    //                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    //                                     0x45, 0x4B, 0x61, 0xFF, 0x5B, 0x5B, 0x5A, 0x52, 
    //                                     0xFF, 0x64, 0x5F, 0x5C, 0x2E, 0x39, 0x5F, 0xFF, 
    //                                     0x26, 0x26, 0x27, 0x27, 0xFF, 0x60, 0x36, 0x24, 
    //                                     0x5F, 0x60, 0x64, 0xFF, 0x4E, 0x51, 0x54, 0x59, 
    //                                     0xFF, 0x61, 0x34, 0x2A, 0xFF, 0xFF, 0xFF, 0xFF, 
    //                                     0x4D, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0x21, 
    //                                     0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0x5F, 0xFF, 0xFF, 
    //                                     0xFF, 0xFF, 0x61, 0x37, 0xFF, 0xFF, 0xFF, 0xFF, 
    //                                     0x27, 0x4E, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0x55, 
    //                                     0xFF, 0xFF, 0xFF, 0xFF, 0x3B, 0x29, 0x59, 0xFF, 
    //                                     0xFF, 0xFF, 0xFF, 0x62, 0xFF, 0xFF, 0xFF, 0xFF, 
    //                                     0x59, 0x28, 0x39, 0x62, 0xFF, 0xFF, 0xFF, 0xFF, 
    //                                     0x64, 0xFF, 0xFF, 0xFF, 0x64, 0x4A, 0x1D, 0x4D, 
    //                                     0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0xFF, 0xFF, 0xFF, 
    //                                     0xFF, 0x53, 0x16, 0x30, 0xFF, 0xFF, 0xFF, 0xFF};
  for(int i=0; i<36; ++i){
    input_spike = 0;
    input_spike |= (input[i*4] & 0xFF) << 24;
    input_spike |= (input[i*4+1] & 0xFF) << 16;
    input_spike |= (input[i*4+2] & 0xFF) << 8;
    input_spike |= (input[i*4+3] & 0xFF);
    input_spike_addr = i;
    tinyODIN_spike_core_write(&tinyODIN, input_spike_addr, input_spike);
  }
  input_spike = 0xFFFFFFFF;
  for(int i=36; i<64; ++i){
      input_spike_addr = i;
      tinyODIN_spike_core_write(&tinyODIN, input_spike_addr, input_spike);
  }
}

void __attribute__ ((noinline)) tinyODIN_synaptic_init (const tinyODIN_t tinyODIN) {
  uint32_t synapse_wdata;
  uint32_t synapse_weight_addr;
  synapse_wdata = 0x00000000;
  for(int i=0; i<8192; ++i){
      synapse_weight_addr = i;
      tinyODIN_synaptic_core_write(&tinyODIN, synapse_weight_addr, synapse_wdata);
  }
}

void __attribute__ ((noinline)) tinyODIN_synaptic_l1_init (const tinyODIN_t tinyODIN) {

  uint32_t synapse_l1[144][13] = {
  {0x1ceeef40, 0x1204cc12, 0xf0330c22, 0x14f02020, 0x2fd224e2, 0xdcf12ce1, 0x24fed120, 0xf10f0011, 0xfd2e3ce0, 0x22310dc2, 0x10211322, 0x313100f0, 0x0000f203},
  {0xfe1f5f5e, 0x1604fe10, 0xe2101e11, 0xe44e2101, 0x01c226e1, 0xbe0111ef, 0x1fff011e, 0xdef0e012, 0x05102e00, 0x113d5b01, 0x1e000201, 0x6e2f0ef3, 0x000002f0},
  {0x0dfee01f, 0x2105d21f, 0xe410ed11, 0xf1212df0, 0xfdb12741, 0xeb2012f0, 0x1ff1d21e, 0xede2f313, 0xf40f333c, 0x11234e10, 0xf00f0f11, 0x3d0e0f04, 0x0000c122},
  {0x0efcff21, 0x3616f100, 0xd1f12201, 0x00121d1f, 0xe29f47ff, 0x1e2103f0, 0x10f1f110, 0xe0f10301, 0x130b213d, 0x01f32ce0, 0x0e1f1d00, 0x2bfeffd4, 0x0000b120},
  {0xdf000023, 0x15040e0f, 0xb0e11500, 0x120101fd, 0xe1d06700, 0x1f0104d0, 0x11f2ef12, 0xe0e10211, 0xd310111d, 0x00ff1f10, 0x0000fe11, 0x2bf0def2, 0x0000b21b},
  {0x1002fb24, 0x00200000, 0xd0012400, 0x2111e00e, 0xe11f26f2, 0x100002e0, 0x0e1f0e02, 0x1fd11001, 0xe2031210, 0x01f10310, 0x001e0f00, 0x2e0ef0e0, 0x0000f14d},
  {0x1f12df26, 0x1f101f11, 0xee211101, 0x2320d10f, 0x001f32e3, 0xf21004f1, 0x0d1d0e00, 0x20c11002, 0x0213130f, 0x00e30511, 0x010f0000, 0x2e1d01ff, 0x0000002f},
  {0x0f22df31, 0x0eff0f02, 0xed211101, 0x2230e2f2, 0x002051f2, 0x00111001, 0x0f1eff01, 0xe0c12214, 0x13021300, 0x0002e3f0, 0x011f0101, 0x1e100111, 0x0000f311},
  {0x1e01ef30, 0x1ef01e03, 0xef1e1000, 0x11200003, 0xff1141f0, 0x02200ee1, 0x0f301f00, 0x1fd14407, 0xe200031f, 0x0001c210, 0x0e000000, 0xfd20e012, 0x0000e4e2},
  {0x0fffbf5f, 0x2d031012, 0xee1c0000, 0x06211e1b, 0x01f031e0, 0x02110ae2, 0x010e1000, 0xe0e165f7, 0x211e24f0, 0x0020d200, 0xff1f01f1, 0x1f21eff1, 0x0000c3e2},
  {0xf01ee030, 0x2f02ff11, 0xdf3e0011, 0x05ff1f2f, 0x101d4100, 0xd2101ce3, 0x012f1410, 0x0201f117, 0x111f1010, 0x1102e021, 0xe0101010, 0x0f2101d1, 0x0000d1f4},
  {0xf00c2f42, 0x11f20311, 0x212ef011, 0x03a12100, 0x10213ff1, 0xd1f21dd6, 0x202f0322, 0x0032e211, 0x1d1e1f4e, 0x11030111, 0xdf101e11, 0x11010f22, 0x00000215},
  {0x1ffb1e30, 0x25e2ff11, 0xef62de11, 0x323c1ef3, 0x10d112d1, 0xbcef2fe0, 0x1de2f21e, 0xea0fdf10, 0x0f1f3ff1, 0x11130001, 0xdd100411, 0x3c2b2d1f, 0x0000c302},
  {0x034cee4f, 0x2705001f, 0xc1400e00, 0x12591224, 0xf3ee0312, 0xdef1002e, 0x01df121f, 0xed01e303, 0x070e0100, 0x00130210, 0x0e0e0100, 0x0f000dff, 0x0000f201},
  {0x223ded11, 0x2515f000, 0xd1201000, 0x141031f5, 0xf40ff401, 0x1e10001e, 0xf201010f, 0xcf100405, 0x270ef010, 0x0001e210, 0x2fff0fff, 0x1f1f1e21, 0x00002402},
  {0x211f0b01, 0x11170f00, 0xb1f02700, 0x2412fff3, 0xd1f016f3, 0x1000001e, 0x010f0f00, 0xef3203f6, 0xd20d1011, 0xf0e0f110, 0xf00f0f00, 0x00120f00, 0x00000400},
  {0x1320eb13, 0x00141100, 0xa1e107f0, 0x2402d001, 0xe1f103f1, 0x00100f0e, 0x01001e02, 0xf0220304, 0xe10e1111, 0x00ef1f20, 0x00f00000, 0xf0e100e0, 0x000004fd},
  {0x02121d12, 0x1f011100, 0xa0100200, 0x1202c0f2, 0xd1e11201, 0x01110f1f, 0x01111e01, 0x3f020004, 0xe0001201, 0x00f02d4f, 0x0201f100, 0x01f000e0, 0x0000231e},
  {0xf0010223, 0x10102001, 0xa100f000, 0x0111d0f5, 0xc1001201, 0xf111011f, 0x01310d02, 0x10f10d07, 0xf002f101, 0x0f0f0f20, 0x02010200, 0x11000000, 0x00000130},
  {0x0ff0e212, 0x02201002, 0x90100100, 0x00110005, 0xe0021201, 0xff10012e, 0x10301f01, 0x00e22e07, 0x01010011, 0x00111010, 0xf20001f0, 0xf0110011, 0x00000142},
  {0x1f01e301, 0xe0011e01, 0xb100f1f0, 0x011200f4, 0xe0122210, 0xf0100020, 0x000f1001, 0xf0b14107, 0x00100e00, 0x0002e200, 0xf20000f0, 0x00110004, 0x00000022},
  {0x3010f1f2, 0xfd000f02, 0xb10f0100, 0x02110f02, 0xe211022f, 0xf2000d01, 0x000f00f2, 0x11c171f4, 0x0f0f00f1, 0x00f1f120, 0x0f00f000, 0x003211f4, 0x0000f202},
  {0x212fe1f6, 0x0c2f2e03, 0xb11e0000, 0x03d02d01, 0xd431f3ef, 0xe4100d13, 0x0f0d0002, 0x00c07103, 0xfe000f1f, 0x0002d310, 0x1f020000, 0x000211e0, 0x000012f2},
  {0x103db001, 0x20e1e102, 0xb11cd011, 0x02c01f00, 0xf506f2fe, 0xe5e01c16, 0x02f01113, 0xceff3012, 0x100e4f3b, 0x1112c211, 0xef120110, 0x0201000e, 0x000001d0},
  {0x213c0f40, 0x1203d01e, 0x0111ea11, 0x036a2004, 0xf0e016e1, 0x9de112de, 0x01ef110f, 0xed0fa003, 0x1d1f0eef, 0x00211ff1, 0xfb0f0200, 0x0103fe0d, 0x0000f1e2},
  {0x122d1e20, 0x2507d00f, 0x0e000f00, 0xf56c1202, 0xf3fef202, 0xff010ffe, 0x022d121d, 0xc012f104, 0xd30fef0e, 0x00100100, 0x2d00ff00, 0x0f01fff0, 0x000003f2},
  {0x212ffd21, 0x11150f00, 0x00001400, 0x052d11f1, 0x10f1f2e0, 0x000f0f0d, 0x022ef20d, 0xa020f106, 0x140eef0f, 0x00eef000, 0x11000000, 0xf112103f, 0x00000401},
  {0x000ffb01, 0xf0130000, 0x00e0f700, 0x04ff0004, 0xfff212e1, 0x0100000d, 0x012e12ff, 0xc1210102, 0xe20f000f, 0x00d0f21f, 0xf10f0100, 0xe2020f11, 0x000003ee},
  {0xf10f1c11, 0xe002110d, 0x01e1f400, 0x0000e013, 0xe002120d, 0xf2000d1e, 0x02002100, 0x20200101, 0xd10f021f, 0x00f0f000, 0xe2f1f100, 0xe211f0df, 0x000002fc},
  {0x01ef2e1f, 0x00f1100e, 0x0f01d000, 0x1f02cf22, 0xe001221e, 0xf20f0f10, 0x01e22100, 0x3f11100f, 0xe20f0110, 0x00f1fa00, 0xf1000100, 0x0120f1ef, 0x000001ff},
  {0x00d0400f, 0x2ff0210e, 0xf010d10f, 0x0e10bf04, 0xb0111100, 0xf0000f00, 0x01012111, 0xdf111f0f, 0x00f20010, 0x00000e00, 0xf0010100, 0x0f2ff01f, 0x0000ee01},
  {0x0eef20e0, 0xe1011200, 0xf101e100, 0x0f0f00f2, 0xc0010f04, 0xf0000210, 0x017f1ff1, 0xe101000f, 0x00001e12, 0x00f03100, 0x11010100, 0xf0100030, 0x0000f022},
  {0x0e0f00f2, 0xc201e10f, 0x1021e100, 0x01f000f1, 0xe1150010, 0x00f0f010, 0x012fff00, 0x11c021f0, 0xf00e0d01, 0x000122ff, 0x2101010f, 0xff030012, 0x0000ff52},
  {0x2f00fff1, 0xf000f010, 0x1131e100, 0x13f0ff12, 0xe114101e, 0x01ff001f, 0x02ee1e01, 0x20f0700f, 0x0f0dfe00, 0x0f10f110, 0x4100f1f0, 0xff122004, 0x0000f110},
  {0x313ef203, 0xef100003, 0x0300f1f0, 0x11e0f012, 0xc213011d, 0x14f01e11, 0x01ed1f02, 0x01e07000, 0x0e00fe0f, 0x001fe220, 0x61021000, 0xf00f10e0, 0x000000f0},
  {0x014df111, 0x000ff213, 0xe11d0201, 0x1fc0f0ff, 0xde26f1fd, 0x14e00c13, 0x01ff0005, 0xee0e7211, 0x1f0e3e1c, 0x1021f011, 0x5e100301, 0x24f10efd, 0x000003de},
  {0x22fd0e01, 0x0106f110, 0x120f0a01, 0x147f0113, 0x0ed114e0, 0xbdf013de, 0x10fe020c, 0xeef0be03, 0xdf0d0b10, 0x003f0100, 0x0b0e0101, 0x0403ed1a, 0x0000f2e2},
  {0x120d1d0f, 0x12061200, 0x13f01100, 0x245f12ff, 0x00ff1110, 0xf10100dd, 0x010df70e, 0xe2f1ff04, 0xf40e101f, 0x001021f0, 0x1d001010, 0xf412f0ef, 0x0000f200},
  {0x0fe00d0f, 0xf00411f0, 0x00e013f0, 0x031ef0f0, 0x10e01f00, 0x1100f0ff, 0xf10ee6fe, 0xa2ff0ff1, 0xf50feff1, 0x00ee0f0f, 0x01000100, 0xf3320040, 0x000001f0},
  {0xff000d00, 0xe11301ff, 0x11e01400, 0x030ee203, 0x10020f1f, 0xf1f0f00f, 0x010f2700, 0xe2011fff, 0xc0f00201, 0x0000f100, 0x02000100, 0xe032f031, 0x000011ff},
  {0x1fff1d31, 0xe002100f, 0x12e10200, 0x00fee210, 0x1202000f, 0x01000000, 0xf102f7f1, 0x2211110f, 0xef00f110, 0x0fe10000, 0x1f000000, 0xe022f0ff, 0x00001f0e},
  {0x12e04f20, 0x2f11100e, 0x1250f001, 0x1ff1d101, 0x001f000f, 0x13000e00, 0x00e0f700, 0x1f00000e, 0x0f0ff001, 0x00f11c00, 0x2f100000, 0xe1100fff, 0x00000fff},
  {0x03e241e1, 0x2f11100e, 0x0320e000, 0x0010d115, 0xf01c1ff0, 0x21f00fe0, 0x0f2f1300, 0xde1fdf0d, 0x1f02e001, 0x00311f00, 0x30010000, 0xe2102f20, 0x0000f0e1},
  {0x0f1111e1, 0xd203020e, 0xf101f000, 0x0e1f2101, 0xf0fe1df6, 0x2ef0030f, 0xff700000, 0xf020c30d, 0x01000011, 0x001011ff, 0x50010f00, 0xe2011011, 0x000001e2},
  {0xfe1000f0, 0xf3f2f10f, 0x00110100, 0x0e101012, 0xe4f71df0, 0x2ce0010f, 0x01f00f00, 0x1100e20f, 0x010e0f01, 0x002120f0, 0x600100ff, 0xf1f0f000, 0x00001f2f},
  {0x1f101f0e, 0xf2f1010f, 0x03401000, 0x00f01000, 0xe3070f1d, 0x1ee0000e, 0x01df0eff, 0x00df510f, 0xf00f0f0e, 0x0020101f, 0x51000000, 0x001f0004, 0x0000f03e},
  {0x311020e1, 0xf3200101, 0x05302100, 0x01f0001f, 0x02150f1d, 0x10e00e11, 0x00ed0e00, 0xf0f0710f, 0x0001ff10, 0x001e0110, 0x61020200, 0x021f1100, 0x0000001e},
  {0xf0201200, 0x04f00201, 0x072d3210, 0x0fe12fe1, 0xfe13ff0e, 0x21e10b02, 0x0ff01101, 0xdfef7010, 0xe00f0ffe, 0x0040f011, 0x5c0203f1, 0x650fe02d, 0x0000f2ee},
  {0x22ee320e, 0xc3272010, 0x220d2c11, 0x025022f0, 0xe1d22232, 0xdee012ff, 0x01ff2f1f, 0xeffbc211, 0x12101f14, 0x012f0f01, 0xe01d0201, 0xf73fee3e, 0x0000f0f2},
  {0x11fe1b00, 0xe0f30100, 0x15101001, 0x113f01ff, 0x02022f22, 0x022000e2, 0x120e050f, 0xc2020d02, 0x030f2000, 0x000e4011, 0xef0f0100, 0xf731f100, 0x00000210},
  {0x2fdf0f12, 0xf1001e00, 0x07e00100, 0x020fe1f0, 0x01021c40, 0x11000004, 0x00101300, 0xd1f312ff, 0xf101e002, 0xfffe0110, 0x10000100, 0x01100041, 0x000003f1},
  {0x2f000e22, 0xf0100f01, 0x06c110f0, 0x11eef10f, 0x1f120d22, 0x00f00f10, 0x01011301, 0xe0011200, 0xbd010001, 0x00ff0200, 0x10000100, 0x0f1f0130, 0x000011f0},
  {0x001f0022, 0x0f110001, 0x24e01e00, 0x1ee0f2f0, 0x101fce13, 0xf1f00e00, 0x02e0ff02, 0x30ff1f01, 0xfd0f1f01, 0x00f11110, 0x2f000000, 0x3000302f, 0x00003000},
  {0xf321f0e1, 0x4f211000, 0x315f2f00, 0x0fe2e102, 0x240deff1, 0x01e00dd1, 0x0fe00f01, 0x2e0d0d00, 0x3e10eff0, 0x000f0c10, 0x10000100, 0x70204ff0, 0x000011f0},
  {0x0221f1e3, 0x13220f0f, 0x302f3f00, 0xf3112ff3, 0x131b1e11, 0x0fd000df, 0x00513011, 0xf01dff0e, 0x100300f0, 0x002d0f10, 0xe10e0f00, 0x20204f10, 0x000002f0},
  {0x0f21f102, 0xd312000f, 0x1fef3f00, 0x01304100, 0x14110ef2, 0xfee0040d, 0x0242210f, 0x1110e200, 0xe10f0201, 0x00eee1e0, 0xf00e0e00, 0x0111300f, 0x000030ff},
  {0x0f100000, 0xf4101f0f, 0x11ff3f00, 0x10401000, 0x03070f11, 0xfcff001e, 0x01c010ff, 0x01f1f000, 0xf10e1214, 0x0fe00000, 0xf0000000, 0x1000f11f, 0x00002ffc},
  {0x41101e1f, 0xf7fe1f0f, 0x15311e00, 0xe011111e, 0x03060010, 0x0af00f11, 0x01ee0e0d, 0x00c0100f, 0xf000110f, 0xf0fe1020, 0x02f10000, 0x111ff003, 0x00000f2c},
  {0x40011f11, 0xe7002e01, 0x16330d0f, 0x02ff202e, 0x1017f01f, 0x0b100f13, 0x11fe0f09, 0x11d0720f, 0xe0021100, 0x002d1010, 0x02020210, 0x222011f1, 0x0000112c},
  {0x11f2f015, 0xf703010f, 0x025f2011, 0xe0005002, 0x4cf5effe, 0x4c110d02, 0x0fef110d, 0xf000710d, 0xf00f11ff, 0x10500010, 0x0f040510, 0x4310010f, 0x0000e0ed},
  {0x33ff3fe2, 0xd5070f10, 0x10111c11, 0x313112df, 0x0eb12c21, 0x00e1100f, 0x121f1e1f, 0xe0fd0101, 0x12110f17, 0x10111f31, 0xc0100101, 0x372f0f50, 0x000011f2},
  {0x30f01f14, 0xf1010001, 0x13200210, 0x1000f0f0, 0x11e41d23, 0x11000ef7, 0x0010f000, 0xef131e01, 0xe10f10e2, 0x110f2000, 0x0f0f0110, 0x462ee032, 0x00002500},
  {0x10f00011, 0x00ff0001, 0xf5f1f100, 0x00ef000f, 0x00140d12, 0x10e00107, 0x01010f0d, 0x1e010100, 0xee0100e1, 0x001e0100, 0x10110000, 0x7120ff20, 0x000022f1},
  {0x11010f02, 0x0f1fff03, 0xf1e11000, 0x01f011fe, 0x021fdd03, 0x01000f22, 0xf1f11e0f, 0xfd0d0001, 0xed0021e1, 0x001001f0, 0x10020f01, 0x71ff3010, 0x00001000},
  {0x1130f000, 0x0ff00104, 0x0df1ff01, 0x0e111fe0, 0x1000cee2, 0x11000f32, 0x00f01d01, 0x0e0c1f02, 0x200f3fe0, 0x0002f100, 0x110f0000, 0x71f041f0, 0x00001101},
  {0x0010f2ee, 0x120e1102, 0x2f2ff000, 0xff000bf2, 0x22ff21f1, 0x21f00f1f, 0x1f312001, 0x111e1e11, 0x310040df, 0x000f0f10, 0x010e0100, 0x4d101fdf, 0x000001ff},
  {0x0ff10100, 0x023eff00, 0x001e0000, 0xf50f2ef2, 0x23f04121, 0x1ff0010e, 0x01301101, 0x040f0000, 0xf10131ef, 0x00fd10f0, 0xff0d0f00, 0x0c011f0f, 0x000000ff},
  {0x1ff01100, 0xf011fe00, 0x0e0de00f, 0x0000f0f1, 0x01003104, 0xf0f0020d, 0x020100f0, 0x120f00f1, 0xff0021f0, 0x00def000, 0xef0c0f0f, 0x1f007f1e, 0x000030f0},
  {0x2e100010, 0x002e0f00, 0x03eff000, 0x1020d0f0, 0x0f021011, 0xdf000f00, 0x00ff11f0, 0x121000f0, 0x00fe2114, 0xffb11020, 0xf00e0ff0, 0x011f002f, 0x0000100f},
  {0x31100010, 0xe10f1f00, 0x07111e00, 0xf210ef2f, 0xff11011f, 0xff000d14, 0x010dff0f, 0x02e101f0, 0x1ff01100, 0x00ae0020, 0xf2ff1000, 0x1230ff02, 0x0000121f},
  {0x31f10f30, 0x07023d00, 0x1323fc00, 0xe40f0021, 0x1ee1e00e, 0x0f110f04, 0x1f2edf0b, 0x33f11101, 0xde021210, 0x0fce0010, 0x00010200, 0x00100f12, 0x0000210f},
  {0x2ff1ef04, 0xd7172101, 0x1162ff00, 0xe2e23113, 0x3fe20001, 0x2f301f03, 0x00ffe20d, 0x34141f01, 0xed1fe1ef, 0x000f0f10, 0x0e120301, 0x0f1f0111, 0x0000f1f0},
  {0x12f02e03, 0xf7e70010, 0x0f30e011, 0x3f2015d1, 0xef804e02, 0x11e01e12, 0x1210111f, 0xbf1e011f, 0xf31e22e6, 0x11210011, 0xe1120000, 0x72110e11, 0x000023d2},
  {0x42100f33, 0x17f0ff00, 0x01400410, 0x1f011200, 0x1012f0ff, 0x0ef10f24, 0x001e1e03, 0x00f41e0f, 0xeefe30e2, 0x003f12f0, 0x00020f00, 0x75f10010, 0x00002200},
  {0x0121ff13, 0x21010e01, 0x0e32f000, 0x0011f11e, 0xf0f2f1d1, 0x00000027, 0x0000ff0d, 0x1ef30ef1, 0xee0f50f0, 0x001311f0, 0x21030ff0, 0x710001f2, 0x00002210},
  {0x00210124, 0x1f1f0ff2, 0x0ef31f00, 0xd0010f2e, 0x022ef0df, 0x10100035, 0x0101010c, 0xf0010d00, 0x100f31e1, 0x10110100, 0x2002ff00, 0x42f00ff0, 0x00001311},
  {0x1020f10f, 0xffee2102, 0x1d11ee0f, 0xdff00b10, 0x110ef1c1, 0x10100054, 0xfe3020fe, 0x02110e01, 0x11ff0fe0, 0x00f00110, 0x100f000f, 0xf0001ff0, 0x00000201},
  {0x101001ee, 0xf2fbf101, 0x002ef100, 0xe2000c02, 0x12f14100, 0x10100121, 0x0e30f1ff, 0x04111001, 0x110010ee, 0x000f2000, 0x1f0c0100, 0xfeff00f0, 0x0000010f},
  {0x00001ffe, 0x010eef01, 0xf02d0000, 0xf3ff0f14, 0x00f21142, 0xf020f01f, 0x0f2fe20f, 0x03100101, 0x010121fe, 0x000d20f0, 0x0f0b0100, 0xffff3f00, 0x00000e00},
  {0x0f1001f1, 0x0000fef2, 0xfe0ee000, 0xfe00f001, 0x00f110f3, 0xe000001e, 0x0101f10f, 0x12000002, 0x10fe1ff1, 0x000e0120, 0x000b0ff0, 0x11117f10, 0x00002001},
  {0x2020f012, 0x0e01f0f2, 0x150fe100, 0x0f30d0ff, 0x0f0130f0, 0xe100ff01, 0x011f2100, 0x011000f0, 0x200d0ff2, 0xffe1003f, 0x120f0f0f, 0x0210ffff, 0x00002201},
  {0x101fe120, 0xfe12f001, 0x141100f0, 0x0211bff1, 0x0f1f20ff, 0xf11000f3, 0x002d01f1, 0x00f001f2, 0x210f0100, 0x00bff01f, 0x00010f00, 0xf2111f02, 0x00002501},
  {0x1f0fef30, 0x12051e02, 0x21131c00, 0xe401dfd1, 0x31dff0ee, 0x12f001f2, 0x012e0100, 0x301401f2, 0xfe01ffd0, 0x0080eff0, 0x0e010000, 0xdff11f02, 0x000033f1},
  {0x222fde05, 0x16051003, 0x04411301, 0x01030ff2, 0x01d0e2d4, 0x111f1fe0, 0x0f12e40d, 0x2026fe01, 0xee0ed0be, 0x01d22fe1, 0x0f000e00, 0x0d1f2011, 0x0000e4e1},
  {0x020e2ff1, 0x0614f11f, 0xde2ff510, 0x424123d5, 0x00e022c0, 0x101111f0, 0x0f204010, 0xee100d0f, 0xf40051f4, 0x101402e0, 0xdf021b10, 0x3fff1f1e, 0x00000101},
  {0x511fe025, 0x1702f000, 0xf2410300, 0x7f4111f1, 0x0105118e, 0xf0000111, 0xf01e1f01, 0xf1002e1e, 0x020d4e01, 0x002600f0, 0xef020ff0, 0x00e11c00, 0x00002d11},
  {0x30310f03, 0x12101e00, 0x0f220e00, 0x7e21ef2f, 0x010012ae, 0x0f10ff2e, 0x02fff0ff, 0x12110f00, 0x010f6f0f, 0x00060100, 0x10010ff0, 0x0ff10ef2, 0x00001ff1},
  {0x00100112, 0x0f104001, 0x00011d00, 0x40d0fc41, 0x021d11d1, 0x0e100010, 0x020fe10c, 0x12e20ff0, 0x1f0e1100, 0x0f002000, 0x20f10f00, 0x03110ef1, 0x00000211},
  {0xf0011011, 0xfeee3101, 0x0f0ffe00, 0x10df0a52, 0x111d3102, 0x0e201f14, 0x0040030c, 0x0302f000, 0x0e0fef0f, 0x00e00020, 0x100e0100, 0x1301ef0f, 0x0000f111},
  {0x31000100, 0x01edee02, 0xf0fd0101, 0x02df1e44, 0x1ff14031, 0xfe300220, 0x0030f21e, 0x130101f0, 0x0f1f000f, 0x000f0010, 0x1f0d0100, 0x02efd100, 0x0000f2ff},
  {0x1130fff0, 0x22e0edf1, 0x0f1e0000, 0xfff20125, 0x0f012f11, 0x0f20000f, 0x011fe000, 0x02010001, 0xf0f0f0ef, 0x001f01f0, 0x1f0d0000, 0x02f050f0, 0x00001100},
  {0x003100e2, 0x00e0f001, 0x0200020f, 0x0e20010f, 0x020f2ff0, 0xf0100010, 0x0011f000, 0x111000f0, 0x100d10e1, 0xf0f10000, 0x00fc0f00, 0x20f14000, 0x00003101},
  {0xf021f013, 0xee03f001, 0x14010100, 0x1010e100, 0x12f020ff, 0x0110ff11, 0x000e2000, 0x1f1f0f00, 0x32fe10c1, 0x00020f0f, 0x010f0f01, 0x0f11f0f0, 0x000022f1},
  {0xf000f01f, 0x11140001, 0x1221000f, 0xf200d0f0, 0x100f00e1, 0xf1200fef, 0x001d1101, 0x2f110d01, 0x1100f0b0, 0x00e1e0e0, 0x0f020f00, 0xfff100f0, 0x000023d1},
  {0x110e0f0e, 0x01012000, 0x03122100, 0x03020fe1, 0x1fede1e2, 0x0110f01f, 0x0110010f, 0x1f251d01, 0xd101019f, 0x00b3e0ff, 0x0e010d0f, 0x0e002f11, 0x00000401},
  {0x211f0fe7, 0xf3010f10, 0x13e01200, 0x0012f1b1, 0x1ed0f2e5, 0x20310010, 0x0002e300, 0x2027fe00, 0xf11dd0ad, 0x10d41110, 0xfd0f1d00, 0x2f0e2fd1, 0x0000d5e2},
  {0x130ef0f0, 0xd106e302, 0x1e3ef300, 0x31700fd4, 0x00f404d1, 0x011f02ff, 0x0f114101, 0xffe14d10, 0xd50e5d00, 0x001514e0, 0x0e010e11, 0x1f0f001e, 0x0000f2f1},
  {0x312e1e02, 0xf313f500, 0x104ff100, 0x6041f0b1, 0x12f421b0, 0x0000020d, 0x0efe1102, 0x21d030ff, 0x030f3ff1, 0x0024e1f0, 0xf0020100, 0xf0f11f0f, 0x0000ff01},
  {0x30210f02, 0xe21f120f, 0x11300f00, 0x7d020ca4, 0x0001f2ce, 0x1f10f10c, 0xfe001101, 0x00002f0f, 0x02ff4e0e, 0xf014f010, 0x0f020000, 0xfff1ede1, 0x00001f0f},
  {0x10f00f21, 0xe21f3101, 0x12210f00, 0x70e01cd4, 0x11100211, 0x0f10010f, 0x1f100ef1, 0x01111000, 0x0000000f, 0x00013110, 0xfff10f01, 0x0100dbe2, 0x00000000},
  {0x11e0f212, 0xd01e1f02, 0x00f00e00, 0x7fdd3e01, 0x110f1234, 0xee100011, 0x0f200f0f, 0x1201f1f0, 0x0f0fe020, 0x00f10f00, 0xe20f0f00, 0x22fed1f0, 0x0000f100},
  {0x4101f102, 0x050fde02, 0xffdf0100, 0x4fef2203, 0x0ff12122, 0xff000102, 0x0111f00f, 0x11f0f000, 0xff01f120, 0x0020f000, 0xe10d0e00, 0x021f1301, 0x0000021f},
  {0x31110fe0, 0x210f0e00, 0xf20f0100, 0x2f21f102, 0x0f001110, 0x11100f00, 0x0010e001, 0x0000fd01, 0x0000f1f1, 0x002100f0, 0xf00e0d00, 0x01214000, 0x00001010},
  {0x10111ff2, 0xffe0f101, 0x06100300, 0x2020010f, 0xf0f01100, 0x10200101, 0x0f00f000, 0x1f100e01, 0x120e01e0, 0xf00310e0, 0x0f0f0e00, 0x1e0f0000, 0x00000f00},
  {0x00f10f23, 0xf002e000, 0x13100200, 0x1010f01f, 0x01f0f101, 0x003f01f0, 0xfe0e2000, 0x1ff00fff, 0x020120e0, 0x0004fff0, 0x00000f0f, 0x1e1fe1f0, 0x000001f1},
  {0xe1ff012f, 0x1010020f, 0x11111100, 0x131fdef1, 0xe000d101, 0x0130001f, 0x0fee0101, 0x0f04ff00, 0xf10120d0, 0x00f2f0f0, 0x10f10f00, 0xfdfff0f0, 0x0000e401},
  {0x02001f0d, 0x110d220f, 0x1201220f, 0x010011e2, 0xefffd203, 0x01200001, 0x0fff11fd, 0x002710f0, 0xd3f1100d, 0x00e2111f, 0x0e000100, 0x0d2e0001, 0x0000d701},
  {0x221f10f4, 0x03fef21f, 0x20ff2400, 0xd0ff11ef, 0xfffe1205, 0x2f21010f, 0x0102e30f, 0x21171000, 0xf40fe01c, 0x00f60321, 0xef001e00, 0x112ffff3, 0x0000e3f0},
  {0x130d011f, 0xc1160700, 0x301c1200, 0x126f1f04, 0x1f1545d3, 0xf20f01cf, 0xfe21240f, 0xde02200e, 0xe60e0141, 0x11f3f3e1, 0x20020f00, 0x1d1e0e10, 0x0000f300},
  {0x111e1000, 0xe2f4070d, 0xfe5011f0, 0xf0211cf4, 0x01142fdf, 0x121f01ff, 0x0ff00100, 0x1ff0310e, 0xe20f1f2f, 0x0021e100, 0x020003f0, 0xf00f0000, 0x00000221},
  {0x3f000fff, 0xf4f2170d, 0xf230f100, 0xf0000db7, 0x1f22100f, 0x001ff1ee, 0xfe1f0201, 0xff010000, 0x01f10f1e, 0x00f10100, 0xf0000200, 0x0f2efce0, 0x00000010},
  {0x30f1fe0f, 0xe300130f, 0xf301020f, 0xf10d1fa5, 0xff112330, 0x100f01ff, 0x0ef1f002, 0x0e110001, 0x0100ff1f, 0x00002f01, 0xe1f10100, 0x0f1dedf1, 0x00000f10},
  {0xf0ffe001, 0xe4120000, 0x12010000, 0xf0fd21a4, 0xe0021312, 0x00f000e1, 0x0ff000f1, 0x1e111101, 0x0000e021, 0x0f010ff0, 0xe1010100, 0x0f0e020f, 0x00000001},
  {0x1feff1f1, 0xf411f1f1, 0x00f00100, 0x010001d4, 0xeff113f0, 0x10f000f2, 0x0f00f0f1, 0xfd0f0ef1, 0x10f2ff11, 0x00100f0f, 0xf100000f, 0xf0102210, 0x0000100f},
  {0x20ff0ff0, 0x1e00f100, 0xf1001100, 0x1311e1ff, 0x0ff11200, 0x100f00f1, 0x0f21f101, 0x0e1f0cf1, 0x00000020, 0x0f1010f0, 0x00f10110, 0xff11f010, 0x00000ff0},
  {0x10ff0f13, 0x0e0fe3f1, 0x0100120f, 0x2211d00e, 0xf001f111, 0xf1ff01f1, 0xfe000100, 0x0f0f0ef1, 0x00000020, 0x0012f0f0, 0x1f000100, 0x0f1f0110, 0x0000fe01},
  {0x1fff0e31, 0x100ff200, 0x0110020f, 0x1120df1e, 0xf110e011, 0xf11001ef, 0x0fff12f0, 0x0ff3ff00, 0xf0f2102f, 0x0001f1ff, 0x2eff0200, 0x0d0ee1f0, 0x0000e201},
  {0x10f0111d, 0x102d2500, 0xf11f0f00, 0x130fef10, 0xf000e011, 0x02000100, 0x0e00f0f0, 0xe0f71f01, 0xf1012020, 0x0ff102f0, 0x2f000200, 0x0e0ef01f, 0x0000d700},
  {0x00f0320f, 0x0ffd16ff, 0x02010100, 0xf01e2011, 0xe0d0e033, 0x122f0f2f, 0xf101f10e, 0xeff7210f, 0xe402102f, 0xf0e13410, 0x2c0002f0, 0xee3ee12f, 0x0000d711},
  {0x40e27204, 0x010de30c, 0xff00e100, 0x0cdc2001, 0x0eef0022, 0x2f1101f0, 0x1130d61f, 0x0fe6111e, 0xf210107f, 0xf0d2432f, 0x1d0e0f00, 0x1f3ddf31, 0x0000e301},
  {0xf1fbff1f, 0xf314271f, 0xd17f1301, 0xf45f0f2d, 0xe2153415, 0xf1010fff, 0x021f2400, 0xdcf2e00e, 0xe71e2d32, 0x11d302d0, 0x30120110, 0x4d0ee002, 0x0000c21f},
  {0xf10f010e, 0x02f5060a, 0x1f3f1000, 0xd01d3e3f, 0xf4150201, 0xf200021f, 0x0010130f, 0xfef1220f, 0x120f2021, 0x002fd2e0, 0x30010100, 0x0f3def11, 0x0000e42f},
  {0x10f11fff, 0x03f5241e, 0xf1300100, 0xe01e2010, 0xff25101e, 0x0f0002fe, 0x00fff201, 0xf00012f2, 0x11011021, 0xf001f100, 0x30000200, 0x004d0c00, 0x0000f310},
  {0x22f01fff, 0xe1f2131d, 0x11111000, 0xe11df100, 0xff230e10, 0x010001f0, 0x0f00f100, 0x0f011102, 0x11011011, 0x00f11f00, 0x1ff00300, 0x0f3e2ef0, 0x00000010},
  {0x11f000ff, 0x0102240f, 0x01111000, 0xf10e0100, 0x01201e12, 0x010000f1, 0x000fef00, 0x1f001102, 0x11010111, 0x00100f10, 0x20000400, 0x100f100f, 0x0000f010},
  {0x10ff01ff, 0xff02120f, 0x00010f00, 0x03100002, 0xf1111f01, 0x01ff0101, 0x001f0000, 0x100f2e01, 0x00000010, 0x00100f0f, 0x20000400, 0x1011f01f, 0x0000ff00},
  {0x10f00020, 0x0f11010f, 0x00000f00, 0x0211e110, 0x01110f11, 0x00d00001, 0x0f101f01, 0x100f1c01, 0x1001ff10, 0x0010f1f0, 0x3000040f, 0x0000010f, 0x0000f0f1},
  {0x21f00020, 0x1f00f20f, 0x01001ff0, 0x1211e00e, 0x1111ff21, 0x00d00000, 0x01201001, 0x1ff20e01, 0x00001f2f, 0x0f10f100, 0x2e000410, 0x011f0210, 0x0000f202},
  {0x10f11f11, 0x102e0100, 0x01101100, 0x1301f00f, 0x0020ef12, 0x00e0001f, 0x0f001ff0, 0x10f70f01, 0x0001202f, 0x0f01f110, 0x1e000400, 0x102df120, 0x0000e703},
  {0x0f112200, 0x100e0100, 0xf00f0100, 0x12f1fe02, 0xf301f021, 0x10f0012f, 0x0f00e200, 0x01f71f11, 0x0001313f, 0x00020110, 0x2f00f300, 0x011df210, 0x00000701},
  {0x1ee123e1, 0x1110e20f, 0xd0e00000, 0x1ef1fe21, 0x0302ff20, 0x21fff04f, 0x0001e30e, 0xff17110f, 0x23031150, 0x00e14151, 0x3d030100, 0x01fce23f, 0x000016f1},
  {0x2eee4f23, 0xc4fcee1d, 0xf212b011, 0x0bee1e04, 0x02c4fe2f, 0x121000f1, 0x1f12c41e, 0xce11211e, 0xd61d3072, 0x11013231, 0x5a1f1f10, 0xe15ef16e, 0x000016f1},
  {0x282b1100, 0x02f3b51c, 0xc1103011, 0xd12e112e, 0x001416e6, 0xe20110e3, 0x120f702d, 0x0c010121, 0xd61f3135, 0x11d4d1c2, 0x102e0011, 0xf22fc131, 0x0000d02d},
  {0x0d0c32e0, 0xe703e418, 0x104f0111, 0xd23a1030, 0xe12405b3, 0xd221001e, 0x140f201e, 0xefdef301, 0xf31f11f5, 0x0002f2e0, 0x35131e00, 0xf04dbe71, 0x0000131c},
  {0xf01150d1, 0x24231100, 0x032e2200, 0xd22af120, 0xf21711f5, 0xe001021f, 0x130d12f2, 0xeffe1100, 0x020221f3, 0x0101f2d0, 0x50021f00, 0xf15c2d52, 0x0000032c},
  {0xf1f35100, 0x12021000, 0x213f1110, 0xf21d0131, 0x10e71ff1, 0x02000020, 0x03012201, 0x1f0f110f, 0xf00211f1, 0x00010100, 0x20011f10, 0x00303e51, 0x0000f21e},
  {0x01034201, 0x1012110f, 0x203f2100, 0x0210f023, 0x011300f1, 0x01100111, 0xf32f2101, 0x10ee010f, 0xf0001101, 0xf0120010, 0x40000000, 0x01112f20, 0x0000f001},
  {0x00f23111, 0x0f111f0e, 0x10202010, 0x11200022, 0x0111f102, 0x11d00221, 0x02102001, 0x210f1d0f, 0xf01f0110, 0x00020020, 0x50010100, 0x02f21020, 0x0000f000},
  {0x00032121, 0xe011000e, 0x1f1e3f00, 0x1011f101, 0x1110f102, 0x11e10121, 0x03011100, 0x111e1c0f, 0xf00ff000, 0x00f1f010, 0x30000000, 0x02f1102f, 0x0000f310},
  {0xf0e12134, 0x1f11001f, 0x112f3000, 0x1112f101, 0x0020f001, 0x1fc11140, 0x0501020f, 0x00041e00, 0xe00f0110, 0x10100010, 0x1f1e0100, 0x01004130, 0x00000301},
  {0x01f23f14, 0x2000ff1f, 0x123e1200, 0x2012f211, 0x0111fe01, 0x2fb0004f, 0x0301f300, 0x01170e00, 0x1e101211, 0x00210010, 0x1e0e0f10, 0x221f2161, 0x00000502},
  {0xf1013f01, 0x1eefce0f, 0xf11e1210, 0x1e131012, 0xff200f13, 0x0fb11150, 0x0132031e, 0x0007190e, 0x10113320, 0x00f22031, 0xfe1c1e01, 0x342e0f30, 0x00002420},
  {0xdf100213, 0x02d1d01f, 0xd32de101, 0xef323021, 0x12061e22, 0x02f01031, 0x11111111, 0xf0f32a1d, 0x22054332, 0x11f32f51, 0x2d1b1e11, 0x235cf12f, 0x00002420},
  {0xc8fb2034, 0xc4eff01d, 0xc3f0c512, 0x1ecf1f01, 0x1fc60fe1, 0x14002f22, 0x1020e41d, 0x0d0f7e1e, 0xe51c4042, 0x11f63f12, 0x3c111f21, 0xf4ee1e20, 0x000002f0},
  };

  for(int i=0; i<144; ++i){
    for(int j=0; j<13; ++j){
      tinyODIN_synaptic_core_write(&tinyODIN, i*32+j+18, synapse_l1[i][j]);
    }
  }

}

void __attribute__ ((noinline)) tinyODIN_synaptic_l2_init (const tinyODIN_t tinyODIN) {
  uint32_t synapse_l2[201] = { 0x1d000000, 0xeeeeeed1, 0xf3000000, 0x10f52f0d, 0x73000000, 0x30121201, 0xff000000, 0x503e0011, 0x32000000, 0x00f11012, 0x00000000, 0xf01003f3, 0x32000000, 0x001f0242, 0xde000000, 0x0c1ee0bd, 0xf0000000, 0x00f30300, 0x75000000, 0x66555566, 0xe5000000, 0xf0f14e43, 0x04000000, 0xf2110102, 0xf1000000, 0xff5e002f, 0xf0000000, 0x0001213f, 0x0e000000, 0xe3eee103, 0x7f000000, 0x01210421, 0x67000000, 0x67677776, 0x66000000, 0x65565676, 0xde000000, 0xf0003051, 0x01000000, 0xff333f1f, 0x86000000, 0x10020ff0, 0x2d000000, 0xff1fef4f, 0xd0000000, 0xe2ff1efc, 0xe2000000, 0x114e2f22, 0xd3000000, 0xfeaeeddf, 0xec000000, 0x1f0e206f, 0x3c000000, 0xf0500f06, 0x72000000, 0x1f137220, 0x5e000000, 0x1f3f14f2, 0x30000000, 0x0f1ff050, 0x5f000000, 0xe010e02e, 0xf1000000, 0xe203ded2, 0xad000000, 0xef001efc, 0x6f000000, 0x01200113, 0x00000000, 0xe0161d11, 0x1f000000, 0x21c1e1e3, 0xe2000000, 0x01fd5fff, 0x11000000, 0x00011f11, 0xcd000000, 0xcc0bee10, 0x04000000, 0x11404e26, 0xe3000000, 0x00051fff, 0x1e000000, 0x210f1300, 0x41000000, 0x002ff032, 0x66000000, 0x66577766, 0x65000000, 0x66665665, 0x0f000000, 0x001201f6, 0x4f000000, 0x10f15e00, 0x01000000, 0x100250f0, 0x2e000000, 0x30fe5f1d, 0x65000000, 0x66667777, 0x0e000000, 0xf1e203ce, 0xe1000000, 0x212f214f, 0x33000000, 0x30110112, 0x0f000000, 0x20ce2bf0, 0xe0000000, 0x203002ff, 0x57000000, 0x76555665, 0x10000000, 0xf0f3f4ff, 0x54000000, 0x55557564, 0x7e000000, 0x207002ef, 0x0d000000, 0x010071ef, 0x1d000000, 0x00e0f0f7, 0x12000000, 0x111f0023, 0x10000000, 0x44020110, 0x21000000, 0x01022400, 0x13000000, 0x3f2f0011, 0x1f000000, 0xeef00c07, 0x21000000, 0x20051f01, 0x0e000000, 0x20e0013d, 0x70000000, 0x2f4f0203, 0x65000000, 0x57666576, 0x1e000000, 0x100f0f41, 0x25000000, 0x00053210, 0x55000000, 0x57545567, 0x00000000, 0x121f0201, 0x5f000000, 0xf1f015f2, 0xf1000000, 0x21111f12, 0x23000000, 0x0e0e1f02, 0x4d000000, 0x00136f00, 0x65000000, 0x65577677, 0x66000000, 0x65576666, 0x67000000, 0x67766766, 0x55000000, 0x56545576, 0x11000000, 0x05d02f20, 0x46000000, 0x76566776, 0x87000000, 0x0f2fd1de, 0x66000000, 0x75566667, 0x01000000, 0x10200011, 0xf2000000, 0x002dc6ff, 0x14000000, 0x21f3ff12, 0x20000000, 0x401f0f0f, 0x05000000, 0xf4131f02, 0xff000000, 0xd2f10b30, 0x1f000000, 0x1f5f0210, 0xff000000, 0xee2edeec, 0xf2000000, 0xf06011f0, 0xff000000, 0x44c10fdd, 0x2f000000, 0xf015231f, 0x12000000, 0x200fed24, 0xfd000000, 0xeeefd0df, 0x12000000, 0x40410411 };

  uint32_t synapse_wdata_l2;
  int synapse_weight_addr_l2;

    for(int i=0; i<100; ++i){
      for(int j=0; j<2; ++j){
        synapse_weight_addr_l2 = i*32 + j +4638;
        synapse_wdata_l2 = synapse_l2[i*2 + j];
        tinyODIN_synaptic_core_write(&tinyODIN, synapse_weight_addr_l2, synapse_wdata_l2);
      }
    } 
}