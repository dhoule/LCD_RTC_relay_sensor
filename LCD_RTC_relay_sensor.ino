#include <Wire.h>
#include <LiquidCrystal.h>
#include "RTClib.h"


// RTC connections:
// SDA to Arduino pin 20(Mega)/4(Uno)
// SCL to Arduino pin 21(Mega)/5(Uno)
RTC_DS1307 RTC;
// LCD Connections:
// rs (LCD pin 4) to Arduino pin 12
// rw (LCD pin 5) to Arduino pin 11
// enable (LCD pin 6) to Arduino pin 10
// LCD pin 15 to Arduino pin 13
// LCD pins d4, d5, d6, d7 to Arduino pins 7, 6, 5, 4
LiquidCrystal lcd(12, 11, 10, 7, 6, 5, 4);
// This array holds the inputs that will be used by the moisture senor(s)
int inputs[] = {A8};
// relayPin: Pin 2 used by the Relay
// backLight: pin 13 will control the backlight of LCD
// ct: The size of 'inputs'
// soil: The mapped value of 'avg'. Range is [0, 100]
// relayOn: Value of 'soil' that will set the state of the system to ON
// relayOff: Value of 'soil' that will set the sate of the system to OFF
// systemState: 1 = soil is wet enough. 0 = soil is too dry.
int relayPin = 2, backLight = 13, ct = (sizeof(inputs)/sizeof(*inputs)), soil = 0, relayOn = 60, relayOff = 80, systemState;
// value: Accumulator for the values of the moisture sensers
// avg: The average of the reading from the moisture sensors
double value = 0.0, avg = 0.0;
// now: Used to display the timestamp of state changes
DateTime now;

/*
  Function clears a single row on the LCD with 20 empty spaces.
*/
boolean clear_row(int row){
  lcd.setCursor(0,row);
  lcd.print("                    "); 
}

/*
  Function changes the message to match the state of the system and the timestamp.
*/
boolean change_message (char *msg) {
  clear_row(1);
  lcd.setCursor(0,1);
  lcd.print(msg);
  write_date_time();
}

/*
  Function displays the current time as a timestamp on the 3rd row of the LCD.
*/
boolean write_date_time() {
  clear_row(2);
  now = RTC.now();
  //We print the day
  lcd.setCursor(0,2);
  lcd.print(now.day(), DEC);
  lcd.setCursor(2,2);
  lcd.print('/');
  lcd.setCursor(3,2);
  //We print the month
  int month = now.month();
  if(month > 9){
    lcd.print(now.month(), DEC);
  }
  else{
    lcd.print(0);
    lcd.setCursor(4,2);
    lcd.print(now.month(), DEC);
  }
  lcd.setCursor(5,2);
  lcd.print('/');
  lcd.setCursor(6,2);
  //We print the year
  lcd.print(now.year(), DEC);
  lcd.setCursor(10,2);
  lcd.print(' ');
  lcd.setCursor(11,2);
  //We print the hour
  int hour = now.hour();
  if(hour > 9){
    lcd.print(hour, DEC);
  }
  else{
    lcd.print(0);
    lcd.setCursor(12,2);
    lcd.print(hour, DEC);
  }  
  lcd.setCursor(13,2);
  //We print the minutes
  lcd.print(now.minute(), DEC);
  lcd.setCursor(15,2);
}

void setup () {
  //Initialize the serial port, wire library and RTC module
  Serial.begin(9600);
  // Set the pins for outputing.
  pinMode(relayPin, OUTPUT);
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(16,4);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();                  // start with a blank screen
  Wire.begin();
  RTC.begin();
  // Sets up the module time and date with the computer one
  RTC.adjust(DateTime(__DATE__, __TIME__));
  systemState = 1;
}

void loop(){
  // set/reset accumulator
  value = 0.0;
  // 3.3V Air = sensorValue of 674
  // 3.3V Straight, filtered water = sensorValue of 324
  // 5V Air = sensorValue of 1023
  // 5V Straight, filtered water = sensorValue of 420
  for(int i = 0; i < ct; i++){ value += constrain(analogRead(inputs[i]), 420, 1023); }

  avg = value / ct;
  // map 'avg' to a range of [0,100]
  soil = map(avg, 420, 1023, 100, 0);
  
  // Determine if the system's state has changed
  if(soil < relayOn && systemState == 1){
    systemState = 0;
    digitalWrite(relayPin, HIGH);
    change_message("Turned ON");
  }
  if(soil > relayOff && systemState == 0){
    systemState = 1;
    digitalWrite(relayPin, LOW);
    change_message("Turned OFF");
  }
  // wait 1/10 of a second before continuing
  delay(100);  
}


