
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <map>

typedef int motorspeed_t;

class IMotorDevice
{
  public:

  virtual motorspeed_t GetSpeed() = 0;
  virtual void SetSpeed( motorspeed_t speed ) = 0;
};

class MotorDevice_PCA9685_L9110S : public IMotorDevice
{
  public:
  
  static const int k_MaxSpeed = 4096;
  static const int k_Forward = HIGH;
  static const int k_Reverse = LOW;
  
  //IMotorDevice start
  virtual motorspeed_t GetSpeed();
  virtual void SetSpeed( motorspeed_t speed );
  //IMotorDevice end

  motorspeed_t m_Speed;
  uint8_t m_DirectionPin;
  uint8_t m_SpeedPin;
};

class MotorControl : public JSONProvider
{
public:
  void AddMotor( const char* name, IMotorDevice& motor );
  void RemoveMotor( const char* name );

  void SetMotorSpeed( const char* name, motorspeed_t speed );

  // JSONProvider
  virtual void AppendMotorJSON( String& json );

private:

  Adafruit_PWMServoDriver m_PWMDriver;
  std::map< const char*, IMotorDevice* > m_Motors;
};
