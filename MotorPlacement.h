#pragma once

#include "Motorinterface.h"

template<class T>
T angularDistance(T a1, T a2) {
  auto dist = abs(a1 - a2);
  return dist > 180 ? 360 - dist : dist;
}

template<class T>
T normalizeAngle(T a) {
  if (a >= 360) return normalizeAngle(a - 360.);
  if (a < 0) return normalizeAngle(a + 360);
  return a;
}



struct MotorInfo {
  float angle = 0.0;
  bool activated = false;

  String toString() {
    return "{" + String(angle) + ", " + (activated ? "true" : "false") + "}";
  }
};

template<size_t NUM_MOTORS>
class MotorPlacement {
  public:
    size_t getMotorForAngle(float angle) {
      auto min_elem = std::min_element(std::begin(motors_), std::end(motors_),
      [angle = normalizeAngle(angle)](MotorInfo lhs, MotorInfo rhs) {
        auto dlhs = lhs.activated ? angularDistance(lhs.angle, angle) : std::numeric_limits<float>::max();
        auto drhs = rhs.activated ? angularDistance(rhs.angle, angle) : std::numeric_limits<float>::max();
        return dlhs < drhs;
      } );
      return std::distance(std::begin(motors_), min_elem);
    }

    bool isOppositeOf(float angle, size_t motor) {
      if (motor >= NUM_MOTORS or not motors_[motor].activated) return false;
      return angularDistance(normalizeAngle(angle), motors_[motor].angle) > 180;
    }

    String toString() {
      String s = "{";
      for(size_t m = 0; m < NUM_MOTORS; m++) {
        s += motors_[m].toString() + (m < NUM_MOTORS-1 ? ", " : "}");
      }
      return s;
    }

    MotorInfo motors_[NUM_MOTORS];

};

template<size_t CHAIN_LENGTH>
class MotorPlacementCalibrator {
  public:
    MotorPlacementCalibrator(Motorinterface<CHAIN_LENGTH>& motors, MotorPlacement<CHAIN_LENGTH * 8>& motor_placement) :
      motors_(motors), motor_placement_(motor_placement)
    {}

    void setReferenceAngle(float angle) {
      reference_angle_ = normalizeAngle(angle);
    }

    void startCalibration(size_t motor) {
      motors_.allOff();
      motors_.set(motor, true);
      if (motors_.getPower() <= 255 * 0.3) motors_.setPower((uint8_t) (255 * 0.4));
      current_motor_ = motor;
    }

    void stopCalibration(float current_angle) {
      if (isCalibrating()) {
        motor_placement_.motors_[current_motor_].angle = normalizeAngle(reference_angle_ - current_angle);
        motor_placement_.motors_[current_motor_].activated = true;
        current_motor_ = CHAIN_LENGTH * 8;
        Serial.println(motor_placement_.toString());
      }
    }

    void abortCalibration() {
      current_motor_ = CHAIN_LENGTH * 8;
    }

    bool isCalibrating() {
      return current_motor_ < CHAIN_LENGTH * 8;
    }

    size_t getCurrentMotor() {
      return current_motor_;
    }

    float getReferenceAngle() {
      return reference_angle_;
    }

  private:
    float reference_angle_ = 0;
    Motorinterface<CHAIN_LENGTH> & motors_;
    MotorPlacement<CHAIN_LENGTH * 8> & motor_placement_;
    size_t current_motor_ = CHAIN_LENGTH * 8;
};
