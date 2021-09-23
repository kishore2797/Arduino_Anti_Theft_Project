//https://www.instructables.com/Interfacing-Servo-Motor-With-NodeMCU/
//https://www.hackster.io/HARGOVIND/nodemcu-based-iot-project-connecting-touch-sensor-6dc507
//https://www.instructables.com/DIY-Touch-SensorSwitch-ESP8266-Node-MCU/

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoJson.h>

// Set these to run example.
#define FIREBASE_HOST "iot-security-8ec28-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "t772Y57HhWtDKZSUdGQNP7c0NEkrLCkxz4sMBz3q"

#define WIFI_SSID "deffe"
#define WIFI_PASSWORD "defetron"

#define BUZZER_PIN_MODE D4

#define TOUCH_PIN_MODE 

FirebaseData fbdo1;

FirebaseData fbdo2;

FirebaseJson json;

bool securitySystemStatus = false;
bool buzzerStatus = false;

String parentPath = "/Security";
String childPath[3] = {"/security_system_status", "/servo_angle", "/buzzer_status"};
size_t childPathSize = 3;

void setup() {
  Serial.begin(9600);

  pinMode(BUZZER_PIN_MODE, OUTPUT);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("wifi connection has been initialized");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Wifi connected to : ");
  Serial.println(WiFi.localIP());

  //Initialize the library with the Firebase authen and config.
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //Optional, set AP reconnection in setup()
  Serial.println("Reconnecting Wifi");
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo1.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo1.setResponseSize(1024);


  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo2.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo2.setResponseSize(1024);

  if (Firebase.getBool(fbdo1, "/Security/security_system_status")) {
    if (fbdo1.boolData() == true) {
      securitySystemStatus = true;

      if (Firebase.getBool(fbdo1, "/Security/buzzer_status")) {
        bool buzzer = fbdo1.boolData();
        if (buzzer == true) {
          buzzerStatus = true;
          digitalWrite(BUZZER_PIN_MODE, HIGH);
        } else {
          buzzerStatus = false;
          digitalWrite(BUZZER_PIN_MODE, LOW);
        }
        Serial.print("buzzerStatus ->");
        Serial.println(buzzerStatus);
      }

      if (Firebase.getBool(fbdo1, "/Security/servo_angle")) {
        int servoAngle = fbdo1.intData();
        StaticJsonDocument<1000> jsonDocument;
        jsonDocument["servoAngle"] = servoAngle;
        serializeJson(jsonDocument, Serial);
      }

    } else {
      securitySystemStatus = false;
    }
    Serial.print("securitySystemStatus ->");
    Serial.println(securitySystemStatus);
  }

  if (!Firebase.beginMultiPathStream(fbdo1, parentPath, childPath, childPathSize))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.setMultiPathStreamCallback(fbdo1, streamCallback, streamTimeoutCallback);
}

void streamCallback(MultiPathStreamData stream)
{
//  Serial.println();
//  Serial.println("Stream Data1 available...");

  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.println("path: " + stream.dataPath + ", type: " + stream.type + ", value: " + stream.value);

      if (stream.type == "bool") {
        if (stream.dataPath == "/security_system_status") {
          bool securityStatus = stream.value;
          if (securityStatus == true) {
            securitySystemStatus = true;
          } else {
            securitySystemStatus = false;
          }
        }
      }
      if (securitySystemStatus) {
        if (stream.type == "int") {
          if (stream.dataPath == "/servo_angle") {
            int servoAngle = stream.value.toInt();
            StaticJsonDocument<1000> jsonDocument;
            jsonDocument["servoAngle"] = servoAngle;
            serializeJson(jsonDocument, Serial);
          }
        }
        if (stream.type == "bool") {
          if (stream.dataPath == "/buzzer_status") {
            bool buzzer = stream.value;
            if (buzzer == true) {
              buzzerStatus = true;
              digitalWrite(BUZZER_PIN_MODE, HIGH);
            } else {
              buzzerStatus = false;
              digitalWrite(BUZZER_PIN_MODE, LOW);
            }
            Serial.print("buzzerStatus ->");
            Serial.println(buzzerStatus);
          }
        }
      }
    }
  }

//  Serial.println();

  //  if (data.dataType() == "bool") {
  //    if (data.streamPath() == "/Security/security_system_status") {
  //      bool securityStatus = data.boolData();
  //      if (securityStatus == true) {
  //        securitySystemStatus = true;
  //      } else {
  //        securitySystemStatus = false;
  //      }
  //    }
  //  }
  //  if (securitySystemStatus) {
  //    if (data.dataType() == "int") {
  //      if (data.streamPath() == "/Security/servo_angle") {
  //        int servoAngle = data.intData();
  //        StaticJsonDocument<1000> jsonDocument;
  //        jsonDocument["servoAngle"] = servoAngle;
  //        serializeJson(jsonDocument, Serial);
  //      }
  //    }
  //    if (data.dataType() == "bool") {
  //      if (data.streamPath() == "/Security/buzzer_status") {
  //        bool buzzer = data.boolData();
  //        if (buzzer == true) {
  //          buzzerStatus = true;
  //          digitalWrite(BUZZER_PIN_MODE, HIGH);
  //        } else {
  //          buzzerStatus = false;
  //          digitalWrite(BUZZER_PIN_MODE, LOW);
  //        }
  //        Serial.print("buzzerStatus ->");
  //        Serial.println(buzzerStatus);
  //      }
  //    }
  //  }
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout) {
    Serial.println("Stream timeout, resume streaming...");
  }
}

void loop() {
}
