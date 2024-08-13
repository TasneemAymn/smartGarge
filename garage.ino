// Smart Garage Project
// Semi-final project for robotics workshop in Beta
// Team ER

// Include libraries
#include <Servo.h>
#include <LiquidCrystal.h>

// Pin definitions
// pins for lighting & gate
int ledPins[] = { 2, 4 };  
int ldrPin = A3;
int irSensorEntry = 7;  
int irSensorExit = 8;  
int servoPin = 5;
int flamePin = A5;  // pin for fire detection
int buzzerPin = A0; // alarm
// pins for ultrasonic sensor
int trigPin = A1;  
int echoPin = A2;  
// Pins for LCD
int rs = 6, en = 9, d4 = 13, d5 = 11, d6 = 3, d7 = A4;

Servo Garagedoor;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variables 
int lightState = 0;
int carCount = 0;  // Number of cars in the garage
int doorState = 0;  

bool carInProgress = false;  // Track if a car is in progress of entering
bool movementDetected = false;  // Track inactivity in the garage 
bool energySavingMode = false;


// Variables to track the time b/w sensor readings
unsigned long lastEntryCheck = 0;
unsigned long lastExitCheck = 0;

// Variables to track the time inactivity in the garage 
unsigned long lastMovementTime = 0;


// LCD
// Temporary messages appear in the middle of the 2nd row
void centerMessage(char message) {
  lcd.clear();  
  int length = strlen(message);
  int padding = (16 - length) / 2;
  lcd.setCursor(padding, 1);
  lcd.print(message);
}

void setup() {
  for (int i = 0; i < 2; i++) {  
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(ldrPin, INPUT);
  pinMode(irSensorEntry, INPUT);
  pinMode(irSensorExit, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Start serial communication 
  Serial.begin(9600);

  // Initialize LCD and servo motor
  Garagedoor.attach(servoPin);
  lcd.begin(16, 2);
  
  // Initialize gate system
  Serial.print("IR Entry Sensor Initial State: ");
  Serial.println(digitalRead(irSensorEntry));
  Serial.print("IR Exit Sensor Initial State: ");
  Serial.println(digitalRead(irSensorExit));
  Garagedoor.write(0);

}
 
void loop() {

  unsigned long currentMillis = millis();

 // Smart lighting system
  int ldrValue = analogRead(ldrPin);  

  //Print LDR Value to Serial Monitor
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  int entryState = digitalRead(irSensorEntry);
  int exitState = digitalRead(irSensorExit);

  //Print irsensor states to Serial Monitor
  Serial.print("Entry Sensor State: ");
  Serial.println(entryState);
  Serial.print("Exit Sensor State: ");
  Serial.println(exitState);

if (entryState == LOW || exitState == LOW) {
    movementDetected = true;
    lastMovementTime = millis();
}

// Switch energy saving mode on or off based on movement detection
if (millis() - lastMovementTime >= 20000) {
    energySavingMode = true;  // lights off after 20 seconds of no movement
} else {
    energySavingMode = false;  // Normal mode 
}

if (energySavingMode && lightState == 0) {
    for (int i = 0; i < 2; i++) {  
        digitalWrite(ledPins[i], LOW); 
    }
} else {
    // Normal mode
    if (ldrValue > 600) {
        for (int i = 0; i < 2; i++) {  
            digitalWrite(ledPins[i], HIGH); 
        }
    } else {
        for (int i = 0; i < 2; i++) {  
            digitalWrite(ledPins[i], LOW); 
        }
    }
}

  // Gate system  
  bool carEntering = false;
  bool carExiting = false;
  // Entering car
  if (entryState == LOW && doorState == 0 && carCount < 5 && !carEntering && !carInProgress) {
     if (currentMillis - lastEntryCheck >= 200) {  
      lastEntryCheck = currentMillis;

      Serial.println("Car entering...");
      carInProgress = true;  // Car started entering

      for (int m = 0; m <= 90; m++) {
        Garagedoor.write(m);
        delay(10);
      }
      delay(1000);

      // Wait for the car to pass the entry sensor
      while (digitalRead(irSensorEntry) == LOW) {
        delay(500);
      }

      // Check if the car has passed the exit sensor
      if (digitalRead(irSensorExit) == LOW) {
        carCount++;
        Serial.print("Car Count increased: ");
        Serial.println(carCount);
        carEntering = true;
      }

      for (int m = 90; m >= 0; m--) {
        Garagedoor.write(m);
        delay(10);
      }

      carInProgress = false;  // Car has fully entered

      // temporary LCD for entering car 
      centerMessage("Welcome!");
      delay(1000);
    }
  } else if (entryState == HIGH) {
    carEntering = false;
  }

  // Entering the car and the garage is full
  if (entryState == LOW && doorState == 0 && carCount >= 5) {
    // temporary LCD for ettempting car to enter but garage is already full
    centerMessage("Garage full!");
    delay(1000); 
  } 

  // Exiting the car
  else if (exitState == LOW && doorState == 0 && carCount > 0 && !carExiting && !carInProgress) {
    if (currentMillis - lastExitCheck >= 200) { 
      lastExitCheck = currentMillis;

      Serial.println("Car exiting...");
      carInProgress = true;  // Car started exiting

      for (int n = 0; n <= 90; n++) {
        Garagedoor.write(n);
        delay(10);
      }
      delay(1000);

      // Wait for the car to pass the exit sensor
      while (digitalRead(irSensorExit) == LOW) {
        delay(500);
      }

      // Check if the car has passed the enter sensor
      if (digitalRead(irSensorEntry) == LOW) {
        carCount--;
        Serial.print("Car Count decreased: ");
        Serial.println(carCount);
        carExiting = true;
      }

      for (int n = 90; n >= 0; n--) {
        Garagedoor.write(n);
        delay(10);
      }

      carInProgress = false;  // Car has fully exited

      //temporary LCD for exiting car
      centerMessage("Goodbye!");
      delay(1000);
    }
  }else if (exitState == HIGH) {
    carExiting = false;
  }

  // Collision detection
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.035 / 2;

  // Print distance that b/w the car and the sidewalk to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 10) {
    tone(buzzerPin, 800);
    delay(1000);
    noTone(buzzerPin);    
    delay(1000);
    // temporary LCD for collision alert
    centerMessage("Collision alert!");
    delay(1000);

  } else {
    noTone(buzzerPin);
  }

  // Fire Detection
  int flameValue = analogRead(flamePin);  

  //Print Flame Value to Serial Monitor
  Serial.print("Flame Value: ");
  Serial.println(flameValue);

  if (flameValue > 200) {  
    tone(buzzerPin, 800);
    delay(1000);
    noTone(buzzerPin);    
    delay(1000);
    // temporary LCD for fire alert
    centerMessage("Fire detected!");
    delay(1000);

  } else {
    noTone(buzzerPin);
  }

  // Empty places LCD
  if (digitalRead(irSensorExit) == HIGH && digitalRead(irSensorEntry) == HIGH && flameValue <= 200 && distance >= 10) {
      lcd.setCursor(0, 1);
      lcd.print("Empty places : ");
      lcd.print(String(5 - carCount));
    }

  // Permanent LCD message 
  lcd.setCursor(4, 0);
  lcd.print("ER Garage");

  }