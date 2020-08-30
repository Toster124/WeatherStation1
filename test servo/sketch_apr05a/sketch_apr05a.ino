#include <Servo.h>
Servo myservo;
void setup() {
  
  pinMode(8, OUTPUT);
  digitalWrite(8, 1);
  Serial.begin(9600);
  myservo.attach(15);
}

void loop() {
  if(Serial.available() > 0) {
    int a = Serial.parseInt();
    int angle = map(a, 400, 2000, 180, 0);
    angle = constrain(angle, 0, 180);
    myservo.write(angle);
  }

}
