
#if !defined( _CONFIG_H_ )
#define _CONFIG_H_

#include <Arduino.h>

#define DBG_OUTPUT_PORT Serial

class JSONProvider
{
  public:
  
  virtual void AppendMotorJSON( String& json ) = 0;
};

#endif // #if defined( _CONFIG_H_ )
