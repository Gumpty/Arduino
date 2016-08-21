#include "config.h"

#include "motor_control.h"

/*
class MotorDevice_PCA9685_L9110S : public IMotorDevice
{
  //IMotorDevice start
  virtual motorspeed_t GetSpeed();
  virtual void SetSpeed( motorspeed_t speed );
  //IMotorDevice end

  uint8_t forwardpin;
  uint8_t reversepin;
};
*/

/*
Motor Truth Table
IA  IB  Motor State
L   L   Off
H   L   Forward
L   H   Reverse
H   H   Off
We recommend applying a PWM signal to input IA to control the motor speed and a digital output to input IB to control its direction.
*/


motorspeed_t MotorDevice_PCA9685_L9110S::GetSpeed()
{
  return m_Speed;
}

void MotorDevice_PCA9685_L9110S::SetSpeed( motorspeed_t speed )
{
  if( speed > k_MaxSpeed )
  {
    m_Speed = k_MaxSpeed;
  }
  else if( speed < -k_MaxSpeed )
  {
    m_Speed = -k_MaxSpeed;
  }
  else
  {
    m_Speed = speed;
  }

  if( speed >= 0 )
  {
    digitalWrite( m_DirectionPin, k_Forward );
    digitalWrite( m_SpeedPin, m_Speed );
  }
  else
  {
    digitalWrite( m_DirectionPin, k_Reverse );
    digitalWrite( m_SpeedPin, -m_Speed );
  }
}


void MotorControl::AddMotor( const char* name, IMotorDevice& motor )
{
  m_Motors[name] = &motor;
  DBG_OUTPUT_PORT.println( "Added motor " + String(name) + ". There are now " + m_Motors.size() + " motors." );
}


void MotorControl::RemoveMotor( const char* name )
{
  m_Motors.erase( name );
  DBG_OUTPUT_PORT.println( "Removed motor " + String(name) + ". There are now " + m_Motors.size() + " motors." );
}


void MotorControl::SetMotorSpeed( const char* name, motorspeed_t speed )
{
  std::map< const char*, IMotorDevice* >::iterator it = m_Motors.find( name );
  if ( it != m_Motors.end() )
  {
    m_Motors.at( name )->SetSpeed( speed );

    DBG_OUTPUT_PORT.println( "Motor " + String(name) + " speed set to " + String(speed) );
  }
  else
  {
    DBG_OUTPUT_PORT.println("Setting motor speed on a motor that cannot be found!");
  } 
}


void MotorControl::AppendMotorJSON( String& json )
{
  DBG_OUTPUT_PORT.println( "Appending motors to JSON" );
  DBG_OUTPUT_PORT.println( "There are " + String(m_Motors.size()) + " motors." );
  
  json += ",\n \"motors\":[\n";

  bool bFirst = true;
  for( std::pair<const char* const, IMotorDevice*> motor : m_Motors )
  {
    motorspeed_t speed = motor.second->GetSpeed();
    if( !bFirst )
    {
      json += ",\n";
    }
    json += "{";
    json += "\"name\":";
    json += String( motor.first );
    json += ", ";
    json += "\"speed\":";
    json += String( speed );
    json += "}";
    
    bFirst = false;
  }
    
  json += "]";

  DBG_OUTPUT_PORT.println( json );
}

