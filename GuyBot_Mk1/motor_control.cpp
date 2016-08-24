#include "config.h"

#include "motor_control.h"

#include <algorithm>

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

static const char* k_SpeedKey = "name";
static const char* k_SpeedValue = "value";

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

  ApplySpeed();
}


void MotorDevice_PCA9685_L9110S::ApplySpeed()
{
  if( m_Speed >= 0 )
  {
    //DBG_OUTPUT_PORT.println("MotorDevice_PCA9685_L9110S::SetSpeed " + String(m_DirectionPin) + " " + String(k_Forward));
    digitalWrite( m_DirectionPin, k_Forward );
    //DBG_OUTPUT_PORT.println("MotorDevice_PCA9685_L9110S::SetSpeed " + String(m_SpeedPin) + " " + String(m_Speed));
    digitalWrite( m_SpeedPin, m_Speed );
  }
  else
  {
    //DBG_OUTPUT_PORT.println("MotorDevice_PCA9685_L9110S::SetSpeed " + String(m_DirectionPin) + " " + String(k_Reverse));
    digitalWrite( m_DirectionPin, k_Reverse );
    //DBG_OUTPUT_PORT.println("MotorDevice_PCA9685_L9110S::SetSpeed " + String(m_SpeedPin) + " " + String(-m_Speed));
    digitalWrite( m_SpeedPin, -m_Speed );
  }
}


MotorControl::MotorControl()
{
  m_SpeedArgs.push_back( k_SpeedKey );
  m_SpeedArgs.push_back( k_SpeedValue );
}


void MotorControl::AddMotor( const char * name, IMotorDevice& motor )
{
  m_Motors[ name ] = &motor;
  DBG_OUTPUT_PORT.println( "Added motor " + String( name ) + ". There are now " + m_Motors.size() + " motors." );
}


void MotorControl::RemoveMotor( const char* name )
{
  m_Motors.erase( name );
  DBG_OUTPUT_PORT.println( "Removed motor " + String( name ) + ". There are now " + m_Motors.size() + " motors." );
}


const std::list<String>& MotorControl::GetMotorSpeedArgs()
{
   return m_SpeedArgs;
}


void MotorControl::OnMotorSpeed( std::list<std::pair<String,String>> args )
{
  DBG_OUTPUT_PORT.println( "MotorControl::OnMotorSpeed" );
  
  String nameValue;
  auto name_it = std::find_if( args.begin(), args.end(), 
  [](std::pair<String,String> const& arg) 
  { 
    return arg.first.equalsIgnoreCase( k_SpeedKey ); 
  });
  if( name_it != args.end() )
  {
    nameValue = (*name_it).second;
  }

  String speedValue;
  auto speed_it = std::find_if( args.begin(), args.end(), 
  [](std::pair<String,String> const& arg) 
  { 
    return arg.first.equalsIgnoreCase( k_SpeedValue ); 
  });
  if( speed_it != args.end() )
  {
    speedValue = (*speed_it).second;
  }

  SetMotorSpeed( nameValue.c_str(), speedValue.toInt() );
}


void MotorControl::SetMotorSpeed( const char* name, motorspeed_t speed )
{
  DBG_OUTPUT_PORT.println( "MotorControl::SetMotorSpeed - motor: " + String( name ) + " speed: " + String( speed ) );
  
  std::map< const char*, IMotorDevice* >::iterator it = std::find_if( m_Motors.begin(), m_Motors.end(), 
  [ name ](std::pair< String, IMotorDevice* > const& motor) 
  { 
    return motor.first.equalsIgnoreCase( name ); 
  });
  
  if ( it != m_Motors.end() )
  {
    (*it).second->SetSpeed( speed );

    DBG_OUTPUT_PORT.println( "Motor " + String(name) + " speed set to " + String(speed) );
  }
  else
  {
    DBG_OUTPUT_PORT.println("Setting motor speed on a motor that cannot be found!");
  } 
}


void MotorControl::AppendMotorJSON( String& json )
{
  //DBG_OUTPUT_PORT.println( "Appending motors to JSON" );
  //DBG_OUTPUT_PORT.println( "There are " + String(m_Motors.size()) + " motors." );
  
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
    json += "\"" + String( motor.first ) + "\"";
    json += ", ";
    json += "\"speed\":";
    json += "\"" + String( speed ) + "\"";
    json += "}";
    
    bFirst = false;
  }
    
  json += "]";

  //DBG_OUTPUT_PORT.println( json );
}


void MotorControl::Update()
{
  for( std::pair<const char* const, IMotorDevice*> motor : m_Motors )
  {
    motor.second->ApplySpeed();
  }
}

