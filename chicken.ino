#include <Wire.h>
#include "RTClib.h"
#include <math.h>
#include <Dusk2Dawn.h>

#define DEBUG true
#define Serial if(DEBUG)Serial

enum LoopState { checkTime, doorOpening, doorClosing };

const int doorOpenedPin = 4;
const int doorClosedPin = 5;

const int motorUpPin = 7;
const int motorDownPin = 8;
const int motorPWM = 9;

const int sunriseDelay = -30;
const int sunsetDelay = 30;

int currentMins;
int sunrise;
int sunset;

DateTime now;

RTC_DS1307 rtc;

Dusk2Dawn city(46.504810, 6.625692, +2);

LoopState loopState = checkTime;

void setup() {
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif

  Serial.begin(9600);

  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }

  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(doorOpenedPin, INPUT);
  pinMode(doorClosedPin, INPUT);
  pinMode(motorPWM, OUTPUT);
  pinMode(motorUpPin, OUTPUT);
  pinMode(motorDownPin, OUTPUT);

  digitalWrite(motorUpPin, LOW);
  digitalWrite(motorDownPin, LOW);

  if (digitalRead(doorOpenedPin) == HIGH) {
    Serial.println("Door is open");
  }

  if (digitalRead(doorClosedPin) == HIGH) {
    Serial.println("Door is closed");
  }

  delay(3000);
}

void loop() {

  now = rtc.now();

  sunrise = city.sunrise(now.year(), now.month(), now.day(), false);
  sunset = city.sunset(now.year(), now.month(), now.day(), false);

  currentMins = now.hour() * 60 + now.minute();

  /*
  char time[6];

  Dusk2Dawn::min2str(time, sunrise + sunriseDelay);
  Serial.println("opening door at "); // 06:58
  Serial.println(time); // 06:58

  Dusk2Dawn::min2str(time, sunset + sunsetDelay);
  Serial.println("closing door at");
  Serial.println(time);

  Dusk2Dawn::min2str(time, currentMins);
  Serial.println("currentMins");
  Serial.println(time);
  */

  switch(loopState) {
    case checkTime :
      if (sunrise + sunriseDelay <= currentMins && currentMins < sunset + sunsetDelay) {
        checkDoorOpened();
      } else {
        checkDoorClosed();
      }
      break;
    case doorOpening :
      Serial.println("Opening door");
      if(digitalRead(doorOpenedPin) == LOW) {
        Serial.println("Door not fully openend");
        delay(150);
      } else {
        Serial.println("Door is openend");
        loopState = checkTime;
        stopDoor();
      }
      break;
    case doorClosing :
      Serial.println("Closing door");
      if(digitalRead(doorClosedPin) == LOW) {
        Serial.println("Door not fully closed");
        delay(150);
      } else {
        Serial.println("Door is closed");
        loopState = checkTime;
        stopDoor();
      }
      break;
  }
}

void checkDoorOpened() {
  Serial.println("Door should be opened");
  if (digitalRead(doorOpenedPin) == LOW) {
    Serial.println("Door isn't opened.");
    openDoor();
    loopState = doorOpening;
  } else {
    long minutesTillSunset = sunset + sunsetDelay - currentMins;
      Serial.println("sunset");
      Serial.println(sunset);
      Serial.println("currentMins");
      Serial.println(currentMins);
      Serial.print("Door is opened, wait ");
      Serial.print(minutesTillSunset);
      Serial.println("min till sunset");

      long waiting_time =  minutesToMillis(minutesTillSunset);
      Serial.println("WAITING TIME");
      Serial.println(waiting_time);
    delay(waiting_time);
  }
}

void checkDoorClosed() {
  Serial.println("Door should be closed");
  if (digitalRead(doorClosedPin) == LOW) {
    Serial.println("Door isn't closed.");
    closeDoor();
    loopState = doorClosing;
  } else {
    Serial.print("Door is closed and it's the ");
      long minutesTillSunrise;
      int delayedSunrise = sunrise + sunriseDelay;
    if(currentMins <= delayedSunrise) {
      Serial.println("morning.");
        minutesTillSunrise = delayedSunrise - currentMins;
    } else {
      Serial.print("night. ");
        minutesTillSunrise = delayedSunrise + (1440 - currentMins);
    }
    Serial.print("Wait ");
      Serial.print(minutesTillSunrise);
      Serial.println("min till sunrise");

      long waiting_time =  minutesToMillis(minutesTillSunrise);
      Serial.println("WAITING TIME");
      Serial.println(waiting_time);
    delay(waiting_time);
  }
}

void openDoor() {
  analogWrite(motorPWM, 90);
  digitalWrite(motorUpPin, HIGH);
  digitalWrite(motorDownPin, LOW);
  Serial.println("Opening door");
}


void closeDoor() {
  analogWrite(motorPWM, 80);
  digitalWrite(motorDownPin, HIGH);
  digitalWrite(motorUpPin, LOW);
  Serial.println("Closing door");
}

void stopDoor() {
  analogWrite(motorPWM, 0);
  digitalWrite(motorUpPin, LOW);
  digitalWrite(motorDownPin, LOW);
  Serial.println("Door Stoped");
}

long minutesToMillis(long minutes) {
  return minutes * 60 * 1000;
}

void printDate(DateTime dateTime) {
  Serial.println();
  Serial.print(dateTime.year());
  Serial.print('/');
  Serial.print(dateTime.month());
  Serial.print('/');
  Serial.print(dateTime.day());
  Serial.print(" - ");
  Serial.print(dateTime.hour());
  Serial.print(':');
  Serial.print(dateTime.minute());
  Serial.print(':');
  Serial.println(dateTime.second());
}
