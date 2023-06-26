// Initialization FIFO function_______________________
// https://github.com/GyverLibs/GyverFIFO
#include <GyverFIFO.h>
  GyverFIFO <int, 16> WaitList;

// Initialization RFID________________________________
// https://github.com/miguelbalboa/rfid
#include <SPI.h>
#include <MFRC522.h>
  #define RST_PIN 49
  #define SS_PIN  53
  MFRC522 CScanner(SS_PIN, RST_PIN);
  String Administrator = "B0 19 0F 54";
  boolean NeedScan, Unlock;
  
// Initialization LCD with I2C________________________
// https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C (0x27,16,2);

// Initialization 5V Stepper control__________________
#include <Stepper.h>
  const int DoorStep = 2048 + 128;
  Stepper Door = Stepper(2048, 48, 44, 46, 42);

// Initialization 12V Stepper control_________________
const int stepperPin_Dir  = 11,
          stepperPin_Step = 10,
          stepperPin_En   = 9;

const int StepTon   = 1500,   // delayMicrosecond On Time
          StepToff  = 150,    // delayMicrosecond Off Time
          FloorStep = 1495;   // Step each floor

// Set stepper lift direction
#define Lift_UP     digitalWrite(stepperPin_Dir, HIGH)
#define Lift_DOWN   digitalWrite(stepperPin_Dir, LOW)

///////////////////////////////////////////////////////////////////

const int NLimit[2]     = {12,40};                    // lift, door
const int NButton[2][3] = {{47,43,39}, {35,33,31}};   // Out,  In
const int NLED[2][3]    = {{45,41,37}, {29,27,25}};   // Out,  In
const int LaserPin      = A0;

const int Switchstate   = 0,
          Threshold     = 960;

boolean DoorClosing = false,
        LiftRun     = false,
        asktoopen   = true;

int  Current_Floor,
     Prev_Push = 3;

//////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  SPI.begin();
  CScanner.PCD_Init();
  Door.setSpeed(12);
  
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  
  pinMode(NLimit[0],INPUT);
  pinMode(NLimit[1],INPUT);
  pinMode(A0, INPUT);

  pinMode(stepperPin_En, OUTPUT);
  pinMode(stepperPin_Dir, OUTPUT);
  pinMode(stepperPin_Step, OUTPUT);
  digitalWrite(stepperPin_En, LOW);
  
  for (int s = 0; s < 2; s++){
    for (int a = 0; a < 3; a++){
      pinMode(NButton[s][a],INPUT);
      pinMode(NLED[s][a],OUTPUT); delay(100);
      digitalWrite(NLED[s][a],HIGH);
    }
  }

  Door_Action(0);  delay(500);
  Lift_UP;
  while(!Limit(0)) Lift_Move();
  Current_Floor = 2;

  lcd.print("Lantai 3");
}

void loop() {
  Button();
  if (WaitList.available()){
    int floor = WaitList.read();
    lcd.clear(); 
    lcd.print("Lantai " + String(floor+1));
    GoFloor(floor);
  }
}

//////////////////////////////////////////////////////////////////

//________________________________________ Function to scan button
void Button(){
  for (int s = 0; s < 2; s++){    // out in button
    for (int a = 0; a < 3; a++){  // floor button

      if (digitalRead(NButton[s][a]) == Switchstate){
        if (a == Current_Floor && DoorClosing) asktoopen = true;
        if ((a != Prev_Push && a != Current_Floor) || (!WaitList.available() && !LiftRun)){
          if (!LiftRun){
            while (digitalRead(NButton[s][a]) == Switchstate);
            delay(100);
          }
          if (s == 1 && a == 2) NeedScan = true;
          Prev_Push = a;
          WaitList.write(a);
          digitalWrite(NLED[s][a],LOW);
        }
      }

    }
  }
}

//_____________________________________ Function to scan RFID card
void Scanning(){
  if (!CScanner.PICC_IsNewCardPresent()) return;
  if (!CScanner.PICC_ReadCardSerial())   return;
  
  String content= "";
  byte letter;
  
  for (byte i = 0; i < CScanner.uid.size; i++){
     content.concat(String(CScanner.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(CScanner.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  if (content.substring(1) == Administrator){
    Unlock = true;
  } else {
    lcd.setCursor(0,1);
    lcd.print("Unverified      ");
  }
}

//______________________________________ Function to move the lift
void GoFloor(int value){
  int Left_Floor = Current_Floor;
  int Lift_Step = FloorStep * (abs(Left_Floor-value));
  long starttime = millis();
  LiftRun = true;
  Current_Floor = value;
  wait(1000);

  // go to F1
  if (value == 0){
    Lift_DOWN;
    for (int s = 0; s < Lift_Step; s++) Lift_Move();
  } 

  // go to F2
  if (value == 1){
    if (Left_Floor-1 < 0) Lift_UP;
    else                  Lift_DOWN;
    for (int s = 0; s < Lift_Step; s++) Lift_Move();
  } 

  // go to F3
  if (value == 2){
    long startwait = millis() + 10000;
    if (NeedScan){
      lcd.setCursor(0,1);
      lcd.print("Scanning..");
      while (NeedScan){
        Scanning();
        Button();
        if (startwait <= millis() && WaitList.available()){
          digitalWrite(NLED[1][2], HIGH);
          break;
        }
        if (Unlock){
          Unlock = false;
          NeedScan = false;
          lcd.setCursor(0,1);
          lcd.print("Verified        ");
          break;
        }
      }
    }
    
    if (!NeedScan){
      Lift_UP;
      while(!Limit(0)) Lift_Move();
    }
  }

  // open the door
  LiftRun = false;
  if (!NeedScan || value != 2){    
    digitalWrite(NLED[0][value], HIGH);
    digitalWrite(NLED[1][value], HIGH);
    
    wait(1000);  Door_Action(1);
    wait(5000);  Door_Action(0);
  }
}

//___________________________ Function to Open/Cloce the lift door
void Door_Action(int value){
  if (value){   //________________  OPEN
    for (int s = 0; s < (DoorStep/4); s++){
      Door.step(-4);
      Button();
    }
  }
  
  else{   //______________________  CLOSE
    DoorClosing = true;
    int count = 0;
    for (int s = 0; s < (DoorStep/2); s++){
      if (Limit(1)) break;
      Door.step(2);
      Button();
      Serial.println(analogRead(LaserPin));
      
      // if laser detector active
      if (analogRead(LaserPin) > Threshold){
        count++;
        delayMicroseconds(10);
      } else count = 0;
      
      // opening...
      if (count > 8 || asktoopen){
        wait(500);
        asktoopen = false;
        DoorClosing = false;
        for (;s > 0; s--, Door.step(-2)) Button();
        wait(3000);
        DoorClosing = true;
      }
    }
    
    while(!Limit(1)){
      Door.step(1);
      Button();
    }
    
    DoorClosing = false;
  }
}

//_________ Function to use delay and scanning button at same time
void wait(long value){
  long a = millis() + value;
  while (a > millis()) Button();
}

//________________________________ Function to detect limit switch
boolean Limit(int num){
  if (!digitalRead(NLimit[num])) return true;
  else return false;
}

//____________ Function one step Stepper motor and scanning button
void Lift_Move(){
  digitalWrite(stepperPin_Step, HIGH); delayMicroseconds(StepTon);
  digitalWrite(stepperPin_Step, LOW);  delayMicroseconds(StepToff);
  Button();
}
