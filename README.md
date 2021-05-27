# viper-temp-alarm-2.0

## System Overview

The OM-THA2-U Temperature/Humidity/Dewpoint Alarm's (120-000060-000) NO (normally open) relay output 
is connected to a INPUT pin on the WeMos D1 Mini ESP8266 micro controller. When tripped this program will send
an email using Standard Mail Transfer Protocol (SMTP) which can then be forwarded to any desired email. It wll also
begin a light blinking sequence on the external LED.

### Detailed Description

There is one input available to the user and one to the OM-THA2-U Temperature Alarm.
- User Input:
    - Red button
- Alarm Input:
    - Trigger pin

The only output of the system is a red LED that is built into the button.

There are two states the system can be in: 
- Active
- Not active

When the system is in a deactivated state it will continually loop to check if the Trigger pin
has been tripped.
When the System first becomes Active it will attempt to connect to the Virtual-Incision network
Upon successes it will attempt to send an email with the current date and time and a warning message
that the alarm has been tripped.
After this it will begin a light flashing sequence on the output LED to visually notify that the alarm is active.

## System Use

The system once powered on should imminently start in a deactivated state. It will begin checking the Trigger pin.
If the system is tripped an email should be sent to the desired addresses and the LED should begin blinking.

To deactivate the alarm press and hold the Red button for 3 seconds. The LED should flash rapidly for a few seconds and then turn solid and finally turn off. This indicates that the alarm has been deactivated and it will again begin checking the Trigger pin. 

## Wiring information

- Red button
    - D1
    - Gnd
- Trigger pin
    - D2
    - Gnd
- Led output
    - Neg: D3
    - Pos: Gnd
- DC input
    - Pos: 5V
    - Neg: Gnd

Polarities on Led are switched to allow greater current draw.

## Account information:

const String ssid = "VirtualIncision-Internal";
const String networkPassword = "R0b0tsCutY0u";

// email recipient account info
const String recipientEmail = "vipermtempalarm@gmail.com";
const String recipientEmailPassword = "";

// smtp account info
const String smtpUserName = "vipermtempalarmsender@gmail.com";
const String smtpPassword = "PNiXugnd0cYt";

// email sender account info
const String senderEmail = "vipermtempalarmsender@gmail.com";
const String senderEmailPassword = "XXaJQnV2t3utL7bu";

## Rebuild and edit this project

This project must me built as a PlatformIO project.
- Install VS Code and PlatformIO extension
- Locate PlatformIO project folder and clone this git repo into it
- Install 2 library dependencies using PIO 
    - densaugeo/base64@^1.2.0
	- arduino-libraries/NTPClient@^3.1.0
review the paltformio.ini file for further information

## Ownership and contact

This software and belongs to Virtual Incision Corporation
Last Modified: 5/27/2021
Author: David Ryan
   E: davidryn6@gmail.com
   C: (402)-499-8715