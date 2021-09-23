#include <Servo.h>

#include <SoftwareSerial.h>

#include <ArduinoJson.h>

SoftwareSerial nodeMCUSerial(5, 6);

Servo servoCamera;

int servoAngle;

void setup() {
  Serial.begin(9600);
  nodeMCUSerial.begin(9600);
  Serial.println("Serial Attached");
  servoCamera.attach(7);
  delay(2000);

}

void loop() {
  StaticJsonDocument<1000> jsonDocument;

  DeserializationError error = deserializeJson(jsonDocument, nodeMCUSerial);

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
  Serial.println(servoCamera.read());

  if (angleInt != servoAngle) {
    Serial.print("angleInt -> ");
    Serial.println(angleInt);

    servoAngle = angleInt;

    servoCamera.write(servoAngle);

    Serial.print("servoAfterWriteAngle -> ");
    Serial.println(servoCamera.read());
  }
}
