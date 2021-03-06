
#include <Arduino.h>

#include <Wire.h>

extern "C" {
#include "user_interface.h"
}

#include "config.h"
#include "wifi.h"
#include "file_server.h"
#include "motor_control.h"
#include "gpio.h"

const int sclPin = D1;
const int sdaPin = D2;

static const int g_FrontLeftNameIndex = 0;
static const int g_FrontRightNameIndex = 1;

static const char* g_MotorNames[] = {
  "FrontLeft",
  "FrontRight"
};

BotWebServer g_WebServer;
MotorControl g_MotorControl;

MotorDevice_PCA9685_L9110S g_FrontLeftWheel;
MotorDevice_PCA9685_L9110S g_FrontRightWheel;

void setup(void)
{
  DBG_OUTPUT_PORT.begin(9600);
  DBG_OUTPUT_PORT.println("Hello");
  //DBG_OUTPUT_PORT.setDebugOutput(true);

  setupPins( OUTPUT, LOW );
  
  setupPin( D5, OUTPUT, HIGH ); // Motor power relay
  setupPin( LED_BUILTIN, OUTPUT, LOW );

  {
    SPIFFS.begin();
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), g_WebServer.formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }

  connect_wifi();
  
  Wire.pins(sdaPin, sclPin);

/*
  pwm.begin();
  pwm.setPWMFreq(60);  // 1600 is the maximum PWM frequency

  for (uint8_t pwmnum=0; pwmnum < 16; pwmnum++) 
  {
    pwm.setPWM(pwmnum, 0, 0 );
  }
*/
  //pwm.setPWM(0, 0, 1024 );

  g_FrontLeftWheel.m_DirectionPin = 0;
  g_FrontLeftWheel.m_SpeedPin = 1;
  g_FrontRightWheel.m_DirectionPin = 2;
  g_FrontRightWheel.m_SpeedPin = 3;
  
  g_MotorControl.AddMotor( g_MotorNames[ g_FrontLeftNameIndex ], g_FrontLeftWheel );
  g_MotorControl.AddMotor( g_MotorNames[ g_FrontRightNameIndex ], g_FrontRightWheel );

  g_MotorControl.SetMotorSpeed( g_MotorNames[ g_FrontLeftNameIndex ], 0 );
  g_MotorControl.SetMotorSpeed( g_MotorNames[ g_FrontRightNameIndex ], 0 );

  g_WebServer.AddJSONProvider( &g_MotorControl );
  g_WebServer.AddRequestHandler( "motorspeed", HTTP_POST, std::bind( &MotorControl::OnMotorSpeed, &g_MotorControl, std::placeholders::_1 ), g_MotorControl.GetMotorSpeedArgs() );

  g_WebServer.start();
}

uint pwmnum=0;
uint pwmspeed=0;
int dir=1;
  
void loop(void) {
  //DBG_OUTPUT_PORT.println("Hello World.");
  g_WebServer.update();

  g_MotorControl.Update();
/*
  for (uint8_t pwmnum=0; pwmnum < 16; pwmnum++) 
  {
    pwm.setPWM(pwmnum, 0, 0 );
  }
/*
  
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

