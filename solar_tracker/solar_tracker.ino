#include <Servo.h>

// MARK: - Properties
// Objects
Servo servoPin;

// Pinouts
int ledForAutoIndicator = 2;
int potentiometerPin = A0;
int buttonPin = 4;

// Default Values
bool autoTrack = true;
int servoStepValue = 0;
int buttonState = LOW;

// Delay Handling
unsigned long previousMillis = 0;
const long interval = 15;

// MARK: - Lifecycle
void setup() {
    Serial.begin(9600);
    servoPin.attach(3);
    servoPin.write(servoStepValue);
    pinMode(ledForAutoIndicator, OUTPUT);
    pinMode(buttonPin, INPUT);
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

    if (computedValueA > computedValueB) {
        if (servoShouldMoveWithCalculatedValues(computedValueA, computedValueB)) {
            increaseServoStep();
        }
    } else if (computedValueA < computedValueB) {
        if (servoShouldMoveWithCalculatedValues(computedValueB, computedValueA)) {
            decreaseServoStep();
        }
    }
}

void trackManually() {
    digitalWrite(ledForAutoIndicator, LOW);

    int currentFinalValue = analogRead(potentiometerPin);
    currentFinalValue = map(currentFinalValue, 0, 1023, 0, 180);

    servoPin.write(currentFinalValue);
}

boolean servoShouldMoveWithCalculatedValues(int computedValueA, int computedValueB) {
    int difference = computedValueA - computedValueB;
    int diffPercentage = computedValueA * 0.04;

    if (diffPercentage <= difference) {
        return true;
    }

    return false;
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
