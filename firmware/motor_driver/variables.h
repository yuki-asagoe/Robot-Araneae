// If define this, Motor 0 cannot be used because the D0, D1 pins are used for Serial Communication.
#define DEBUG_PRINT

const int I2C_ADDRESS=55;
const int PIN_LED1=2;
const int PIN_LED2=4;

enum class DrivingMode{
  Stop,Brake,Drive,ReverseDrive,
};