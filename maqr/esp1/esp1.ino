#include <Adafruit_MLX90614.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include "DFRobotDFPlayerMini.h"
#include "MAX30100_PulseOximeter.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); 
DFRobotDFPlayerMini dfpm;
PulseOximeter pox;

#define REPORTING_PERIOD_MS 1000
#define WIFI_SSID "HUEUNI-OFFICE"   //Thay wifi và mật khẩu
#define WIFI_PASSWORD "hueuniair"
#define FIREBASE_HOST "arduino-firebase-vippro-default-rtdb.firebaseio.com" //Thay bằng địa chỉ firebase của bạn
#define FIREBASE_AUTH "DeRJ5uR1rA5nfPL2FCjdUZTKcttOx2Wim4TpUa2P"   //Không dùng xác thực nên không đổi
#define hn 19
#define bom 27

uint32_t mcs = 2;
uint32_t mcsLastReport = 0;
uint32_t mlsLastReport = 0;
uint32_t saing = 0;
const int trig = 14;     // chân trig của HC-SR04
const int echo = 12;     // chân echo của HC-SR04
unsigned long duration; // biến đo thời gian
int distance;           // biến lưu khoảng cách
float hr, t[3], lcd_hr;
int spo2, dem, dem2, sai_nguoi, lcd_spo2;
float maxVal = 0;
float sai_so;
bool beat = false;
uint32_t tsLastReport = 0;
uint32_t msbom;
bool check_bom = true;
// Callback routine is executed when a pulse is detected

void onBeatDetected()
{
  beat = true;
  dem++;
}

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup()
{
  Serial.begin(9600);
  pinMode(hn, INPUT);
  pinMode(bom, OUTPUT);
  pinMode(trig,OUTPUT);   // chân trig sẽ phát tín hiệu
  pinMode(echo,INPUT);    // chân echo sẽ nhận tín hiệu
  dem = 1;
  dem2 = 1;
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
  dfpm.begin(Serial);
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
  digitalWrite(bom, LOW); 
  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin())
  {
    Serial.println("FAILED");
    for(;;);
  }
  else
  {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_50MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
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
      
      /* In kết quả ra Serial Monitor */
      Serial.print(distance);
      Serial.println("cm");

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
  pox.update();
  int dis = getDis();
  sai_so = Firebase.getFloat("informationStudent/sai_so");
  sai_nguoi = Firebase.getBool("informationStudent/sai_nguoi");
  if (sai_nguoi)
  {
    if (millis() - saing > 15000)
    {
      saing = millis();
      Firebase.setBool("informationStudent/sai_nguoi", false);
      dfpm.play(3);
    }
  }
    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    if (beat)
    {
      if (hr<pox.getHeartRate())
      {
        hr = pox.getHeartRate();
        spo2 = pox.getSpO2();
      }
      tsLastReport = millis();
    }
  }
  if (digitalRead(hn) == HIGH)
  {
    msbom = millis();
  }
  else
  {
    check_bom = true;
  }
  if (millis() - msbom < 300)
  {
    if (check_bom)
    {
      digitalWrite(bom, HIGH);
      check_bom = false;
    }
    else
    {
      digitalWrite(bom, LOW);
    }
  }
  else
  {
    digitalWrite(bom, LOW);
  }
  if (dem>5)
  {
    pox.shutdown();
    Firebase.setFloat("informationStudent/nhip_tim", hr);
    Firebase.setInt("informationStudent/spo2", spo2);
    lcd_hr = hr;
    lcd_spo2 = spo2;
    hr=0;
    spo2=0;
    dem=1;
    pox.begin();
  }
  beat = false;
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
      if (dem2 > sizeof(t)/sizeof(t[0]))
      {
        Firebase.setFloat("informationStudent/temp", getMax());
        dfpm.play(1);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(maxVal);
        lcd.setCursor(5, 0);
        lcd.print("*C");
        lcd.setCursor(0, 1);
        lcd.print(lcd_hr);
        lcd.setCursor(5, 1);
        lcd.print("bpm");
        lcd.setCursor(9, 1);
        lcd.print(lcd_spo2);
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
        dem2 = 1;
        maxVal = 0;
        delay(6000);
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("KHOA KT-CN");
        lcd.setCursor(0, 1);
        lcd.print("Xin chao cac ban");
      }
      else
      {
        t[dem2] = mlx.readObjectTempC()+sai_so;
        dem2++;
      }
    }
  }
}
