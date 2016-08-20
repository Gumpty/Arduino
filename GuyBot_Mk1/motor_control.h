
#include <map>

typedef int motorspeed_t;

class IMotorDevice
{
  public:

  virtual void SetSpeed( motorspeed_t speed ) = 0;
};

class PCA9685MotorDevice : public IMotorDevice
{
  //IMotorDevice start
  virtual void SetSpeed( motorspeed_t speed );
  //IMotorDevice end

  uint8_t forwardpin;
  uint8_t reversepin;
};

class MotorControl
{
public:
  void AddMotor( const char* name, IMotorDevice& motor );
  void RemoveMotor( const char* name );

  void SetMotorSpeed( const char* name, motorspeed_t speed );

  void AppendMotorJSON( String& json );

private:

  std::map< const char*, IMotorDevice* > m_Motors;
};
