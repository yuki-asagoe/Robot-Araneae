#include <Wire.h>
#include "variables.h"

class MotorDriver{
  protected: 
    int pin_in1,pin_in2;
    MotorDriver(int pin_in1,int pin_in2);
    virtual void changeState(DrivingMode mode,unsigned int speed);
};
MotorDriver::MotorDriver(int pin_in1,int pin_in2){
  pinMode(pin_in1,OUTPUT);
  pinMode(pin_in2,OUTPUT);
  this->pin_in1=pin_in1;
  this->pin_in2=pin_in2;
}
void MotorDriver::changeState(DrivingMode mode,unsigned int speed){}
//----

class AnalogMotorDriver:MotorDriver{
  public:
    AnalogMotorDriver(int pin_in1,int pin_in2);
    void changeState(DrivingMode mode,unsigned int speed);
};
AnalogMotorDriver::AnalogMotorDriver(int pin_in1,int pin_in2):MotorDriver(pin_in1,pin_in2){}

void AnalogMotorDriver::changeState(DrivingMode mode,unsigned int speed){
  switch(mode){
    case DrivingMode::Stop:
      digitalWrite(pin_in1,LOW);
      digitalWrite(pin_in2,LOW);
      break;
    case DrivingMode::Brake:
      digitalWrite(pin_in1,HIGH);
      digitalWrite(pin_in2,HIGH);
      break;
    case DrivingMode::Drive:
      digitalWrite(pin_in1,LOW);
      analogWrite(pin_in1,speed);
      break;
    case DrivingMode::ReverseDrive:
      analogWrite(pin_in1,speed);
      digitalWrite(pin_in2,LOW);
      break;
  }
}

//----

class DigitalMotorDriver:MotorDriver{
  public:
    DigitalMotorDriver(int pin_in1,int pin_in2);
    void changeState(DrivingMode mode,unsigned int _);
};
DigitalMotorDriver::DigitalMotorDriver(int pin_in1,int pin_in2):MotorDriver(pin_in1,pin_in2){}

void DigitalMotorDriver::changeState(DrivingMode mode,unsigned int _){
  switch(mode){
    case DrivingMode::Stop:
      digitalWrite(pin_in1,LOW);
      digitalWrite(pin_in2,LOW);
      break;
    case DrivingMode::Brake:
      digitalWrite(pin_in1,HIGH);
      digitalWrite(pin_in2,HIGH);
      break;
    case DrivingMode::Drive:
      digitalWrite(pin_in1,LOW);
      digitalWrite(pin_in2,HIGH);
      break;
    case DrivingMode::ReverseDrive:
      digitalWrite(pin_in1,HIGH);
      digitalWrite(pin_in2,LOW);
      break;
  }
}

//----

DigitalMotorDriver d_drivers[]={
  DigitalMotorDriver(0,1)
};
AnalogMotorDriver a_drivers[]={
  AnalogMotorDriver(5,3),
  AnalogMotorDriver(6,9),
  AnalogMotorDriver(10,11)
};
unsigned long lastI2CTimeStamp=0;

void onI2CReceive(int length){
  lastI2CTimeStamp=millis();
  while(Wire.available()){
    unsigned int data=Wire.read();
    int motorID=(data & 0b00110000) >> 4;
    DrivingMode mode=(DrivingMode)(data & 0b00000011);
    if(mode == DrivingMode::Stop || mode == DrivingMode::Brake){
      if(motorID==0){
        d_drivers[motorID].changeState(mode,0); 
      }else{
        a_drivers[motorID].changeState(mode,0); 
      }
      
      #ifdef DEBUG_PRINT
        Serial.print("change status of ");
        Serial.print(motorID);
        Serial.print(" motor to");
        Serial.println((int)mode);
      #endif
    }else{
      if(!Wire.available()) break;
      unsigned int speed=Wire.read();
      if(motorID==0){
        d_drivers[motorID].changeState(mode,speed); 
      }else{
        a_drivers[motorID].changeState(mode,speed); 
      }
      
      #ifdef DEBUG_PRINT
        Serial.print("change status of ");
        Serial.print(motorID);
        Serial.print(" motor to");
        Serial.print((int)mode);
        Serial.print(" / speed : ");
        Serial.println(speed);
      #endif
    }
  }
}
void onI2CRequest(void){
  lastI2CTimeStamp=millis();
  Wire.write(I2C_ADDRESS);
  #ifdef DEBUG_PRINT
    Serial.println("I2C Request Received: Response Sent");
  #endif
}

void setup() {
  Serial.begin(9600);
  Serial.println("- Araneae Motor Driver -");
  Serial.print("I2C Address:");
  Serial.println(I2C_ADDRESS,HEX);
  pinMode(PIN_LED1,OUTPUT);
  pinMode(PIN_LED2,OUTPUT);

  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);

  Serial.println("Initialization Finished : Ready..>");
}

void loop() {
  long int time = millis();
  long int timePassedFromI2CReceived=lastI2CTimeStamp-time;

  if(timePassedFromI2CReceived<500){
    digitalWrite(PIN_LED1,HIGH);
  }else{
    digitalWrite(PIN_LED1,LOW);
  }
}
