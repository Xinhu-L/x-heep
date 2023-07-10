module xilinx_core_v_mini_mcu_top
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter COREV_PULP           = 0,
    parameter FPU                  = 0,
    parameter ZFINX                = 0,
    parameter X_EXT                = 0,  // eXtension interface in cv32e40x
    parameter CLK_LED_COUNT_LENGTH = 27
)(

    inout logic clk_i,
    inout logic rst_i,

    //visibility signals
    output logic rst_led,
    output logic clk_led,
    output logic clk_out,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic jtag_tck_i,
    inout logic jtag_tms_i,
    inout logic jtag_trst_ni,
    inout logic jtag_tdi_i,
    inout logic jtag_tdo_o,

    inout logic uart_rx_i,
    inout logic uart_tx_o,
  
    // inout logic [19:0] gpio_io,

    output logic exit_value_o,
    inout  logic exit_valid_o,

    inout logic [3:0] spi_flash_sd_io,
    inout logic spi_flash_csb_o,
    inout logic spi_flash_sck_o,

    inout logic [3:0] spi_sd_io,
    inout logic spi_csb_o,
    inout logic spi_sck_o,

    inout logic spi2_sd_0_io,
    inout logic spi2_sd_1_io,
    inout logic spi2_sd_2_io,
    inout logic spi2_sd_3_io,
    inout logic [1:0] spi2_csb_o,
    inout logic spi2_sck_o,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io,

    inout logic pdm2pcm_clk_io,
    inout logic pdm2pcm_pdm_io,

    inout logic i2s_sck_io,
    inout logic i2s_ws_io,
    inout logic i2s_sd_io

);


  logic  [                      24:0] gpio_io;
  logic  [                      24:0] gpio_io_o;
  assign gpio_io[24] = gpio_io_o[17:0];

  xilinx_core_v_mini_mcu_wrapper #(
      .X_EXT(X_EXT),
      .COREV_PULP(COREV_PULP),
      .FPU(FPU),
      .ZFINX(ZFINX)
  ) xilinx_core_v_mini_mcu_wrapper_i (
      .clk_i,
      .rst_i,
      .rst_led,
      .clk_led,
      .clk_out,
      .boot_select_i,
      .execute_from_flash_i,
      .jtag_tck_i,
      .jtag_tms_i,
      .jtag_trst_ni,
      .jtag_tdi_i,
      .jtag_tdo_o,
      .uart_rx_i,
      .uart_tx_o,
      .gpio_io(gpio_io[17:0]),
      .exit_value_o,
      .exit_valid_o,
      .spi_flash_sd_io,
      .spi_flash_csb_o,
      .spi_flash_sck_o,
      .spi_sd_io,
      .spi_csb_o,
      .spi_sck_o,
      .spi2_sd_0_io(gpio_io[26]),
      .spi2_sd_1_io(gpio_io[27]),
      .spi2_sd_2_io(gpio_io[28]),
      .spi2_sd_3_io(gpio_io[29]),
      .spi2_csb_o(gpio_io[24:23]),
      .spi2_sck_o(gpio_io[25]),
      .i2c_scl_io(gpio_io[31]),
      .i2c_sda_io(gpio_io[30]),
      .pdm2pcm_clk_io(gpio_io[19]),
      .pdm2pcm_pdm_io(gpio_io[18]),
      .i2s_sck_io(gpio_io[20]),
      .i2s_ws_io(gpio_io[21]),
      .i2s_sd_io(gpio_io[22])
  );

  Xheep ps_i (
    .DDR_addr(),
    .DDR_ba(),
    .DDR_cas_n(),
    .DDR_ck_n(),
    .DDR_ck_p(),
    .DDR_cke(),
    .DDR_cs_n(),
    .DDR_dm(),
    .DDR_dq(),
    .DDR_dqs_n(),
    .DDR_dqs_p(),
    .DDR_odt(),
    .DDR_ras_n(),
    .DDR_reset_n(),
    .DDR_we_n(),
    .FIXED_IO_ddr_vrn(),
    .FIXED_IO_ddr_vrp(),
    .FIXED_IO_mio(),
    .FIXED_IO_ps_clk(),
    .FIXED_IO_ps_porb(),
    .FIXED_IO_ps_srstb(),
    .GPIO_O(gpio_io_o)
  );


endmodule
