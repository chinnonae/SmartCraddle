#include <Servo.h>
#include <pt.h>

#define servo1Pin 3
#define servo2Pin 11
#define moisturePin 13
#define soundPin A1
#define buzzerPin 10

#define PT_DELAY(pt, ms, ts)\
  ts = millis();\
  PT_WAIT_WHILE(pt, millis()-ts < (ms));

struct pt pt_soundTask;
struct pt pt_servoTask;
struct pt pt_moistureTask;
struct pt pt_buzzerTask;
struct pt pt_sendSerial;
struct pt pt_maxTask;

int soundLevel = 0;
int moisture = 1;
int maxDegree = 20;
int servoSpeed = 0;
int deltaDegree = 0;
int tempServoSpeed = 0;
int maxInSecond = 0;
String wet = "D";
String recievedMSG = "0";
#define detected 0
#define notDetected 1

Servo servo1;
Servo servo2;


PT_THREAD(soundTask(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    soundLevel = analogRead(soundPin);
    if (maxInSecond < soundLevel) maxInSecond = soundLevel;
    PT_DELAY(pt, 500, ts);
  }
  PT_END(pt);
}

PT_THREAD(servoTask(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {

    if ( (servoSpeed == 0 && deltaDegree != 0) || servoSpeed) {
      if (deltaDegree <= 20) {
        servo1.write(90 + deltaDegree);
        servo2.write(90 - deltaDegree);
      } else if ( deltaDegree <= 40 ) {
        servo1.write(90 + 40 - deltaDegree);
        servo2.write(90 - 40 + deltaDegree);
      } else if ( deltaDegree <= 60 ) {
        servo1.write(90 + 40 - deltaDegree);
        servo2.write(90 - 40 + deltaDegree);
      } else if ( deltaDegree <= 80 ) {
        servo1.write(90 - 80 + deltaDegree);
        servo2.write(90 + 80 - deltaDegree);
      }
      if (servoSpeed != 0) {
        tempServoSpeed = servoSpeed;
        deltaDegree += servoSpeed;
      } else {
        deltaDegree += tempServoSpeed;
      }
      if (deltaDegree > 80) {
        deltaDegree = 0;
      }
      PT_DELAY(pt, 100, ts);
    } else {
      servo1.write(90);
      servo2.write(90);
      PT_DELAY(pt, 2000, ts);
    }

  }
  PT_END(pt);
}

PT_THREAD(moistureTask(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    moisture = digitalRead(moisturePin);
    PT_DELAY(pt, 300, ts);
  }
  PT_END(pt);
}

PT_THREAD(buzzerTask(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {

    if (moisture == detected) {
      wet = "W";
      analogWrite(buzzerPin, 100);
    } else {
      analogWrite(buzzerPin, 0);
      wet = "D";
    }
    PT_DELAY(pt, 300, ts);
  }
  PT_END(pt);
}

PT_THREAD(sendSerial (struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    Serial1.println(wet + String(maxInSecond));
    maxInSecond = 0;
    PT_DELAY(pt, 1000, ts);
  }
  PT_END(pt);
}


void serialEvent() {
  if (Serial1.available() > 0) {
    recievedMSG = Serial1.readStringUntil('\r');
    Serial.println("Value Recieve : " + recievedMSG);
    switch (recievedMSG.charAt(0)) {
      case '0':
        servoSpeed = 0;
        break;
      case '1':
        servoSpeed = 1;
        break;
      case '2':
        servoSpeed = 2;
        break;
    }
    Serial1.flush();
  }
}

void setup() {
  pinMode(soundPin, INPUT);
  pinMode(moisturePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  PT_INIT(&pt_moistureTask);
  PT_INIT(&pt_buzzerTask);
  PT_INIT(&pt_soundTask);
  PT_INIT(&pt_sendSerial);
  PT_INIT(&pt_servoTask);
  Serial.begin(9600);
  Serial1.begin(115200);



}

void loop() {
  moistureTask(&pt_moistureTask);
  buzzerTask(&pt_buzzerTask);
  soundTask(&pt_soundTask);
  sendSerial(&pt_sendSerial);
  servoTask(&pt_servoTask);
  serialEvent();
  Serial.println("Moisture : " + String(moisture));
  Serial.println("SoundLevel : " + String(soundLevel));

}
