// Ownership: Virtual Incision Corp
// Last Modified: 5/27/2021
// Author: David Ryan
//   E: davidryn6@gmail.com
//   C: (402)-499-8715

// ++Virtual Incision Permanent Temperature Alarm++

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <base64.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const long utcOffsetInSeconds = -18000; // utc offset

String daysOfTheWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// network info
const String ssid = "VirtualIncision-Internal";
const String networkPassword = "R0b0tsCutY0u";

// email recipient account info
const String recipientEmail = "vipermtempalarm@gmail.com";
const String recipientEmailPassword = ""; // not used

// smtp account info
const String smtpUserName = "vipermtempalarmsender@gmail.com";
const String smtpPassword = "2gLg3vELCUYSbLHR";

// email sender account info
const String senderEmail = "vipermtempalarmsender@gmail.com";
const String senderEmailPassword = "47LnE6eyJW9W9Kmh";

String server = "mail.smtp2go.com"; // The SMTP Server

struct Button
{
  uint8_t PIN;
  bool pressed;
};

struct Email
{
  String smtpUserName;
  String smtpPassword;
  String sender;
  String senderPassword;
  String recipient;
};


bool alarmActive = false; // system state

unsigned long pressTime = 0; // time of current press

unsigned long pressReleaseDelta = 2000; // required press time to reset in ms

// button declarations
Button alarmTrigger{2, false};
Button alarmReset{3, false};

uint8_t ledPin = 4; // led pin number

WiFiClient espClient;

// Interupt Service Routine
void ICACHE_RAM_ATTR isr()
{
  alarmReset.pressed = !alarmReset.pressed;
  if (alarmReset.pressed == true)
  {
    pressTime = millis();
  }
  else
  {
    if (millis() - pressTime > pressReleaseDelta)
    {
      alarmActive = false;
    }
  }
}

// function prototypes
void connectToWifi();
int emailResp();
int sendAlarmEmail();
String getCurrentTime();
void disconnectFromWifi();

void setup()
{

  Serial.begin(9600);

  Serial.println("");

  Serial.println("Beginning...");

  pinMode(alarmReset.PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(alarmReset.PIN), isr, CHANGE);

  pinMode(alarmTrigger.PIN, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);

  Serial.println("Setup compleate.");

    // connectToWifi(); // establish network connection

    // int error = sendAlarmEmail(); // send formated email with current time

    // Serial.println("error: " + error);

    // disconnectFromWifi(); // disconnect from network



}

void loop()
{

  Serial.println("Checking...");
  delay(1000);


  if (digitalRead(alarmTrigger.PIN) == LOW) // check for alarm trigger
  {

    alarmActive = true;

    connectToWifi(); // establish network connection

    int error = sendAlarmEmail(); // send formated email with current time

    Serial.println("error sending email " + error);

    disconnectFromWifi(); // disconnect from network

    // blink light wait for isr
    while (alarmActive == true)
    {
      // D1-mini chip io pins can draw more then they can
      // output so inverted wiring in order to make led brighter
      Serial.println("blinking...");
      digitalWrite(ledPin, LOW); // turn LED ON
      delay(500);
      digitalWrite(ledPin, HIGH); // turn LED OFF
      delay(200);
      digitalWrite(ledPin, LOW); // turn LED ON
      delay(500);
      digitalWrite(ledPin, HIGH); // turn LED OFF
      delay(1200);
    }

    // arming blinking signal
    for (uint8_t i = 0; i < 10; i++)
    {
      digitalWrite(ledPin, LOW);
      delay(100);
      digitalWrite(ledPin, HIGH);
      delay(100);
    }
    digitalWrite(ledPin, LOW);
    delay(2000);
    digitalWrite(ledPin, HIGH);
  }
}

int emailResp()
{
  byte responseCode;
  byte readByte;
  int loopCount = 0;

  while (!espClient.available())
  {
    delay(1);
    loopCount++;

    if (loopCount > 20000) // Wait for 20 seconds and if nothing is received, stop.
    {
      espClient.stop();
      return 0;
    }
  }

  responseCode = espClient.peek();
  while (espClient.available())
  {
    readByte = espClient.read();
    Serial.write(readByte);
  }

  if (responseCode >= '4')
  {
    return 0;
  }
  return 1;
}

void connectToWifi()
{
  // attempt a connection at least 5 times
  for (uint8_t attempts = 0; attempts <= 5; attempts++)
  {
    Serial.println("Connecting to WiFi");
    WiFi.begin(ssid, networkPassword);
    for (uint8_t i = 0; i <= 20; i++)
    {
      delay(500);
      Serial.print(".");
      // wait for connection to be established
      if (WiFi.status() == WL_CONNECTED)
      {
        // break both loops
        Serial.println();
        i = 100;
        attempts = 100;
        Serial.println("Wifi connection succesfull");
      }
    }
    delay(1500);
  }
}

void disconnectFromWifi()
{
  WiFi.disconnect(); // disconect from WiFi
}

int sendAlarmEmail()
{

  Email email{smtpUserName, smtpPassword, senderEmail, senderEmailPassword, recipientEmail}; // email struct

  
  if (espClient.connect(server, 2525) == 1) // attempt connection to server
  {
    Serial.println("connected");
  }
  else
  {
    Serial.println("connection failed");
    return 0;
  }
  if (!emailResp())
    return 0;

  espClient.println("EHLO www.example.com");
  if (!emailResp())
    return 0;

  // only use if STARTTLS seccurity is used
  /*
  espClient.println("STARTTLS");
  if (!emailResp()) 
  return 0;
  */

  espClient.println("AUTH LOGIN");
  if (!emailResp())
    return 0;

  espClient.println(base64::encode(email.smtpUserName)); //base64, ASCII encoded Username
  if (!emailResp())
    return 0;

  espClient.println(base64::encode(email.smtpPassword)); //base64, ASCII encoded Password
  if (!emailResp())
    return 0;

  String from = "MAIL From: " + email.sender;
  espClient.println(from);
  if (!emailResp())
    return 0;

  String to = "RCPT To: " + email.recipient;
  espClient.println(to);
  if (!emailResp())
    return 0;

  // sending data
  espClient.println("DATA");
  if (!emailResp())
    return 0;

  // email content
  espClient.println("To: " + email.recipient);
  espClient.println("From: " + email.sender);
  String currentTime = getCurrentTime();
  String subject = "Subject: Viper Alarm Triggered on " + currentTime + "\r\n";
  espClient.println(subject);
  espClient.println("THE VIPER TEMP ALARM HAS BEEN TRIGGERED!!!");
  espClient.println("A data dump is required from the INVENTORY TEMPERATURE MONITORING SYSTEM.");
  espClient.println("Triggered at: " + currentTime);
  espClient.println("Press and hold the Red Button on the Viper Temp Alarm for 3 seconds to reset it.");
  espClient.println("");
  espClient.println("**VIPER TEMP ALARM**");
  espClient.println("   \"Keeping your stuff cool, since 2019\"");

  espClient.println(".");
  if (!emailResp())
    return 0;

  espClient.println("QUIT");
  if (!emailResp())
    return 0;

  espClient.stop();
  return 1;
}

String getCurrentTime()
{
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

  timeClient.begin();

  timeClient.update();

  String currentTime = daysOfTheWeek[timeClient.getDay()] + ", " + timeClient.getHours() 
    + ":" + timeClient.getMinutes() + ":" + timeClient.getSeconds();

  timeClient.end();

  return currentTime;
}

// -eof