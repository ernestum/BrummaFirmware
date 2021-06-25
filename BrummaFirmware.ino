#include "Shiftregister.h"
#include "Motorinterface.h"


const auto enable_pin = 17;
const auto num_motors = 9;


void setup(void) {
  pinMode(2, OUTPUT);
  Serial.begin(115200);


  auto motor_interface = Motorinterface<num_motors>(enable_pin);
}

void loop(void) {
  
}
