#include <Wire.h>

#include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C (0x27,20,4);

#include <DS3231.h>
  DS3231  rtc(SDA, SCL);

#include <EEPROM.h>
#include <GravityTDS.h>
  GravityTDS gravityTds;

#include <SPI.h>
#include <SD.h>
  File myFile;

//// ________________________________________ Arduino Pin Initialization
#define GravityTDSPin A0    // PPM
#define LDRPin        A1    // Lux
#define RelayPin1     A3    // Water Pump
#define RelayPin2     A4    // LED
#define StartPin      4     // Push Button (Start New)
#define BuzzerPin     7     // Buzzer
#define SDCSPin       53    // SD Card

//// _______________________________________ Declaration Global Variabel
int     Day;                          // Today Monitoring
float   lux, ppm;                     // lux and ppm value
String  WPStatus = "OFF";             // Water Pump: Current status
boolean LEDStatus;                    // LED: Current status
boolean NotRecord[20];                // SwitchDay(); NewFiles(); ResetNR();
    /*  NotRecord[0]    = Switch day
     *  NotRecord[1]    = water pump logging
     *  NotRecord[2-10] = 9x logging every day
     *  NotRecord[11]   = logging every 1 min
     */

int    RelayTime[2][2] = {{2,5},         // Switching (mins), On time (sec)
                         {7,17}};        // On time: Start, end
                          
float  LimitLux[2]     = {1520, 1750};
float  LimitPPM[3][2]  = {{500,600},
                         {1000,1050},
                         {1100,1400}};

String LogFile[14]     = {"logging1.txt",                                    // 0
                          "first1.txt","first2.txt","first3.txt","first.txt",// 1-4
                          "last1.txt", "last2.txt", "last3.txt", "last.txt", // 5-8
                          "logging2.txt",                                    // 9
                          "logging3.txt","tmp1.txt","tmp2.txt",              // 10-12
                          "outlog.txt"};                                     // 13

int    LogTime[9][2]   = {{ 7,0},{ 7,30},{ 8,0},
                          {11,0},{11,30},{12,0},
                          {16,0},{16,30},{17,0}};

////////////////////////////////////////////////////////////////////////////////////

void setup(){
  Serial.begin(115200);
  rtc.begin();
  
  lcd.init();
  lcd.backlight();
  
  gravityTds.setPin(GravityTDSPin);
  gravityTds.setAref(5.0);
  gravityTds.setAdcRange(1024);
  gravityTds.begin();

  pinMode(LDRPin,    INPUT);
  pinMode(StartPin,  INPUT);
  pinMode(SDCSPin,   OUTPUT);
  pinMode(RelayPin1, OUTPUT); digitalWrite(RelayPin1, HIGH);
  pinMode(RelayPin2, OUTPUT); digitalWrite(RelayPin2, HIGH);
  pinMode(BuzzerPin, OUTPUT); digitalWrite(BuzzerPin, HIGH);
  delay(1000);
  
  Serial.println("\n===============================================");
  Serial.println("==================== SETUP ====================");
  Serial.println("Current Date & Time: \n" + String(rtc.getDOWStr()) + ", " + String(rtc.getDateStr()) + " | " + String(rtc.getTimeStr()));
  
  if (GetSplit(1,2) < 2021) FixTimeRTC();//_________________________________________

  Serial.print("\nRTC is ready to use.");
  lcd.setCursor(0,0); lcd.print("RTC is ready");
  delay(400); 

  if (!SD.begin()){
    Serial.print("\nWaiting for SD Card");
    lcd.setCursor(0,1); lcd.print("SD Card Not Found");
    lcd.setCursor(0,2); lcd.print(" waiting");
    Buzzer(2);
    while(!SD.begin()){
      for (int a = 0; a < 3; a++){ 
        lcd.print("."); 
        delay(400);
      }
      Serial.print(".");
      lcd.setCursor(8,2); lcd.print("   ");
      lcd.setCursor(8,2);
    }
    lcd.setCursor(8,1); lcd.print("is ready  ");
    lcd.setCursor(0,2); lcd.print("            ");
    Buzzer(2);
  } else {
    delay(400);
    lcd.setCursor(0,1); lcd.print("SD Card is ready");
  }
  
  Serial.println("\nSD Card is ready to use.");
  delay(1500); lcd.clear();
  
  PreLogging();

  // checking LED time
  if (GetSplit(2,0) >= RelayTime[1][0] && GetSplit(2,0) < RelayTime[1][1]) LEDStatus = false;
  else LEDStatus = true;
      
  lcd.clear();
  Serial.println("\n==================== START ====================\n");
}

////////////////////////////////////////////////////////////////////////////////////

void loop() {
  lux = GetLux();
  ppm = GetPPM();
  
  SwitchDay();
  WrittingDataLog();
  
  WaterPump();
  AutoLED();
  
  ShowLCD(1);
  
  if (GetWarn1() || GetWarn2()) Buzzer(1); else 
  delay(400);

  if (PushButton()) ShowLCD(3);
}

////////////////////////////////////////////////////////////////////////////////////

void ShowLCD(int a){
  switch (a){
    
    case 1: //// __________________________ Daily Monitoring
      lcd.setCursor(0,3);   lcd.print("Water Pump " + WPStatus);
      
      lcd.setCursor(0,2);   lcd.print("Intensity: " + String(lux,1) + "lux   ");
      
      lcd.setCursor(0,1);   lcd.print("TDS: " + String(ppm,1) + "ppm   ");
      lcd.setCursor(16,1);  if (Day > 99) lcd.print("D.out"); else
                            if (Day < 10) lcd.print(" D." + String(Day));
                            else          lcd.print("D."  + String(Day));
                            
      lcd.setCursor(0,0);   lcd.print(rtc.getTimeStr());
      lcd.setCursor(10,0);  lcd.print(rtc.getDateStr());
    break;

    case 2: //// __________________________ Setup Report RTC
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("RTC Time ERROR !");
      lcd.setCursor(0,1); lcd.print("1.Connect to PC");
      lcd.setCursor(0,2); lcd.print("2.Serial Monitor");
      lcd.setCursor(0,3); lcd.print("3.Set 115200 baud");
      Buzzer(2);
    break;

    case 3: //// __________________ Setup for new monitoring
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Today is Day " + String(Day));
      lcd.setCursor(0,1); lcd.print("Reset Day?"); delay(800);
      lcd.setCursor(0,2); lcd.print("> Reset");
      
      int a[4] = {1, 1, 0, 7}; 
      // loop, s.now, s.prev, timer(s)
      
      while (1){
        if (PushButton()){
          Day  = 0;
          lcd.clear(); lcd.print("Resetting..");
            delay(500);
          NewFiles();
          break;
        }
        
        if (a[2] != a[1]){
          lcd.setCursor(0,3); lcd.print("  Skip..(" + String(a[3]) + ")");
          a[2] = a[1];
          a[3]--;
          if (a[3] < 0){ // Skipped..
            lcd.setCursor(0,2); lcd.print("  Reset");
            lcd.setCursor(0,3); lcd.print("> Skip");  
              delay(700); 
            lcd.clear(); lcd.print("Skipped..");
              Buzzer(2); 
              delay(700);
            ResetSelectedNR(); 
            break;
          }
        }

        a[1] = GetSplit(2,2);
      }

      // checking LED time
      if (GetSplit(2,0) >= RelayTime[1][0] && GetSplit(2,0) < RelayTime[1][1]) LEDStatus = false;
      else LEDStatus = true;
      
      // checking water pump time
      if (GetSplit(2,1)%2 == 0 && GetSplit(2,2) <= 5) WPStatus = "ON ";
      else WPStatus = "OFF";
    break;
    
  }
}

//// Re:Create New files for data logging __________________________________________
void NewFiles(){
  for (int a = 0; a < 20; a++)  NotRecord[a] = true;
  for (int a = 0; a < 14; a++){
    if (SD.exists(LogFile[a])) SD.remove(LogFile[a]);
  }
  
  // _______________________________________________ Main log
  myFile = SD.open(LogFile[0], FILE_WRITE);
  if (myFile) {
    myFile.println("No.,Tanggal,Jam,Intensitas Cahaya,Jumlah Zat Padat Terlarut (TDS)");
    myFile.close();
    lcd.setCursor(0,1); lcd.print("Log 1 created"); delay(400);
  }
  DataLogging(2);

  // _______________________________ First day monitoring log
  myFile = SD.open(LogFile[4], FILE_WRITE);
  if (myFile) {
    myFile.println("Tanggal,Jam");
    myFile.print(rtc.getDateStr()); myFile.print(",");
    myFile.print(rtc.getTimeStr());
    myFile.close();
    lcd.setCursor(0,2); lcd.print("Log 2 created."); delay(100);
  }
  
  for (int a = 0; a < 3; a++){
    myFile = SD.open(LogFile[a+1], FILE_WRITE);
    if (myFile) {
      myFile.print(String(GetSplit(1,a)));
      myFile.close();
    }
    delay(100); lcd.print(".");
  }

  // ________________________________________ Last online log  
  lcd.setCursor(0,3); lcd.print("Log 3 created");
  for (int a = 5; a < 9; a++){
    myFile = SD.open(LogFile[a], FILE_WRITE);
    myFile.close();
    lcd.print("."); delay(100);
  }

  // _________________________________________ water pump log
  myFile = SD.open(LogFile[9], FILE_WRITE);
  if (myFile) {
    myFile.println(",Tanggal,Jam");
    myFile.close();
  }
  
  // ______________________________________________ third log
  myFile = SD.open(LogFile[10], FILE_WRITE);
  if (myFile) {
    myFile.println(",LED,Pompa Air");
    myFile.close();
  }
  for (int a = 11; a <= 12; a++){ // WPTmp, LEDTmp
    myFile = SD.open(LogFile[a], FILE_WRITE);
    if (myFile){
      myFile.print("0");
      myFile.close();
    }
  }
  
  // ____________________________________________ Offline log
  myFile = SD.open(LogFile[13], FILE_WRITE);
  if (myFile) {
    myFile.println("Tanggal (off),Jam (off),Tanggal (on),Jam (on)");
    myFile.close();
  }
  
  Serial.println("Monitoring  : Resets!\n");
  delay(1000); lcd.clear();
}

//// Check for available previous data log _________________________________________
void PreLogging(){
  if (SD.exists(LogFile[4])){
    DataLogging(4); // Offline log
    
    // read first on
    String FirstOn[3];
    for (int s = 0; s < 3; s++){
      myFile = SD.open(LogFile[s+1]); 
      if (myFile){
        while (myFile.available()) FirstOn[s] += (char)myFile.read();
        myFile.close();
      }
    }
    
    Day = Counting(FirstOn[0].toInt(),FirstOn[1].toInt(),FirstOn[2].toInt(),GetSplit(1,0),GetSplit(1,1),GetSplit(1,2));
    
    if (Day > 0) {Buzzer(2);ShowLCD(3);}
    else ResetSelectedNR();
    
  } else NewFiles();
}

//// Data Logging Function__________________________________________________________
void DataLogging(int a){
  switch (a){
    
    case 1:   //_________________________________ Last online monitoring
      SD.remove(LogFile[8]);
      myFile = SD.open(LogFile[8], FILE_WRITE);
      if (myFile) {
        myFile.println("Tanggal,Jam");
        myFile.print(rtc.getDateStr()); myFile.print(",");
        myFile.print(rtc.getTimeStr());
        myFile.close();
        Serial.println("\nLast Online : Reset!");
      }
      
      for (int a = 0; a < 3; a++){
        SD.remove(LogFile[a+5]);
        myFile = SD.open(LogFile[a+5], FILE_WRITE);
        if (myFile) {
          if (a == 2) myFile.print(String(GetSplit(1,0)));
          else        myFile.print(String(GetSplit(2,a)));
          myFile.close();
        }
      }
    break;

    case 2:   //_________________________________________ Main Logging 1
      myFile = SD.open(LogFile[0], FILE_WRITE);
      if (myFile) {
        if (NotRecord[0]) {
          myFile.print("D" + String(Day));
          myFile.print("," + String(rtc.getDateStr()));
          NotRecord[0] = false;
        }else myFile.print(",");
        myFile.print("," + String(rtc.getTimeStr()));
        myFile.print("," + String(lux,1));
        myFile.println("," + String(ppm,1));
        myFile.close();
        Serial.println("Logging     : 1.Update!");
      }
    break;
      
    case 3:   //___________________________________ Water pump logging 2
      myFile = SD.open(LogFile[9], FILE_WRITE);
      if (myFile){
        if (NotRecord[1]) {
          myFile.print("D" + String(Day));
          myFile.print("," + String(rtc.getDateStr()));
          NotRecord[1] = false;
        }else myFile.print(",");
        myFile.println("," + String(rtc.getTimeStr()));
        myFile.close();
        Serial.println("Logging     : 2.Update!");
      }
    break;

    case 4:   //________________________________________ Offline logging
      // read last on
      String LastOn[3];
      for (int s = 0; s < 3; s++){
        myFile = SD.open(LogFile[s+5]); 
        if (myFile){
          while (myFile.available()) LastOn[s] += (char)myFile.read();
          myFile.close();
        }
      }

      int mm;
      String mmmm = "";
      if (GetSplit(1,0) >= LastOn[2].toInt())
        mmmm = Split(rtc.getDateStr(), '.', 1);
      else{
        mm = GetSplit(1,1) - 1;
        if (mm < 10) mmmm += "0";
        mmmm += String(mm);
      }
      
      myFile = SD.open(LogFile[13], FILE_WRITE);
      if (myFile){
        String datelog4 = LastOn[2] + "." + mmmm + "." + String(GetSplit(1,2));
        String timelog4 = LastOn[0] + ":" + LastOn[1] + ":00";
        myFile.print(datelog4 + ",");
        myFile.print(timelog4 + ",");
        myFile.print(String(rtc.getDateStr()) + ",");
        myFile.println(String(rtc.getTimeStr()));
        myFile.close();
        Serial.println("\nMonitoring  : Off Time Update!");
      }
    break;

  }
}

//// Call logging function 1 & 2 ___________________________________________________
void WrittingDataLog(){
  
  // _________________________ Logging every 1 mins
  if (GetSplit(2,2) <= 5 && NotRecord[11]){
    DataLogging(1); // last online log
    delay(10);

    // _____________________ Logging 9 times everyday
    for (int a = 0; a < 9; a++){
      if (GetSplit(2,0) == LogTime[a][0] && GetSplit(2,1) == LogTime[a][1]){
        if (NotRecord[a+2]){
          DataLogging(2); // main log: logging 1.
          NotRecord[a+2] = false;
        }
      }
    }

    // tmp led log
    if (GetSplit(2,0) >= RelayTime[1][0] && GetSplit(2,0) < RelayTime[1][1]){
      String tmplog5 = "";
      myFile = SD.open(LogFile[11]);
      while (myFile.available()) tmplog5 += (char)myFile.read();
      myFile.close();
      delay(10);
      SD.remove(LogFile[11]);
      myFile = SD.open(LogFile[11], FILE_WRITE);
      if (myFile){
        myFile.print(tmplog5.toInt()+1);
        myFile.close();
        Serial.println("Tmp log LED : Update!");
      }
    }
    
    NotRecord[11] = false;
  } else 
    if (GetSplit(2,2) > 5) 
    NotRecord[11] = true;
}

//// Reset notrecord[] data log (if day continue) __________________________________
void ResetSelectedNR(){
  String LastOn[3];
  int    resetfrom = 0;
  
  for (int s = 0; s < 3; s++){
    myFile = SD.open(LogFile[s+5]); 
    if (myFile){
      while (myFile.available()) LastOn[s] += (char)myFile.read();
      myFile.close();
    }
  }

  if (LastOn[2].toInt() == GetSplit(1,0)){
    for (int s = 8; s >= 0; s--){
      if (LastOn[0].toInt() >= LogTime[s][0] && LastOn[1].toInt() > LogTime[s][1]){
        resetfrom = s + 2;
        break;
      }
    }
  }
  
  for (int a = resetfrom; a < 20; a++)  NotRecord[a] = true;
}

//// Reset NotRecord[] & daily logging _____________________________________________
void SwitchDay(){
  if (GetSplit(2,0) < 1 && !NotRecord[0]){
    
    // reset all notrecord[]
    for (int a = 0; a < 20; a++)  NotRecord[a] = true;

    // logging 3: led & wp (%)
    String tmplog[2] = {"",""};
    for (int s = 0; s < 2; s++){
      myFile = SD.open(LogFile[s+11]);
      while (myFile.available()) tmplog[s] += (char)myFile.read();
      myFile.close();
    }
    delay(10);
    float m2tmp[2] = {tmplog[0].toInt(),tmplog[1].toInt()};
    myFile = SD.open(LogFile[10], FILE_WRITE);
    if (myFile) {
      myFile.print("D" + String(Day));
      myFile.print("," + String((m2tmp[0]/600)*100,1) + "%"); // LED log
      myFile.println("," + String((m2tmp[1]/720)*100,1) + "%"); // WP log
      myFile.close();
      Serial.println("Logging     : 3.Update!");
    }

    // reset led &  wp tmp log
    delay(10);
    for (int s = 0; s < 2; s++){
      SD.remove(LogFile[s+11]);
      myFile = SD.open(LogFile[s+11], FILE_WRITE);
      if (myFile) {
        myFile.print("0");
        myFile.close();
        Serial.println("Tmp log (" + String(s) + ") : Reset!");
      }
    }
    
    Day++;
  }
}

//// _______________________________________________________________________________
void WaterPump(){
  if (GetSplit(2,1)%RelayTime[0][0] == 0 && GetSplit(2,2) < RelayTime[0][1] && WPStatus == "OFF"){
    digitalWrite(RelayPin1, LOW);
    WPStatus = "ON ";

    // tmp log wp
    delay(10);
    String tmplog6 = "";
    myFile = SD.open(LogFile[12]);
    while (myFile.available()) tmplog6 += (char)myFile.read();
    myFile.close();
    delay(10);
    SD.remove(LogFile[12]);
    myFile = SD.open(LogFile[12], FILE_WRITE);
    if (myFile){
      myFile.print(tmplog6.toInt()+1);
      myFile.close();
      Serial.println("Tmp log WP  : Update!");
    }
    
    delay(10);
    DataLogging(3); // wp log
    
    //Serial.println("Water Pump  : ON");
  }else
  if (GetSplit(2,2) >= RelayTime[0][1] && WPStatus == "ON "){
    digitalWrite(RelayPin1, HIGH);
    WPStatus = "OFF";
    //Serial.println("Water Pump  : OFF");
  }
}

//// _______________________________________________________________________________
void AutoLED(){
  if ((GetSplit(2,0) >= RelayTime[1][0] && GetSplit(2,0) < RelayTime[1][1]) && !LEDStatus){
    digitalWrite(RelayPin2, LOW);
    LEDStatus = true;
    Serial.println("LED switch  : ON");
  }else 
  if ((GetSplit(2,0) < RelayTime[1][0] || GetSplit(2,0) >= RelayTime[1][1]) && LEDStatus){
    digitalWrite(RelayPin2, HIGH);
    LEDStatus = false;
    Serial.println("LED switch  : OFF");
  }
}

//// Return warning by sensor value (ppm / lux) ____________________________________
boolean GetWarn1(){
  if (GetSplit(2,0) >= 7){
    for (int s = 0; s < 3; s++){
      if (Day > (s*10) && Day <= ((s+1)*10) && (ppm < LimitPPM[s][0] || ppm > LimitPPM[s][1])){
        Serial.println("Warning     : Water TDS!");
        return true;
      } else
        return false;
    }
  }
}

boolean GetWarn2(){
  // _________________________________ Intensity: out of value
  if (LEDStatus && (lux < LimitLux[0] || lux > LimitLux[1])){
    Serial.println("Warning     : Light intensity!");
    return true;
  } else
    return false;
}

//// Return sensor value (lux / ppm) _______________________________________________
float GetLux(){
  float  ADC_value = 0.0048828125;
  float  LDR_value = analogRead(LDRPin);
  float  result = ((250.000000/(ADC_value*LDR_value))-50.000000) * 10;
  return result;      
}

float GetPPM(){
    float temperature = 25;
    gravityTds.setTemperature(temperature);
    gravityTds.update();
    float result = gravityTds.getTdsValue();
    return result;
}

//// _______________________________________________________________________________
boolean PushButton(){
  if (digitalRead(StartPin) == HIGH){
    Buzzer(2);
    lcd.clear();
    delay(600);
    return true;
  }else 
    return false;
}

//// _______________________________________________________________________________
void Buzzer(int a){
  switch (a){
    
    case 1:
      digitalWrite(BuzzerPin, LOW);  delay(0.25 *400);  //on
      digitalWrite(BuzzerPin, HIGH); delay(0.125*400);  //off
      digitalWrite(BuzzerPin, LOW);  delay(0.25 *400);  //on
      digitalWrite(BuzzerPin, HIGH); delay(0.375*400);  //off
    break;

    case 2:
      digitalWrite(BuzzerPin, LOW);  delay(200);
      digitalWrite(BuzzerPin, HIGH); delay(100);
    break;
    
  }
}

////////////////////////////////////////////////////////////////////////////////////

int GetSplit(int value, int index){
  if (value == 1) return Split(rtc.getDateStr(), '.', index).toInt(); //Tanggal
  if (value == 2) return Split(rtc.getTimeStr(), ':', index).toInt(); //Jam
}

String Split(String data, char separator, int index){
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

long Counting(int fd, int fm, int fy, int sd, int sm, int sy){
  int  m[12]    = {31,28,31,30,31,30,31,31,30,31,30,31};
  int  y[4]     = {366,365,365,365};
  long distance = 0;

  if (sy > fy){
    for (int s = fy; s < sy; s++) distance += y[(s%4)];
    for (int s = 0; s < (fm-1); s++) distance -= m[s];
    for (int s = 0; s < (sm-1); s++) distance += m[s];
      distance -= fd;
      distance += sd;
    if(fy%4 == 0 && fm > 2) distance--;
    if(sy%4 == 0 && sm > 2) distance++;
    
  } else if (sy == fy){
    if (sm > fm){
      for (int s = fm; s < (sm-1); s++) distance += m[s];
      if (sm > 2 && fm <= 2 && fd <= 28 && sy%4 == 0) distance ++;
      distance += ((m[(fm-1)]) - fd) + sd;
    } else if (sm == fm) distance = sd - fd;
  }
  
  return distance;
}

int GetDay(int d1, int d2, int d3){
  int  m[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int  y[4]  = {366,365,365,365};
  long day   = d1 - 1;
  
  for (int s = 1; s < d3; s++) day += y[s%4];
  for (int s = 0; s < (d2-1); s++) day += m[s];
  if  (d3%4 == 0 && d2 > 2) day++;
  
  return day%7;
}

void FixTimeRTC(){
  int     DataIn[3];
  boolean looping;
  ShowLCD(2);
  
  Serial.println("\nSetting Date . . .");
  Serial.println("Send: Day, Month, Year           *Ex. 16,4,2021");
  looping = true;
  while(looping){
    if (Serial.available() > 0){
      for (int a=0 ; a<3 ; a++)  DataIn[a] = Serial.parseInt();
      rtc.setDate(DataIn[0], DataIn[1], DataIn[2]);
      rtc.setDOW (GetDay(DataIn[0], DataIn[1], DataIn[2]));
      if (Serial.available() > 0){
        Serial.println("      " + String(rtc.getDateStr())); delay(500);
        looping = false;
      }
    }
  }
  
  Serial.println("\nSetting Time . . .");
  Serial.println("Send: Hour, Minute, Second       *Ex. 19,30,20");
  looping = true;
  while(looping){
    if (Serial.available() > 0){
      for (int a=0 ; a<3 ; a++)  DataIn[a] = Serial.parseInt();
      rtc.setTime(DataIn[0], DataIn[1], DataIn[2]);
      if (Serial.available() > 0){
        Serial.println("      " + String(rtc.getTimeStr())); delay(500);
        looping = false;
      }
    }
  }
  delay(500); 
  Serial.println("\nCurrent Date & Time: \n" + String(rtc.getDOWStr()) + ", " + String(rtc.getDateStr()) + " | " + String(rtc.getTimeStr()));
  lcd.clear();
}
