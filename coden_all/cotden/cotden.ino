#include <Firebase.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WIFI_SSID "HUEUNI-OFFICE"   //Thay wifi và mật khẩu
#define WIFI_PASSWORD "hueuniair"
#define FIREBASE_HOST "arduino-firebase-vippro-default-rtdb.firebaseio.com" //Thay bằng địa chỉ firebase của bạn
#define FIREBASE_AUTH "DeRJ5uR1rA5nfPL2FCjdUZTKcttOx2Wim4TpUa2P"   //Không dùng xác thực nên không đổi
const long utcOffsetInSeconds = 25200;

#define den1 05 //d1
#define den2 04 //d2
#define den3 02 //d4 
#define quat 14 //d5
#define sac  12 //d6

ADC_MODE(ADC_VCC);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

int moc, moc1, moc2, h, mocsac1, mocsac2;

void setup()
{
  pinMode(den1, OUTPUT);
  pinMode(den2, OUTPUT);
  pinMode(den3, OUTPUT);
  pinMode(quat, OUTPUT);  
  pinMode(sac, OUTPUT);
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  timeClient.begin();
  digitalWrite(den1, HIGH);
  digitalWrite(den2, HIGH);
  digitalWrite(den3, HIGH);
  digitalWrite(sac, HIGH);
}

void loop()
{
  timeClient.update();
  Serial.print(timeClient.getHours());
  Serial.print(":"); Serial.print(timeClient.getMinutes());
  Serial.print(":"); Serial.println(timeClient.getSeconds());
  moc = Firebase.getInt("cotden/moc");
  moc1 = Firebase.getInt("cotden/moc1");
  moc2 = Firebase.getInt("cotden/moc2");
  mocsac1 = Firebase.getInt("cotden/mocsac1");
  mocsac2 = Firebase.getInt("cotden/mocsac2");
  h = Firebase.getInt("cotden/h");
  Firebase.setInt("vcc", ESP.getVcc());
  Firebase.setInt("ana", analogRead(A0));
  if (ESP.getVcc()-mocsac1<0)
  {
    Serial.println("Sac low");
    digitalWrite(sac, LOW);
  }
  if (ESP.getVcc()-mocsac2>0)
  {
    Serial.println("Sac high");
    digitalWrite(sac, HIGH);
  }
  digitalWrite(quat, LOW);
  if (timeClient.getHours()>h)
  {
    digitalWrite(den1, LOW);
    if (moc>=moc1)
    {
      digitalWrite(den2, LOW);
    }
    else
    {
      digitalWrite(den2, HIGH);
    }
    if (moc>=moc2)
    {
      digitalWrite(den2, LOW);
      digitalWrite(den3, LOW);
    }
    else
    {
      digitalWrite(den3, HIGH);
    }
  }
  else
  {
    digitalWrite(den1, HIGH);
    digitalWrite(den2, HIGH);
    digitalWrite(den3, HIGH);
  }
}
