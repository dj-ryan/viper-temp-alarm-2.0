#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <base64.h>

// network info
const String ssid = "VirtualIncision-Internal";
const String networkPassword = "R0b0tsCutY0u";

// email recipient account info
const String recipientEmail = "vipermtempalarm@gmail.com";
const String recipientEmailPassword = "";

const String smtpUserName = "vipermtempalarmsender@gmail.com";
const String smtpPassword = "PNiXugnd0cYt";


// email sender account info
const String senderEmail = "vipermtempalarmsender@gmail.com";
const String senderEmailPassword = "XXaJQnV2t3utL7bu";

String server = "mail.smtp2go.com"; // The SMTP Server 

struct Button {
  uint8_t PIN;
  bool pressed;
};

struct Email {
  String smtpUserName;
  String smtpPassword;
  String sender;
  String senderPassword;
  String recipient;





};

bool alarmActive = false;

unsigned long pressTime = 0;

unsigned long pressReleaseDelta = 2000; // in ms

Button alarmTrigger {2, false};
Button alarmReset {3, false};

uint8_t ledPin = 4;

WiFiClient espClient;

void ICACHE_RAM_ATTR isr() {
  alarmReset.pressed = !alarmReset.pressed;
  if(alarmReset.pressed == true) {
    pressTime = millis();
  } else {
    if(millis() - pressTime > pressReleaseDelta) {
      alarmActive = false;
    }
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
    // Wait for 20 seconds and if nothing is received, stop.
    if (loopCount > 20000) 
    {
      espClient.stop();
      Serial.println(F("\r\nTimeout"));
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
    //  efail();
    return 0;
  }
  return 1;
}

void connectToWifi() {
  delay(10);
  Serial.println("");
  Serial.println("");
  Serial.print("Connecting To: ");
  Serial.println(ssid);
  WiFi.begin(ssid, networkPassword);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



int sendAlarmEmail() {


Email email {smtpUserName, smtpPassword, senderEmail, senderEmailPassword, recipientEmail};

if (espClient.connect(server, 2525) == 1) 
  {
    Serial.println(F("connected"));
  } 
  else 
  {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending EHLO"));
  espClient.println("EHLO www.example.com");
  if (!emailResp()) 
    return 0;
  //
  /*Serial.println(F("Sending TTLS"));
  espClient.println("STARTTLS");
  if (!emailResp()) 
  return 0;*/
  //  
  Serial.println("Sending auth login");
  espClient.println("AUTH LOGIN");
  if (!emailResp()) 
    return 0;
  //  
  Serial.println("Sending User");
  // Change this to your base64, ASCII encoded username
  /*
  For example, the email address test@gmail.com would be encoded as dGVzdEBnbWFpbC5jb20=
  */
  espClient.println(base64::encode(email.smtpUserName)); //base64, ASCII encoded Username
  if (!emailResp()) 
    return 0;
  //
  Serial.println("Sending Password");
  // change to your base64, ASCII encoded password
  /*
  For example, if your password is "testpassword" (excluding the quotes),
  it would be encoded as dGVzdHBhc3N3b3Jk
  */
  espClient.println(base64::encode(email.smtpPassword));//base64, ASCII encoded Password
  if (!emailResp()) 
    return 0;
  

  String from = "MAIL From: " + email.sender;
  Serial.println(from);
  espClient.println(from);
  if (!emailResp()) 
    return 0;

  String to = "RCPT To: " + email.recipient;
  Serial.println(to);
  espClient.println(to);
  if (!emailResp()) 
    return 0;
  

  Serial.println("Sending DATA");
  espClient.println("DATA");
  if (!emailResp()) 
    return 0;


  Serial.println(F("Sending email"));
  // change to recipient address
  espClient.println("To: " + email.recipient);
  // change to your address
  espClient.println("From: " + email.sender);
  espClient.println(F("Subject: ESP8266 test e-mail\r\n"));
  espClient.println(F("This is is a test e-mail sent from ESP8266.\n"));
  espClient.println(F("Second line of the test e-mail."));
  espClient.println(F("Third line of the test e-mail."));
  //
  espClient.println(F("."));
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending QUIT"));
  espClient.println(F("QUIT"));
  if (!emailResp()) 
    return 0;
  //
  espClient.stop();
  Serial.println(F("disconnected"));
  return 1;


  
}






void setup() {

  Serial.begin(9600);

  Serial.println("Beginning...");

  pinMode(alarmReset.PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(alarmReset.PIN), isr, CHANGE);
  
  pinMode(alarmTrigger.PIN, INPUT_PULLUP);  
  
  pinMode(ledPin, OUTPUT);
  


  connectToWifi();

  Serial.println("connection to wifi successfull");

  sendAlarmEmail();

}

void loop() {

  delay(1000);
  Serial.println("checking...");

  if(digitalRead(alarmTrigger.PIN) == HIGH) {
    
    alarmActive = true;
    
    
      
  
    // blink light wait for isr
      while(alarmActive == true) {
        // D1-mini chip io pins can draw more then they can 
        // output so inverted wiring in order to make led brighter
        Serial.println("blinking...");
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
    for(uint8_t i = 0; i < 10; i++) {
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

