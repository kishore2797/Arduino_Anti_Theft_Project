#include <Servo.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

Servo servoCamera;

SoftwareSerial gsmSerial(9, 10);

int servoAngle;

void setup() {
  Serial.begin(112500);
  gsmSerial.begin(4800);

  servoCamera.attach(8);
}

void loop() {
  StaticJsonDocument<1000> jsonDocument;

  DeserializationError error = deserializeJson(jsonDocument, Serial);

  if (error) {
    //    Serial.print("Error Occurred -> ");
    //    Serial.println(error.c_str());
    return;
  }

  //  Serial.println("Json received and parsed successfully.");

  int angleInt = jsonDocument["servoAngle"];
  bool alert = jsonDocument["alert"];

  Serial.print("angleInt -> ");
  Serial.println(angleInt);
  //
  Serial.print("alert -> ");
  Serial.println(alert);
  //
  //  Serial.print("servoCurrentAngle -> ");
  //  Serial.println(servoCamera.read());

  if (angleInt != servoAngle) {
    //    Serial.print("angleInt -> ");
    //    Serial.println(angleInt);

    servoAngle = angleInt;

    servoCamera.write(servoAngle);

    //    Serial.print("servoAfterWriteAngle -> ");
    //    Serial.println(servoCamera.read());
  }

  if (alert) {
    SendMessage("Intruder Has Been Detected");
    Serial.println("Intruder Has Been Detected");
    //    StaticJsonDocument<1000> jsonDocument;
    //    jsonDocument["user_alerted"] = true;
    //    serializeJson(jsonDocument, Serial);
    //    Serial.println("Alert has been sent to the user.");
  }
}

void SendMessage(String msg)
{
  gsmSerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  gsmSerial.println("AT+CMGS=\"+919944336496\"\r"); // Replace x with mobile number
  delay(1000);
  gsmSerial.println(msg);// The SMS text you want to send
  delay(100);
  gsmSerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}
