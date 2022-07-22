#define PUL 3 //define Pulse pin xung
#define DIR 2 //define Direction pin chieu
#define BUT 13

int microStep = 16 ; //200 xung/vong
float angleStep = 1.8;
float stepsPerRound = microStep * 360.0 / angleStep; //200 xung
long steps = 1 * stepsPerRound;
char state;

void setup()
{
  Serial.begin(9600);
  // Declare pins as output:
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(BUT, INPUT_PULLUP);
}

void loop()
{
  if (Serial.available()>0)
  {
    state = Serial.read();
    if (state == 'm')
    {
      Serial.println("Serial");
      Serial.println("len");
      digitalWrite(DIR, LOW);
      steps = 11 * stepsPerRound; //400
      for (long i = 1; i <= steps; i++)
      {
        digitalWrite(PUL, HIGH); 
        delayMicroseconds(60);
        digitalWrite(PUL, LOW);
        delayMicroseconds(60);
      }
      delay(5000);
      Serial.println("xuong");
      digitalWrite(DIR, HIGH);
      for (long i = 1; i <= steps; i++)
      {
        digitalWrite(PUL, HIGH); 
        delayMicroseconds(100);
        digitalWrite(PUL, LOW);
        delayMicroseconds(100);
      }
    }
  }
  
  if (digitalRead(BUT)==0)
  {
    Serial.println("BUTTON");
    Serial.println("len");
    digitalWrite(DIR, LOW);
    steps = 11 * stepsPerRound; //400
    for (long i = 1; i <= steps; i++)
    {
      digitalWrite(PUL, HIGH); 
      delayMicroseconds(60);
      digitalWrite(PUL, LOW);
      delayMicroseconds(60);
    }
    delay(5000);
    Serial.println("xuong");
    digitalWrite(DIR, HIGH);
    for (long i = 1; i <= steps; i++)
    {
      digitalWrite(PUL, HIGH); 
      delayMicroseconds(100);
      digitalWrite(PUL, LOW);
      delayMicroseconds(100);
    }
  }
}
