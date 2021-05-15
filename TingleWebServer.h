#pragma once

#include <WebServer.h>
#include <HTTPUpdateServer.h>

#include "MotorPlacement.h"
#include "Motorinterface.h"
#include "DirectionTingler.h"

template<size_t CHAIN_LENGTH>
class TingleWebServer {
  public:
    TingleWebServer(uint16_t port, Motorinterface<CHAIN_LENGTH> motors) : server_(port), motors_(std::move(motors)), calibrator_(motors_, motor_placement_), direction_tingler_(motors_, motor_placement_) {
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

      server_.on("/angle", [this] () {
        for (uint8_t i = 0; i < server_.args(); i++) {
          auto argname = server_.argName(i);
          auto arg = server_.arg(i);
          if (argname == "heading") {
            direction_tingler_.current_heading_ = normalizeAngle(arg.toFloat());
          }

          if (argname == "target") {
            direction_tingler_.target_heading_ = normalizeAngle(arg.toFloat());
          }

          direction_tingler_.doTingle();
        }

        server_.send(200, "text/html", "OK: " + String(direction_tingler_.current_heading_) + " " + String(direction_tingler_.target_heading_));

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
    MotorPlacement<CHAIN_LENGTH * 8> motor_placement_ = {};
    HTTPUpdateServer firmware_updater_;
    MotorPlacementCalibrator<CHAIN_LENGTH> calibrator_;
    DirectionTingler<CHAIN_LENGTH> direction_tingler_;


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

        if (argname == "set_calibration_reference") {
          calibrator_.setReferenceAngle(direction_tingler_.current_heading_);
        }

        if (argname == "start_calibrate") {
          auto motor = arg.toInt();
          if (motor > 0) {
            calibrator_.startCalibration(motor - 1);
          }
        }

        if (argname == "stop_calibrate") {
          calibrator_.stopCalibration(direction_tingler_.current_heading_);
        }

        if (argname == "abort_calibrate") {
          calibrator_.abortCalibration();
        }

        if ( argname == "tingle") {
          direction_tingler_.is_tingling = (arg == "on");
        }
      }
    }

    String command(String cmd, String name) {
      return "<a href=.?" + std::move(cmd) + ">" + name + "</a>";
    }

    String cell(String body) {
      return "<td>" + std::move(body) + "</td>";
    }

    String colored_cell (String body, String color) {
      return "<td style=\"background-color:" + std::move(color) + ";\">" + std::move(body) + "</td>";
    }


    String motor(size_t idx) {
      auto next_state = String(motors_.get(idx) ? "OFF" : "ON");
      auto cmd = "state=" + std::move(next_state) + "&motor=" + String(idx + 1);
      auto name = "M" + String((idx % 8) + 1);
      return command(std::move(cmd), std::move(name));
    }

    String motor_cell(size_t idx) {
      return motors_.get(idx) ? colored_cell(motor(idx), "#333333") : cell(motor(idx));
    }


    void serveMainPage() {
      // <meta http-equiv='refresh' content='1'/>
      String header = "<head><title>Tingledongle</title></head>";

      String title = "<h1>Tingledongle</h1>";

      String tingle_ctrl = "<h1>" + (direction_tingler_.is_tingling ? command("tingle=off", "TINGLE OFF") : command("tingle=on", "TINGLE ON")) + "</h1>";

      String allon = command("command=ALLON", "ON");
      String alloff = command("command=ALLOFF", "OFF");


      String main_ctrl = "<table>" + cell("All Motors") + colored_cell(std::move(allon), "#8AE234") + colored_cell(std::move(alloff), "#FF403B") + "</table>";
      String calibration_ctrl = "<table>" + cell("Calibration") +
                                colored_cell(command("set_calibration_reference=here", "Set Reference Direction"), "#6666FF") +
                                colored_cell(command("abort_calibrate=now", "Not feeling it!"), "#FF403B") + cell("Reference: " + String(calibrator_.getReferenceAngle())) + "</table>";

      String motor_ctrl = "<table><tr>" + cell("Motors");
      for (size_t m = 0; m < motors_.size(); m++) {
        motor_ctrl += motor_cell(m);
      }
      motor_ctrl += "</tr><tr>" + cell("Calibration");

      for (size_t m = 0; m < motors_.size(); m++) {
        if (calibrator_.isCalibrating()) {
          motor_ctrl += calibrator_.getCurrentMotor() == m ? cell(command("stop_calibrate=now", "x")) : cell("");
        } else {
          motor_ctrl += motor_placement_.motors_[m].activated ? cell("") : cell(command("start_calibrate=" + String(m + 1), "c"));
        }
      }
      motor_ctrl += "</tr></table>";

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

      String body = "<body>" + title + "<br>" + tingle_ctrl + "<br>" + main_ctrl + "<br>" + calibration_ctrl + "<br>" + motor_ctrl + "<br>" + powerlevels(10) + "<br>" + sys + "</body>";

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
