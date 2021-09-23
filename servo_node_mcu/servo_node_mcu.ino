#include <Servo.h>

#include <SoftwareSerial.h>

#include <ArduinoJson.h>

SoftwareSerial s(5, 6);

Servo servo;

int servoAngle;

void setup() {
  Serial.begin(9600);
  s.begin(9600);
  Serial.println("Serial Attached");
  servo.attach(9);
  delay(2000);
}

void loop() {
  StaticJsonDocument<1000> jsonDocument;

  DeserializationError error = deserializeJson(jsonDocument, s);

  if (error) {
    Serial.print("Error Occurred -> ");
    Serial.println(error.c_str());
    return;
  }

  Serial.println("Json received and parsed successfully.");

  int angleInt = jsonDocument["servoAngle"];

  Serial.print("servoAngle -> ");
  Serial.println(servoAngle);

  Serial.print("servoCurrentAngle -> ");
  Serial.println(servo.read());

  if (angleInt != servoAngle) {
    Serial.print("angleInt -> ");
    Serial.println(angleInt);

    servoAngle = angleInt;

    servo.write(servoAngle);

    Serial.print("servoAfterWriteAngle -> ");
    Serial.println(servo.read());
  }
}
