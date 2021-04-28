#pragma once

#include "Shiftregister.h"

template<size_t CHAIN_LENGTH>
class Motorinterface {
  public:
    Motorinterface(int8_t power_pin, Shiftregister<CHAIN_LENGTH> shri) : shri_(std::move(shri)) {
      ledcAttachPin(power_pin, 1);
      ledcSetup(1, 12000, 8);

      setPower(0);
    }

    void allOn() {
      for (size_t i = 0; i < CHAIN_LENGTH; i++) motor_states_[i] = 0b11111111;
      updateShiftRegister();
    }


    void allOff() {
      for (size_t i = 0; i < CHAIN_LENGTH; i++) motor_states_[i] = 0;
      updateShiftRegister();
    }

    void set(size_t idx, boolean enabled) {
      size_t register_id = idx / 8;
      if (register_id >= CHAIN_LENGTH) return;
      uint8_t bit_id = idx % 8;

      motor_states_[register_id] ^= (-enabled ^ motor_states_[register_id]) & (1UL << bit_id);
      updateShiftRegister();
    }

    bool get(size_t idx) const {
      size_t register_id = idx / 8;
      if (register_id >= CHAIN_LENGTH) return false;
      uint8_t bit_id = idx % 8;

      return (motor_states_[register_id] >> bit_id) & 0x1;
    }

    void setPower(uint8_t power) {
      ledcWrite(1, 255 - power);
      current_power_ = power;
    }

    uint8_t getPower() const {
      return current_power_;
    }

    size_t size() const {
      return CHAIN_LENGTH * 8;
    }

  private:
    Shiftregister<CHAIN_LENGTH> shri_;
    byte motor_states_[CHAIN_LENGTH];
    uint8_t current_power_;

    void updateShiftRegister() {
      shri_.send(motor_states_);
    }
};
