// Smart Garage Project
// Semi-final project for robotics workshop in Beta
// Team ER

#define BLYNK_PRINT Serial  

#define BLYNK_TEMPLATE_ID           "TMPL2gzc_2L7B"
#define BLYNK_TEMPLATE_NAME         "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "umNDAFL8xWFozGB_op2WOShTwZbLYyDD"

// Include libraries
#include <BlynkSimpleStream.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// WiFi credentials
char ssid[] = "Joudi's iPhone";  
char pass[] = "joudi1234";  

// Initialize SoftwareSerial to communicate with ESP8266
#define RX_PIN 10
#define TX_PIN 12
SoftwareSerial espSerial(RX_PIN, TX_PIN); // RX, TX

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
int energySavingMode = 0; 
int lightState = 0;
int carCount = 0;  // Number of cars in the garage
int doorState = 0;  

// Variables to track Blynk notifications:
bool lightOn = false;  
bool lightOff = false;
bool fullGarage = false;
bool collision = false;
bool fire = false;

// Blynk virtual pins
// Energy Saving Mode Control
BLYNK_WRITE(V1) {
  energySavingMode = param.asInt();
}

// Manual Light Control
BLYNK_WRITE(V2) {
  lightState = param.asInt();
  for (int i = 0; i < 2; i++) {  
    digitalWrite(ledPins[i], lightState);
  }
}

// Manual gate control
BLYNK_WRITE(V3) {
  doorState = param.asInt();
  if (doorState == 1) {
    for (int m = 0; m <= 90; m++) {
      Garagedoor.write(m);
      delay(10);
    }
  } else {
    for (int m = 90; m >= 0; m--) {
      Garagedoor.write(m);
      delay(10);
    }
  }
}

// LCD
// Temporary messages appear in the middle of the 2nd row
void centerMessage(const char* message) {
  lcd.clear();  // Clear the LCD before writing new message
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
  // Define pin modes for TX and RX for SoftwareSerial
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  
  // Start serial communication and Blynk
  Serial.begin(9600);
  espSerial.begin(9600); 
  Blynk.begin(espSerial, BLYNK_AUTH_TOKEN);

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
  Blynk.run();

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