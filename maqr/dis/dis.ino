#include <Adafruit_MLX90614.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include "DFRobotDFPlayerMini.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); 
DFRobotDFPlayerMini dfpm;

#define WIFI_SSID "HUEUNI-OFFICE"   //Thay wifi và mật khẩu
#define WIFI_PASSWORD "hueuniair"
#define FIREBASE_HOST "arduino-firebase-vippro-default-rtdb.firebaseio.com" //Thay bằng địa chỉ firebase của bạn
#define FIREBASE_AUTH "DeRJ5uR1rA5nfPL2FCjdUZTKcttOx2Wim4TpUa2P"   //Không dùng xác thực nên không đổi

uint32_t mcs = 2;
uint32_t mcsLastReport = 0;
uint32_t mlsLastReport = 0;
uint32_t ntms = 0;
uint32_t saing = 0;
const int trig = 14;     // chân trig của HC-SR04
const int echo = 12;     // chân echo của HC-SR04
unsigned long duration; // biến đo thời gian
int distance;           // biến lưu khoảng cách
float hr, t[2];
int spo2, i;
float maxVal = 0;
float sai_so;
bool sai_nguoi;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup()
{
  Serial.begin(9600);
  pinMode(trig,OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(echo,INPUT);    // chân echo sẽ nhận tín hiệu
  i = 1;
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
  if(!dfpm.begin(Serial))
  {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  else
  {
    Serial.println(F("DFPlayer Mini online."));
  }
  dfpm.setTimeOut(500);
  dfpm.volume(30);
  dfpm.EQ(DFPLAYER_EQ_NORMAL);
  dfpm.outputDevice(DFPLAYER_DEVICE_SD);
  
  lcd.begin();                    
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("KHOA KT-CN");
  lcd.setCursor(0, 1);
  lcd.print("Xin chao cac ban");
  mlx.begin();
}
int getDis()
{
  if (micros() - mcsLastReport > mcs)
  {
    if (digitalRead(trig) == LOW)
    {
      digitalWrite(trig, HIGH);
      mcs = 2;
    }
    else
    {
      mcs = 5;
      digitalWrite(trig, LOW);
    }
    duration = pulseIn(echo, HIGH);  
    if (duration != 0)
    {
      distance = int(duration/2/29.412);
      
//      /* In kết quả ra Serial Monitor */
//      Serial.print(distance);
//      Serial.println("cm");

      mcsLastReport = micros();
    }
  }
  return distance;
}
float getMax()
{
  for (int n = 0; n < (sizeof(t)/sizeof(t[0])); n++)
  {
    if (t[n] > maxVal)
    {
      maxVal = t[n];
    }
   }
   return maxVal;
}
void loop()
{
  int dis = getDis();
  Firebase.setInt("informationStudent/khoang_cach", dis);
  sai_so = Firebase.getFloat("informationStudent/sai_so");
  hr = Firebase.getFloat("informationStudent/nhip_tim");
  spo2 = Firebase.getInt("informationStudent/spo2");
  sai_nguoi = Firebase.getBool("informationStudent/sai_nguoi");
//  nt = Firebase.getBool("informationStudent/nt");
//  if (nt)
//  {
//    if (millis() - ntms > 5000)
//    {
//      ntms = millis();
//      Firebase.setBool("informationStudent/nt", false);
//      dfpm.play(1);
//    }
//  }
  if (20 <= dis && dis < 65)
  {
    if (millis() - mlsLastReport > 15000)
    {
      mlsLastReport = millis();
      dfpm.play(2);
    }
  }
  else if (dis<=3)
  {
    if (!isnan(mlx.readObjectTempC()))
    {
      if (i < sizeof(t)/sizeof(t[0]))
      {
        t[i] = mlx.readObjectTempC()+sai_so;
//        Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
//        Serial.print("*CtObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
//        Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
//        Serial.print("*FtObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
//        Serial.print("Lan "); Serial.println(i);
        i++;
      }
      else
      {
        Firebase.setFloat("informationStudent/temp", getMax());
        dfpm.play(1);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(maxVal);
        lcd.setCursor(5, 0);
        lcd.print("*C");
        lcd.setCursor(0, 1);
        lcd.print(hr);
        lcd.setCursor(5, 1);
        lcd.print("bpm");
        lcd.setCursor(9, 1);
        lcd.print(spo2);
        lcd.setCursor(11, 1);
        lcd.print("%");
        if (maxVal >= 38)
        {
          dfpm.play(4);
          Firebase.setBool("informationStudent/opendoor", false);
        }
        else
        {
          Firebase.setBool("informationStudent/opendoor", true);
        }
        i = 1;
        maxVal = 0;
        delay(6000);
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("KHOA KT-CN");
        lcd.setCursor(0, 1);
        lcd.print("Xin chao cac ban");
      }
    }
  }
  if (sai_nguoi)
  {
    if (millis() - saing > 15000)
    {
      saing = millis();
      Firebase.setBool("informationStudent/sai_nguoi", false);
      dfpm.play(3);
    }
  }
}
