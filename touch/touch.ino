#define touch D4

int value = 0;
int touchSeconds = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(touch, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  value = digitalRead(touch);

  if (value == HIGH) {
    touchSeconds++;
    Serial.println(touchSeconds);
    
    if (touchSeconds >= 1000) {
      Serial.print("value -> ");
      Serial.println(value);
      
      touchSeconds = 0;
      delay(5000);
    }
  } else {
    touchSeconds = 0;
  }
}
