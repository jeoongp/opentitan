// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/lib/dif/dif_sysrst_ctrl.h"

#include <assert.h>

#include "sw/device/lib/base/bitfield.h"
#include "sw/device/lib/base/macros.h"
#include "sw/device/lib/dif/dif_base.h"

#include "sysrst_ctrl_regs.h"  // Generated.

dif_result_t dif_sysrst_ctrl_key_combo_detect_configure(
    const dif_sysrst_ctrl_t *sysrst_ctrl, dif_sysrst_ctrl_key_combo_t key_combo,
    dif_sysrst_ctrl_key_combo_config_t config) {
  if (sysrst_ctrl == NULL || config.keys > kDifSysrstCtrlKeyAll ||
      config.actions > kDifSysrstCtrlKeyComboActionAll) {
    return kDifBadArg;
  }

  ptrdiff_t combo_select_ctl_reg_offset;
  ptrdiff_t combo_detect_ctl_reg_offset;
  ptrdiff_t combo_action_ctl_reg_offset;

  switch (key_combo) {
    case kDifSysrstCtrlKeyCombo0:
      combo_select_ctl_reg_offset = SYSRST_CTRL_COM_SEL_CTL_0_REG_OFFSET;
      combo_detect_ctl_reg_offset = SYSRST_CTRL_COM_DET_CTL_0_REG_OFFSET;
      combo_action_ctl_reg_offset = SYSRST_CTRL_COM_OUT_CTL_0_REG_OFFSET;
      break;
    case kDifSysrstCtrlKeyCombo1:
      combo_select_ctl_reg_offset = SYSRST_CTRL_COM_SEL_CTL_1_REG_OFFSET;
      combo_detect_ctl_reg_offset = SYSRST_CTRL_COM_DET_CTL_1_REG_OFFSET;
      combo_action_ctl_reg_offset = SYSRST_CTRL_COM_OUT_CTL_1_REG_OFFSET;
      break;
    case kDifSysrstCtrlKeyCombo2:
      combo_select_ctl_reg_offset = SYSRST_CTRL_COM_SEL_CTL_2_REG_OFFSET;
      combo_detect_ctl_reg_offset = SYSRST_CTRL_COM_DET_CTL_2_REG_OFFSET;
      combo_action_ctl_reg_offset = SYSRST_CTRL_COM_OUT_CTL_2_REG_OFFSET;
      break;
    case kDifSysrstCtrlKeyCombo3:
      combo_select_ctl_reg_offset = SYSRST_CTRL_COM_SEL_CTL_3_REG_OFFSET;
      combo_detect_ctl_reg_offset = SYSRST_CTRL_COM_DET_CTL_3_REG_OFFSET;
      combo_action_ctl_reg_offset = SYSRST_CTRL_COM_OUT_CTL_3_REG_OFFSET;
      break;
    default:
      return kDifBadArg;
  }

  if (!mmio_region_read32(sysrst_ctrl->base_addr,
                          SYSRST_CTRL_REGWEN_REG_OFFSET)) {
    return kDifLocked;
  }

  mmio_region_write32(sysrst_ctrl->base_addr, combo_select_ctl_reg_offset,
                      config.keys);
  mmio_region_write32(sysrst_ctrl->base_addr, combo_detect_ctl_reg_offset,
                      config.detection_time_threshold);
  mmio_region_write32(sysrst_ctrl->base_addr, combo_action_ctl_reg_offset,
                      config.actions);

  if (config.actions & kDifSysrstCtrlKeyComboActionEcReset) {
    mmio_region_write32(sysrst_ctrl->base_addr,
                        SYSRST_CTRL_EC_RST_CTL_REG_OFFSET,
                        config.embedded_controller_reset_duration);
  }

  return kDifOk;
}

dif_result_t dif_sysrst_ctrl_input_change_detect_configure(
    const dif_sysrst_ctrl_t *sysrst_ctrl,
    dif_sysrst_ctrl_input_change_config_t config) {
  if (sysrst_ctrl == NULL || config.input_changes & (1U << 7) ||
      config.input_changes > kDifSysrstCtrlInputAll) {
    return kDifBadArg;
  }

  if (!mmio_region_read32(sysrst_ctrl->base_addr,
                          SYSRST_CTRL_REGWEN_REG_OFFSET)) {
    return kDifLocked;
  }

  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_KEY_INTR_CTL_REG_OFFSET,
                      config.input_changes);
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_KEY_INTR_DEBOUNCE_CTL_REG_OFFSET,
                      config.debounce_time_threshold);

  return kDifOk;
}

dif_result_t dif_sysrst_ctrl_output_pin_override_configure(
    const dif_sysrst_ctrl_t *sysrst_ctrl, dif_sysrst_ctrl_pin_t output_pin,
    dif_sysrst_ctrl_pin_config_t config) {
  if (sysrst_ctrl == NULL ||
      (config.override_value == true && config.allow_one == false) ||
      (config.override_value == false && config.allow_zero == false) ||
      !dif_is_valid_toggle(config.enabled)) {
    return kDifBadArg;
  }

  uint32_t pin_out_ctl_bit_index;
  uint32_t pin_out_value_bit_index;
  uint32_t pin_out_allow_0_bit_index;
  uint32_t pin_out_allow_1_bit_index;

  switch (output_pin) {
    case kDifSysrstCtrlPinKey0Out:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_KEY0_OUT_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_KEY0_OUT_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_KEY0_OUT_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_KEY0_OUT_1_BIT;
      break;
    case kDifSysrstCtrlPinKey1Out:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_KEY1_OUT_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_KEY1_OUT_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_KEY1_OUT_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_KEY1_OUT_1_BIT;
      break;
    case kDifSysrstCtrlPinKey2Out:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_KEY2_OUT_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_KEY2_OUT_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_KEY2_OUT_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_KEY2_OUT_1_BIT;
      break;
    case kDifSysrstCtrlPinPowerButtonOut:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_PWRB_OUT_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_PWRB_OUT_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_PWRB_OUT_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_PWRB_OUT_1_BIT;
      break;
    case kDifSysrstCtrlPinBatteryDisableOut:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_BAT_DISABLE_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_BAT_DISABLE_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_BAT_DISABLE_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_BAT_DISABLE_1_BIT;
      break;
    case kDifSysrstCtrlPinZ3WakeupOut:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_Z3_WAKEUP_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_Z3_WAKEUP_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_Z3_WAKEUP_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_Z3_WAKEUP_1_BIT;
      break;
    case kDifSysrstCtrlPinEcResetInOut:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_EC_RST_L_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_EC_RST_L_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_EC_RST_L_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_EC_RST_L_1_BIT;
      break;
    case kDifSysrstCtrlPinFlashWriteProtectInOut:
      pin_out_ctl_bit_index = SYSRST_CTRL_PIN_OUT_CTL_FLASH_WP_L_BIT;
      pin_out_value_bit_index = SYSRST_CTRL_PIN_OUT_VALUE_FLASH_WP_L_BIT;
      pin_out_allow_0_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_FLASH_WP_L_0_BIT;
      pin_out_allow_1_bit_index = SYSRST_CTRL_PIN_ALLOWED_CTL_FLASH_WP_L_1_BIT;
      break;
    default:
      return kDifBadArg;
  }

  if (!mmio_region_read32(sysrst_ctrl->base_addr,
                          SYSRST_CTRL_REGWEN_REG_OFFSET)) {
    return kDifLocked;
  }

  // Configure output pin control register.
  uint32_t pin_out_ctl_reg = mmio_region_read32(
      sysrst_ctrl->base_addr, SYSRST_CTRL_PIN_OUT_CTL_REG_OFFSET);
  pin_out_ctl_reg = bitfield_bit32_write(pin_out_ctl_reg, pin_out_ctl_bit_index,
                                         dif_toggle_to_bool(config.enabled));
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_PIN_OUT_CTL_REG_OFFSET, pin_out_ctl_reg);

  // Configure output pin override value register.
  uint32_t pin_out_value_reg = mmio_region_read32(
      sysrst_ctrl->base_addr, SYSRST_CTRL_PIN_OUT_VALUE_REG_OFFSET);
  pin_out_value_reg = bitfield_bit32_write(
      pin_out_value_reg, pin_out_value_bit_index, config.override_value);
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_PIN_OUT_VALUE_REG_OFFSET, pin_out_value_reg);

  // Configure output pin allowed values register.
  uint32_t pin_out_allowed_values_reg = mmio_region_read32(
      sysrst_ctrl->base_addr, SYSRST_CTRL_PIN_ALLOWED_CTL_REG_OFFSET);
  pin_out_allowed_values_reg = bitfield_bit32_write(
      pin_out_allowed_values_reg, pin_out_allow_0_bit_index, config.allow_zero);
  pin_out_allowed_values_reg = bitfield_bit32_write(
      pin_out_allowed_values_reg, pin_out_allow_1_bit_index, config.allow_one);
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_PIN_ALLOWED_CTL_REG_OFFSET,
                      pin_out_allowed_values_reg);

  return kDifOk;
}

dif_result_t dif_sysrst_ctrl_ulp_wakeup_configure(
    const dif_sysrst_ctrl_t *sysrst_ctrl,
    dif_sysrst_ctrl_ulp_wakeup_config_t config) {
  if (sysrst_ctrl == NULL || !dif_is_valid_toggle(config.enabled)) {
    return kDifBadArg;
  }

  if (!mmio_region_read32(sysrst_ctrl->base_addr,
                          SYSRST_CTRL_REGWEN_REG_OFFSET)) {
    return kDifLocked;
  }

  mmio_region_write32(sysrst_ctrl->base_addr, SYSRST_CTRL_ULP_CTL_REG_OFFSET,
                      dif_toggle_to_bool(config.enabled));
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_ULP_AC_DEBOUNCE_CTL_REG_OFFSET,
                      config.ac_power_debounce_time_threshold);
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_ULP_LID_DEBOUNCE_CTL_REG_OFFSET,
                      config.lid_open_debounce_time_threshold);
  mmio_region_write32(sysrst_ctrl->base_addr,
                      SYSRST_CTRL_ULP_PWRB_DEBOUNCE_CTL_REG_OFFSET,
                      config.power_button_debounce_time_threshold);

  return kDifOk;
}