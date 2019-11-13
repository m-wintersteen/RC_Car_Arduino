#define Bin1 6
#define Bin2 9
#define Ain1 3
#define Ain2 5

void setup() 
{ 
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(Ain1, OUTPUT);
  pinMode(Ain2, OUTPUT);
  Serial.begin(9600);
  while (! Serial);
  Serial.println("Speed 0 to 255");
} 
 
 
void loop() 
{ 
  if (Serial.available())
  {
    int speed = Serial.parseInt();
    if (speed >= 0 && speed <= 1000)
    {
      analogWrite(Bin1, speed);
      analogWrite(Ain1, speed);
      delay(4000);
      analogWrite(Bin1,0);
      analogWrite(Ain1,0);
      delay(500);
      analogWrite(Bin2, speed);
      analogWrite(Ain2,speed);
      delay(4000);
      analogWrite(Bin2,0);
      analogWrite(Ain2,0);
      delay(500);
    }
  }
} 
