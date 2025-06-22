#include "Stepper.h"

const int PUNCH_PERIOD = 100; // Solenoid pulse duration

const int INDEX_PUNCH = 18;
const int DATA_PUNCHES[] = {6, 7, 19, 17, 16, 15, 14, 13}; // LSB ... MSB

const int stepsPerRevolution = 2038;
const int stepsPerRow = -111; // Adjust for 2.54mm spacing
Stepper theStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);

const int GROUND_THRESHOLD = 50; // Value to detect "grounded" condition for analog switch inputs
#define FEED_SW0 A6
#define FEED_SW1 A7

void punchByte(byte data) {
  fireSolenoid(INDEX_PUNCH);
  if (data) {
    for (int i = 0; i < 8; i++) {
      if (data & (1 << i)) {
        fireSolenoid(DATA_PUNCHES[i]);
      }
    }
  }
  delay(400); // The solenoids needs enough time to return before feeding
  feed();
}

void feed() {
  theStepper.step(stepsPerRow);
}

void fireSolenoid(int punch) {
  digitalWrite(punch, HIGH);
  delay(PUNCH_PERIOD);
  digitalWrite(punch, LOW);
}

void testFire() {
  // Slowly fire each punch
  fireSolenoid(INDEX_PUNCH);
  for (int i = 0; i < 8; i++) {
    delay(500);
    fireSolenoid(DATA_PUNCHES[i]);
  }
  feed();
  feed();
  feed();
}

void pollFeedButtons() {
  int sw0Initial = analogRead(FEED_SW0);
  int sw1Initial = analogRead(FEED_SW1);

  if (sw0Initial > GROUND_THRESHOLD && sw1Initial > GROUND_THRESHOLD) {
    return;
  }

  // delay(100);

  int sw0Final = analogRead(FEED_SW0);
  int sw1Final = analogRead(FEED_SW1);

  bool groundedsw0 = sw0Final < GROUND_THRESHOLD;
  bool groundedsw1 = sw1Final < GROUND_THRESHOLD;

  if (groundedsw0 && groundedsw1) {
    // Pressing both switches punches code holes
    punchByte(255);
  } else if (groundedsw0) {
    // Switch 0 only feeds
    feed();
  } else if (groundedsw1) {
    // Switch 1 punches feed holes
    punchByte(0);
  }
}

void setup() {
  pinMode(INDEX_PUNCH, OUTPUT);
  for (int i = 0; i < 9; i++) {
    pinMode(DATA_PUNCHES[i], OUTPUT);
  }

  theStepper.setSpeed(15);

  // Run test on boot if SW0 is pressed
  if (analogRead(FEED_SW0) < GROUND_THRESHOLD) {
    testFire();
  }

  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    punchByte(Serial.read());
  } else {
    pollFeedButtons();
  }
}
