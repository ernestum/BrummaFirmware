#pragma once

#include "Shiftregister.h"

template<size_t NUM_MOTORS>
class Motorinterface {
    constexpr static auto NUM_BYTES = (int)ceil(NUM_MOTORS / 8.);
  public:
    Motorinterface(int8_t power_pin) : power_pin_(power_pin) {
      pinMode(power_pin, OUTPUT);
      setPower(0);
    }

    void allOn() {
      for (size_t i = 0; i < NUM_BYTES; i++) motor_states_[i] = 0b11111111;
    }


    void allOff() {
      for (size_t i = 0; i < NUM_BYTES; i++) motor_states_[i] = 0;
    }

    void set(size_t idx, boolean enabled) {
      if (idx >= size()) return;
      size_t register_id = idx / 8;
      uint8_t bit_id = idx % 8;

      motor_states_[register_id] ^= (-enabled ^ motor_states_[register_id]) & (1UL << bit_id);
    }

    bool get(size_t idx) const {
      if (idx >= size()) return false;
      size_t register_id = idx / 8;
      uint8_t bit_id = idx % 8;

      return (motor_states_[register_id] >> bit_id) & 0x1;
    }

    void setPower(uint8_t power) {
      analogWrite(power_pin_, power);
      current_power_ = power;
    }

    uint8_t getPower() const {
      return current_power_;
    }

    size_t size() const {
      return NUM_MOTORS;
    }

    void spin() {
      if (millis() - last_shiftregister_update_ > MAX_UPDATE_PERIOD) {
        shri_.send(motor_states_);
        last_shiftregister_update_ = millis();
      }
    }
    
  private:
    Shiftregister<NUM_BYTES> shri_;
    int8_t power_pin_;
    byte motor_states_[NUM_BYTES];
    uint8_t current_power_;
    unsigned long last_shiftregister_update_ = 0;
    static const auto MAX_UPDATE_PERIOD = 2;
};
