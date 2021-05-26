#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <base64.h>


struct Button {
  const uint8_t PIN;
  bool pressed;
};

bool alarmActive = false;

unsigned long pressTime = 0;

unsigned long pressReleaseDelta = 2000; // in ms

Button alarmTrigger {2, false};
Button alarmReset {3, false};

int ledPin = 4;

void isr() {
  alarmReset.pressed = !alarmReset.pressed;
  if(alarmReset.pressed == true) {
    pressTime = millis();
  } else {
    if(millis() - pressTime > pressReleaseDelta) {
      alarmActive = false;
    }
  }
}

void sendAlarmEmail() {




  
}

void setup() {

  pinMode(alarmReset.PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(alarmReset.PIN), isr, CHANGE);
  
  pinMode(alarmTrigger.PIN, INPUT_PULLUP);  
  
  pinMode(ledPin, OUTPUT);
  
  //Serial.begin(9600);

}

void loop() {


  if(digitalRead(alarmTrigger.PIN) == HIGH) {
    
    alarmActive = true;
    
    sendAlarmEmail();
      
  
    // blink light wait for isr
      while(alarmActive == true) {
        // D1-mini chip io pins can draw more then they can 
        // output so inverted wiring in order to make led brighter
        digitalWrite(ledPin, LOW);  // turn LED ON
        delay(500);
        digitalWrite(ledPin, HIGH);  // turn LED OFF
        delay(200);
        digitalWrite(ledPin, LOW);  // turn LED ON
        delay(500);
        digitalWrite(ledPin, HIGH);  // turn LED OFF
        delay(1200);
    }


    // arming blinking signal
    for(int i = 0; i < 10; i++) {
      digitalWrite(ledPin, LOW);
      delay(100);
      digitalWrite(ledPin, HIGH);
      delay(100);
    }
      digitalWrite(ledPin,LOW);
      delay(2000);
      digitalWrite(ledPin,HIGH);
  }


}

