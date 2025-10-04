#include <Servo.h>

#define PIN_TRIG 12
#define PIN_ECHO 13
#define PIN_SERVO 10

Servo myServo;
unsigned long MOVING_TIME = 3000;
unsigned long moveStartTime;
unsigned long lastDetectTime = 0;

int startAngle = 90; 
int stopAngle  = 30; 
bool moving = false;
bool gateUp = false;

float smoothStep (float x){
  const float k =8.0;
  if (x < 0.0) x = 0.0;
  if (x > 1.0) x = 1.0;
  return x * x * (3 - 2 * x);
}

float getDistance(){
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000);
  return duration * 0.0343 / 2.0;
}

void setup() {

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
 
  myServo.attach(PIN_SERVO);
  myServo.write(startAngle); // Set position
  delay(500);
}

void loop() {
  float distance = getDistance();

  if (!moving && !gateUp && distance < 15.0){
   gateUp = true;
   moveStartTime = millis();
   moving = true;
   lastDetecttime = millis();
  }

  if (!moving && gateUp && distance > 30.0 && millis() - lastDetectTime > 2000){
   gateUp = false;
   moveStartTime = millis();
   moving = true;
  }

  if (moving){
    unsigned long progress = millis() - moveStartTime;

    if (progress <= MOVING_TIME) {
    // while moving
      float t = (float)progress / MOVING_TIME;
      float s = smoothStep(t);
      
      int angle;
      if (gateUp){
        angle = startAngle + (stopAngle - startAngle) * s;
      }
      else{
        angle = stopAngle + (startAngle - stopAngle) * s;
      }
      myServo.write(angle); 
      
    } else {
      moving = false;
      myServo.write(gateUp ? stopAngle : startAngle);
    }
  }
  delay(30);
}
