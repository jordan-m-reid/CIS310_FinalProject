/*
Jordan Reid, Hussein Chami, Hakan Turk
CIS310 - Dr. Roy
Final Project
UM-Dearborn CECS
*/

#include <Adafruit_WS2801.h>          // Library for driving the WS2801 chip on the addressable LED strips
#include <SPI.h>                      // Required for communicating with the LED strips from the Arduino board

// Define analog input pins
int photoresistor = A0;               // Green, measures brightness with an analog signal between 0 - 1023

// Define digital output pins           Used for communicating data signal to LED strips
int LED_data1 = 6;                    // Yellow
int LED_data2 = 8;                    // Yellow
int LED_data3 = 11;                   // Yellow
                                      //Used for coordinating signals via clock pulses
int LED_clock1 = 7;                   // Green
int LED_clock2 = 9;                   // Green
int LED_clock3 = 12;                  // Green

// LED specific variables             // Combindations of RED, GREEN, and BLUE allow for color variation, all same is WHITE
int lengthLED1 = 4;                   // Length of LED strips in number of pixels
int lengthLED2 = 3;
int lengthLED3 = 3;
const int WHITE_ALL = 255;            // Maximum value of each RGB pixel (0 - 255)
int RED1 = 255;                       // Variables for controlling the RED LEDs
int RED2 = 255;
int RED3 = 255;
int GREEN1 = 255;                     // Variables for controlling the GREEN LEDs
int GREEN2 = 255;
int GREEN3 = 255;
int BLUE1 = 255;                      // Variables for controlling the BLUE LEDs
int BLUE2 = 255;
int BLUE3 = 255;
int pixel = 0;                        // Variable to keep track of location along each LED strip
int GBR = 0;                          // Variable for color changing

// Declare LED objects
Adafruit_WS2801 LED_strip1 = Adafruit_WS2801(lengthLED1, LED_data1, LED_clock1);    // Adafruit_WS2801 constructor used to create LED strip objects
Adafruit_WS2801 LED_strip2 = Adafruit_WS2801(lengthLED2, LED_data2, LED_clock2);    // Constructor takes the following parameters:
Adafruit_WS2801 LED_strip3 = Adafruit_WS2801(lengthLED3, LED_data3, LED_clock3);    // Adafruit_WS2801(LENGTH_OF_LED_STRIP, DATA_PIN, CLOCK_PIN)

int brightness;                       // Variable to store measured brightness value from photoresistor (0-1023)

void setup() {
  // Begin serial monitoring at 9600 bps
  Serial.begin(9600);

  // Set up input pins
  pinMode(photoresistor, INPUT);      // Assign input status to photoresistor pin, enabling data collection

  // Set up output pins for LEDs
  pinMode(LED_data1, OUTPUT);         // Assign output status to LED pins to enable LED driving
  pinMode(LED_data2, OUTPUT);
  pinMode(LED_data3, OUTPUT);
  pinMode(LED_clock1, OUTPUT);
  pinMode(LED_clock2, OUTPUT);
  pinMode(LED_clock3, OUTPUT);

  // Initialize and turn on LED strips
  LED_strip1.begin();                 // Initialize LED objects
  LED_strip2.begin();
  LED_strip3.begin();
  LED_strip1.show();                  // Display current pixel values of LED objects
  LED_strip2.show();
  LED_strip3.show();

}

void loop() {
  brightness = analogRead(photoresistor);                               // Read value from photoresistor (0-1023)

  if (brightness < 200) {                                               // If dark, set LEDs to be brightest possible
    for (int i = 0; i < 3; i+=1) {
      LED_strip1.setPixelColor(i, WHITE_ALL, WHITE_ALL, WHITE_ALL);     // setPixelColor(PIXEL_NUMBER, GREEN_VALUE, BLUE_VALUE, RED_VALUE)
      LED_strip2.setPixelColor(i, WHITE_ALL, WHITE_ALL, WHITE_ALL);     // PIXEL_NUMBER = 0 - (number of pixels - 1)
      LED_strip3.setPixelColor(i, WHITE_ALL, WHITE_ALL, WHITE_ALL);     // RED, GREEN, BLUE must be 0-255
    }
    LED_strip1.setPixelColor(3, WHITE_ALL, WHITE_ALL, WHITE_ALL);       // One strip has one more LED than the others
    LED_strip1.show();
    LED_strip2.show();
    LED_strip3.show();
  }
  else if (brightness >= 200 && brightness < 400) {                     // If some light, dip brightness
    int dim = WHITE_ALL - 100;
    for (int i = 0; i < 3; i+=1) {
      LED_strip1.setPixelColor(i, dim, dim, dim);
      LED_strip2.setPixelColor(i, dim, dim, dim);
      LED_strip3.setPixelColor(i, dim, dim, dim);
    }
    LED_strip1.setPixelColor(3, dim, dim, dim);
    LED_strip1.show();
    LED_strip2.show();
    LED_strip3.show();
  }
  else if (brightness >= 400 && brightness < 800) {                     // If a good amount of light, dim lighting on LEDs
    int dim = WHITE_ALL - 200;
    for (int i = 0; i < 3; i+=1) {
      LED_strip1.setPixelColor(i, dim, dim, dim);
      LED_strip2.setPixelColor(i, dim, dim, dim);
      LED_strip3.setPixelColor(i, dim, dim, dim);
    }
    LED_strip1.setPixelColor(3, dim, dim, dim);
    LED_strip1.show();
    LED_strip2.show();
    LED_strip3.show();
  }
  else {                                                                // Otherwise shut LEDs off completely
    for (int i = 0; i < 3; i+=1) {
      LED_strip1.setPixelColor(i, 0, 0, 0);
      LED_strip2.setPixelColor(i, 0, 0, 0);
      LED_strip3.setPixelColor(i, 0, 0, 0);
    }
    LED_strip1.setPixelColor(3, 0, 0, 0);
    LED_strip1.show();
    LED_strip2.show();
    LED_strip3.show();
  }
  delay(100);                                                          // Allow LEDs to update before next brightness reading
}
