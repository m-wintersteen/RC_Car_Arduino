// Include the Servo library 
#include <Servo.h> 
// Declare the Servo pin 
int servoPin = 2; 
// Create a servo object 
int max = 170;
int min = 30;
int mid = (max-min)/2 + min;
Servo Servo1; 

void setup() { 
   // We need to attach the servo to the used pin number 
   Servo1.attach(servoPin); 
   /*Serial.begin(9600);
   while (! Serial);
   Serial.println("Angle:");*/
}
void loop(){
  /*if (Serial.available())
  {
    int angle = Serial.parseInt();
     // Make servo go to angle 
     Servo1.write(angle); 
     delay(1000); 
  }*/
  for (int i = min; i < mid; i++) {
    int left = i;
    int right = max-(i-min);
    int dif = right - left;
    int time_to_pause = dif*3;
    if (time_to_pause < 200) {
      time_to_pause = 200;
    }
    
    Servo1.write(left);
    delay(time_to_pause);
    Servo1.write(right);
    delay(time_to_pause);
  }
}
