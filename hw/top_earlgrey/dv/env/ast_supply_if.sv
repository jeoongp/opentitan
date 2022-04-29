// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// This interface is used to force the voltage supply indicators, to trigger resets due to
// power-not-ok conditions.
//
// The glitches of interest are on the vcaon_supp_i and vcmain_supp_i AST inputs.
// Forcing the values to 0 causes the AST to deassert the corresponding pok output: vcaon should
// cause a POR reset, and vcmain a non-aon reset.
//
// We need to disable some pwrmgr design assertions when wiggling vcmain_supp_i.
interface ast_supply_if (
  input logic clk,
  input logic trigger
);
  import uvm_pkg::*;

  // The amount of time to hold the glitch. Should be enough to span more than one aon_clk cycles
  // so it is sampled.
  localparam time GlitchTimeSpan = 10us;
  localparam int GlitchCycles = 2;
  localparam int PulseCycles = 2;

  // The number of cycles after stopping the glitch in vcmain before restarting
  // assertions in pwrmgr.
  localparam int CyclesBeforeReenablingAssert = 7;

  function static void force_vcaon_pok(bit value);
    force u_ast.u_rglts_pdm_3p3v.vcaon_pok_h_o = value;
  endfunction

  // Create glitch in vcaon_pok_h_o some cycles after this is invoked. Hold vcaon_pok_h_o low for
  // a fixed time: we cannot use clock cycles because the clock will stop.
  task automatic glitch_vcaon_pok(int cycles);
    repeat (cycles) @(posedge clk);
    `uvm_info("ast_supply_if", "forcing vcaon_pok_h_o=0", UVM_MEDIUM)
    force_vcaon_pok(1'b0);
    #(GlitchTimeSpan);
    `uvm_info("ast_supply_if", "forcing vcaon_pok_h_o=1", UVM_MEDIUM)
    force_vcaon_pok(1'b1);
  endtask

  // Wait some clock cycles due to various flops in the logic.
  task automatic reenable_vcmain_assertion();
    repeat (CyclesBeforeReenablingAssert) @(posedge clk);
    `uvm_info("ast_supply_if", "re-enabling vcmain_supp_i related SVA", UVM_MEDIUM)
    $asserton(1, top_earlgrey.u_pwrmgr_aon.u_slow_fsm.IntRstReq_A);
  endtask

  task static force_vcmain_pok(bit value);
    `uvm_info("ast_supply_if", $sformatf("forcing vcmain_pok_h_o to %b", value), UVM_MEDIUM)
    if (!value) begin
      `uvm_info("ast_supply_if", "disabling vcmain_supp_i related SVA", UVM_MEDIUM)
      $assertoff(1, top_earlgrey.u_pwrmgr_aon.u_slow_fsm.IntRstReq_A);
    end
    force u_ast.u_rglts_pdm_3p3v.vcmain_pok_h_o = value;
    if (value) reenable_vcmain_assertion();
  endtask

  // Create glitch in vcmain_pok_h_o some cycles after a trigger transitions high.
  task automatic glitch_vcmain_pok_on_next_trigger(int cycles);
    @(posedge trigger);
    repeat (cycles) @(posedge clk);
    force_vcmain_pok(1'b0);
    repeat (GlitchCycles) @(posedge clk);
    force_vcmain_pok(1'b1);
  endtask

  // Create pulse in aon_sysrst_ctrl_rst_req_o some cycles after a trigger transitions high.
  task automatic pulse_aon_sysrst_ctrl_rst_req_o_next_trigger(int cycles);
//    @(posedge trigger);
    wait(top_earlgrey.u_pwrmgr_aon.reg2hw.reset_en[0] == 1'b1);
    `uvm_info("ast_supply_if", $sformatf("sysrst configured, cycles : %0d",cycles), UVM_MEDIUM)
    repeat (cycles) @(posedge clk);
    force_syrst_ctrl_req_o(1'b1);
    `uvm_info("ast_supply_if", "sysrst ctrl output high", UVM_MEDIUM)
    repeat (PulseCycles) @(posedge clk);
    force_syrst_ctrl_req_o(1'b0);
    `uvm_info("ast_supply_if", "sysrst ctrl output low", UVM_MEDIUM)
  endtask

  task static force_syrst_ctrl_req_o(bit value);
    `uvm_info("ast_supply_if", $sformatf("forcing syrst_ctrl_req_o to %b", value), UVM_MEDIUM)
    force top_earlgrey.u_sysrst_ctrl_aon.aon_sysrst_ctrl_rst_req_o = value;
  endtask
endinterface
