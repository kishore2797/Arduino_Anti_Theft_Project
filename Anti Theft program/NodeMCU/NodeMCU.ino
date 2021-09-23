#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// Set these to run example.
#define FIREBASE_HOST "iot-security-8ec28-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "t772Y57HhWtDKZSUdGQNP7c0NEkrLCkxz4sMBz3q"

#define WIFI_SSID "AR"
#define WIFI_PASSWORD "12345678"

#define TOUCH_PIN_MODE D4
#define PIR_PIN_MODE D2
#define BUZZER_PIN_MODE D3

FirebaseData fbdo1; // Read

FirebaseData fbdo2; // Write

FirebaseJson json;

bool securitySystemStatus = false;
bool buzzerStatus = false;

String parentPath = "/Security";
String childPath[5] = { "/security_system_status", "/user_alerted", "/alert", "/servo_angle", "/buzzer_status"};
size_t childPathSize = 5;

int touchValue = 0;
int touchingSeconds = 0;

bool intruderAlert = false;
bool userAlerted = false;
int servoAngle = 0;

int wifiConnectionTime = 0;

void setup() {
  Serial.begin(112500);

  pinMode(PIR_PIN_MODE, INPUT);
  pinMode(TOUCH_PIN_MODE, INPUT);
  pinMode(BUZZER_PIN_MODE, OUTPUT);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("wifi connection has been initialized");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    //    wifiConnectionTime += 500;
    //
    //    if (wifiConnectionTime > 10000) {
    //
    //    }
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
        //        Serial.print("buzzerStatus ->");
        //        Serial.println(buzzerStatus);
      }

      if (Firebase.getBool(fbdo1, "/Security/servo_angle")) {
        int servoAngle = fbdo1.intData();
        StaticJsonDocument<1000> jsonDocument;
        jsonDocument["servoAngle"] = fbdo1.intData();
        serializeJson(jsonDocument, Serial);
      }

      if (Firebase.getBool(fbdo1, "/Security/alert")) {
        bool alert = fbdo1.boolData();
        StaticJsonDocument<1000> jsonDocument;
        jsonDocument["alert"] = alert;
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
        if (stream.dataPath == "/servo_angle") {
          servoAngle = stream.value.toInt();
          StaticJsonDocument<1000> jsonDocument;
          jsonDocument["servoAngle"] = stream.value.toInt();
          jsonDocument["alert"] = intruderAlert;
          serializeJson(jsonDocument, Serial);
          //            Serial.print("servoAngle ->");
          //            Serial.println(servoAngle);
        }
        if (stream.dataPath == "/alert") {
          StaticJsonDocument<1000> jsonDocument;
          if (stream.value == "true") {
            intruderAlert = true;
          } else {
            intruderAlert = false;
          }
          Serial.print("intruderAlert ->");
          Serial.println(intruderAlert);
          jsonDocument["servoAngle"] = servoAngle;
          jsonDocument["alert"] = intruderAlert;
          serializeJson(jsonDocument, Serial);
        }
        if (stream.dataPath == "/buzzer_status") {
          if (stream.value == "true") {
            buzzerStatus = true;
            digitalWrite(BUZZER_PIN_MODE, HIGH);
          } else {
            buzzerStatus = false;
            digitalWrite(BUZZER_PIN_MODE, LOW);
          }
          //          Serial.print("buzzerStatus ->");
          //          Serial.println(buzzerStatus);
        }
      }
    }
  }
}


void streamTimeoutCallback(bool timeout)
{
  if (timeout) {
    Serial.println("Stream timeout, resume streaming...");
  }
}

void loop() {

  touchValue = digitalRead(TOUCH_PIN_MODE);



  //  Serial.println(touchValue);

  if (touchValue == HIGH) {
    touchingSeconds++;
    //    Serial.println(touchingSeconds);

    if (touchingSeconds >= 500) {
      Serial.print("touchValue -> ");
      Serial.println(touchValue);

      //Also can use Firebase.set instead of Firebase.setDouble
      if (Firebase.setBool(fbdo2, "/Security/alert", true))
      {
        //        Serial.println("PASSED");
        //        Serial.println("PATH: " + fbdo2.dataPath());
        //        Serial.println("TYPE: " + fbdo2.dataType());
        //        Serial.println("ETag: " + fbdo2.ETag());
        //        Serial.print("VALUE: ");
        //        Serial.println("------------------------------------");
        //        Serial.println();
      }
      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo2.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
      }

      touchingSeconds = 0;
      delay(5000);
    }
  } else {
    touchingSeconds = 0;
  }

  long state = digitalRead(PIR_PIN_MODE);

  if (state == HIGH) {
    //      Serial.print("touchValue -> ");
    //      Serial.println(touchValue);

    //Also can use Firebase.set instead of Firebase.setDouble
    if (Firebase.setBool(fbdo2, "/Security/alert", true))
    {
      //        Serial.println("PASSED");
      //        Serial.println("PATH: " + fbdo2.dataPath());
      //        Serial.println("TYPE: " + fbdo2.dataType());
      //        Serial.println("ETag: " + fbdo2.ETag());
      //        Serial.print("VALUE: ");
      //        Serial.println("------------------------------------");
      //        Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo2.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    delay(5000);
  }

  //  StaticJsonDocument<1000> jsonDocument;
  //
  //  DeserializationError error = deserializeJson(jsonDocument, Serial);
  //
  //  if (error) {
  //    //    Serial.print("Error Occurred -> ");
  //    //    Serial.println(error.c_str());
  //    return;
  //  }
  //
  //  //  Serial.println("Json received and parsed successfully.");
  //
  //  bool intruderFaceStatus = jsonDocument["face_status"];
  //
  //  //  Serial.print("face_status -> ");
  //  //  Serial.println(intruderFaceStatus);
  //
  //  if (intruderFaceStatus) {
  //    if (Firebase.setBool(fbdo2, "/Security/intruder_face_status", true))
  //    {
  //      //        Serial.println("PASSED");
  //      //        Serial.println("PATH: " + fbdo2.dataPath());
  //      //        Serial.println("TYPE: " + fbdo2.dataType());
  //      //        Serial.println("ETag: " + fbdo2.ETag());
  //      //        Serial.print("VALUE: ");
  //      //        Serial.println("------------------------------------");
  //      //        Serial.println();
  //    }
  //    else
  //    {
  //      Serial.println("FAILED");
  //      Serial.println("REASON: " + fbdo2.errorReason());
  //      Serial.println("------------------------------------");
  //      Serial.println();
  //    }
  //  }

}
