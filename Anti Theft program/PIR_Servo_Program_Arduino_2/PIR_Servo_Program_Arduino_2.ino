#include <Servo.h>

Servo servo;

void setup() {
  servo.attach(8);
  servo.write(0);
  delay(2000);

}

void loop() {
  for(int i=113; i<157; i+=5) {
    servo.write(i);
    delay(120);
  }
  for(int i=157; i>113; i-=5) {
    servo.write(i);
    delay(120);
  }
}
