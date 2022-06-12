#include <SoftwareSerial.h>
  SoftwareSerial esp(2,3);  // RX, TX

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
  //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C (0x27,16,2);

#define LED_ON  digitalWrite(5,HIGH)
#define LED_OFF digitalWrite(5,LOW)

#define BUZZ_ON  digitalWrite(6,LOW)
#define BUZZ_OFF digitalWrite(6,HIGH)

#define BUTTON digitalRead(7)

const uint8_t MyAnti[3] = {A2, A1, A0};  // Anti A, Anti B, Anti D
const int Anti[3] = {950, 950, 950};  // Anti A, Anti B, Anti D
const int N = 100;

String ssid = "yourNetwork";
String password = "secretPassword";

String server = "yourServer.com";
String path = "/collecting.php";

//////////////////////////////////////////////////////////////////////////

void setup() {
  inisialisasi();
  esp_reset();
  
  lcd.clear();
  lcd.print("READY");
  Buzzer(2,100);
}

void loop() {
  if (Button()){
    fungsi_utama();
    delay(3000);
    lcd.setCursor(0,1);
    lcd.print("READY");
    Buzzer(2,100);
  }
}

//////////////////////////////////////////////////////////////////////////

void fungsi_utama() {
  int value[100];
  boolean sensor[3];
  String result[2];

  lcd.clear();
  lcd.print("LOADING...");
  
  LED_ON;
  delay(4000);

  for (int s = 0; s < 3; s++){
    for (int a = 0; a < N; a++){
      value[a] = analogRead(MyAnti[s]);
      delay(20);
    }
    sensor[s] = kalkulasi(value,s) >= 50 ? true : false ;
  }
  
  LED_OFF;
  
  if (   sensor[0] && ! sensor[1]) result[0] = "A";  else
  if ( ! sensor[0] &&   sensor[1]) result[0] = "B";  else
  if (   sensor[0] &&   sensor[1]) result[0] = "AB"; else
  if ( ! sensor[0] && ! sensor[1]) result[0] = "O";
  
  result[1] = sensor[2] ? "POSITIF" : "NEGATIF";

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String(result[0]) + String(result[1] == "POSITIF" ? "+" : "-"));
  Serial.println("\nDarah  : " + result[0] + String(result[1] == "POSITIF" ? "+" : "-") + "\n\n");

  lcd.setCursor(6,0); 
  lcd.print("SENDING... ");
  httppost(result);
}

int kalkulasi(int value[100], int n) {
  char x[3] = "ABD";
  int result = 0;
  for (int s = 0; s < N; s++){
    if (value[s] >= Anti[n]) result++;
  }

  result = result/N*100;
  Serial.println("Anti " + String(x[n]) + " : " + String(result) + "%");
  return result;
}


void inisialisasi() {
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("SET UP...");
  delay(1000);

  esp.begin(9600);
  Serial.begin(9600);
  Serial.println("\nSTART\n");
  
  LED_OFF;
  BUZZ_OFF;
  
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,INPUT);
}

boolean Button() {
  if (!BUTTON){
    while (!BUTTON) BUZZ_ON;
    delay(100);
    BUZZ_OFF;
    return true;
  }
  return false;
}

void Buzzer(int num, int del) {
  for (int s = 0; s < num; s++){
    BUZZ_ON;  delay(del);
    BUZZ_OFF; delay(50);
  }
}


void esp_reset() {
  esp.println("AT+RST");
  delay(1000);
  if (esp.find("OK")){
    Serial.println("Module: Reset");
    connectWifi();
  }
}

void connectWifi() {
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  if (esp.find("OK")) {
    Serial.println("WiFi: Connected!\n");
  } else {
    Serial.print(".");
    connectWifi();
  }
}

void httppost (String value[2]) {
  String content = "from=arduino&direct=yes&groups=" + value[0] + "&rhesus=" + value[1];
  
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  
  if (esp.find("OK")) Serial.println("TCP connection ready\n");
  delay(1000);

  String postRequest =
    "POST " + path + " HTTP/1.0\r\n" +
    "Host: " + server + "\r\n" +
    "Accept: *" + "/" + "*\r\n" +
    "Content-Length: " + content.length() + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + content;

  //Serial.println("Post Request:\n\n" + postRequest + "\n");
  esp.println("AT+CIPSEND=" + String(postRequest.length()));
  delay(500);

  if (esp.find(">")){
    Serial.print("Sending...");
    esp.print(postRequest);
    
    if (esp.find("SEND OK")){
      esp.println("AT+CIPCLOSE");
      
      Serial.println(" > Packet sent!\n\n");
      lcd.setCursor(6,0);
      lcd.print("DATA SENT! ");
      Buzzer(1,150);
    }
    
  }
}
