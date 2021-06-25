#include "Shiftregister.h"
#include "Motorinterface.h"


const auto enable_pin = 17;
const auto num_motors = 9;

auto motor_interface = Motorinterface<num_motors>(enable_pin);

void setup(void) {
  Serial.begin(115200);
  while(!Serial);  // Wait until serial device is up
  Serial.print("==== Welcome to Brumma! ====\n\n");
  printHelp();
  Serial.setTimeout(1000*60*60); // 1h timeout
  
}

void loop(void) {
  String command = Serial.readStringUntil('\n');

  if(command.startsWith("h") or command.startsWith("H")) {
    printHelp();
  }
  else if(command.startsWith("p")) {
    auto power = command.substring(1).toFloat();
    Serial.print("Setting power to ");
    Serial.println(power);
    motor_interface.setPower(map(power, 0, 100, 0, 255));
  }
  else if(command.length() != num_motors) {
    Serial.print("Please provide command with length equal to the number of motors (");
    Serial.print(num_motors);
    Serial.println(")!");
    Serial.print("You command had length ");
    Serial.println(command.length());
  } else {
    for(size_t i = 0; i < command.length(); i++) {
      motor_interface.set(i, command.charAt(i) != '0');
    }
    Serial.println("Updated Motors");
  }
  
}

void printHelp() {
  Serial.println(
    "Commands:\n"
    "'h' or 'H'\tdisplay this help text\n"
    "'p<number>'\tset the power of the motors between 0 and 100\n"
    "0010010\tor any variation thereof enables/disables the motors. The length has to match the number of motors\n"
    );
}
