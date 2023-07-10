
module pre_load_bram (clk_i, addr, dout);
input clk_i;
input [31:0] addr;
output [7:0] dout;

   xpm_memory_sprom #(
      .ADDR_WIDTH_A(32),              // DECIMAL
      .AUTO_SLEEP_TIME(0),           // DECIMAL
      .CASCADE_HEIGHT(0),            // DECIMAL
      .ECC_MODE("no_ecc"),           // String
      .MEMORY_INIT_FILE("app.mem"),     // String
      .MEMORY_INIT_PARAM("0"),       // String
      .MEMORY_OPTIMIZATION("true"),  // String
      .MEMORY_PRIMITIVE("block"),     // String
      .MEMORY_SIZE(100000),            // DECIMAL
      .MESSAGE_CONTROL(0),           // DECIMAL
      .READ_DATA_WIDTH_A(8),        // DECIMAL
      .READ_LATENCY_A(1),            // DECIMAL
      .READ_RESET_VALUE_A("0"),      // String
      .RST_MODE_A("SYNC"),           // String
      .SIM_ASSERT_CHK(0),            // DECIMAL; 0=disable simulation messages, 1=enable simulation messages
      .USE_MEM_INIT(1),              // DECIMAL
      .USE_MEM_INIT_MMI(0),          // DECIMAL
      .WAKEUP_TIME("disable_sleep")  // String
   )
   xpm_memory_sprom_inst (
      .dbiterra(),             // 1-bit output: Leave open.
      .douta(dout),                   // READ_DATA_WIDTH_A-bit output: Data output for port A read operations.
      .sbiterra(),             // 1-bit output: Leave open.
      .addra(addr),                   // ADDR_WIDTH_A-bit input: Address for port A read operations.
      .clka(clk_i),                     // 1-bit input: Clock signal for port A.
      .ena(1'b1),                       // 1-bit input: Memory enable signal for port A. Must be high on clock
                                       // cycles when read operations are initiated. Pipelined internally.

      .injectdbiterra(1'b0), // 1-bit input: Do not change from the provided value.
      .injectsbiterra(1'b0), // 1-bit input: Do not change from the provided value.
      .regcea(1'b1),                 // 1-bit input: Do not change from the provided value.
      .rsta(1'b0),                     // 1-bit input: Reset signal for the final port A output register stage.
                                       // Synchronously resets output port douta to the value specified by
                                       // parameter READ_RESET_VALUE_A.

      .sleep(1'b0)                    // 1-bit input: sleep signal to enable the dynamic power saving feature.
   );

endmodule
