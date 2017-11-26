/*
  There is no REAL TIME API: Understanding Networks Fall 2017
  Test HTTP Client
  Purpose: 4 LEDs to show the passage of time. Seconds: full fade cycle each second, consistent except for when resetting
  the time. Minutes: Blinks for 1 second on each minute with a default of 3 seconds Call the API & find the remaining seconds
  in the current minute, blink, and then reset default interval. Repeat with hour and day. A switch or button controls
  when to call the API to reset to current NO REAL TIME. All LEDs off during time reset.
  Note: include config.h for current network.
  HTTP: https://github.com/tigoe/MakingThingsTalk2/blob/master/3rd_edition/chapter4/TestHttpClient/TestHttpClient.ino
  Modified by JAS & Grau: 11/26/2017
*/

// include required libraries and config files
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoHttpClient.h>
#include "config.h"

WiFiClient netSocket;                         // network socket to server
const char server[] = "174.138.50.88";       // server name

// declare GET API routes
String getTimeDivision = "/getTimeDivision";  
String currentTime = "/currentTime";          

// Define LED pins - Maker1000 pins
const int DayLED = 8;           
const int HourLED = 7;         
const int MinuteLED = 6;         
const int SecondLED = 5; 

const int switchPin = 1;     // the number of the button pin
int buttonState = 0;         // variable for reading the status
int prevState = 0 ;

// Consistent fade, change interval of fade
int fadeAmount = 1 ;

// Define fade amounts
int SecondBrightness = 0;        // how bright the LED is

// Each LED's time
long secondMillis = 0 ;
long minuteMillis = 0 ;
long hourMillis = 0 ;
long dayMillis = 0 ;

// fade needs to happen on 2 second increments to 
// fade in 1 second cycles @ 250 max brightness
long secondInterval = 2;

// how often to blink - based on API
long minToMillis;
long hrToMillis;
long dayToMillis;
    
long minuteInterval = 3000;
long hourInterval = 6000;
long dayInterval = 9000;

long minuteOff = minuteInterval - 1000;
long hourOff = hourInterval - 1000;
long dayOff = dayInterval - 1000;

// if the LED is on
boolean minuteOn = false;
boolean hourOn = false;
boolean dayOn = false;

// API values
long curHour ;
long curMin ;
long curSec ;

long Hour ;
long Min ;
long Sec ;

// If the program is calling the API
boolean httpMinReset = false ;
boolean httpHrReset = false ;
boolean httpDayReset = false ;

//Setup
void setup() {
  Serial.begin(9600);               // initialize serial communication

  // declare LED pins to be output:
  pinMode(DayLED, OUTPUT);
  pinMode(HourLED, OUTPUT);
  pinMode(MinuteLED, OUTPUT);
  pinMode(SecondLED, OUTPUT);

  //set Button PinMode
  pinMode(switchPin, INPUT);

  // while you're not connected to a WiFi AP,
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);           // print the network name (SSID)
    WiFi.begin(ssid, pass);         // try to connect
    delay(2000);
  }

  // When you're connected, print out the device's network status:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

//Loop
void loop() {

  //used to determine when to stop everything and call the api
  buttonState = digitalRead(switchPin);

  //make calls when button is Pressed
  if (buttonState == HIGH and prevState == 0) {
    // turn off LEDs while working, also because this includes a delay
    analogWrite(SecondLED, 0);
    digitalWrite(MinuteLED, LOW);
    digitalWrite(HourLED, LOW);
    digitalWrite(DayLED, LOW);
   
    // HTTP call & reset boolean
    HTTPCall();
    httpMinReset = true;
    httpHrReset = true;
    httpDayReset = true;

    // set when to Blink next
    // for minutes, calculate remaining seconds in this minute, * 1000
    minToMillis =  (Sec - curSec) * 1000;
    
    // for hours, calculate remaning minutes in this hour, convert, + minToMillis
    hrToMillis = ((Min - curMin) * Min * 1000) + minToMillis;
    
     // for days, calculate remaning hours in this day, convert, + minToMillis, hrToMillis
    dayToMillis = ((Hour - curHour) * Hour * Min * 1000) + minToMillis + hrToMillis;

    //also reset global intervals
    minuteInterval = Sec * 1000;
    hourInterval = Min * Sec * 1000;
    dayInterval = Hour * Min * Sec * 1000;

    minuteOff = minuteInterval - 1000;
    hourOff = hourInterval - 1000;
    dayOff = dayInterval - 1000;

    minuteOn = false;
    hourOn = false;
    dayOn = false;

    secondMillis = millis();
    minuteMillis = millis();
    hourMillis = millis();
    dayMillis = millis();

    Serial.println("minutes");
    Serial.println(minToMillis);
    Serial.println(minuteInterval);
 
    Serial.println("hour");
    Serial.println(hrToMillis);
    Serial.println(hourInterval);

    Serial.println("day");
    Serial.println(dayToMillis);
    Serial.println(dayInterval);

  }
  else if (buttonState == LOW) {
    //reset
    prevState = 0 ; 

    // for timing
    unsigned long currentMillis = millis();

      // for Seconds LED fading
    if (currentMillis - secondMillis > secondInterval) {
        // save the last time you blinked the LED
        secondMillis = currentMillis;
        secondFade();
    }
    
    // for minute LED blinking
    if (httpMinReset == true) {
      if (currentMillis - minuteMillis > minToMillis ) {
        // save the last time you blinked the LED
        if (!minuteOn) {
          minuteOn = true;
          digitalWrite(MinuteLED, HIGH);
          Serial.println("turned on");
          minuteMillis = currentMillis-minuteOff;
          //reset
          httpMinReset = false ;
        } else {
          minuteOn = false;
          digitalWrite(MinuteLED, LOW);
          Serial.println("turned off");
          minuteMillis = currentMillis;
        }
      }
    }
    else {
      if (currentMillis - minuteMillis > minuteInterval) {
        // save the last time you blinked the LED
        if (!minuteOn) {
          minuteOn = true;
          digitalWrite(MinuteLED, HIGH);
          minuteMillis = currentMillis-minuteOff;
        } else {
          minuteOn = false;
          digitalWrite(MinuteLED, LOW);
          minuteMillis = currentMillis;
        }
      }
    } // httpMinReset == false

  
    // for hour LED blinking
    if (httpHrReset == true) {
      if (currentMillis - hourMillis > hrToMillis) {
        // save the last time you blinked the LED
        if (!hourOn) {
          hourOn = true;
          digitalWrite(HourLED, HIGH);
          hourMillis = currentMillis-hourOff;
          //reset
          httpHrReset = false ;
        } else {
          hourOn = false;
          digitalWrite(HourLED, LOW);
          hourMillis = currentMillis;
        }
      }
    }
    else {
      if (currentMillis - hourMillis > hourInterval) {
        // save the last time you blinked the LED
        if (!hourOn) {
          hourOn = true;
          digitalWrite(HourLED, HIGH);
          hourMillis = currentMillis-hourOff;
        } else {
          hourOn = false;
          digitalWrite(HourLED, LOW);
          hourMillis = currentMillis;
        }
      }
    } // httpMinReset == false
  
    // for day LED blinking
    if (httpDayReset == true) {
      if (currentMillis - dayMillis > dayToMillis) {
        // save the last time you blinked the LED
        if (!dayOn) {
          dayOn = true;
          digitalWrite(DayLED, HIGH);
          dayMillis = currentMillis-dayOff;
          //reset
          httpDayReset = false ;
        } else {
          dayOn = false;
          digitalWrite(DayLED, LOW);
          dayMillis = currentMillis;
        }
      }
    }
    else {
      if (currentMillis - dayMillis > dayInterval) {
        // save the last time you blinked the LED
        if (!dayOn) {
          dayOn = true;
          digitalWrite(DayLED, HIGH);
          dayMillis = currentMillis-dayOff ;
        } else {
          dayOn = false;
          digitalWrite(DayLED, LOW);
          dayMillis = currentMillis ;
        }
      }
    } // httpDayReset == false
    
  } //buttonState LOW
} // loop

int HTTPCall(){

  //reset
  prevState = 1 ;

  // 1 call
  HttpClient http1(netSocket, server, 9090);      // make an HTTP client
  http1.get(currentTime);  // make a GET request

  while (http1.connected()) {       // while connected to the server,
    if (http1.available()) {        // if there is a response from the server,
      String curTime = http1.readString();  // read it

      // parse the data
      int curHourStart = curTime.indexOf("curHour");
      int curHourEnd = curTime.indexOf(",", curHourStart);
      int curMinStart = curTime.indexOf("curMin");
      int curMinEnd = curTime.indexOf(",", curMinStart);
      int curSecStart = curTime.indexOf("curSec");
      int curSecEnd = curTime.indexOf("}", curSecStart);

      curHour = curTime.substring((curHourStart + 9), curHourEnd).toInt();
      curMin = curTime.substring((curMinStart + 8), curMinEnd).toInt();
      curSec = curTime.substring((curSecStart + 8), (curSecEnd)).toInt(); 
    }
  }
  // when there's nothing left to the response,
  http1.stop();                  // close the request
  delay(250);                    // wait a little

  
  // 2 call
  HttpClient http2(netSocket, server, 9090);      // make an HTTP client
  http2.get(getTimeDivision);  // make a GET request

  while (http2.connected()) {       // while connected to the server,
    if (http2.available()) {        // if there is a response from the server,
      String timeDivis = http2.readString();  // read it

      // parse the data
      int HourStart = timeDivis.indexOf("hrInDay");
      int HourEnd = timeDivis.indexOf(",", HourStart);
      int MinStart = timeDivis.indexOf("minInHr");
      int MinEnd = timeDivis.indexOf(",", MinStart);
      int SecStart = timeDivis.indexOf("secInMin");
      int SecEnd = timeDivis.indexOf("}", SecStart);

      Hour = timeDivis.substring((HourStart + 9), HourEnd).toInt();
      Min = timeDivis.substring((MinStart + 9), MinEnd).toInt();
      Sec = timeDivis.substring((SecStart + 10), (SecEnd)).toInt();
    }
  }
  // when there's nothing left to the response,
  http2.stop();                  // close the request
  delay(250);                    // wait a little
  
}

// function for Seconds LED
void secondFade () {
  analogWrite(SecondLED, SecondBrightness);

  // change the brightness for next time through the loop:
  SecondBrightness = SecondBrightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (SecondBrightness <= 0 || SecondBrightness >= 250) {
    fadeAmount = -fadeAmount;
  }
}
