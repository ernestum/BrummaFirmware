#include "Shiftregister.h"
#include "Motorinterface.h"
#include "MotorPlacement.h"
#include "MotorIntensityinterface.h"
#include <ArduinoBLE.h>

const auto enable_pin = 10;
//const auto num_motors = 5; // PT1
const auto num_motors = 4; // PT2

auto motor_interface = Motorinterface<num_motors>(enable_pin);
//auto motor_placement = MotorPlacement<num_motors> {{{45}, {90}, {180}, {360 - 90}, {360 - 45}}}; // PT1
auto motor_placement = MotorPlacement<num_motors> {{
    {360 - 90}, {360 - 45}, {45}, {90}
  }
}; // PT2
auto motor_intensities = MotorintensityInterface<num_motors>(motor_interface);

BLEService directionService("22d40000-458f-44cc-b199-2d6ae6c69984");
BLEBoolCharacteristic enableCharacteristic("22d40001-458f-44cc-b199-2d6ae6c69984", BLERead | BLEWrite);
BLEByteCharacteristic targetYawCharacteristic("22d40002-458f-44cc-b199-2d6ae6c69984", BLERead | BLEWrite);


void setup(void) {
  pinMode(enable_pin, OUTPUT);
  digitalWrite(enable_pin, LOW);

  Serial.begin(115200);
  //  while (!Serial); // Wait un  til serial device is up
  Serial.print("==== Welcome to Brumma! ====\n\n");
  printHelp();
  Serial.setTimeout(1000 * 60 * 60); // 1h timeout

  SPI.begin(); // TODO: move this somewhere else
  motor_interface.allOff();
  digitalWrite(enable_pin, HIGH);

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
  } else {
    BLE.setLocalName("BrummaBelt");
    BLE.setAdvertisedService(directionService);
    directionService.addCharacteristic(enableCharacteristic);
    directionService.addCharacteristic(targetYawCharacteristic);
    BLE.addService(directionService);
    BLE.advertise();
  }
}

void loop(void) {
  handleSerialInterface();

  motor_intensities.spin();
  motor_interface.spin();


  BLE.poll();
  if (targetYawCharacteristic.written() and enableCharacteristic.value()) {
    applyTargetYawCharacteristic();
  }

  if (enableCharacteristic.written() ) {
    if (enableCharacteristic.value() ) {
      applyTargetYawCharacteristic();
    } else {
      motor_intensities.allOff();
    }
  }
}

void applyTargetYawCharacteristic() {
  auto newTargetAngle = (targetYawCharacteristic.value() / 255.) * 360;
  //    auto motor = motor_placement.getMotorForAngle(newTargetAngle);
  for (size_t i = 0; i < num_motors; ++i) {
    auto angular_distance = motor_placement.angularDistanceToMotor(i, newTargetAngle);
    auto intensity = constrain(map(angular_distance, 0, 180, 255, -255), 0, 255);
    Serial.print(angular_distance); Serial.print("\t");
    Serial.print(map(angular_distance, 0, 180, 255, -255)); Serial.print("\t");
    Serial.println(intensity);
    motor_intensities.set(i, intensity);
  }
  Serial.println();
}



void handleSerialInterface() {
  if (Serial.available()) {
    Serial.print("Waiting for command ...>");
    String command = Serial.readStringUntil('\n');
    Serial.println(command);

    if (command.startsWith("ON")) {
      motor_intensities.allOn();
    } else if (command.startsWith("OFF")) {
      motor_intensities.allOff();
    }
    else if (command.startsWith("h") or command.startsWith("H")) {
      printHelp();
    }
    else if (command.startsWith("p")) {
      auto power = command.substring(1).toFloat();
      Serial.print("Setting power to ");
      Serial.println(power);
      motor_interface.setPower(map(power, 0, 100, 0, 255));
    }
    else if (command.length() != num_motors) {
      Serial.print("Please provide command with length equal to the number of motors (");
      Serial.print(num_motors);
      Serial.println(")!");
      Serial.print("You command had length ");
      Serial.println(command.length());
    } else {
      for (size_t i = 0; i < command.length(); i++) {
        Serial.print("Set motor "); Serial.println(i);
        motor_intensities.set(i, command.charAt(i) == '0' ? 0 : 255);
      }
      Serial.println("Updated Motors");
    }
  }
}

void printHelp() {
  Serial.println(
    "Commands:\n"
    "'h' or 'H'\tdisplay this help text\n"
    "'p<number>'\tset the power of the motors between 0 and 100\n"
    "0010010\tor any variation thereof enables/disables the motors. The length has to match the number of motors\n"
    "'ON'\tturn all motors on\n"
    "'OFF'\tturn all motors off\n"
  );
}
