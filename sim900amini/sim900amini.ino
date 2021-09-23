#include <SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial gsmSerial(9, 10);
SoftwareSerial nodeMCUSerial(5, 6);


bool intruderAlertStatus = false;

void setup()
{
  Serial.begin(115200);
  gsmSerial.begin(9600);
  nodeMCUSerial.begin(4800);
  Serial.println("GSM SIM900A BEGIN");
}

void loop()
{
  StaticJsonDocument<1000> jsonDocument;

  DeserializationError error = deserializeJson(jsonDocument, nodeMCUSerial);

  if (error) {
    Serial.print("Error Occurred -> ");
    Serial.println(error.c_str());
    return;
  }

  Serial.println("Json received and parsed successfully.");

  intruderAlertStatus = jsonDocument["alert"];

  Serial.print("Intruder Alert Status -> ");
  Serial.println(intruderAlertStatus);

  if (intruderAlertStatus) {
    SendMessage("Intruder Has Been Detected");
    delay(1000);
    Serial.println("Alert has been sent to the user.");
  }

  delay(2000);
}

void SendMessage(String msg)
{
  gsmSerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  gsmSerial.println("AT+CMGS=\"+917010481557\"\r"); // Replace x with mobile number
  delay(1000);
  gsmSerial.println(msg);// The SMS text you want to send
  delay(100);
  gsmSerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}
