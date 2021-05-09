#include <memory>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "Shiftregister.h"
#include "Motorinterface.h"
#include "TingleWebServer.h"

const auto ssid = "Tingledongle";
const auto password = "tingletingle";

const auto enable_pin = 17;
const auto mosi_pin  = 23;
const auto clock_pin = 19;
const auto chip_select_pin = 16;
const auto num_chained_dongles = 2;

using TingleServer = TingleWebServer<num_chained_dongles>;

std::unique_ptr<TingleServer> tingleserver;


void setup(void) {

  pinMode(2, OUTPUT);
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("");


  if (MDNS.begin("tingledongle")) {
    Serial.println("MDNS responder started");
  }

  MDNS.addService("http", "tcp", 80);

  auto shiftregister = Shiftregister<num_chained_dongles>(mosi_pin, clock_pin, chip_select_pin);
  auto motor_interface = Motorinterface<num_chained_dongles>(enable_pin, std::move(shiftregister));
  tingleserver = std::unique_ptr<TingleServer>(new TingleServer(80, std::move(motor_interface)));
  
  Serial.println("HTTP server started");
}

void loop(void) {
  tingleserver->loop();
  delay(2); //allow the cpu to switch to other tasks
}
