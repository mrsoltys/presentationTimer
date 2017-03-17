
/*
  Large 7 Seg Timer
  By: Dr. Soltys
  University of Colorado
  Date: 3/6/2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Adapted from:
  Controlling large 7-segment displays
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 25th, 2015

  Here's how to hook up the Arduino pins to the Large Digit Driver IN
  Arduino
  6 -> CLK (Green on the 6-pin cable)
  5 -> LAT (Blue)
  7 -> SER on the IN side (Yellow)
  5V -> 5V (Orange)
  Power Arduino with 12V and connect to Vin -> 12V (Red)
  GND -> GND (Black)

  Additional hookups for buttons: one side is hooked up to D2 and D3,
  with a 10kOhm pullup resistor connected to GND.
  The other side is connected to 5V:

  2----10 kOhm - GND
    |
    Start button -- 5V
  3----10 kOhm - GND
    |
    Stop button -- 5V
  4----10 kOhm - GND
    |
    Clear button -- 5V
*/

//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
byte segmentClock = 6;
byte segmentLatch = 5;
byte segmentData = 7;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Timer Variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
volatile unsigned long startTime;
volatile unsigned long stopTime = 0;
volatile bool isRunning = false;
// This might change w/ board time and manufacturer, but this corrects for that error.
// Use a negative value if timer runs slow.
//  6 seconds / 4800 seconds = 0.00125
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  // Serial.begin(9600);
  pinMode(segmentClock, OUTPUT);
  pinMode(segmentData, OUTPUT);
  pinMode(segmentLatch, OUTPUT);

  digitalWrite(segmentClock, LOW);
  digitalWrite(segmentData, LOW);
  digitalWrite(segmentLatch, LOW);
  showNumber(0); //Test pattern

  // Define Button Inputs
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), startTimer, RISING);
  pinMode(3, INPUT);
  attachInterrupt(digitalPinToInterrupt(3), stopTimer, RISING);
  pinMode(4, INPUT);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// starTimer()
// Interrupt function if timer button is pressed.
// Starts the timer if it is currently stopped,
// otherwise does nothing.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void startTimer() {
  if (!isRunning) {
    startTime = millis() - stopTime;
    isRunning = true;
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// stopTimer()
// Interrupt function if timer button is pressed.
// pauses the timer if it is currently running,
// otherwise does nothing.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void stopTimer() {
  if (isRunning) {
    isRunning = false;
    stopTime = millis() - startTime;
  }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// stopTimer()
// clears the timer to zer0 of timer is stopped
// otherwise does nothing.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void clearTimer() {
  if (!isRunning) {
    stopTime = 0;
    showNumber(0);
  }
}

void loop() {
  if (isRunning) {
    unsigned long seconds = (millis() - startTime) / 1000 * (1 - timerError);
    unsigned int number = floor(seconds / 60) * 100 + seconds % 60;
    showNumber(number);
    //Serial.println(number); //For debugging
  }
  else if (digitalRead(4) == HIGH) {
    clearTimer();
  }
}

//Takes a number and displays 2 numbers. Displays absolute value (no negatives)
void showNumber(float value){
  int number = abs(value); //Remove negative signs and any decimals

  for (byte x = 0 ; x < 4 ; x++){
    int remainder = number % 10;

    if (x == 2) postNumber(remainder, true);  //Decimil after 2nd digit
    else        postNumber(remainder, false);

    number /= 10;
  }

  //Latch the current segment data
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH); //Register moves storage register on the rising edge of RCK
}

//Given a number, or '-', shifts it out to the display
void postNumber(byte number, boolean decimal){
  //    -  A
  //   / / F/B
  //    -  G
  //   / / E/C
  //    -. D/DP

#define a  1<<0
#define b  1<<6
#define c  1<<5
#define d  1<<4
#define e  1<<3
#define f  1<<1
#define g  1<<2
#define dp 1<<7

  byte segments;

  switch (number) {
    case 1: segments = b | c; break;
    case 2: segments = a | b | d | e | g; break;
    case 3: segments = a | b | c | d | g; break;
    case 4: segments = f | g | b | c; break;
    case 5: segments = a | f | g | c | d; break;
    case 6: segments = a | f | g | e | c | d; break;
    case 7: segments = a | b | c; break;
    case 8: segments = a | b | c | d | e | f | g; break;
    case 9: segments = a | b | c | d | f | g; break;
    case 0: segments = a | b | c | d | e | f; break;
    case ' ': segments = 0; break;
    case 'c': segments = g | e | d; break;
    case '-': segments = g; break;
  }

  if (decimal) segments |= dp;

  //Clock these bits out to the drivers
  for (byte x = 0 ; x < 8 ; x++){
    digitalWrite(segmentClock, LOW);
    digitalWrite(segmentData, segments & 1 << (7 - x));
    digitalWrite(segmentClock, HIGH); //Data transfers to the register on the rising edge of SRCK
  }
}
