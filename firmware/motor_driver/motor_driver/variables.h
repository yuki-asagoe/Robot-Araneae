#define DEBUG_PRINT true

const int I2C_ADDRESS=0x0000001;
const int PIN_LED1=2;
const int PIN_LED2=4;

enum class DrivingMode{
  Stop,Brake,Drive,ReverseDrive,
};