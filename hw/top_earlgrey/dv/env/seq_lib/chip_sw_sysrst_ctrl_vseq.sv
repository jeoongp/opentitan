// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

class chip_sw_sysrst_ctrl_vseq extends chip_sw_base_vseq;
  `uvm_object_utils(chip_sw_sysrst_ctrl_vseq)

  `uvm_object_new

  rand int cycles_after_trigger;

  constraint cycles_after_trigger_c {cycles_after_trigger inside {[3 : 9]};}

  virtual task body();
    super.body();
    // Wait until we reach the SW test state.
    wait(cfg.sw_test_status_vif.sw_test_status == SwTestStatusInTest);
    `uvm_info(`gfn, "SW test ready", UVM_LOW)

    cfg.ast_supply_vif.pulse_aon_sysrst_ctrl_rst_req_o_next_trigger(cycles_after_trigger);
  endtask

endclass
