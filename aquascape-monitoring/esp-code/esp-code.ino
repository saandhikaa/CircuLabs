// // // MOVE JUMPER DOWN FOR PROGRAMMING // // //

#define BLYNK_TEMPLATE_ID "XXXXXXXXXXXX"
#define BLYNK_DEVICE_NAME "Aquascape"
#define BLYNK_AUTH_TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "queen";
char pass[] = "snowhite";

void setup() {
  Blynk.begin(auth, ssid, pass);
  
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V1, 0);
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V4, 0);
  Blynk.virtualWrite(V5, 0);
  Blynk.virtualWrite(V11,0);

  Serial.begin(9600);
  Serial.print("READY");
}

void loop() {
  Blynk.run();
  receiving();
}

void receiving(){
  String data = "";
  while(Serial.available() > 0){
    delay(10);
    char d = Serial.read();
    data += d;
  }
  if (data.length() > 1){
    if (splitting(data, ',', 0) == "FAN")      Blynk.virtualWrite(V0, splitting(data, ',', 1).toInt()); else
    if (splitting(data, ',', 0) == "HEATER")   Blynk.virtualWrite(V1, splitting(data, ',', 1).toInt()); else
    if (splitting(data, ',', 0) == "LED")      Blynk.virtualWrite(V2, splitting(data, ',', 1).toInt()); else
    if (splitting(data, ',', 0) == "WATERIN")  Blynk.virtualWrite(V3, splitting(data, ',', 1).toInt()); else
    if (splitting(data, ',', 0) == "WATEROUT") Blynk.virtualWrite(V4, splitting(data, ',', 1).toInt()); else
    if (splitting(data, ',', 0) == "WLEVEL")   Blynk.virtualWrite(V6, splitting(data, ',', 1).toFloat()); else
    if (splitting(data, ',', 0) == "WTEMPR")   Blynk.virtualWrite(V7, splitting(data, ',', 1).toFloat()); else
    if (splitting(data, ',', 0) == "FTIME1")   Blynk.virtualWrite(V8, splitting(data, ',', 1)); else  //  if error, use this -> if (splitting(data, ',', 0) == "FTIME1"){  Blynk.virtualWrite(V8, splitting(data, ',', 1)); if (splitting(data, ',', 1) == "-") Blynk.virtualWrite(V10,0);} else
    if (splitting(data, ',', 0) == "FTIME2")   Blynk.virtualWrite(V9, splitting(data, ',', 1)); else
    if (splitting(data, ',', 0) == "FTODAY")   Blynk.virtualWrite(V10,splitting(data, ',', 1).toInt());
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


//BLYNK_WRITE = READ CHANGES FROM BLYNK SERVER

BLYNK_WRITE(V0) {
  Serial.print("FAN," + String(param.asInt()));
}

BLYNK_WRITE(V1) {
  Serial.print("HEATER," + String(param.asInt()));
}

BLYNK_WRITE(V2) {
  Serial.print("LED," + String(param.asInt()));
}

BLYNK_WRITE(V3) {
  Serial.print("WATERIN," + String(param.asInt()));
}

BLYNK_WRITE(V4) {
  Serial.print("WATEROUT," + String(param.asInt()));
}

BLYNK_WRITE(V5) {
  Serial.print("FEEDER,FEED");
  delay(1000);
  Blynk.virtualWrite(V5,0);
}

BLYNK_WRITE(V11) {
  Serial.print("MANUAL," + String(param.asInt()));
  String data = "";
  for (int s = 0; s <= 100; s++){
    while (Serial.available() > 0){
      delay(10);
      char d = Serial.read();
      data += d;
    }
    if (data.length() > 1) if (splitting(data, ',', 0) == "CONFIRMED") break;
    if (s == 99) Blynk.virtualWrite(V11,0);
    delay(50);
  }
}
