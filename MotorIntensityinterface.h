#pragma once
#include "Motorinterface.h"

template<size_t NUM_MOTORS>
class MotorintensityInterface {
  public:
    MotorintensityInterface(Motorinterface<NUM_MOTORS>& motors) : motors_(motors) {
      for (size_t i = 0; i < NUM_MOTORS; ++i)
        intensities_[i] = 0;
    }

    void set(size_t idx, uint8_t intensity) {
      if (idx < NUM_MOTORS)
        intensities_[idx] = intensity;
    }

    void allOn() {
      for (size_t i = 0; i < NUM_MOTORS; i++) intensities_[i] = 255;
    }


    void allOff() {
      for (size_t i = 0; i < NUM_MOTORS; i++) intensities_[i] = 0;
    }

    void spin() {
      // TODO: these updates should be synced to the updates of the motor interface
      const uint8_t levels = 5; // number of possible intensity levels. More levels = more shaky output

      //    Serial.println("INT begin");
      auto current_level = ctr_ % levels;

      for (size_t i = 0; i < NUM_MOTORS; ++i) {
        motors_.set(i, current_level < (levels * (intensities_[i] / 255.)));
      }
      //    Serial.println("INT end");
      ctr_++;
    }

  private:
    Motorinterface<NUM_MOTORS>& motors_;
    uint8_t intensities_[NUM_MOTORS];
    size_t ctr_;
};
