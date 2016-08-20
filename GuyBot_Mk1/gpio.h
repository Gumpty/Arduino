#include "config.h"

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

void setupPins( uint8_t uMode, uint8_t uInitialValue )
{
  setupPin( D1, uMode, uInitialValue );
  setupPin( D2, uMode, uInitialValue );
  setupPin( D3, uMode, uInitialValue );
  setupPin( D4, uMode, uInitialValue );
  setupPin( D5, uMode, uInitialValue );
  setupPin( D6, uMode, uInitialValue );
  setupPin( D7, uMode, uInitialValue );
  setupPin( D8, uMode, uInitialValue );
  setupPin( RX, uMode, uInitialValue );

    //Uncommenting this will break the serial text output
  //setupPin( TX, OUTPUT, LOW );
}

