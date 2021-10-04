// Ownership: Virtual Incision Corp
// Last Modified: 6/1/2021
// Author: David Ryan
//   E: davidryn6@gmail.com
//   C: (402)-499-8715

// ++Virtual Incision Permanent Temperature Alarm++

#include <Arduino.h>

#include <ESP8266WiFi.h> // wifi lib
#include <WiFiUdp.h>     // time server lib

#include <base64.h>    // base 64 conversion lib
#include <NTPClient.h> // time server lib

#include <time.h>

const long utcOffsetInSeconds = -18000; // utc offset

// to get day of week from number
String daysOfTheWeek[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// network info
const String ssid = "";
const String networkPassword = "";

// email recipient account info
const String recipientEmail = "";
const String recipientEmailPassword = ""; // not used

// smtp account info
const String smtpUserName = "";
const String smtpPassword = "";

// email sender account info
const String senderEmail = "";
const String senderEmailPassword = "";

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

bool debugInfo = false; // debug info to Serial Monitor

unsigned long pressTime = 0; // time of current press

unsigned long pressReleaseDelta = 1000; // required press time to reset in ms

uint16_t checkDelta = 1000; // loop time delay

uint16_t smtpPort = 2525;

// button declarations
Button alarmTrigger{D3, false};
Button alarmReset{D5, false};

uint8_t ledPin = D4; // led pin number

WiFiClient espClient;

// Interupt Service Routine
void IRAM_ATTR isr()
{
  alarmReset.pressed = !alarmReset.pressed;

  // toggle function based on press and release
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
int connectToWifi();
void disconnectFromWifi();
int emailResp();
int sendAlarmEmail();
String getCurrentTime(String s);
inline const String BoolToString(bool b);

void setup()
{
  if (debugInfo)
  {
    Serial.begin(9600);

    Serial.println();

    Serial.println("Beginning...");
  }

  pinMode(alarmReset.PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(alarmReset.PIN), isr, CHANGE);

  pinMode(alarmTrigger.PIN, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, HIGH); // output is inverted

  if (debugInfo)
  {
    Serial.println("Setup compleate.");
  }
}

void loop()
{
  delay(checkDelta); // delay to check

  if (debugInfo)
  {
    Serial.println("Alarm: " + BoolToString(alarmActive) + ", Checking Trigger Pin...");
  }

  if (digitalRead(alarmTrigger.PIN) == LOW) // check for alarm trigger pull up resistor enabled check for low
  {

    if (debugInfo)
    {
      Serial.println();
      Serial.println("ALARM TRIPPED!!!");
    }

    alarmActive = true;

    digitalWrite(ledPin, LOW);

    int wifiError = connectToWifi(); // establish network connection

    if (debugInfo)
    {
      String printWifiError = "WiFi retrun: " + String(wifiError);
      Serial.println(printWifiError);
      /**
       * 2 - Error: connection unsuccessful after 5 attempts
       * 1 - Connection successful
       */
    }

    int emailError = sendAlarmEmail(); // send formated email with current time

    if (debugInfo)
    {
      String printEmailError = "Email return: " + String(emailError);
      Serial.println(printEmailError);
      /**
       * 2 - Error: failed to connect to server
       * 3 - Error: failed to greet server
       * 4 - Error: failed to begin authentication
       * 5 - Error: failed to encode username
       * 6 - Error: failed to encode password
       * 7 - Error: failed to enter sender
       * 8 - Error: failed to enter recipient
       * 9 - Error: failed to start email
       * 10 - Error: failed to end email
       * 11 - Error: failed to quit server
       * 12 - Error: failed to authenticate STARTTLS security 
       * 1 - Email successfully sent
       */
      Serial.println("Current time: " + getCurrentTime("hard"));
    }

    //disconnectFromWifi(); // disconnect from network

    // blink light wait for isr
    while (alarmActive == true)
    {
      // D1-mini chip io pins can draw more then they can
      // output so inverted wiring in order to make led brighter

      if (debugInfo)
      {
        Serial.println("Alarm: " + BoolToString(alarmActive) + ", blinking LED...");
      }

      digitalWrite(ledPin, LOW); // turn LED ON
      delay(300);
      digitalWrite(ledPin, HIGH); // turn LED OFF
      delay(200);
      digitalWrite(ledPin, LOW); // turn LED ON
      delay(300);
      digitalWrite(ledPin, HIGH); // turn LED OFF
      delay(800);
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

    if (debugInfo)
    {
      Serial.println("Alarm Reset.");
    }
  }
}

int connectToWifi()
{
  // attempt a connection at least 5 times
  for (uint8_t attempts = 0; attempts <= 5; attempts++)
  {
    if (debugInfo)
    {
      Serial.println("Connecting to WiFi");
    }
    WiFi.begin(ssid, networkPassword);
    for (uint8_t i = 0; i <= 20; i++)
    {
      delay(500);
      if (debugInfo)
      {
        Serial.print(".");
      }
      // wait for connection to be established

      if (WiFi.status() == WL_CONNECTED)
      {
        // break both loops
        if (debugInfo)
        {
          Serial.println(); // WiFi connection output
          Serial.println("Wifi connection successfull");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
        }
        return 1;
      }
    }
    if (debugInfo)
    {
      Serial.println();
    }
    delay(2000);
  }
  return 2;
}

void disconnectFromWifi()
{
  WiFi.disconnect(); // disconect from WiFi
  if (debugInfo)
  {
    Serial.println("Disconnected from WiFi.");
  }
}

// send an email using SMTP
int sendAlarmEmail()
{

  Email email{smtpUserName, smtpPassword, senderEmail, senderEmailPassword, recipientEmail}; // email struct

  if (espClient.connect(server, smtpPort) == 1) // attempt connection to server
  {
    if (debugInfo)
    {
      Serial.println("connected to SMTP server");
      Serial.println("Server Response:");
    }
  }
  else
  {
    if (debugInfo)
    {
      Serial.println("connection to SMTP server failed");
    }
    return 2;
  }

  if (!emailResp())
  {
    return 2;
  }

  espClient.println("EHLO");

  if (!emailResp())
  {
    return 3;
  }

  // only use if STARTTLS network security is used
  /*
  espClient.println("STARTTLS");
  if (!emailResp()) 
  return 12;
  */

  espClient.println("AUTH LOGIN");

  if (!emailResp())
  {
    return 4;
  }

  espClient.println(base64::encode(email.smtpUserName)); //base64, ASCII encoded Username

  if (!emailResp())
  {
    return 5;
  }

  espClient.println(base64::encode(email.smtpPassword)); //base64, ASCII encoded Password

  if (!emailResp())
  {
    return 6;
  }

  String from = "MAIL From: " + email.sender;
  espClient.println(from);
  if (!emailResp())
  {
    return 7;
  }
  String to = "RCPT To: " + email.recipient;
  espClient.println(to);
  if (!emailResp())
    return 8;

  // sending data
  espClient.println("DATA");
  if (!emailResp())
  {
    return 9;
  }

  // email content
  espClient.println("To: " + email.recipient);
  espClient.println("From: " + email.sender);
  String softCurrentTime = getCurrentTime("Soft");
  String hardCurrentTime = getCurrentTime("Hard");
  String subject = "Subject: Viper Alarm Triggered on " + softCurrentTime + "\r\n";
  espClient.println(subject);
  espClient.println("THE VIPER TEMP ALARM HAS BEEN TRIGGERED!!!");
  espClient.println("Triggered: " + hardCurrentTime);
  espClient.println();
  espClient.println("A data dump is required from the INVENTORY TEMPERATURE MONITORING SYSTEM.");
  espClient.println("OM-THA2-U Temperature/Humidity/Dew-point Alarm (120-000060-001)");
  espClient.println();
  espClient.println("TO RESET: Press and hold the Red Button on the Viper Temp Alarm for 2 seconds and then release.");

  espClient.println(".");
  if (!emailResp())
  {
    return 8;
  }

  espClient.println("QUIT");
  if (!emailResp())
  {
    return 11;
  }

  espClient.stop();

  return 1;
}

int emailResp()
{
  byte responseCode;
  byte readByte;
  uint16_t loopCount = 0;

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

  responseCode = espClient.peek(); // peek response code
  while (espClient.available())
  {
    readByte = espClient.read();
    if (debugInfo)
    {
      Serial.write(readByte); // code received in ASCII bytes that can be printed to serial
    }
  }

  if (responseCode >= '4')
  {
    return 0;
  }

  return 1;
}

// get current date and time
String getCurrentTime(String s)
{
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "us.pool.ntp.org", utcOffsetInSeconds);

  timeClient.begin();

  timeClient.update(); // get current time

  String currentTime;



    time_t rawtime = timeClient.getEpochTime();
    struct tm ts;
    char buf[80];

    ts = *localtime(&rawtime);


  if (s == "s" || s == "soft" || s == "Soft" || s == "SOFT")
  {

    strftime(buf, sizeof(buf), "%A, %r", &ts);
    currentTime = buf;

    // alternate date format for soft version
    //currentTime = daysOfTheWeek[timeClient.getDay()] + ", " + timeClient.getFormattedTime(); // construct soft string for time
  }
  else
  {
    strftime(buf, sizeof(buf), "%c", &ts);
    currentTime = buf; // convert to string
  }

  timeClient.end();

  return currentTime;
}

// Bool to String
inline const String BoolToString(bool b)
{
  return b ? "true" : "false";
}

// -eof