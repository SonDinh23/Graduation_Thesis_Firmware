#include <ESP32Servo.h>
#include <Adafruit_INA3221.h>
#include <Wire.h>

#define adrIna1 0x41
#define adrIna2 0x40
#define SvoThumbverPin 16
#define SvoThumbhorPin 17
#define SvoIndexPin 5
#define SvoMiddlePin 18
#define SvoRingPin 19
#define minUs 500           // Value Init of Servo
#define maxUs 2500
#define minUs_Thumbver 1500     // Range Control of thumb finger
#define maxUs_Thumbver_Grip0 650
#define maxUs_Thumbver_Grip1 700
#define maxUs_Thumbver_Grip2 825
#define minUs_Thumbhor 625    // Range Control of Little finger
#define maxUs_Thumbhor 2500
#define minUs_Index 625     // Range Control of Index finger
#define maxUs_Index 2500
#define minUs_Mid 625       // Range Control of Middle finger
#define maxUs_Mid 2500
#define minUs_Ring 700      // Range Control of Ring finger
#define maxUs_Ring 2500
#define minStep_Count 0     // general Step Control
#define maxStep_Count 1500
#define SpeedUS_LV1 1
#define SpeedUS_LV2 2
#define SpeedUS_LV3 3
#define Current_Threshold_Thumbver -200
#define TimeDELAYThreshold 100

enum FingerName {
  THUMBVER = 0,
  THUMBHOR = 1,
  INDEX = 2,
  MIDDLE = 3,
  RING = 4
  
};

enum Mode{
  Close,
  Open,
  Hold,
  Change
};

struct StateFinger{
  bool Flag_threshold = false;
  bool Flag_stateGrip_1 = true;
  bool Flag_stateGrip_2 = true;
  float valueAngleCurrent;
  uint8_t state = 0;
  uint8_t delayThreshold ;
};



String inputString = "";
bool stringComplete = false;


uint16_t StepControl = 0;
uint8_t stepUs = 2;
uint8_t grip = 0;
uint8_t stepChange;
bool flag_ChangeGrip = true;
int maxUs_Thumbver[3] = {maxUs_Thumbver_Grip0, maxUs_Thumbver_Grip1, maxUs_Thumbver_Grip2};
int Current_Threshold[3][5] = {
                                {-500,-200,-150,-200,-200},
                                {-500,-300,-210,-300,-250},
                                {-500,-320,-280,-350,-300}
};

Adafruit_INA3221 INA1;
Adafruit_INA3221 INA2;
Servo servos[5];

enum Mode mode = Open;
enum Mode lastMode = Hold;
struct StateFinger Fingers[5];


void setup() {
  // init Serial (Can skip if not use)
  Serial.begin(115200);
  // init INA3221
  InitINA();
  // init Servo
  InitServo();

}

void loop() {
  // put your main code here, to run repeatedly:
  Receive();
  if (stringComplete) {
      inputString.trim(); // xóa khoảng trắng, xuống dòng
      processCommand(inputString);  // xử lý lệnh
      inputString = "";
      stringComplete = false;
    }

  

  switch(mode){
    case Close:
    Serial.println("Close:");
    if(StepControl < maxStep_Count)  StepControl = StepControl + stepUs;
    flag_ChangeGrip = true;
    break;
    case Open:
    Serial.println("open:");
    if(StepControl > minStep_Count)  StepControl = StepControl - stepUs;
    Fingers[1].Flag_threshold = Fingers[2].Flag_threshold = Fingers[3].Flag_threshold = Fingers[4].Flag_threshold = false;
    flag_ChangeGrip = true;
    break;
    case Hold:
    Serial.println("hold:");
    flag_ChangeGrip = true;
    Fingers[1].Flag_threshold = Fingers[2].Flag_threshold = Fingers[3].Flag_threshold = Fingers[4].Flag_threshold = false;
    break;
    case Change:
    Serial.println("Change Grip");
    if(flag_ChangeGrip){
      grip = (grip+1)%3;
      if(grip == 0){
        stepChange = stepUs;
      }
      else if(grip == 1){
        Fingers[4].state =  3;
      }
      else if(grip == 2){
        Fingers[3].state = 3;
      }
      flag_ChangeGrip = false;
      mode = lastMode;
      lastMode = Change;
    }
    break;
  }
  Control_Grip();
  if(Fingers[1].Flag_threshold && Fingers[2].Flag_threshold 
      && Fingers[3].Flag_threshold && Fingers[4].Flag_threshold) mode = Hold;
}

void Control_Grip(){
    int CurrentThumbver = INA1.getCurrentAmps(0)* 1000;
    int CurrentThumbhor = INA1.getCurrentAmps(1)* 1000;
    int CurrentIndex = INA1.getCurrentAmps(2)* 1000;
    int CurrentMid = INA2.getCurrentAmps(0)* 1000;
    int CurrentRing = INA2.getCurrentAmps(1)* 1000;
    Serial.print(CurrentThumbver);
    Serial.print(", ");
    Serial.print(CurrentThumbhor);
    Serial.print(", ");
    Serial.print(CurrentIndex);
    Serial.print(", ");
    Serial.print(CurrentMid);
    Serial.print(", ");
    Serial.print(CurrentRing);
    Control_Finger(0,maxUs_Thumbver[grip], minUs_Thumbver, minStep_Count, maxStep_Count/2, CurrentThumbver);
    Control_Finger(1,maxUs_Thumbhor, minUs_Thumbhor, maxStep_Count/2, maxStep_Count, CurrentThumbhor);
    Control_Finger(2,maxUs_Index, minUs_Index, minStep_Count, maxStep_Count, CurrentIndex);
    Control_Finger(3,maxUs_Mid, minUs_Mid, minStep_Count, maxStep_Count, CurrentMid);
    Control_Finger(4,maxUs_Ring, minUs_Ring, minStep_Count, maxStep_Count, CurrentRing);
    
}


void Control_Finger(int Finger,int max, int min, int min_IN, int max_IN,  int Current_sensor){
  int StepControl_inFcn = constrain(StepControl, min_IN, max_IN);
  float AngleControl = mapFloat(StepControl_inFcn,min_IN,max_IN,min,max);
  switch(Fingers[Finger].state){
    case 0:
    Fingers[Finger].valueAngleCurrent = AngleControl;
    servos[Finger].writeMicroseconds(Fingers[Finger].valueAngleCurrent);
    if( Current_sensor  <= Current_Threshold[stepUs-1][Finger]){
      Fingers[Finger].state = 1;
      Fingers[Finger].Flag_threshold = true;
    }
    break;
    case 1:
    if(Current_sensor <= Current_Threshold[stepUs-1][Finger] - 50){
      Fingers[Finger].valueAngleCurrent = Fingers[Finger].valueAngleCurrent <= min ? min : Fingers[Finger].valueAngleCurrent - stepUs;
      servos[Finger].writeMicroseconds(Fingers[Finger].valueAngleCurrent);
    }
    else Fingers[Finger].state = 2;
    break;
    case 2:
    if(mode == Open && AngleControl <=  Fingers[Finger].valueAngleCurrent) Fingers[Finger].state = 0;
    else servos[Finger].writeMicroseconds(Fingers[Finger].valueAngleCurrent);
    break;
    case 3:
    if(grip > 0)
    {
      if(Fingers[Finger].Flag_stateGrip_1)
      {
        if(StepControl <= minStep_Count){
          Fingers[Finger].valueAngleCurrent = min;
          Fingers[Finger].Flag_stateGrip_1 = false;
          break;
        }
        Fingers[Finger].valueAngleCurrent = Fingers[Finger].valueAngleCurrent <= min ? min : Fingers[Finger].valueAngleCurrent - stepUs;
      }
      else{
        Fingers[Finger].valueAngleCurrent = Fingers[Finger].valueAngleCurrent >= max ? max : Fingers[Finger].valueAngleCurrent + stepUs;
      }
    }
    else{
        if(StepControl <= minStep_Count){
          Fingers[Finger].Flag_stateGrip_2 = false;
      }
      if(!Fingers[Finger].Flag_stateGrip_2){
        if(Fingers[Finger].valueAngleCurrent <= AngleControl){
          Fingers[Finger].Flag_stateGrip_1 = true;
          Fingers[Finger].Flag_stateGrip_2 = true;
          Fingers[Finger].valueAngleCurrent = AngleControl;
          Fingers[Finger].state = 0;
          break;
        }
        Fingers[Finger].valueAngleCurrent = Fingers[Finger].valueAngleCurrent <= min ? min : Fingers[Finger].valueAngleCurrent - stepUs;
        
      }
    }
    servos[Finger].writeMicroseconds(Fingers[Finger].valueAngleCurrent);
    break;
  }

}





void processCommand(String command) {
  if(command == "O"){
    lastMode = mode;
    mode = Open;
  }
  else if(command == "C"){
    lastMode = mode;
    mode = Close;
  }
  else if(command == "H"){
    lastMode = mode;
    mode = Hold;
  }
  else if(command == "D"){
    lastMode = mode;
    mode = Change;
  }
  else if(command == "S"){
    stepUs = stepUs % 3 + 1 ;
  }
  else Serial.println("nhập O C H");
}

void Receive(){
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}


float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// Initial

void InitINA() {
  if (!INA1.begin(0x41, &Wire)) { // can use other I2C addresses or buses
    Serial.println("Failed to find INA1 chip");
    while (1)
      delay(10);
  }
  if (!INA2.begin(0x40, &Wire)) { // can use other I2C addresses or buses
    Serial.println("Failed to find INA2 chip");
    while (1)
      delay(10);
  }
  Serial.println("INA3221 Found!");

  INA1.setAveragingMode(INA3221_AVG_16_SAMPLES);
  INA2.setAveragingMode(INA3221_AVG_16_SAMPLES);

  for (uint8_t i = 0; i < 3; i++) {
    INA1.setShuntResistance(i, 0.1);
     if(i<2) INA2.setShuntResistance(i, 0.1);
  }
}


// basic
void InitServo() {
  servos[0].attach(SvoThumbverPin,minUs,maxUs);
  servos[1].attach(SvoThumbhorPin,minUs,maxUs);
  servos[2].attach(SvoIndexPin,minUs,maxUs);
  servos[3].attach(SvoMiddlePin,minUs,maxUs);
  servos[4].attach(SvoRingPin,minUs,maxUs);
  
}


