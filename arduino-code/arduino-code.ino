#include <SoftwareSerial.h>
  SoftwareSerial esp(2,3);  // RX, TX

String ssid = "sweet";
String password = "Tiramisu";

String server = "eoiigimyy5mb7kp.m.pipedream.net";
String path = "/verifying.php";

void setup() {
  esp.begin(9600);
  Serial.begin(9600);
  Serial.println("\nSTART\n");
  
  esp_reset();
  String data[] = {"A","POSITIF"};
  httppost(data);
}

void loop() {
  // put your main code here, to run repeatedly:

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


void httppost (String value[2]){
  String content = "types=" + value[0] + "&rhesus=" + value[1];
  
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

  Serial.println("Post Request:\n\n" + postRequest + "\n");
    
  esp.println("AT+CIPSEND=" + String(postRequest.length()));
  delay(500);
  
  if (esp.find(">")){
    Serial.print("Sending...");
    esp.print(postRequest);
    
    if (esp.find("SEND OK")){
      Serial.println(" > Packet sent!\n\n");
      
      while (esp.available()){
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
      
      esp.println("AT+CIPCLOSE");
    }
    
  }
}
