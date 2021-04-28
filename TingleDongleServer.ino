#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "Shiftregister.h"
#include "Motorinterface.h"
#include "TingleWebServer.h"

const auto ssid = "Tingledongle";
const auto password = "tingletingle";

TingleWebServer<2>* tingleserver;


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

  auto shiftregister = Shiftregister<2>(23, 19, 16);
  auto motors = Motorinterface<2>(17, std::move(shiftregister));

  tingleserver = new TingleWebServer<2>(80, std::move(motors));
  Serial.println("HTTP server started");
}

void loop(void) {
  tingleserver->loop();
  delay(2); //allow the cpu to switch to other tasks
}
