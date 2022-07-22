#include "MAX30100_PulseOximeter.h"
#include "WiFi.h"
#include "FirebaseESP32.h"

#define REPORTING_PERIOD_MS 1000
#define WIFI_SSID "HUEUNI"   //Thay wifi và mật khẩu
#define WIFI_PASSWORD "hueuniair"
#define FIREBASE_HOST "arduino-firebase-vippro-default-rtdb.firebaseio.com" //Thay bằng địa chỉ firebase của bạn
#define FIREBASE_AUTH "DeRJ5uR1rA5nfPL2FCjdUZTKcttOx2Wim4TpUa2P"   //Không dùng xác thực nên không đổi

PulseOximeter pox;
FirebaseData fbdt;

#define hn 19
#define bom 27

bool beat = false;
uint32_t tsLastReport = 0;
uint32_t msbom = 0;
float hr;
int spo2, i=0; 
bool check_bom = true;
// Callback routine is executed when a pulse is detected
void onBeatDetected()
{
  beat = true;
}

void setup()
{
  pinMode(hn, INPUT);
  pinMode(bom, OUTPUT);
  i = 1;
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  i = 1;
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
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

void loop()
{
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    if (beat)
    {
//      Serial.print("Lan ");
//      Serial.println(i);
      if (hr<pox.getHeartRate())
      {
        hr = pox.getHeartRate();
      }
      if (spo2<pox.getSpO2())
      {
        spo2 = pox.getSpO2();
      }
//      Serial.print("Heart rate:");
//      Serial.print(hr);
//      Serial.print("bpm / SpO2:");
//      Serial.print(spo2);
//      Serial.println("%");
      i++;
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
  if (i>5)
  {
    pox.shutdown();
    Firebase.setFloat(fbdt, F("informationStudent/nhip_tim"), hr);
    Firebase.setInt(fbdt, F("informationStudent/spo2"), spo2);
    hr=0;
    spo2=0;
    i=1;
    pox.begin();
  }
  beat = false;
}
