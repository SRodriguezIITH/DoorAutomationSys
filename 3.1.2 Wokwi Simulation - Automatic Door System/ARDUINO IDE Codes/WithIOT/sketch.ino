
/*Automatic Door System with Smart Alarm System
Submitted By: Sonit Patil
*/
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>

/* 
  Arduino IoT Cloud Variables description

  bool accessMode; READ ONLY
  bool alarmStatus; READ/WRITE (TO ALLOW STOPPING ALARM BY DEVICE)
*/

#include "thingProperties.h"


//Pin Bindings
#define pir 2
#define Am 4
#define Ap 13
#define Bm 16
#define Bp 17
#define gLed 25
#define rLed 26
#define bzr 27
#define sw 14
#define ringLed 5
#define whiteRing 18

Adafruit_NeoPixel strip = Adafruit_NeoPixel(48, ringLed, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripW = Adafruit_NeoPixel(48, whiteRing, NEO_GRB + NEO_KHZ800);

//var declarations
int i = 120; //Loop Variable
int pirval = 0;
int flag = 0; //To remember if door opening occured

void setup() {
  Serial.begin(9600);

  Serial.print("Connecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  delay(1000);
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(Ap,OUTPUT);  pinMode(Am,OUTPUT);
  pinMode(Bp,OUTPUT);  pinMode(Bm,OUTPUT);
  pinMode(gLed, OUTPUT); pinMode(rLed, OUTPUT);
  pinMode(bzr, OUTPUT); pinMode(pir, INPUT);
  pinMode(sw, INPUT_PULLUP);

  //Initializing Ring Leds to OFF
  strip.begin();
  strip.show();
  stripW.begin();
  stripW.show();

  alarmStatus = false;


  Serial.println("The Door System Is now Active!");
}

void loop() {
  ArduinoCloud.update();

  pirval = digitalRead(pir);

  accessMode = digitalRead(sw)?true:false;

  if (digitalRead(sw) == LOW){
    
    digitalWrite(rLed, HIGH);
    digitalWrite(gLed, LOW);

    
    if(pirval == HIGH){
      delay(5000); //Wait for 5s
      digitalWrite(gLed,HIGH);
      pirval = digitalRead(pir);
      
      if(pirval == HIGH){
        delay(5000);
        
        if(pirval == HIGH){
        Serial.println("Unauthorized Entry Detected!");
        alarmStatus = true;
        tone(bzr, 500, 60000); //Buzzer for 1min
          colorRingW(stripW.Color(255,255,255),10); //White
          int j = 200;
          while(j>0){
            colorRing(strip.Color(255, 0, 0), 30); //Red
            colorRing(strip.Color(0, 0, 255), 30); // Blue
            j--;
          }

        }


        //IOT Alert Here

      }
      
    }

  }
  if(digitalRead(sw)==HIGH){
    
    digitalWrite(rLed, LOW);
    digitalWrite(gLed, HIGH);

    pirval = digitalRead(pir);
    
    if(pirval==HIGH){
      i=120;
      while(i>1){
        stepperMotor();
        i--;
      }

      i=120;
      flag = 1;
    }

    if(pirval==LOW and flag!=0) {
      delay(5000); //Waiting with door open for 5 sec
      pirval = digitalRead(pir);
      if(pirval==LOW){//Final Check
        i=120;

        //another digital read ensures closing stops in case there is entry/exit during this
        while(i>=1 && digitalRead(pir)==LOW){ 
        stepperMotorRev();
        i--;
        }
        i=120;
      }
      flag = 0;
    }
    

  }


}

void stepperMotor(){
    delay(10);
    digitalWrite(Bm,LOW) ;  digitalWrite(Ap,HIGH);
    delay(10);
    digitalWrite(Ap,LOW) ;  digitalWrite(Bp,HIGH);
    delay(10);
    digitalWrite(Bp,LOW) ;  digitalWrite(Am,HIGH);
    delay(10);
    digitalWrite(Am,LOW) ;  digitalWrite(Bm,HIGH);
}

void stepperMotorRev(){
  delay(10);
  digitalWrite(Am,LOW);   digitalWrite(Bm,HIGH);
  delay(10);
  digitalWrite(Bp,LOW);   digitalWrite(Am,HIGH);
  delay(10);
  digitalWrite(Ap,LOW);   digitalWrite(Bp,HIGH);
  delay(10);
  digitalWrite(Bm,LOW);   digitalWrite(Ap,HIGH);
}

//Functions to generate Color Rings in Neopixel LED RIng
//FOR RB Oscillatory Alarm RIng
void colorRing(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

//For Constant White LED
void colorRingW(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<stripW.numPixels(); i++) {
    stripW.setPixelColor(i, c);
    stripW.show();
    delay(wait);
  }
}

void onAlarmStatusChange()  {
  Serial.println("WARNING - SUSPICIOUS ACTIVITY");
}




