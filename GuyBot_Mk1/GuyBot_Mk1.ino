/*
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/
#include "Arduino.h"

#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

extern "C" {
#include "user_interface.h"
}

#include "file_server.h"

#define DBG_OUTPUT_PORT Serial

uint16_t g_aMotorValues[ 16 ] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void getMotorStatusJSON( String& rString )
{
  rString += "test: yes";
}


const int sclPin = D1;
const int sdaPin = D2;

void print_i2c_devices()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("Done.\n");
  }
}

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

/*
static const uint8_t SDA = 4;
static const uint8_t SCL = 5;

static const uint8_t LED_BUILTIN = 2;
static const uint8_t BUILTIN_LED = 2;

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t RX   = 3;
static const uint8_t TX   = 1;
 */

void setupPin( uint8_t uPin, uint8_t uMode, uint8_t uInitialValue )
{
  pinMode(uPin, uMode);
  digitalWrite(uPin, uInitialValue);
}

BotWebServer server;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup(void)
{
  DBG_OUTPUT_PORT.begin(9600);
  DBG_OUTPUT_PORT.println("Hello");
  //DBG_OUTPUT_PORT.setDebugOutput(true);

  DBG_OUTPUT_PORT.printf("LED_BUILTIN is %d\n", LED_BUILTIN);

  setupPin( D1, OUTPUT, LOW );
  setupPin( D2, OUTPUT, LOW );
  setupPin( D3, OUTPUT, LOW );
  setupPin( D4, OUTPUT, LOW );
  setupPin( D5, OUTPUT, HIGH );
  setupPin( D6, OUTPUT, LOW );
  setupPin( D7, OUTPUT, LOW );
  setupPin( D8, OUTPUT, LOW );
  setupPin( RX, OUTPUT, LOW );
  //Uncommenting this will break the serial text output
  //setupPin( TX, OUTPUT, LOW );

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);


  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), server.formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }

  connect_wifi();

  server.start();

  //wire.begin
  //Wire.begin(sdaPin, sclPin);
  //print_i2c_devices();
  //delay(1000);
  //wire.begin end

  //Wire.begin(sdaPin, sclPin);

  
  Wire.pins(sdaPin, sclPin);

  pwm.begin();
  pwm.setPWMFreq(60);  // 1600 is the maximum PWM frequency

  for (uint8_t pwmnum=0; pwmnum < 16; pwmnum++) 
  {
    pwm.setPWM(pwmnum, 0, 0 );
  }

  //pwm.setPWM(0, 0, 1024 );
}

uint pwmnum=0;
uint pwmspeed=0;
int dir=1;
  
void loop(void) {
  //DBG_OUTPUT_PORT.println("Hello World.");
  server.update();

  for (uint8_t pwmnum=0; pwmnum < 16; pwmnum++) 
  {
    pwm.setPWM(pwmnum, 0, 0 );
  }
  
  //delay(1000);
  //yield();

/*
  pwmspeed += dir;
  if( pwmspeed > 4096 )
  {
    dir *= -1;
  }
  else if( pwmspeed == 0 )
  {
    dir *= -1;
    
    pwm.setPWM(pwmnum, 0, pwmspeed );
    
    //switch forward/reverse pins
    pwmnum = 0 == pwmnum ? 1 : 0;
  }

  //DBG_OUTPUT_PORT.println(pwmspeed);
  pwm.setPWM(pwmnum, 0, pwmspeed );
  */
}

