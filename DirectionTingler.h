#pragma once

#include "MotorPlacement.h"
#include "Motorinterface.h"

template<size_t CHAIN_LENGTH>
class DirectionTingler {
  public:
    DirectionTingler(Motorinterface<CHAIN_LENGTH>& motors,
                     MotorPlacement<CHAIN_LENGTH * 8>& placement) : motors_(motors), placement_(placement) {}
    Motorinterface<CHAIN_LENGTH>& motors_;
    MotorPlacement<CHAIN_LENGTH * 8>& placement_;

    void doTingle() {
      if(is_tingling) {
        size_t m = placement_.getMotorForAngle(normalizeAngle(target_heading_ - current_heading_));
        motors_.allOff();
        motors_.set(m, true);
      }
    }

    float current_heading_ = -1;
    float target_heading_ = -1;

    bool is_tingling = false;
};
