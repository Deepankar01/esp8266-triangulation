// Credits
// Original RTOS version https://github.com/espressif/esp8266-rtos-sample-code/tree/master/03Wifi/Sniffer_DEMO/sniffer
// Converted to Arduino https://github.com/RandDruid/esp8266-deauth and https://github.com/kripthor/WiFiBeaconJam
// Code refactor and improvements Ray Burnette https://www.hackster.io/rayburne/esp8266-mini-sniff-f6b93a
// This version fixes handling of PROBE packets, further refactors code
// This version compiled on Windows 10/Arduino 1.6.5 for Wemos D1 mini but should work on any ESP8266 28Jul2017
// Tested on SDK version:1.5.4(baaeaebb)
// Using Visual Studio Code and the Arduino extension https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>


#include "functions.h"


#define MAX_APS_TRACKED 100
#define MAX_CLIENTS_TRACKED 100
#undef PRINT_RAW_HEADER   // define this to print raw packet headers
#define PERIODIC  //define this to get summary of new and expired entries periodically

beaconinfo aps_known[MAX_APS_TRACKED];                    // Array to save MACs of known APs
int aps_known_count = 0;                                  // Number of known APs
int nothing_new = 0;
clientinfo clients_known[MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
int clients_known_count = 0;                              // Number of known CLIENTs
probeinfo probes_known[MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
int probes_known_count = 0;
int known = 0;
#define disable 0
#define enable  1
#define MAX_CLIENT_AGE 60  //age before entry is considered old (seconds)
#define CHECK_INTERVAL 30   // periodic check interval (seconds)

unsigned int channel = 1;
uint32_t last_check_time, next_check_time;
const char* ssid     = "198 SF";
const char* password = "world198";
const char* host = "192.168.2.111";
const int httpPort = 8080;
WiFiClient client;

String response = "";
void setup() {
  Serial.begin(57600);
  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.println(F("ESP8266 mini-sniff"));
  Serial.println(F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI"));

  wifi_set_opmode(STATION_MODE);            // Promiscuous works only with station mode
  wifi_set_channel(channel);
  wifi_promiscuous_enable(disable);
  wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
  wifi_promiscuous_enable(enable);
  last_check_time = 0;
  next_check_time = last_check_time + CHECK_INTERVAL;

}

void loop() {
  channel = 1;
  wifi_set_channel(channel);
  while (true) {
    nothing_new++;                          // Array is not finite, check bounds and adjust if required
    if (nothing_new > 200) {
      nothing_new = 0;
      channel++;
      if (channel == 15) break;             // Only scan channels 1 to 14
      wifi_set_channel(channel);
    }
    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
    // Press keyboard ENTER in console with NL active to repaint the screen
    if ((Serial.available() > 0) && (Serial.read() == '\n')) {
      Serial.println("\n-------------------------------------------------------------------------------------\n");
      for (int u = 0; u < clients_known_count; u++) print_client(clients_known[u]);
      for (int u = 0; u < aps_known_count; u++) print_beacon(aps_known[u]);
      for (int u = 0; u < probes_known_count; u++) print_probe(probes_known[u]);
      Serial.println("\n-------------------------------------------------------------------------------------\n");

    }

#ifdef PERIODIC
    uint32_t now = millis() / 1000;
    if (now >= next_check_time) {
      Serial.println("Periodic ");
      Serial.print(now);
      Serial.print("-");
      Serial.print(next_check_time);

      wifi_promiscuous_enable(disable);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }

      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());






      for (int u = 0; u < clients_known_count; u++) {

        if ( !clients_known[u].reported && clients_known[u].last_heard >= last_check_time ) {
          known = 0;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
          }
          else {
            Serial.println("connection success");
          }
          Serial.print("New ");
          response = "'type':'DEVICE','RSSI':" + String(clients_known[u].rssi) + ",'channel':" + String(clients_known[u].channel) + ",'SSID':'";

          for (int k = 0; k < aps_known_count; k++)
          {
            if (! memcmp(aps_known[k].bssid, clients_known[u].bssid, ETH_MAC_LEN)) {
              response = response + (char*)aps_known[k].ssid;
              known = 1;     // AP known => Set known flag
              break;
            };
          };

          if (! known)  {
            response = response + "??";
          };
          response = response + "','station':'";
          for (int i = 0; i < 6; i++) response = response + String(clients_known[u].station[i], HEX);
          response = response + "','BSSID':'";
          for (int i = 0; i < 6; i++) response = response + String(clients_known[u].bssid[i], HEX);
          response = response + "'";
          Serial.print("Sending Data");

          Serial.print("Requesting POST: ");
          // Send request to the server:
          client.println("POST /1 HTTP/1.1");
          client.println("Host: server_name");
          client.println("Accept: */*");
          client.println("Content-Type: application/x-www-form-urlencoded");
          client.print("Content-Length: ");
          client.println(response.length());
          client.println();
          client.print(response);

          delay(100); // Can be changed
          if (client.connected()) {
            client.stop();  // DISCONNECT FROM THE SERVER
          }
          Serial.println("Data sent");
          print_client(clients_known[u]);

          clients_known[u].reported = 1;


        } else if ( clients_known[u].reported && now > MAX_CLIENT_AGE &&
                    clients_known[u].last_heard <= (last_check_time - MAX_CLIENT_AGE) ) {

          known = 0;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
          }
          else {
            Serial.println("connection success");
          }
          Serial.print("Old ");
          response = "'type':'DEVICE','RSSI':" + String(clients_known[u].rssi) + ",'channel':" + String(clients_known[u].channel) + ",'SSID':'";

          for (int k = 0; k < aps_known_count; k++)
          {
            if (! memcmp(aps_known[k].bssid, clients_known[u].bssid, ETH_MAC_LEN)) {
              response = response + (char*)aps_known[k].ssid;
              known = 1;     // AP known => Set known flag
              break;
            };
          };

          if (! known)  {
            response = response + "??";
          };
          response = response + "','station':'";
          for (int i = 0; i < 6; i++) response = response + String(clients_known[u].station[i], HEX);
          response = response + "','BSSID':'";
          for (int i = 0; i < 6; i++) response = response + String(clients_known[u].bssid[i], HEX);
          response = response + "'";
          Serial.print("Sending Data");

          Serial.print("Requesting POST: ");
          // Send request to the server:
          client.println("POST /1 HTTP/1.1");
          client.println("Host: server_name");
          client.println("Accept: */*");
          client.println("Content-Type: application/x-www-form-urlencoded");
          client.print("Content-Length: ");
          client.println(response.length());
          client.println();
          client.print(response);

          delay(100); // Can be changed
          if (client.connected()) {
            client.stop();  // DISCONNECT FROM THE SERVER
          }
          Serial.println("Data sent");
          print_client(clients_known[u]);
          clients_known[u].reported = 0;
        };
      };


      for (int u = 0; u < aps_known_count; u++) {
        if ( !aps_known[u].reported && aps_known[u].err == 0
             && aps_known[u].last_heard >= last_check_time ) {
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
          }
          else {
            Serial.println("connection success");
          }
          Serial.print("New ");


          if (aps_known[u].err != 0) {
            Serial.printf("BEACON ERR: (%d)  \r\n", aps_known[u].err);
          } else {
            response = "'type':'BEACON','RSSI':" + String(aps_known[u].rssi) + ",'channel':" + String(aps_known[u].channel) + ",'SSID':'";
            response = response + (char*)aps_known[u].ssid;
            response = response + "','BSSID':'";
            for (int i = 0; i < 6; i++) response = response + String(aps_known[u].bssid[i], HEX);
            response = response + "'";
            Serial.print("Sending Data");
            Serial.print("Requesting POST: ");
            // Send request to the server:
            client.println("POST /1 HTTP/1.1");
            client.println("Host: server_name");
            client.println("Accept: */*");
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(response.length());
            client.println();
            client.print(response);

            delay(100); // Can be changed
            if (client.connected()) {
              client.stop();  // DISCONNECT FROM THE SERVER
            }
            Serial.println("Data sent");

          }

          print_beacon(aps_known[u]);
          aps_known[u].reported = 1;

        } else if ( aps_known[u].reported && aps_known[u].err == 0 && now > MAX_CLIENT_AGE &&
                    aps_known[u].last_heard <= (last_check_time - MAX_CLIENT_AGE) ) {
          Serial.print("Old ");
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
          }
          else {
            Serial.println("connection success");
          }

          if (aps_known[u].err != 0) {
            Serial.printf("BEACON ERR: (%d)  \r\n", aps_known[u].err);
          } else {
            response = "'type':'BEACON','RSSI':" + String(aps_known[u].rssi) + ",'channel':" + String(aps_known[u].channel) + ",'SSID':'";
            response = response + (char*)aps_known[u].ssid;
            response = response + "','BSSID':'";
            for (int i = 0; i < 6; i++) response = response + String(aps_known[u].bssid[i], HEX);
            response = response + "'";
            Serial.print("Sending Data");
            Serial.print("Requesting POST: ");
            // Send request to the server:
            client.println("POST /1 HTTP/1.1");
            client.println("Host: server_name");
            client.println("Accept: */*");
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(response.length());
            client.println();
            client.print(response);

            delay(100); // Can be changed
            if (client.connected()) {
              client.stop();  // DISCONNECT FROM THE SERVER
            }
            Serial.println("Data sent");

          }

          print_beacon(aps_known[u]);
          aps_known[u].reported = 0;
        };
      };


      for (int u = 0; u < probes_known_count; u++) {

        if ( !probes_known[u].reported && probes_known[u].last_heard >= last_check_time ) {

          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
          }
          else {
            Serial.println("connection success");
          }


          if (probes_known[u].err != 0) {
            Serial.printf("ci.err %02d", probes_known[u].err);
            Serial.printf("\r\n");
          } else {
            response = "'type':'PROBE','RSSI':" + String(probes_known[u].rssi) + ",'channel':" + String(probes_known[u].channel) + ",";
            response = response + "'station':'";
            for (int i = 0; i < 6; i++) response = response + String(probes_known[u].station[i], HEX);
            response = response + "','SSID':'" + (char*)probes_known[u].ssid + "','BSSID':'";
            for (int i = 0; i < 6; i++) response = response + String(probes_known[u].bssid[i], HEX);
            response = response + "'";

            Serial.print("Sending Data");

            Serial.print("Requesting POST: ");
            // Send request to the server:
            client.println("POST /1 HTTP/1.1");
            client.println("Host: server_name");
            client.println("Accept: */*");
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(response.length());
            client.println();
            client.print(response);

            delay(100); // Can be changed
            if (client.connected()) {
              client.stop();  // DISCONNECT FROM THE SERVER
            }
            Serial.println("Data sent");
          }



          print_probe(probes_known[u]);
          probes_known[u].reported = 1;

        } else if ( probes_known[u].reported && now > MAX_CLIENT_AGE &&
                    probes_known[u].last_heard <= (last_check_time - MAX_CLIENT_AGE) ) {
          Serial.print("Old ");
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
          }
          else {
            Serial.println("connection success");
          }


          if (probes_known[u].err != 0) {
            Serial.printf("ci.err %02d", probes_known[u].err);
            Serial.printf("\r\n");
          } else {
            response = "'type':'PROBE','RSSI':" + String(probes_known[u].rssi) + ",'channel':" + String(probes_known[u].channel) + ",";
            response = response + "'station':'";
            for (int i = 0; i < 6; i++) response = response + String(probes_known[u].station[i], HEX);
            response = response + "','SSID':'" + (char*)probes_known[u].ssid + "','BSSID':'";
            for (int i = 0; i < 6; i++) response = response + String(probes_known[u].bssid[i], HEX);
            response = response + "'";

            Serial.print("Sending Data");

            Serial.print("Requesting POST: ");
            // Send request to the server:
            client.println("POST /1 HTTP/1.1");
            client.println("Host: server_name");
            client.println("Accept: */*");
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(response.length());
            client.println();
            client.print(response);

            delay(100); // Can be changed
            if (client.connected()) {
              client.stop();  // DISCONNECT FROM THE SERVER
            }
            Serial.println("Data sent");
          }
          print_probe(probes_known[u]);
          probes_known[u].reported = 0;
        };
      };

      last_check_time = now;
      next_check_time = now + CHECK_INTERVAL;
      wifi_promiscuous_enable(enable);
      //ESP.restart();
    };

#endif
  }

}

