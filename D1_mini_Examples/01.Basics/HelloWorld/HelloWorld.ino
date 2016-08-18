/*
 * Hello World
 * Simply prints Hello World to the serial monitor.
 */

extern "C" {
#include "user_interface.h"
}

 #define DBG_OUTPUT_PORT Serial

void derp()
{
  DBG_OUTPUT_PORT.println("Cheese World."); 
}

void setup() {
  // initialize serial communications at 9600 bps
  DBG_OUTPUT_PORT.begin(9600);

  derp();
}

void loop() {
  DBG_OUTPUT_PORT.println("Hello World.");
  delay(2000);
}
