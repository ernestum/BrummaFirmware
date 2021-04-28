#pragma once

#include <WebServer.h>
#include <HTTPUpdateServer.h>


#include "Motorinterface.h"

template<size_t CHAIN_LENGTH>
class TingleWebServer {
  public:
    TingleWebServer(uint16_t port, Motorinterface<CHAIN_LENGTH> motors) : server_(port), motors_(std::move(motors)) {
      server_.on("/", [this]() {
        handleMotorArgs();
        serveMainPage();
      });

      server_.on("/auth", [this]() {
        handleWIFIArgs();
      });

      server_.onNotFound([this]() {
        handleNotFound();
      });

      firmware_updater_.setup(&server_);

      server_.begin();
    }

    void loop() {
      server_.handleClient();
    }
  private:
    WebServer server_;
    Motorinterface<CHAIN_LENGTH> motors_;
    HTTPUpdateServer firmware_updater_;

    void handleMotorArgs() {
      bool target_state = false;

      for (uint8_t i = 0; i < server_.args(); i++) {
        auto argname = server_.argName(i);
        auto arg = server_.arg(i);

        if (argname == "state") {
          target_state = (arg == "ON");
        }

        if (argname == "motor") {
          auto motor = arg.toInt();
          if (motor > 0) {
            motors_.set(motor - 1, target_state);
          }
        }

        if (argname == "command") {
          if (arg == "ALLON") {
            motors_.allOn();
          } else if (arg == "ALLOFF") {
            motors_.allOff();
          }
        }

        if (argname == "power") {
          motors_.setPower(arg.toInt());
        }
      }
    }

    void serveMainPage() {
      String header = "<head><title>Tingledongle</title></head>";

      String title = "<h1>Tingledongle</h1>";

      String allon = "<a href=.?command=ALLON>ON</a>";
      String alloff = "<a href=.?command=ALLOFF>OFF</a>";

      auto cell = [](String && body) {
        return "<td>" + std::move(body) + "</td>";
      };

      auto colored_cell = [](String && body, String && color) {
        return "<td style=\"background-color:" + std::move(color) + ";\">" + std::move(body) + "</td>";
      };

      String main_ctrl = "<table>" + cell("All Motors") + colored_cell(std::move(allon), "#8AE234") + colored_cell(std::move(alloff), "#FF403B") + "</table>";

      auto motor = [&](size_t idx) {
        auto next_state = String(motors_.get(idx) ? "OFF" : "ON");
        return "<a href=.?state=" + std::move(next_state) + "&motor=" + String(idx + 1) + ">M" + String((idx % 8) + 1) + "</a>";
      };

      auto motor_cell = [&](size_t idx) {
        return motors_.get(idx) ? colored_cell(motor(idx), "#333333") : cell(motor(idx));
      };

      String motor_ctrl = "<table>" + cell("Motors");
      for (size_t m = 0; m < motors_.size(); m++) {
        motor_ctrl += motor_cell(m);
      }
      motor_ctrl += "</table>";

      auto powerlevel = [](int level) {
        return "<a href=.?power=" + String(map(level, 0, 100, 0, 255)) + ">" + String(level) + "</a>";
      };

      auto powerlevelcell = [&](int level) {
        if (map(level, 0, 100, 0, 255) == motors_.getPower()) {
          return colored_cell(powerlevel(level), "#FFA798");
        } else {
          return cell(powerlevel(level));
        }
      };

      auto powerlevels = [&](int stepsize) {
        String table = "<table><td>Power</td>";
        for (int l = 0; l <= 100; l += stepsize) {
          table += powerlevelcell(l);
        }
        return table + "</table>";
      };

      auto uptime = []() -> String {
        char uptime[50];

        int sec = millis() / 1000;
        int minutes = sec / 60;
        int hr = minutes / 60;
        snprintf(uptime, 50, "<p>Uptime: %02d:%02d:%02d</p>", hr, minutes % 60, sec % 60);
        return uptime;
      };

      String sys = "<table>" + cell(uptime()) + cell("<a href=/update>Update Firmware</a>") + cell("<a href=/auth>Change WLAN</a>") + "</table>";

      String body = "<body>" + title + "<br>" + main_ctrl + "<br>" + motor_ctrl + "<br>" + powerlevels(10) + "<br>" + sys + "</body>";

      String mainpage = "<html>" + header + body + "</html>";

      server_.send(200, "text/html", mainpage);
    }

    void handleWIFIArgs() {
      if (server_.hasArg("ssid") && server_.hasArg("password")) {
        String password;
        String ssid;
        for (uint8_t i = 0; i < server_.args(); i++) {
          auto argname = server_.argName(i);
          auto arg = server_.arg(i);
          if (argname == "ssid") ssid = arg;
          if (argname == "password") password = arg;
        }

        server_.send(200, "text/html", "Now connecting to '" + ssid + "' using '" + password + "'. <br>This is the wrong password? Just reset the device<br>Hint: bookmark this page to quickly connect the dongle to your network.");

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

        // Wait for connection
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }

        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

      } else {

        server_.send(200, "text/html",
                     "<form action=\"/auth\">"
                     "<label for=\"ssid\">WLAN Name (SSID):</label><br>"
                     "<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"\"><br>"
                     "<label for=\"password\">Password:</label><br>"
                     "<input type=\"text\" id=\"password\" name=\"password\" value=\"\"><br><br>"
                     "<input type=\"submit\" value=\"Login\">"
                     "</form>");
      }


    }

    void handleNotFound() {
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += server_.uri();
      message += "\nMethod: ";
      message += (server_.method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += server_.args();
      message += "\n";

      for (uint8_t i = 0; i < server_.args(); i++) {
        message += " " + server_.argName(i) + ": " + server_.arg(i) + "\n";
      }

      server_.send(404, "text/plain", message);
    }

};
