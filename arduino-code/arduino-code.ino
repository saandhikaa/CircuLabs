#define LED_ON  digitalWrite(5,HIGH)
#define LED_OFF digitalWrite(5,LOW)

const uint8_t MyAnti[3] = {A2, A1, A0};  // Anti A, Anti B, Anti D
const int Anti[3] = {950, 950, 950};  // Anti A, Anti B, Anti D
const int N = 100;

//////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  Serial.println("\nSTART\n");
  
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(5,OUTPUT);

  fungsi_utama();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

//////////////////////////////////////////////////////////////////////////

void fungsi_utama() {
  int value[100];
  boolean sensor[3];
  String result[2];

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

  Serial.println("\nDarah  : " + result[0] + String(result[1] == "POSITIF" ? "+" : "-") + "\n\n");

}

int kalkulasi(int value[100], int n) {
  char x[3] = "ABD";
  int result = 0;
  long mean  = 0;

  for (int s = 0; s < N; s++){
    if (value[s] >= Anti[n]) result++;
    mean+=value[s];
  }

  result = result/N*100;
  mean /= N;
  Serial.print("Anti " + String(x[n]) + " : " + String(result) + "%");
  Serial.println("\tLDR : " + String(mean));
  return result;
}
