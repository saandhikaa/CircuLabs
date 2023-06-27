#define motor  4
#define IR0    2
#define IR1   A0
#define IR2   A1
#define IR3   A2
#define IR4   A3
#define S0    13
#define S1    12
#define S2    11
#define S3    10
#define Sout   8

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C (0x27,16,2);
#include <Servo.h>
  Servo servo1;
  Servo servo2;

int Rfrequency = 0;
int Gfrequency = 0;
int Bfrequency = 0;

int red    = 0;
int green  = 0;
int blue   = 0;
int yellow = 0;

int ex;

void setup() {
  pinMode(motor, OUTPUT);
  pinMode(IR0, INPUT);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(Sout, INPUT);
  
  digitalWrite(motor,HIGH);
  servo1.attach(5);//pin servo1
  servo2.attach(3);//pin servo2
  servo1.write(0);
  servo2.write(0);
  
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  delay(1000);

  // Setting output frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  delay(3000);

  digitalWrite(motor,LOW);
  lcd.clear();
  lcd_total();
  Serial.println("Start-------------");
  Serial.println("   ");
}

void loop() {
  ColorDetect();
  ColorSelect();
}

//============================fungsi============================//

void ColorDetect(){
  // Setting red filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  Rfrequency = pulseIn(Sout, LOW);// Reading the output frequency
  Rfrequency = map(Rfrequency, 25,72,255,0);//Remaping the value of the frequency to the RGB Model of 0 to 255
  delay(10);

  // Setting Green filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  Gfrequency = pulseIn(Sout, LOW);// Reading the output frequency
  Gfrequency = map(Gfrequency, 30,90,255,0);//Remaping the value of the frequency to the RGB Model of 0 to 255
  delay(10);

  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  Bfrequency = pulseIn(Sout, LOW);// Reading the output frequency
  Bfrequency = map(Bfrequency, 25,70,255,0);//Remaping the value of the frequency to the RGB Model of 0 to 255
  delay(10);

  limit();
}



void ColorSelect(){
    
  if(Rfrequency>= 180 && Rfrequency<= 250 && Gfrequency>= 180 && Gfrequency<= 255 && Bfrequency>= 0 && Bfrequency<= 80){
//  R:180-250  G:180-255  B:0-30
    lcd.clear();
    lcd.setCursor(0,0);
    lcd_rgb();
    lcd.setCursor(0,1);
    lcd.print("YELLOW");
    
    data_SerialMonitor();
    Serial.println("Detected..!   - Yellow");
    yellowbox();
    ex=0;
    delay(1000);
  }else
 
  if(Rfrequency>= 170 && Rfrequency<= 255 && Gfrequency<= 0 && Bfrequency<= 0){
//  R:170-255  G:0  B:0
    lcd.clear();
    lcd.setCursor(0,0);
    lcd_rgb();
    lcd.setCursor(0,1);
    lcd.print("RED");
    
    data_SerialMonitor();
    Serial.println("Detected..!   - Red");
    redbox();
    ex=0;
    delay(1000);
  }else
  
  if(Rfrequency<= 0 && Gfrequency>= 110 && Gfrequency<= 170 && Bfrequency<= 0){
//  R:0  G:110-170  B:0  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd_rgb();
    lcd.setCursor(0,1);
    lcd.print("GREEN");
    
    data_SerialMonitor();
    Serial.println("Detected..!   - Green");
    greenbox();
    ex=0;
    delay(1000);
  }else
  
  if(Rfrequency<= 0 && Gfrequency>= 50 &&Gfrequency<= 90 && Bfrequency>= 140 && Bfrequency<= 255){
//  R:0  G:50-90  B:140-255
    lcd.clear();
    lcd.setCursor(0,0);
    lcd_rgb();
    lcd.setCursor(0,1);
    lcd.print("BLUE");
    
    data_SerialMonitor();
    Serial.println("Detected..!   - Blue");
    bluebox();
    ex=0;
    delay(1000);
  }else
 
  if(Rfrequency> 0 || Gfrequency> 0 || Bfrequency> 0){
    ex++;
    data_SerialMonitor();
    Serial.print("Retrying..(");
    Serial.print(ex);
    Serial.print(")");
    if(ex>=8){
      lcd.clear();
      lcd.print("Color Undefined");
      Serial.println(" - Color Undefined!");
      delay(3000);
    }
  }else{    
    lcd.clear();
    lcd_total();
    ex=0;
  }
}



void yellowbox(){
  delay(500);
  servo1.write(103);
  for(ex=113;ex<170;ex+=10){
    servo1.write(ex);
    delay(400);
    if(digitalRead(IR1) == LOW){
      ex=173;
    }
  }
  servo1.write(0);
  delay(100);
  yellow++;
  lcd_total();
}

void redbox(){
  delay(1000);
  servo1.write(48);
  ex=1;
  while(ex==1){
    if(digitalRead(IR2) == LOW){
      servo1.write(0);
      ex=0;
    }
    delay(100);
  }
  red++;
  lcd_total();
}

void greenbox(){
  delay(3400);
  servo2.write(108);
  for(ex=118;ex<170;ex+=10){
    servo2.write(ex);
    delay(400);
    if(digitalRead(IR3) == LOW){
      ex=178;
    }
  }
  servo2.write(0);
  delay(100);
  green++;
  lcd_total();
}

void bluebox(){
  delay(4000);
  servo2.write(53);
  ex=1;
  while(ex==1){
    if(digitalRead(IR4) == LOW){
      servo2.write(0);
      ex=0;
    }
    delay(100);
  }
  blue++;
  lcd_total();
}



void data_SerialMonitor(){
  Serial.println("  ");
  
  Serial.print(" R:");
  Serial.print(Rfrequency);
  if(Rfrequency<100)Serial.print(" ");
  if(Rfrequency<10) Serial.print(" ");
  
  Serial.print("  G:");
  Serial.print(Gfrequency);
  if(Gfrequency<100)Serial.print(" ");
  if(Gfrequency<10) Serial.print(" ");
  
  Serial.print("  B:");
  Serial.print(Bfrequency);
  if(Bfrequency<100)Serial.print(" ");
  if(Bfrequency<10) Serial.print(" ");
  
  Serial.print("  |  ");
}

void lcd_total(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("R:");lcd.print(red);
  lcd.setCursor(8,0);
  lcd.print("Y:");lcd.print(yellow);
  lcd.setCursor(0,1);
  lcd.print("B:");lcd.print(blue);
  lcd.setCursor(8,1);
  lcd.print("G:");lcd.print(green);
}

void lcd_rgb(){
  lcd.print("R:");lcd.print(Rfrequency);
  lcd.print(" G:");lcd.print(Gfrequency);
  lcd.print(" B:");lcd.print(Bfrequency);
}

void limit(){
  if(Rfrequency<0)Rfrequency=0;
  if(Gfrequency<0)Gfrequency=0;
  if(Bfrequency<0)Bfrequency=0;
  if(Rfrequency>255)Rfrequency=255;
  if(Gfrequency>255)Gfrequency=255;
  if(Bfrequency>255)Bfrequency=255;
}
