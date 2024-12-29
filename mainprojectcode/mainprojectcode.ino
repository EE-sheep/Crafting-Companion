/*

Main Code File For Craft Time Reminder



Team Chestnuts: Patrick Zhu, Allison Feldman, Emily Lau



The Craft Time Reminder helps a user remember take breaks during a crafting session. The device

includes a necklace that can be used to hold a crafting tool in place. When this necklace is removed

from its container, the timer starts. The total time can be adjusted in 15-minute intervals before

or after the timer starts. Returning the necklace to its container stops and resets the timer. The

device also includes a multi-purpose pause, start, and mute button.





Code written for an Arduino Uno



Pin Mapping:



Arduino pin |  Role  | Description

-------------------------------------------------------------------

A4          | output | Used as SDA for the 7-segment clock display

A5          | output | Used as SCL for the 7-segment clock display

2           | input  | Reads the TX signal from the DFPlayer Mini

3           | output | Transmits to the RX pin on the DFPlayer Mini

5           | output | Controls the 12-segment LED Ring

6           | input  | Reads the signal from the "-15" button

7           | input  | Reads the signal from the "+15" button

8           | input  | Reads the "busy" pin from the DFPlayer Mini

10          | input  | Reads the signal from the LED Arcade Button

11          | output | Controls the LED on the LED Arcade Button

12          | input  | Reads the signal from the IR Break Beam Sensor



*/





#include <Wire.h>

#include <Adafruit_GFX.h>

#include <Adafruit_LEDBackpack.h>

#include <Adafruit_NeoPixel.h>

#include "SoftwareSerial.h"

#include "DFRobotDFPlayerMini.h"

#include <Arduino.h>

#include "filemapping.h"



// Pin definitions

#define PIN_MP3_TX 2

#define PIN_MP3_RX 3

#define LEDRINGPIN 5

#define MINUSBUTTONPIN 6

#define PLUSBUTTONPIN 7

#define BUSYPIN 8

#define BIGBUTTONPIN 10

#define BIGBUTTONLEDPIN 11

#define SENSORPIN 12



// Define colors in RGB

const int pink_rgb[3] = {76, 19, 30};

const int yellow_rgb[3] = {82, 71, 3};

const int red_rgb[3] = {85, 0, 0};



unsigned long currentMillis;



bool isPlayerIdle;

unsigned long playerIdleStart; // when an "idle" period (nothing is playing) starts



// Timer

unsigned long totalTime_ms = 60 * 60000; // 60 minutes



bool timerReset; // Whether or not the device is in the "reset" state

bool timerStarted = false;

bool timerPaused = false;

bool ledRingBlinkOn = false;

bool timerFinished = false;

bool alarmStopped = false; // alarm == the sound

unsigned long elapsedTime;

long remaining_ms = totalTime_ms;

int remaining_minutes = remaining_ms / 60000;

unsigned long timerStartMillis = 0;

unsigned long pauseStart; // stores the time that a "pause" period was started

unsigned long pauseDuration = 0;



// Button LED

const int buttonBlinkMs = 1000;

const int buttonFlashMs = 500;

unsigned long lastButtonBlinkStart = 0;

bool buttonLEDOn = false;



// LED RING

const int LEDCount = 12;

int lightsOn = 0;

unsigned long intervalLength_ms = totalTime_ms / LEDCount;

const int flashInterval = 1000; // LED flashing interval

unsigned long lastFlashMillis = 0;



// NECKLACE SENSOR

const unsigned long unbrokenThreshold = 2000; // 2 seconds

const unsigned long brokenThreshold = 2000; // 2 seconds

unsigned long unbrokenStartMillis = 0;

unsigned long brokenStartMillis = 0;

int sensorState = 0;

int sensorLastState = 0;



// Button/Sensor States

bool plusButtonLastState = LOW;

bool minusButtonLastState = LOW;

bool bigButtonLastState = LOW;



// Audio variables

bool soundPlaying = false;

int currentSoundFileNum;

int word_i = 0; // the current index of an audio phrase's words[] array



// Audio sequences control variables

bool playStartSound = false;

bool playTotalTimeSound = false;

bool playTimeRemainingSound = false;

bool playTimerPausedSound = false;

bool playTimerStartedSound = false;

bool playTimerFinishedSound = false;

bool playNecklaceDetectedSound = false;



SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

DFRobotDFPlayerMini player;

Adafruit_7segment display = Adafruit_7segment();

Adafruit_NeoPixel ring(LEDCount, LEDRINGPIN, NEO_GRB + NEO_KHZ800);



void setup() {

 // Set up pins

 // pinMode(SENSORPIN, INPUT);

 pinMode(SENSORPIN, INPUT_PULLUP);

 pinMode(PLUSBUTTONPIN, INPUT);

 pinMode(MINUSBUTTONPIN, INPUT);

 pinMode(BIGBUTTONPIN, INPUT); 



 // pinMode(pinButton, INPUT_PULLUP);  // Set button pin as input with pull-up resistor

 pinMode(BIGBUTTONLEDPIN, OUTPUT);           // Set LED pin as output

 digitalWrite(BIGBUTTONLEDPIN, LOW);

 digitalWrite(SENSORPIN, HIGH); // Enable pull-up resistor



 // Set up NeoPixel ring

 ring.begin();  

 ring.setBrightness(100);

 ring.show();



 Serial.begin(9600);

 softwareSerial.begin(9600);



 initializeNumberMappings();

 initializeOtherSoundMappings();



 if (player.begin(softwareSerial)) {

   // Set volume to maximum (0 to 30).

   player.volume(30);   



 } else {

   Serial.println("Connecting to DFPlayer Mini failed!");

 }

 resetTimer();

 display.begin(0x70); // Default I2C address for the 7-segment display

 displayTime(); // Display initial total time

}



void loop() {

 currentMillis = millis(); // Get the current time



 // Read sensor state

 sensor();



 // Handle all buttons

 plusminusButton();

 bigButton();



 // Update timer

 updateTimer();



 // Update the LED ring and display

 ledRing();

 displayTime();



 // Play any/all relevant audio

 audio_all();

 delay(50);

}



void sensor() {

 sensorState = digitalRead(SENSORPIN);

 if (timerReset && sensorState) {

   // If unbroken, start tracking the time

   if (!sensorLastState) {

     unbrokenStartMillis = currentMillis;

   } else if (currentMillis - unbrokenStartMillis >= unbrokenThreshold && !timerStarted) {

     startTimer();

   }

 } else if (!timerStarted) {

   // Reset the unbroken timer if the state becomes "broken"

   unbrokenStartMillis = 0;

 }



 if (!timerReset && !sensorState) { // if the timer hasn't been reset yet and the sensor is "blocked"

   // If broken, start tracking the time

   if (sensorLastState) {

     brokenStartMillis = currentMillis;

   } else if (currentMillis - brokenStartMillis >= brokenThreshold) {

     resetTimer();

     playNecklaceDetectedSound = true;

    

   }

 } else {

   // Reset the unbroken timer if the state becomes "broken"

   brokenStartMillis = 0;

 }



 // Print state changes

 if (sensorState && !sensorLastState) {

   Serial.println("Unbroken");

 }

 if (!sensorState && sensorLastState) {

   Serial.println("Broken");

 }

 sensorLastState = sensorState;

}



void plusminusButton() {

 bool plusButtonState = digitalRead(PLUSBUTTONPIN);

 bool minusButtonState = digitalRead(MINUSBUTTONPIN);



 if (!timerFinished && plusButtonLastState == LOW && plusButtonState == HIGH) {

   if (totalTime_ms == (60000)) {

     totalTime_ms = 15 * 60000;

   } else {

     totalTime_ms += (15 * 60000);

   }

   stopAllSounds();



   playTotalTimeSound = true;

   if (totalTime_ms > (90 * 60000)) {

     totalTime_ms = (90 * 60000);

   }

   intervalLength_ms = totalTime_ms / LEDCount;

 }



 if (!timerFinished && minusButtonLastState == LOW && minusButtonState == HIGH && totalTime_ms > 60000) {

   if (totalTime_ms == (15 * 60000)) {

     totalTime_ms = 60000;

   }

   else {

     totalTime_ms -= (15 * 60000);

   }

   stopAllSounds();

   playTotalTimeSound = true;

   if (totalTime_ms < 0) {

     totalTime_ms = 0;

   }

   intervalLength_ms = totalTime_ms / LEDCount;



 }

 plusButtonLastState = plusButtonState;

 minusButtonLastState = minusButtonState;

}





void bigButton() {

 bool bigButtonState = digitalRead(BIGBUTTONPIN);

 if (bigButtonLastState == LOW && bigButtonState == HIGH) {

   if(timerStarted) {

     if (timerPaused) {

       timerPaused = false;

       pauseDuration = pauseDuration + (currentMillis - pauseStart);

      

       stopAllSounds();

       playTimerStartedSound = true;

     }

     else {

       pauseStart = currentMillis;

       timerPaused = true;

       stopAllSounds();

       playTimerPausedSound = true;

     }

   } else if (timerFinished && !alarmStopped) {

     alarmStopped = true;

   }

 }

 // Light the LED

 if (timerStarted && !timerPaused) {

   digitalWrite(BIGBUTTONLEDPIN, HIGH);

 }

 else if (timerStarted && timerPaused) {

   if ((currentMillis - lastButtonBlinkStart) >= buttonBlinkMs) {

   lastButtonBlinkStart = currentMillis;

     if(!buttonLEDOn) {

       digitalWrite(BIGBUTTONLEDPIN, HIGH); 

       buttonLEDOn = true;

     } else {

       digitalWrite(BIGBUTTONLEDPIN, LOW); 

       buttonLEDOn = false;

     }

   }

 }

 else if (timerFinished && !timerReset && !alarmStopped) {

   if ((currentMillis - lastButtonBlinkStart) >= buttonFlashMs) {

   lastButtonBlinkStart = currentMillis;

     if(!buttonLEDOn) {

       digitalWrite(BIGBUTTONLEDPIN, HIGH); 

       buttonLEDOn = true;

     } else {

       digitalWrite(BIGBUTTONLEDPIN, LOW); 

       buttonLEDOn = false;

     }

   }

 }

 else {

   digitalWrite(BIGBUTTONLEDPIN, LOW);

 }

 bigButtonLastState = bigButtonState;

}



void updateTimer() {

  if (timerStarted && !timerFinished) {

   elapsedTime = currentMillis - timerStartMillis - pauseDuration;

   if (!timerPaused) {

     // Countdown calculation

     remaining_ms = totalTime_ms - elapsedTime;

   }

   if (remaining_ms <= 0) { // Check to see if timer is finished

     remaining_ms = 0;

     timerStarted = false;

     timerFinished = true;

     ledRingBlinkOn = true;

     stopAllSounds();

     playTimerFinishedSound = true;

   }

 }

 else if (timerFinished && !timerReset) {

   remaining_ms = 0;

 }

 else {

   // Before the timer starts, display total time and ensure LEDs are off

   remaining_ms = totalTime_ms;

 }

 remaining_minutes = remaining_ms / 60000;

 // remaining_seconds = (remaining_ms % 60000) / 1000;

 }



void ledRing() {

 if (timerStarted) {

   int ledsToLight = elapsedTime / intervalLength_ms;

     for (int i = 0; i < LEDCount; i++) {

       if (i < ledsToLight) {

         if (!timerPaused) {

           ring.setPixelColor(i, ring.Color(pink_rgb[0], pink_rgb[1], pink_rgb[2]));

         } else {

           ring.setPixelColor(i, ring.Color(yellow_rgb[0], yellow_rgb[1], yellow_rgb[2]));

         }

        

       } else if (!timerPaused && i == ledsToLight && !timerFinished) {

         flashPendingLED(i, currentMillis);

       } else {

         // Don't flash last LED on the ring if the timer is paused

         ring.setPixelColor(i, 0, 0, 0);

       }

     }

     ring.show();

 }

 else if (timerFinished && !alarmStopped) {

   if ((currentMillis - lastFlashMillis) >= 500) {

     lastFlashMillis = currentMillis;

     if (ledRingBlinkOn) {

       ring.fill(0, 0, 0);

       ring.show();

       ledRingBlinkOn = false;

     } else {

       ring.fill(ring.Color(red_rgb[0], red_rgb[1], red_rgb[2]));

       ring.show();

       ledRingBlinkOn = true;

     }

   }

 }

 else {

   ring.clear();   // Turn off all LEDs

   ring.show();

 }

 }



void displayTime() {

 // Format as MM:SS for the 7-segment display

 // int displayValue = (remaining_minutes * 100) + remaining_seconds;

 int displayValue = ((remaining_ms / 60000) * 100) + ((remaining_ms % 60000) / 1000);

  display.print(displayValue); // Print the time in MMSS format

 display.drawColon(true);

 display.writeDisplay();

}



void audio_all() {

 if (playStartSound) {

   audio_timerStart();

 }

 else if (playTotalTimeSound) {

   audio_totalTime();

 }

 else if (playTimeRemainingSound) {

   audio_timeRemaining();

 }

 else if (playTimerPausedSound) {

   audio_timerPaused();

 }

 else if (playTimerStartedSound) {

   audio_timerStarted();

 }

 else if (playTimerFinishedSound) {

   audio_timerFinished();

 }

 else if (playNecklaceDetectedSound) {

   audio_necklaceDetected();

 }

}



void resetTimer() {

 // player.pause();

 stopAllSounds();

 totalTime_ms = 60 * 60000;

 remaining_ms = totalTime_ms;

 timerStarted = false;

 timerPaused = false;

 alarmStopped = false;

 pauseDuration = 0;

 timerFinished = false;

 timerReset = true;

}



void startTimer() {

  timerStarted = true;

 timerPaused = false;

 pauseDuration = 0;

 timerFinished = false;

 timerStartMillis = currentMillis;

 lastFlashMillis = timerStartMillis;

 playStartSound = true;

 timerReset = false;

 }



void flashPendingLED(int ledNum, unsigned long currentMillis) {

 if (currentMillis >= lastFlashMillis + flashInterval) {

   if (!ledRingBlinkOn) {

     ring.setPixelColor(ledNum, ring.Color(pink_rgb[0], pink_rgb[1], pink_rgb[2]));

   } else {

     ring.setPixelColor(ledNum, 0, 0, 0);

   }

   ledRingBlinkOn = !ledRingBlinkOn;

   lastFlashMillis = currentMillis;

 }

}



void stopAllSounds() {

 word_i = 0;

 soundPlaying = false;



 playStartSound = false;

 playTotalTimeSound = false;

 playTimeRemainingSound = false;

 playTimerPausedSound = false;

 playTimerStartedSound = false;

 playTimerFinishedSound = false;

 playNecklaceDetectedSound = false;

}



void playSound(int soundNumber) {

 if (!soundPlaying) {

   player.play(soundNumber);

   soundPlaying = true;

   isPlayerIdle = false;

 }

  if (soundPlaying) {

   if ((digitalRead(BUSYPIN) == 1) && isPlayerIdle == false) {

     isPlayerIdle = true;

     playerIdleStart = currentMillis;

   }

   else if(isPlayerIdle == true && digitalRead(BUSYPIN) == 0) {

     isPlayerIdle = false;

   }

   else if (isPlayerIdle == true && (currentMillis - playerIdleStart > 100)) {

     soundPlaying = false;

   }

 }

}



void audio_timeRemaining() {

 int words[7] = {

   otherSoundMappings.youHave,                                                         //"You have..."

   getNumberSound(remaining_ms / 60000 ),    //"___"            

   ((remaining_ms / 60000) != 1 ? otherSoundMappings.minutes : otherSoundMappings.minute),  //"minute(s)

   otherSoundMappings.andSound,                                                        //"and" 

   getNumberSound( (remaining_ms % 60000) / 1000 ),             //"___"

   (((remaining_ms % 60000) / 1000 ) != 1 ? otherSoundMappings.seconds : otherSoundMappings.second),  //"second(s)"

   otherSoundMappings.remaining                                                        //"remaining"

 };



 currentSoundFileNum = words[word_i];

 playSound(currentSoundFileNum);

 if (!soundPlaying) {

   word_i += 1;

 }

 if (word_i >= 7) {

   stopAllSounds();

 }

}



void audio_timerPaused() {

 int words[1] = {

   otherSoundMappings.timerPaused    //"Timer paused"

 };



 currentSoundFileNum = words[word_i];

 playSound(currentSoundFileNum);

 if (!soundPlaying) {

   word_i += 1;

 }

 if (word_i >= 1) {

   stopAllSounds();

 }

}



void audio_necklaceDetected() {

  int words[1] = {

   otherSoundMappings.necklaceDetected       //"Necklace Detected"

 };

  currentSoundFileNum = words[word_i];

 playSound(currentSoundFileNum);

 if (!soundPlaying) {

   word_i += 1;

 }

 if (word_i >= 1) {

 

   stopAllSounds();

 }

}



void audio_timerStarted() {

 int words[1] = {

   otherSoundMappings.timerStarted   //"Timer started"

 };

 currentSoundFileNum = words[word_i];

 playSound(currentSoundFileNum);

 if (!soundPlaying) {

   word_i += 1;

 }

 if (word_i >= 1) {

   stopAllSounds();

 }

}



void audio_totalTime() {

 int words[4] = {

   otherSoundMappings.beep,                                                            //*beep sound*

   otherSoundMappings.totalTime,                                                       //"Total time:"

   getNumberSound(totalTime_ms / 60000),                                                  //"___"

   ((totalTime_ms / 60000) != 1 ? otherSoundMappings.minutes : otherSoundMappings.minute),  //"minute(s)"

 };



 currentSoundFileNum = words[word_i];



 playSound(currentSoundFileNum);

 if (!soundPlaying) {

   word_i += 1;

 }

 if (word_i >= 4) {

   stopAllSounds();

   if (timerStarted) {

     playTimeRemainingSound = true;

   }

 }

}

void audio_timerFinished() {

 int words[1] = {

   otherSoundMappings.timerUp // *ringtone audio

 };

  currentSoundFileNum = words[word_i];

 playSound(currentSoundFileNum);



 if (alarmStopped || !soundPlaying) {

   player.pause();

   stopAllSounds();

 }

}



void audio_timerStart() {

 int words[5] = {

   otherSoundMappings.necklaceRemoved,                                                 //"Necklace removed."

   otherSoundMappings.totalTime,                                                       //"Total time:"

   getNumberSound(totalTime_ms / 60000),                                                  //"___"

   ((totalTime_ms / 60000) != 1 ? otherSoundMappings.minutes : otherSoundMappings.minute),  //"minute(s)"

   otherSoundMappings.happyCrafting,                                                   //"Happy crafting!"

 };



 currentSoundFileNum = words[word_i];

 playSound(currentSoundFileNum);

  

 if (!soundPlaying) {

   word_i += 1;

 }

 if (word_i >= 5) {

   stopAllSounds();

 }

}