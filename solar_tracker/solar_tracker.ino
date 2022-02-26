#include <Servo.h>

// MARK: - Properties
// Objects
Servo servoPin;

// Pinouts
const int ledForAutoIndicator = 2;
const int potentiometerPin = A0;
const int buttonPin = 4;

// Default Values
bool autoTrack = true;
bool didFindSunlight = false;
bool findSunlightInReverse = false;
int servoStepValue = 0;
int buttonState = LOW;
int targetSunlight = 960;
int recordedHighestSunlight = 0;

// Delay Handling
unsigned long previousMillis = 0;
const long interval = 20;

// MARK: - Lifecycle
void setup() {
    servoPin.attach(3);
    servoPin.write(servoStepValue);
    pinMode(ledForAutoIndicator, OUTPUT);
    pinMode(buttonPin, INPUT);
    delay(1000);
}

void loop() {   
    buttonHandler();

    if (autoTrack) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            trackSunlightAutomatically();
        }
    } else {
        trackManually();
    }
}

// MARK: - Helpers
void buttonHandler() {
    if (digitalRead(buttonPin) == HIGH && buttonState == LOW) {
        buttonState = HIGH;
        autoTrack = !autoTrack;
    } else {
        buttonState = digitalRead(buttonPin);
    }
}

void trackSunlightAutomatically() {
    digitalWrite(ledForAutoIndicator, HIGH);

    int ldrValueA = analogRead(A1);
    int ldrValueB = analogRead(A2);
    int ldrValueC = analogRead(A3);

    int computedValueA = (ldrValueA + ldrValueB) / 2;
    int computedValueB = (ldrValueB + ldrValueC) / 2;

    if (didFindSunlight) {
        trackSunlight(computedValueA, computedValueB);
    } else {
        findSunlight(computedValueA, computedValueB);
    }
}

void findSunlight(int valueA, int valueB) {
    int highestValue = 0;

    if (valueA > valueB) {
        highestValue = valueA;
    } else {
        highestValue = valueB;
    }

    if (highestValue > recordedHighestSunlight) {
        recordedHighestSunlight = highestValue;
    }

    if (recordedHighestSunlight >= targetSunlight) {
        didFindSunlight = true;
    } else {

        if (findSunlightInReverse) {
            servoStepValue--;

            if (servoStepValue == 0) {
                findSunlightInReverse = false;
                targetSunlight = recordedHighestSunlight;
                recordedHighestSunlight = 0;
            }

        } else {
            servoStepValue++;

            if (servoStepValue == 180) {
                findSunlightInReverse = true;
                targetSunlight = recordedHighestSunlight;
                recordedHighestSunlight = 0;
            }
        }

        servoPin.write(servoStepValue);
    }
}


void trackSunlight(int valueA, int valueB) {
    if (valueA > valueB) {
        if (servoShouldMoveWithCalculatedValues(valueA, valueB)) {
            increaseServoStep();
        }
    } else if (valueA < valueB) {
        if (servoShouldMoveWithCalculatedValues(valueB, valueA)) {
            decreaseServoStep();
        }
    }
}

void trackManually() {
    digitalWrite(ledForAutoIndicator, LOW);
    didFindSunlight = true;

    servoStepValue = analogRead(potentiometerPin);
    servoStepValue = map(servoStepValue, 0, 1023, 0, 180);

    servoPin.write(servoStepValue);
}

boolean servoShouldMoveWithCalculatedValues(int valueA, int valueB) {
    int difference = valueA - valueB;
    int diffPercentage = valueA * 0.01;

    return diffPercentage <= difference;
}

void decreaseServoStep() {
    if (servoStepValue == 0) return; 

    servoStepValue--;
    servoPin.write(servoStepValue);
}

void increaseServoStep() {
    if (servoStepValue == 180) return;

    servoStepValue++;
    servoPin.write(servoStepValue);
}
