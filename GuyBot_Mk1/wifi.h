
#include "config.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

void connect_wifi()
{  
  const char* ssid = "SamNet";
  const char* password = "SamAndGuyYeah1";
  const char* host = "guybot";

  DBG_OUTPUT_PORT.println("Starting up wifi...");

  wifi_set_sleep_type(LIGHT_SLEEP_T);

  WiFi.hostname(host);
  WiFi.mode(WIFI_STA);

  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, password);
  }

  int ledState = LOW;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
    ledState = !ledState;
    digitalWrite(BUILTIN_LED, ledState);
  }

  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");
}

