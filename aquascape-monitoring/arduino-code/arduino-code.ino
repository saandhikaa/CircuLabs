#include <EEPROM.h>
  // NOTE:  addr 0    : last active date
  //        addr 1    : last active month
  //        addr 2    : last active year[2]
  //        addr 100  : total feed today
  //        addr 101  : feed at 08.00
  //        addr 102  : feed at 20.00
  
#include <Wire.h>

// https://github.com/PaulStoffregen/OneWire
#include <OneWire.h>

// https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <DallasTemperature.h>
  #define ONE_WIRE_BUS 10
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature stempr(&oneWire);
  #define wtemprmin 30
  #define wtemprmax 32
  
#include <SoftwareSerial.h>
  SoftwareSerial esp(2,3);

// https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C (0x27,16,2);

#include <DS3231.h>
  DS3231 rtc(SDA, SCL);
  Time t;

#include <Servo.h>
  Servo myservo;
  #define servopin 9
  
#define trig_pin 12
#define echo_pin 11

#define buzz_on digitalWrite(A2,HIGH);
#define buzz_off digitalWrite(A2,LOW);

#define push analogRead(A3)

#define ndisplay 9

float wlevel, wtempr;
long waits[2]; // lcd show, send sensor data

const int relay_pin[5] = {8,7,6,5,4};
boolean rstatus[5] = {0,0,0,0,0};
int lstatus = 0;
boolean manual = false;
boolean blynk_manual = false;

/////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  Serial.println("\n================= START =================\n");

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("CONNECTING..");
    
  esp.begin(9600);
  
  rtc.begin();
  Serial.println(" Tanggal  : " + String(rtc.getDateStr()));
  Serial.println(" Jam      : " + String(rtc.getTimeStr()) + "\n");

  stempr.begin();
  
  pinMode(echo_pin, INPUT);
  pinMode(trig_pin, OUTPUT);
  digitalWrite(trig_pin, LOW);

  pinMode(A3, INPUT);
  pinMode(A2,OUTPUT);
  buzz_off;
  
  for (int r = 0; r < 5; r++){
    pinMode(relay_pin[r],OUTPUT);
    relay(r,rstatus[r]);
  }

  pinMode(servopin,OUTPUT);
  myservo.attach(servopin);
  myservo.write(30);
  delay(1000);
  myservo.detach();
  
  String tmp = "";
  while(1){
    while (esp.available() > 0){
      delay(10);
      char d = esp.read();
      tmp += d;
    }
    if (tmp == "READY"){
      Serial.println("\nESP ready");
      break;
    }
    if (button() == 1){
      Serial.println("\nESP skip");
      lstatus--;
      break;
    }
  }
}

void loop() {
  receiving();
  day_check();
  update_time();
  button();
  if (!blynk_manual) automatic();
  
  ////________________________________________________________________ update lcd display
  if (millis() > waits[0]){
    lcd_show(lstatus);
    waits[0] = millis() + 300;
  }

  ////_________________________________________________________ send sensor's data to esp
  if (millis() > waits[1]){
    water_level();
    esp.print("WLEVEL," + String(wlevel) + ",M");
    delay(100);
    
    water_temperature();
    esp.print("WTEMPR," + String(wtempr) + ",M");
    delay(100);
    
    waits[1] = millis() + 2000;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////

void lcd_show(int value){
  if (value == 1) water_temperature();
  if (value == 2) water_level();
  
  String line[ndisplay][2] = {
    { rtc.getDateStr(), rtc.getTimeStr() },
    { "Temperature", String(wtempr,1) + " C" },
    { "Water Level", String(wlevel,1) + " cm" },
    { "Fan Cooler", rstatus[0] ? "ON" : "OFF" },
    { "Heater", rstatus[1] ? "ON" : "OFF" },
    { "LED", rstatus[2] ? "ON" : "OFF" },
    { "Water In", rstatus[3] ? "ON" : "OFF" },
    { "Water Out", rstatus[4] ? "ON" : "OFF" },
    { "Fish Feeder", "    "}
  };
    
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(line[value][0]);
  lcd.setCursor(0,1); lcd.print(line[value][1]);
}

void receiving(){
  String data = "";
  while(esp.available() > 0){
    delay(10);
    char d = esp.read();
    data += d;
  }
  if (data.length() > 1){
    if (splitting(data, ',', 0) == "FAN") relay(0, splitting(data, ',', 1).toInt());
    if (splitting(data, ',', 0) == "HEATER") relay(1, splitting(data, ',', 1).toInt());
    if (splitting(data, ',', 0) == "LED") relay(2, splitting(data, ',', 1).toInt());
    if (splitting(data, ',', 0) == "WATERIN") relay(3, splitting(data, ',', 1).toInt());
    if (splitting(data, ',', 0) == "WATEROUT") relay(4, splitting(data, ',', 1).toInt());
    if (splitting(data, ',', 0) == "FEEDER") feeder();
    if (splitting(data, ',', 0) == "MANUAL"){
      blynk_manual = splitting(data, ',', 1).toInt();
      esp.print("CONFIRMED,M");
      delay(100);
    }
  }
}

void water_level(){
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  
  // speed of sound = 344 m/s = 34400 cm/s = 0.0344 cm/us
  // pulseIn() in microsecond(us)
  
  long duration = pulseIn(echo_pin, HIGH);
  wlevel = 22.4 - duration * 0.0344 / 2;
  if (wlevel < 0) wlevel = 0;
}

void water_temperature(){
  stempr.requestTemperatures();
  wtempr = stempr.getTempCByIndex(0);
}

int button(){
  int result[2] = {0,0};
  int range[2][2] = {{950,1050}, {800,900}};
  int value = push;

  if (value > 500){
    long wait = millis() + 200;
    buzz_on;
    while (value > 500){
      value = push;
      result[0] = result[1];
      for (int s = 2; s > 0; s--)
        if (value > range[s-1][0] && value < range[s-1][1]) 
          result[1] = abs(s - 3);    // ganti tombol s atau s-3
      
      delay(10);
      if (millis() > wait) buzz_off;
    }
    buzz_off;
    delay(100);
  }
  if (result[0]) Serial.println("Tombol: " + String(result[0]));

  if (!manual){
    if (result[0] == 1) {
      lstatus++;
      if (lstatus >= ndisplay) lstatus = 0;
    }
    else if (result[0] == 2 && lstatus >= 3 && lstatus <= 9){
      manual = true;
      control_manual(lstatus);
    }
  }
  else 
    return result[0];
}

void control_manual(int value){
  lcd.setCursor(15,1);
  lcd.print("M");
  int num = value - 3;
  long wait = millis();
  long countdown = millis() + 20000;
  
  while (manual){
    receiving();
    
    if (millis() >= wait){
      lcd_show(value);
      lcd.setCursor(15,1);
      lcd.print("M");
      wait = millis() + 300;
    }

    int a = button();
    if (a == 1){
      countdown = millis() + 20000;
      if (value == 8) {
        lcd.setCursor(0,1);
        lcd.print("Feed");
        feeder();
      } else {
        relay(num, 1-rstatus[num]);
        if (num == 0) esp.print("FAN," + String(rstatus[0]) + ",M");
        if (num == 1) esp.print("HEATER," + String(rstatus[1]) + ",M");
        if (num == 2) esp.print("LED," + String(rstatus[2]) + ",M");
        if (num == 3) esp.print("WATERIN," + String(rstatus[3]) + ",M");
        if (num == 4) esp.print("WATEROUT," + String(rstatus[4]) + ",M");
        delay(100);
      }
    }
    else if (a == 2) manual = false;
  }
  if (millis() >= countdown) manual = false;
  
}

void relay(int num, int value){
  digitalWrite(relay_pin[num], 1-value);
  rstatus[num] = value;
  Serial.println("Relay " + String(num) + " : " + String(rstatus[num] ? "ON" : "OFF") );
}

void feeder(){
  myservo.attach(servopin);
  myservo.write(140); delay(2000);  //open
  myservo.write(30);                //close
  Serial.println("Feed!");
  
  String value = "";
  t = rtc.getTime();
  value += t.hour < 10 ? "0" : "" ;
  value += t.hour;
  value += t.min < 10 ? ":0" : ":" ;
  value += t.min;
  value += " WIB,M";
  
  if (EEPROM.read(101) == 0){
    esp.print("FTIME1," + value + ",M");
    EEPROM.write(101,1);
  } else esp.print("FTIME2," + value + ",M");
  delay(500);

  const int count = EEPROM.read(100) + 1;
  EEPROM.write(100,count);
  esp.print("FTODAY," + String(count) + ",M");
  delay(100);
  
  myservo.detach();
}

void automatic(){
  ////_________________________________________________________________ Water temperature
  water_temperature();
  if (wtempr >= wtemprmax && (!rstatus[0] || rstatus[1])){
    Serial.println("\nSuhu air tinggi : " + String(wtempr,1) + " C");
    relay(0,1);
    relay(1,0);
    esp.print("FAN," + String(rstatus[0]) + ",M"); delay(100);
    esp.print("HEATER," + String(rstatus[1]) + ",M"); delay(100);
  } else if (wtempr <= wtemprmin && (rstatus[0] || !rstatus[1])){
    Serial.println("\nSuhu air rendah : " + String(wtempr,1) + " C");
    relay(0,0);
    relay(1,1);
    esp.print("FAN," + String(rstatus[0]) + ",M"); delay(100);
    esp.print("HEATER," + String(rstatus[1]) + ",M"); delay(100);
  } else if (wtempr < wtemprmax-0.5 && wtempr > wtemprmin+0.5 && (rstatus[0] || rstatus[1])){
    Serial.println("\nSuhu air normal : " + String(wtempr,1) + " C");
    relay(0,0);
    relay(1,0);
    esp.print("FAN," + String(rstatus[0]) + ",M"); delay(100);
    esp.print("HEATER," + String(rstatus[1]) + ",M"); delay(100);
  }
  
  ////_______________________________________________________________________________ LED
  if (t.hour >= 8 && t.hour < 20){
    if (!rstatus[2]){
      relay(2,1);
      esp.print("LED," + String(rstatus[2]) + ",M"); delay(100);
    }
  }else if (rstatus[2]){
    relay(2,0);
    esp.print("LED," + String(rstatus[2]) + ",M"); delay(100);
  }
  
  ////____________________________________________________________________________ Feeder
  if (t.hour >= 8) 
    if (EEPROM.read(101) == 0) feeder();
    else if (t.hour >= 20 && EEPROM.read(102) == 0){
      feeder();
      EEPROM.write(102,1);
    }
}

void day_check(){
  t = rtc.getTime();

  if (EEPROM.read(0) != t.date || EEPROM.read(1) != t.mon || EEPROM.read(2) != t.year - 2000){

    EEPROM.write(0,t.date);
    EEPROM.write(1,t.mon);
    EEPROM.write(2,t.year-2000);

    EEPROM.write(100,0);
    EEPROM.write(101,0);
    EEPROM.write(102,0);
    
    esp.print("FTODAY,0,M"); delay(100);
    esp.print("FTIME1,-,M"); delay(100);
    esp.print("FTIME2,-,M"); delay(100);

    Serial.println("\nHari Baru");
  }
}

void update_time(){
  if (lstatus == 0){
    int datain[3];
    String data = "";
    char d;
    while (Serial.available() > 0){
      delay(10);
      d = Serial.read();
      data += d;
    }
    if (data.length() > 1){
      Serial.println("\n------------------------------------");
      Serial.println("---------- Waktu Saat Ini ----------\n");
      Serial.println(" Tanggal  : " + String(rtc.getDateStr()));
      Serial.println(" Jam      : " + String(rtc.getTimeStr()));

      Serial.print("\n Atur ulang waktu ? (1/0)      .. ");
      data = "";
      while(1){
        while(Serial.available() > 0) {
          d = Serial.read();
          data += d;
        }
        if (data.length() > 0){
          Serial.println(data);
          break;
        }
      }
      if (data != "1") {
        Serial.println("\n-------------- Keluar --------------");
        goto finish;
      }
      
      Serial.println("\n------------ Atur Waktu ------------\n");
      Serial.print(" Tanggal  : ");
      while(1){
        data = "";
        while(Serial.available() > 0){
          delay(10);
          d = Serial.read();
          data += d;
        }
        if (data.length() > 1){
          rtc.setDate(splitting(data, '.', 0).toInt(), splitting(data, '.', 1).toInt(), splitting(data, '.', 2).toInt());
          Serial.println(rtc.getDateStr());
          break;
        }
      }

      Serial.print(" Jam      : ");
      while(1){
        data = "";
        while(Serial.available() > 0){
          delay(10);
          d = Serial.read();
          data += d;
        }
        if (data.length() > 1){
          rtc.setTime(splitting(data, ':', 0).toInt(), splitting(data, ':', 1).toInt(), splitting(data, ':', 2).toInt());
          Serial.println(rtc.getTimeStr());
          Serial.println("\n------------ Tersimpan! ------------");
          break;
        }
      }
      finish:
      Serial.println("------------------------------------\n");
    }
  }
}

String splitting(String data, char separator, int index){
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
 
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  } 
 
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
