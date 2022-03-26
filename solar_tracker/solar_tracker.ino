#include <Servo.h>

// MARK: - Properties
// Objects
Servo servoPin;

// Pinouts
const int ledIndicatorPin = 2;
const int potentiometerPin = A0;
const int buttonPin = 4;

// Default Values
bool autoTrack = true;
bool didFindSunlight = false;
bool findSunlightInReverse = false;
int servoStepValue = 0;
int buttonState = LOW;
int ledIndicatorState = LOW;
int targetSunlight = 960;
int recordedHighestSunlight = 0;
int trackedValuesA[20];
int trackedValuesB[20];
int trackingIndex = 0;
float tolerance = 0.028;

// Delay Handling
unsigned long previousMillisForLedBlink = 0;
unsigned long previousMillis = 0;
const long interval = 20;
const long ledIndicatorBlinkInterval = 500;

// MARK: - Lifecycle
void setup() {
    servoPin.attach(3);
    servoPin.write(servoStepValue);
    pinMode(ledIndicatorPin, OUTPUT);
    pinMode(buttonPin, INPUT);
    Serial.begin(9600);
    delay(1000);
}

void loop() {   
    // MARK: - For Debugging only!
    // servoPin.write(90);
    // ldrChecker();

    // MARK - Main Loop logic
    buttonHandler();

    if (autoTrack) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            trackSunlightAutomatically();
        }

        findingSunlightBlinkHandler(currentMillis);
    } else {
        trackManually();
    }
}

// MARK: - Helpers
void ldrChecker() {
    Serial.print(analogRead(A1));
    Serial.print("-");
    Serial.print(analogRead(A2));
    Serial.print("-");
    Serial.println(analogRead(A3));
    delay(1000);
}

void buttonHandler() {
    if (digitalRead(buttonPin) == HIGH && buttonState == LOW) {
        buttonState = HIGH;
        autoTrack = !autoTrack;
    } else {
        buttonState = digitalRead(buttonPin);
    }
}

void trackSunlightAutomatically() {
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

void findingSunlightBlinkHandler(unsigned long currentMillis) {
    if (currentMillis - previousMillisForLedBlink >= ledIndicatorBlinkInterval) {
        previousMillisForLedBlink = currentMillis;

        if(ledIndicatorState == LOW) {
            ledIndicatorState = HIGH;
        } else {
            ledIndicatorState = LOW;
        }

        digitalWrite(ledIndicatorPin, ledIndicatorState);
    }
}

void findSunlight(int valueA, int valueB) {
    int highestValue = getHigestCalculatedValue(valueA, valueB);

    if (highestValue > recordedHighestSunlight) {
        recordedHighestSunlight = highestValue;
    }

    if (recordedHighestSunlight >= targetSunlight) {
        didFindSunlight = true;
    } else {
        sweep();
    }
}

int getHigestCalculatedValue(int valueA, int valueB) {
    if (valueA > valueB) {
        return valueA;
    } 
    
    return valueA;
}

void sweep() {
    if (findSunlightInReverse) {
        decreaseServoStep();

        if (servoStepValue == 0) {
            findSunlightInReverse = false;
            targetSunlight = recordedHighestSunlight;
            recordedHighestSunlight = 0;
        }

    } else {
        increaseServoStep();

        if (servoStepValue == 180) {
            findSunlightInReverse = true;
            targetSunlight = recordedHighestSunlight;
            recordedHighestSunlight = 0;
        }
    }
}

void trackSunlight(int valueA, int valueB) {
    digitalWrite(ledIndicatorPin, HIGH);

    trackedValuesA[trackingIndex] = valueA;
    trackedValuesB[trackingIndex] = valueB;

    if (trackingIndex == 19) {
        int averageValuesA = 0;
        int averageValuesB = 0;

        for(int index = 0; index < trackingIndex; index++) {
            averageValuesA += trackedValuesA[trackingIndex];
            averageValuesB += trackedValuesB[trackingIndex];
        }

        averageValuesA /= trackingIndex;
        averageValuesB /= trackingIndex;

        if (averageValuesA > averageValuesB) {
            if (shouldServoMoveWithCalculatedValues(averageValuesA, averageValuesB)) {
                increaseServoStep();
            }
        } else if (averageValuesA < averageValuesB) {
            if (shouldServoMoveWithCalculatedValues(averageValuesB, averageValuesA)) {
                decreaseServoStep();
            }
        }
        
        trackingIndex = 0;
    } else {
        trackingIndex++;
    }
}

void trackManually() {
    digitalWrite(ledIndicatorPin, LOW);
    didFindSunlight = true;

    servoStepValue = analogRead(potentiometerPin);
    servoStepValue = map(servoStepValue, 0, 1023, 0, 180);

    servoPin.write(servoStepValue);
}

boolean shouldServoMoveWithCalculatedValues(int valueA, int valueB) {
    int difference = valueA - valueB;
    int diffPercentage = valueA * tolerance;

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
