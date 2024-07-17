// If define this, Motor 0 cannot be used because the D0, D1 pins are used for Serial Communication.
#define DEBUG_PRINT

const int I2C_ADDRESS=55;

enum class DrivingMode{
  Stop,Brake,Drive,ReverseDrive,
};

enum class LimitType{
  None,Drive,Reverse,All
};

struct MotorLimit{
  int motorID;
  LimitType type;
};

//入力に使えるのは9本
MotorLimit MotorLimits[]={
  {0,LimitType::Drive},//1
  {0,LimitType::Reverse},//2
  {1,LimitType::Drive},//3
  {1,LimitType::Reverse},//4
  {2,LimitType::Drive},//5
  {2,LimitType::Reverse},//6
  {3,LimitType::Drive},//7
  {3,LimitType::Reverse},//8
  {-1,LimitType::None},//9
};