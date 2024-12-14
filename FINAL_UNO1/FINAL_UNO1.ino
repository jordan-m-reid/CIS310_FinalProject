/*
Jordan Reid, Hussein Chami, Hakan Turk
CIS310 - Dr. Roy
Final Project
UM-Dearborn CECS
*/

#include <Servo.h>                // Necessary library for use of Servo motors
#include <Relay.h>                // Library for creating relay class objects with relay-specific attributes
#include <DHT.h>                  // Library required for use of DHT sensors
#include <string.h>               // May not really be necessary, but included for UART serial communication
#include <SoftwareSerial.h>       // Used for UART serial communication between boards because the UNO otherwise
                                  // only communicates with the serial monitor on the PC
// Define analog input pins
int tempHum = A0;                 // Analog pin 0 assigned to DHT11 temp/humidity sensor
int soilMoisture = A2;            // Analog pin 2 assigned to capacitative soil moisture sensor

// Define digital output pins
int windowServo = 4;              // Digital pin 4 assigned to servo motor controlling window position
int fanRelay = 7;                 // Digital pin 7 assigned to relay switch controlling fan
int pumpRelay = 9;                // Digital pin 9 assigned to relay switch controlling pump

// Relay, servo, and sensor objects
#define DHT_TYPE DHT11                        // Define sensor type for use with DHT class
DHT tempHumSensor = DHT(tempHum, DHT_TYPE);   // Declare object for DHT11 sensor with DHT class constructor: DHT(pin, sensor_type)
Relay FAN_relay = Relay();                    // Declare relay object for the switch that will control the fan
Relay PUMP_relay = Relay();                   // Declare relay object for the switch that will control the pump
Servo window;                                 // Declare a servo motor object for the servo motor controlling the window

// Variables for data collection
float temperature;          // Temperature from DHT11
float tempNano;             // Temperature sent from NANO
float humidity;             // Humidity from DHT11
float humidityNano;         // Humidity sent from NANO
int moistureLvl;            // Analog reading from soil moisture sensor
float pressureNano;         // Barometric pressure sent from NANO
float SET_TEMP = 26.00;     // Threshold temperature setting (MAXIMUM)
float SET_HUM = 60.00;      // Threshold humidity setting (MAXIMUM)

// Variable for servo control
const int CLOSED = 75;      // Angle of servo motor for CLOSED position
const int OPENED = 0;       // Angle of servo motor for OPENED position

// Variables for holding and parsing received data via UART
int Rx = 6;                             // Receiving pin for UART object
int Tx = 10;                            // Transmission pin for UART object
SoftwareSerial Serial1(Rx,Tx);          // UART object for serial communication
const byte numCharsMax = 32;            // Maximum length of data that can be received in characters
char receivedChars[numCharsMax];        // Array to store received characters via UART 
char tempChars[numCharsMax];            // Array for parsing/manipulating received characters
char sensorType[numCharsMax] = {0};     // Array for parsing received characters
float nanoT = 0.00;                     // Float value to store parsed temperature data from NANO
float nanoH = 0.00;                     // Float value to store parsed humidity data from NANO
float nanoP = 0.00;                     // Float value to store parsed pressure data from NANO
float tempFloat;                        // Float value to store incoming data from NANO before category determined
bool newData = false;                   // Boolean value to track whether incoming data is part of previous packet or new

void setup() {
  // Begin serial monitoring/UART communication at 9600 bps
  Serial.begin(9600);         // Serial monitor on PC
  Serial1.begin(9600);        // Software serial object for communication with NANO (NANO must be at same baud rate)

  // Set input and output pin modes
  pinMode(tempHum, INPUT);        // DHT11 sensor pin assigned as INPUT
  pinMode(soilMoisture, INPUT);   // Soil moisture sensor pin assigned as INPUT
  pinMode(windowServo, OUTPUT);   // Servo motor pin assigned as OUTPUT
  pinMode(fanRelay, OUTPUT);      // Fan relay switch pin assigned as OUTPUT
  pinMode(pumpRelay, OUTPUT);     // Pump relay switch pin assigned as OUTPUT
  pinMode(Rx, INPUT);             // Receiving pin for Serial comm. assigned as INPUT

  // Initialize relays, servo, and temp/humidity sensor
  tempHumSensor.begin();            // Initialize DHT11 sensor
  FAN_relay.begin();                // Initialize fan relay object
  PUMP_relay.begin();               // Initialize pump relay object
  digitalWrite(pumpRelay, HIGH);    // Send signal to pump relay to ensure it is off (This relay is inverted to normal)
  window.attach(windowServo);       // Assigned pin for servo motor attached to digital servo object
  window.write(OPENED);             // Send test signal to servo motor to open window
  delay(500);                       // Wait 500 ms
  window.write(CLOSED);             // Send test signal to servo motor to close window
  pinMode(windowServo, INPUT);      // Change servo motor pin to input for reading position

}

void loop() {
  // Collect data values
  temperature = tempHumSensor.readTemperature();  // Read temperature value from DHT11 sensor
  humidity = tempHumSensor.readHumidity();        // Read humidity value from DHT11 sensor
  moistureLvl = analogRead(soilMoisture);         // Read soil moisture analog signal 
  receiveData();                                  // Receive data from Nano 33 BLE Sense Rev2 
  if (newData == true) {                          // Evaluate whether or not the most recently received data is new
    strcpy(tempChars, receivedChars);             // Copy any received data into a temporary array for parsing
    dataParsing();                                // Parse data into a readable format
    newData = false;                              // Indicate that received data has been processed
  }
  
  // Transform data values as needed
  float moisturePct = map(moistureLvl, 200, 800, 100, 0); // Maps values from a given [low, high] to percentage

  // Display collected and received data to serial monitor
  Serial.print("Temp = ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Moisture Level = ");
  Serial.print(moisturePct);
  Serial.println(" %");
  Serial.print("Nano Temp = ");
  Serial.print(nanoT);
  Serial.println(" C");
  Serial.print("Nano Humidity = ");
  Serial.print(nanoH);
  Serial.println(" %");
  Serial.print("Nano Pressure = ");
  Serial.print(nanoP);
  Serial.println(" kPa");

  // Determine output signals based on collected data values
  if (temperature > SET_TEMP) {       // If temperature is above a set threshold SET_TEMP
    if (!FAN_relay.getState()) {      // If the state of the fan relay switch is OFF
      FAN_relay.turnOn();             // Toggle fan relay switch state to ON
      digitalWrite(fanRelay, HIGH);   // Write signal to fan relay switch to turn ON
    }
  }

  if (temperature < SET_TEMP) {       // If temperature is below the SET_TEMP threshold 
    if (FAN_relay.getState()) {       // If the fan relay switch state is ON
      FAN_relay.turnOff();            // Toggle fan relay switch state to OFF
      digitalWrite(fanRelay, LOW);    // Write signal to fan relay switch to turn OFF
    }
  }

  if (humidity > SET_HUM) {             // If humidity is greater than SET_HUM threshold
    if (window.read() == CLOSED) {      // If the current position of the window servo is CLOSED
      pinMode(windowServo, OUTPUT);     // Set servo pin to OUTPUT to send position signal
      window.write(OPENED);             // Send OPENED position to window servo
      pinMode(windowServo, INPUT);      // Set servo pin back to INPUT to read position signal
    }
  }
  else {                                // If humidity is <= SET_HUM threshold
    if (window.read() == OPENED) {      // If the current position of the window servo is OPENED
      pinMode(windowServo, OUTPUT);     // Set servo pin to OUTPUT to send position signal
      window.write(CLOSED);             // Send CLOSED position to the window servo
      pinMode(windowServo, INPUT);      // Set servo pin back to INPUT to read position signal
    }
  }
  
  // Soil moisture comparison controls the relay module attached to the pump
  if (moistureLvl > 600) {                // Higher values are drier values
    if (!PUMP_relay.getState()) {         // Check whether relay is already active
      PUMP_relay.turnOn();                // Turn on relay object state var
      digitalWrite(pumpRelay, LOW);       // Send turn-on signal to relay switch
      while (PUMP_relay.getState()) {     // Check soil moisture levels continually
        moistureLvl = analogRead(soilMoisture); // while pump is active
        delay(50);                        // 50 ms delay for sensor reading
        if (moistureLvl < 600) {          // Look for moisture level below threshold
          digitalWrite(pumpRelay, HIGH);  // Send relay shut-off signal
          PUMP_relay.turnOff();           // Change relay object state var to off
        }
      }
    }
  }
  
  //delay(1000);

}

void receiveData() {                      // Function to allow for the receipt and storage of available data from NANO
  bool receiveInProg = false;             // Adapted from the example #5 on Serial Input Basics on Arduino Forum
  byte index = 0;                         // URL: https://forum.arduino.cc/t/serial-input-basics-updated/382007/3
  char start = '<';                       // Data sent and received as Cstring arrays
  char end = '>';
  char newChar;

  while (Serial1.available() > 0 && newData == false) {
    newChar = Serial1.read();
    if (receiveInProg == true) {
      if (newChar != end) {
        receivedChars[index] = newChar;
        index += 1;
        if (index >= numCharsMax) {
          index = numCharsMax - 1;         // Keeps new characters while throwing away most recent (if limits exceeded)
        }
      }
      else {
        receivedChars[index] = '\0';       // Cstring end-of-string character
        receiveInProg = false;
        index = 0;
        newData = true;
      }
    }
    else if (newChar == start) {
      receiveInProg = true;
    }
  }
}

void dataParsing() {                          // Function to process stored data from NANO
  char *strtok_Index;                         // Adapted from the example #5 on Serial Input Basics on Arduino Forum
  strtok_Index = strtok(tempChars, ",");      // URL: https://forum.arduino.cc/t/serial-input-basics-updated/382007/3
  strcpy(sensorType, strtok_Index);           // Data sent and received as Cstring arrays
  strtok_Index = strtok(NULL, "\0");          // Function separates the data into readable form
  tempFloat = atof(strtok_Index);             // e.g. <P, 101.32> into 'P' and 101.32
  if (sensorType[0] == 'T') {                 // Character is stored in sensorType array and used to evaluate data type
    nanoT = tempFloat;
  }
  else if (sensorType[0] == 'H') {
    nanoH = tempFloat;
  }
  else if (sensorType[0] == 'P') {
    nanoP = tempFloat;
  }
  else {
    Serial.print(sensorType[0]);
    Serial.println(tempFloat);
  }

}
