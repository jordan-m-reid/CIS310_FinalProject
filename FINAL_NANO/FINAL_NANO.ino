/*
Jordan Reid, Hussein Chami, Hakan Turk
CIS310 - Dr. Roy
Final Project
UM-Dearborn CECS
*/

#include <Arduino_HS300x.h>         // Library for onboard temperature and humidity sensor for NANO 33 BLE Sense Rev2
#include <Arduino_LPS22HB.h>        // Library for onboard barometric pressure sensor for NANO 33 BLE Sense Rev2

// Assign digital output pins
int alarm = 11;                     // Assigns digital pin 11 to a variable representing a buzzer (alarm)
const char T = 'T';                 // Character key for temperature (UART communication)
const char H = 'H';                 // Character key for humidity (UART communication)
const char P = 'P';                 // Character key for pressure (UART communication)

// Assign analog input pins
int fireSensor = A2;                // Assigns analog pin 2 to the flame sensor, a photoresistor sensitive to infrared (radiant heat energy)
int fire;                           // Variable to store reading from flame sensor (0-1023)

void setup() {
  // Begin serial monitor
  Serial.begin(9600);               // Begin communication to serial monitor on PC
  Serial1.begin(9600);              // Begin communication through built-in UART Tx pin on NANO
  if (!HS300x.begin()) {            // Check if onboard temperature and humidity sensor is initialized properly, issue warning if not
    Serial.println("Failed to initialize humidity & temperature sensor.");
  }
  if (!BARO.begin()) {              // Check if onboard barometric pressure sensor is initialized properly, issue warning if not
    Serial.println("Failed to initialize pressure sensor.");
  }
  // Assign output pin to alarm
  pinMode(alarm, OUTPUT);           // Set the pin assigned to the alarm as OUTPUT
  pinMode(fireSensor, INPUT);       // Set the pin assigned to the flame sensor as INPUT
  digitalWrite(alarm, LOW);         // Ensure the alarm is not sounding on start-up
}

void loop() {
  // read all the sensor values
  float temperature = HS300x.readTemperature();   // Read temperature from temperature and humidity sensor
  float humidity    = HS300x.readHumidity();      // Read humidity from temperature and humidity sensor
  float pressure = BARO.readPressure();           // Read pressure from pressure sensor

  // Print each of the sensor values
  Serial.print("Temperature = ");           // Print to serial monitor on PC
  Serial.print(temperature);
  Serial.println(" °C");
  Serial1.write('<');                       // Print to UART serial communication for transmission
  Serial1.write(T);                         // Designed to integrate with a receiver program that will recognize
  Serial1.write(',');                       // start '<' and end '>' characters to separate transmission packets
  Serial1.write(' ');                       // Data written to UART serial communication Tx line is of the following form:
  Serial1.print(temperature);               // '<T, ' TEMPERATURE_READING_AS_FLOAT '>' which may appear as '<T, '24.32'>' for example,
  Serial1.write('>');                       // where the numerical value is printed as a float value comprising 4 bytes of data (32-bit)
                                            // and all other characters (including '<' ',' ' ' '>') are sent as true character values
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial1.write('<');                       // '<H, ' HUMIDITY_READING_AS_FLOAT '>' which may appear as '<H, '24.32'>' for example
  Serial1.write(H);
  Serial1.write(',');
  Serial1.write(' ');
  Serial1.print(humidity);
  Serial1.write('>');

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");
  Serial1.write('<');                       // '<P, ' PRESSURE_READING_AS_FLOAT '>' which may appear as '<P, '101.32'>' for example
  Serial1.write(P);
  Serial1.write(',');
  Serial1.write(' ');
  Serial1.print(pressure);
  Serial1.write('>');

  // print an empty line
  Serial.println();
  fire = analogRead(fireSensor);            // Use analogRead() function to take a reading from the flame sensor pin, value will be between 0-1023
  if (fire > 400) {                         // Arbitrarily assigned detection threshold for when the flame sensor should be considered a logical HIGH signal
    digitalWrite(alarm, HIGH);              // Turn buzzer ON by sending logical HIGH to pin assigned 'alarm'
    delay(500);                             // Wait 500 ms
    digitalWrite(alarm, LOW);               // Turn buzzer OFF
    delay(500);                             // Wait 500 ms
  }                                         // Sensors continue reading on each loop, even if fire is detected
  else {
    digitalWrite(alarm, LOW);               // Ensure alarm is off if fire is not detected
  }
  //delay(1000); removed because of interference with UART communication timing
}
