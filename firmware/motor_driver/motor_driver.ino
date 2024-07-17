#include <Wire.h>
#include "variables.h"

const int PIN_LED1=2;
const int PIN_LED2=4;

const int INPUT_PINS_D[]={
  8,7,13,14,15,16,17
};
const int INPUT_PINS_A[]={
  A6,A7
};

// num : 0 ~ 8
bool system_input_is_on(int num){
  if(num<0||num>=9){
    return false;
  }
  if(num<7){
    return digitalRead(INPUT_PINS_D[num]) == HIGH;
  }else{
    return analogRead(INPUT_PINS_A[num-7]) > 384;
  }
}
void system_setup_input(int num){
  if(num<0||num>=9){
    return;
  }
  if(num<7){
    pinMode(INPUT_PINS_D[num],INPUT_PULLUP);
  }else{
    pinMode(INPUT_PINS_A[num-7],INPUT_PULLUP);
  }
}
int system_get_input_pin(int num){
  if(num<0||num>=9){
    return -1;
  }
  if(num<7){
    return INPUT_PINS_D[num];
  }else{
    return INPUT_PINS_A[num-7];
  }
}

struct MotorLimitPin{
  int input_pin_index;
  LimitType type;
};
//ここを抽象にする元気はない
class MotorStopper{
  private:
    int limits_length;
    MotorLimitPin* limits;
    bool allowed[4]={};
  public:
    MotorStopper();
    MotorStopper(int motorID);
    bool allow(DrivingMode mode); 
    void update();
};
MotorStopper::MotorStopper(){
  this->limits_length=0;
  this->limits=NULL;
}
MotorStopper::MotorStopper(int motorID){
  this->limits_length=0;
  for(int i=0;i<9;i++){
    if(MotorLimits[i].motorID==motorID && MotorLimits[i].type!=LimitType::None)this->limits_length++;
  }
  this->limits=(MotorLimitPin*)malloc(sizeof(MotorLimitPin)*limits_length);
  int current;
  for(int i=0;i<9;i++){
    if(MotorLimits[i].motorID==motorID && MotorLimits[i].type!=LimitType::None){
      MotorLimitPin pin={i,MotorLimits[i].type};
      this->limits[current++]=pin;
      system_setup_input(i);
    }
  }
}
bool MotorStopper::allow(DrivingMode mode){
  switch(mode){
    case DrivingMode::Stop:
    case DrivingMode::Brake:
      return true;
    case DrivingMode::Drive:
      return this->allowed[(int)LimitType::Drive] && this->allowed[(int)LimitType::All];
    case DrivingMode::ReverseDrive:
      return this->allowed[(int)LimitType::Reverse] && this->allowed[(int)LimitType::All];
  }
  return false;
}
void MotorStopper::update(){
  for(int i=0;i<4;i++){
    this->allowed[i]=true;
  }
  for(int i=0;i<this->limits_length;i++){
    if(system_input_is_on(this->limits[i].input_pin_index)){
      this->allowed[(int)(this->limits[i].type)]=false;
    }
  }
}

class MotorDriver{
  protected: 
    int pin_in1,pin_in2;
    DrivingMode current_state;
    MotorStopper stopper;
    MotorDriver(int pin_in1,int pin_in2,MotorStopper stopper);
  public:
    bool checkStopper();
    void updateStopper();
    virtual void changeState(DrivingMode mode,unsigned int speed);
};
MotorDriver::MotorDriver(int pin_in1,int pin_in2,MotorStopper stopper){
  pinMode(pin_in1,OUTPUT);
  pinMode(pin_in2,OUTPUT);
  this->pin_in1=pin_in1;
  this->pin_in2=pin_in2;
  digitalWrite(pin_in1,LOW);
  digitalWrite(pin_in2,LOW);
  this->current_state=DrivingMode::Stop;
  this->stopper=stopper;
}
void MotorDriver::changeState(DrivingMode mode,unsigned int speed){
  this->current_state=mode;
}
bool MotorDriver::checkStopper(){
  if(!(this->stopper.allow(this->current_state))){
    this->changeState(DrivingMode::Stop,0);
    return true;
  }
  return false;
}
void MotorDriver::updateStopper(){
  this->stopper.update();
}
//----

class AnalogMotorDriver:public MotorDriver{
  public:
    AnalogMotorDriver(int pin_in1,int pin_in2,MotorStopper stopper);
    void changeState(DrivingMode mode,unsigned int speed);
};
AnalogMotorDriver::AnalogMotorDriver(int pin_in1,int pin_in2,MotorStopper stopper):MotorDriver(pin_in1,pin_in2,stopper){}

void AnalogMotorDriver::changeState(DrivingMode mode,unsigned int speed){
  if(!(this->stopper.allow(mode))){
    // Stop
    MotorDriver::changeState(DrivingMode::Stop,0);
    digitalWrite(pin_in1,LOW);
    digitalWrite(pin_in2,LOW);
    return;
  }
  MotorDriver::changeState(mode,speed);
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
      analogWrite(pin_in2,speed);
      break;
    case DrivingMode::ReverseDrive:
      analogWrite(pin_in1,speed);
      digitalWrite(pin_in2,LOW);
      break;
  }
}

//----

class DigitalMotorDriver:public MotorDriver{
  public:
    DigitalMotorDriver(int pin_in1,int pin_in2,MotorStopper stopper);
    void changeState(DrivingMode mode,unsigned int _);
};
DigitalMotorDriver::DigitalMotorDriver(int pin_in1,int pin_in2,MotorStopper stopper):MotorDriver(pin_in1,pin_in2,stopper){}

void DigitalMotorDriver::changeState(DrivingMode mode,unsigned int _){
  if(!(this->stopper.allow(mode))){
    // Stop
    MotorDriver::changeState(DrivingMode::Stop,0);
    digitalWrite(pin_in1,LOW);
    digitalWrite(pin_in2,LOW);
    return;
  }
  MotorDriver::changeState(mode,_);
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
  DigitalMotorDriver(0,1,MotorStopper(0))
};
AnalogMotorDriver a_drivers[]={
  AnalogMotorDriver(5,3,MotorStopper(1)),
  AnalogMotorDriver(6,9,MotorStopper(2)),
  AnalogMotorDriver(10,11,MotorStopper(3))
};
unsigned long lastI2CTimeStamp=0;
unsigned long lastStopperTimeStamp=0;

void onI2CReceive(int length){
  lastI2CTimeStamp=millis();
  #ifdef DEBUG_PRINT
    Serial.print("I2C Received:");
  #endif
  while(Wire.available()){
    unsigned int data=Wire.read();
    int motorID=(data & 0b00110000) >> 4;
    DrivingMode mode=(DrivingMode)(data & 0b00000011);
    if(mode == DrivingMode::Stop || mode == DrivingMode::Brake){
      if(motorID==0){
        d_drivers[motorID].changeState(mode,0); 
      }else{
        a_drivers[motorID-1].changeState(mode,0); 
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
        a_drivers[motorID-1].changeState(mode,speed); 
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

  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);
  Wire.begin(I2C_ADDRESS);

  #ifdef DEBUG_PRINT
    Serial.println("DEBUG PRINT : ON / Motor 0 is diabled.");
  #elif
    Serial.println("DEBUG PRINT : OFF / Motor 0 is available");
  #endif

  Serial.println("Initialization Finished : Ready..>");

  #ifndef DEBUG_PRINT
    Serial.end();
  #endif
}

void loop() {
  long int time = millis();
  long int timePassedFromI2CReceived=lastI2CTimeStamp-time;
  long int timePassedFromStopperCheck=lastStopperTimeStamp-time;

  if(timePassedFromI2CReceived>500){
    digitalWrite(PIN_LED1,HIGH);
  }else{
    digitalWrite(PIN_LED1,LOW);
  }
  if(timePassedFromStopperCheck>100){
    bool stopperWorking=false;
    lastStopperTimeStamp=time;
    for(int i=0;i<1;i++){
      d_drivers[i].updateStopper();
      stopperWorking|=d_drivers[i].checkStopper();
    }
    for(int i=0;i<3;i++){
      a_drivers[i].updateStopper();
      stopperWorking|=a_drivers[i].checkStopper();
    }
    if(stopperWorking){
      digitalWrite(PIN_LED2,HIGH);
    }else{
      digitalWrite(PIN_LED2,LOW);
    }
  }
}
