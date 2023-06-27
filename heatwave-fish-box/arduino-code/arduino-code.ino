#include <HX711.h>
  HX711 scale;
  #define W0 0        // berat nyata minimal
  #define W1 9800     // berat nyata maksimal
  #define S0 46500    // skala minimal
  #define S1 169000   // skala maksimal
  
  float WT;
  int cut;
  long target_gr;
  
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C (0x27,16,2);
  #define ndisplay 4
  int lstatus = 1;
  
#include <DHT.h>
  DHT dht(4, DHT11);
  float humidity, temperature;
  
#define BUZZ_ON digitalWrite(8,LOW)
#define BUZZ_OFF digitalWrite(8,HIGH)

#define B1 digitalRead(A3)
#define B2 digitalRead(A2)

const int relay_pin[3] = {5,6,7};
boolean rstatus[3];

boolean drying = false;

long wait = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("\nSTART\n");
  
  scale.begin(2,3);
  
  dht.begin();

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  
  pinMode(8,OUTPUT);
  BUZZ_OFF;

  pinMode(A2,INPUT);
  pinMode(A3,INPUT);

  for (int n = 0; n < 3; n++){
    pinMode(relay_pin[n],OUTPUT);
    relay(n,0);
  }
  
  delay(1000);
}

void loop() {
  autocut();
  autofan();
  menu();
  
  if (millis() >= wait) {
    read_dht();
    lcd_show();
    wait = millis() + 300;
  }
}

//////////////////////////////////////////////////////////////////////////

// ________________________________________________Tampilkan status di LCD
void lcd_show(){
  float weight0, weight, weight_gr;
  
  weight_gr = read_weight_gr();
  weight = weight_gr / 1000;
  if (!drying) weight0 = weight;
  
  long process = map(weight_gr, target_gr*2, target_gr, 0, 100);
  if (process <=0) process = 0;
  Serial.println("Proses : " + String(process));
  
  String line[ndisplay][2] = {
    {"Processing...", String(process) + "%"},
    {"Real  : " + String(weight,1) + " kg", drying ? "Target: " + String(WT,1) + " kg" : " "},
    {"Temperature", String(temperature,1) + " C"},
    {"Humidity", String(humidity,0) + " %"}
  };

  lcd.clear();
  lcd.setCursor(0,0); lcd.print(line[lstatus][0]);
  lcd.setCursor(0,1); lcd.print(line[lstatus][1]);
}

int Button(){
  int result = 0;
  if (B1){
    BUZZ_ON;
    while (B1);
    delay(100);
    BUZZ_OFF;
    result = 1;
    delay(200);
  }
  else if(B2){
    BUZZ_ON;
    while(B2);
    delay(100);
    BUZZ_OFF;
    result = 2;
    delay(200);
  }
  
  return result;
}

void read_dht(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  //Serial.println("Suhu : " + String(temperature,1) + " C | Kelembaban : " + String(humidity,1) + " %");
  if (isnan(humidity) || isnan(temperature)){
    humidity = 0;
    temperature = 0;
    return;
  }
}

long read_weight_gr(){
  long weight;
  if (scale.is_ready()) {
    long reading = scale.read();
    weight = map(reading,S0,S1,W0,W1) - 1050;
    if (weight < 0) weight = 0;
    Serial.println("Scale : " + String(reading) + " | Weight : " + String(weight) + " | S.Target : " + String(target_gr));
  } else {
    weight = 0;
  }
  return weight;
}

void relay(int num, int value){
  digitalWrite(relay_pin[num], value);
  rstatus[num] = value;
  Serial.println("Relay " + String(num) + " : " + String(value));
}

void menu(){
  int b = Button();
  if(b == 1) {
    lstatus++;
    if (lstatus >= ndisplay) lstatus = 0;
    if (!drying && lstatus == 0) lstatus = 1;
  } 
  else if(b == 2) {
    if(drying){
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Finish ?");
      lcd.setCursor(0,1); lcd.print("YES           NO");
      while(1){
        int f = Button();
        if (f == 1){ finished(); break; }
        else if (f == 2) break;
      }
    }
    else started();
  }
}

void autofan(){
  read_dht();

  if (temperature >= 65 && (!rstatus[1] || rstatus[0])){
    relay(0,0);
    relay(1,1);
  }else if (temperature <= 60 && (!rstatus[0] || rstatus[1])){
    relay(0,1);
    relay(1,0);
  }
}

void autocut(){
  float weight_gr = read_weight_gr();
  float weight = weight_gr / 1000;
  if (drying && weight_gr <= target_gr + 200){
    Serial.println("limit : " + String(weight_gr));
    cut++;
  }
  else cut = 0;

  if (cut >= 10) {
    finished();
    long waits = millis();
    while (1){
      Serial.println("weight = " + String(weight,1) + " | " + String(WT,1));
      
      if (waits <= millis()){
        BUZZ_ON;  delay(300);
        BUZZ_OFF; delay(200);
        BUZZ_ON;  delay(300);
        BUZZ_OFF;
        waits = millis() + 1000;
      }
      
      int b = Button();
      if (b == 1) break;
    }
  }
}

void started(){
  float wtmp = 0;
  long stmp = 0;
  for (int tmp = 0; tmp < 10; tmp++) {
    wtmp += read_weight_gr();
    delay(100);
  }
  WT = wtmp / 20000;
  target_gr = wtmp / 20;
  drying = true;
  lstatus = 0;
  
  lcd.clear();
  lcd.print("Target: " + String(WT,1) + " kg");
  
  delay(2000);
}

void finished(){
  drying = false;
  WT = 0;
  lstatus = 1;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Finished!");
  lcd.setCursor(0,1); lcd.print("OK");

  delay(1000);
}