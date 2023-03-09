#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "SoftwareSerial.h"
//const uint8_t fingerprint[20] = {0x4a, 0x2e, 0xb2, 0xa8, 0x29, 0x12, 0x9a, 0xca, 0xac, 0xe1, 0xe0, 0xf4, 0xa0, 0x6c, 0x74, 0x4b, 0x4b, 0x7d, 0x5b, 0xab};
// 4a 2e b2 a8 29 12 9a ca ac e1 e0 f4 a0 6c 74 4b 4b 7d 5b ab

// Global Variables 
bool startSession = false;
const String students[2] = {"84B2C333","D438CA33"};
const String sreg[2] = {"RA2011003020041","RA2011003020104"};
const String snam[2] = {"Arjun","Atharv"};
const String teachers[2] = {"EA97EEAF","C37D1AAA"};
const String tnam[2] = {"Mr.Sadhasivam","Ms.Swathi"};
int flag = -1;
bool toCheck = false;
String techerName = "";
const uint8_t fingerprint[20] = {0x86, 0x6D, 0x53, 0xA4, 0x65, 0x2C, 0xEC, 0xE3, 0x62, 0xF2, 0x37, 0xBE, 0xA7, 0x28, 0x16, 0x45, 0x51, 0xC0, 0x5D, 0x02};
//                                86 6D 53 A4 65 2C EC E3 62 F2 37 BE A7 28 16 45 51 C0 5D 02

#define RST_PIN  D3     // Configurable, see typical pin layout above
#define SS_PIN   D4     // Configurable, see typical pin layout above
#define BUZZER   D2     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;  
ESP8266WiFiMulti WiFiMulti;
MFRC522::StatusCode status;  
SoftwareSerial ser(5, 16); // RX, TX    

/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  

/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

String data2;
const String urlLink = "https://script.google.com/macros/s/AKfycbz4HLjed4iQCdcdtSC4Tf-qVNq3zsqsxto3BGOBUqAjsh4VxPAozCDuBk7ArH47zCGplg/exec";
void setup() 
{
  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  // Serial.setDebugOutput(true);
    ser.begin (115200);
    
  for (uint8_t t = 4; t > 0; t--) 
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  
  /* Put your WIFI Name and Password here */
  WiFiMulti.addAP("r", "Riteshrj");

  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);

  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(2);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop()
{
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  //Show UID on serial monitor
    delay(2000);
    String tagID = "";
    
    for ( uint8_t i = 0; i < 4; i++) {                  // The MIFARE tag in use has a 4 byte UID
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single string variable
    }
    tagID.toUpperCase();
    mfrc522.PICC_HaltA(); // Stop reading
    Serial.println(tagID);
    toCheck = true;
    
    for(int i=0;i<2;i++)
    {
      if(flag == -1)
       { //session has not been started
        if(teachers[i] == tagID)
        {
          flag = 1;
          startSession = true;
          Serial.print("Teacher = ");
          toCheck = false;
          Serial.println(tnam[i]);
          techerName = tnam[i];
          mfrc522.PICC_HaltA(); // Stop reading
          delay(2000);
          break;      
        }  
       }
      else
      if(flag == 1)
      { // session going on 
        if(teachers[i] == tagID)
        {
          flag = 0;
          techerName = "";
          startSession = false;
          delay(2000);
          break;
        }
      }
    }

    
    if(flag == -1 and startSession == false)
    { // session not started
      Serial.println("The session is not started please wait");
    }
    else 
    if(flag == 0 and startSession == false)
    { // close the session
      techerName = "";
      Serial.println("The session is CLOSED");
    }
    else 
    if(flag == 1 and startSession == true)
    {
       //update student
        
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      bool isStd = false;
      String data1 = "";
      for(int i=0;i<2;i++){
        if((tagID == students[i]) && (toCheck == true)){
            isStd = true;
            data1 = "?data1="+ snam[i]+"&data2=" +sreg[i]+"&data3="+ techerName;
//            Serial.print("data1=");
//            Serial.println(data1);
            ser.write(1);
            delay(2000);          
        }
      }
      if(isStd = false){
        Serial.println("Student was not registered");
          delay(2000);
      }
    
      // wait for WiFi connection
      if ((WiFiMulti.run() == WL_CONNECTED) && (toCheck==true)) 
      {
        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    
        client->setFingerprint(fingerprint);
        // Or, if you happy to ignore the SSL certificate, then use the following line instead:
        // client->setInsecure();
    
    //    data2 = urlLink + String((char*)readBlockData);
        data2 = urlLink + String(data1);
        data2.trim();
        Serial.println(data2);
        
        HTTPClient https;
        Serial.print(F("[HTTPS] begin...\n"));
        if (https.begin(*client, (String)data2) && (isStd=true))
        {  
          // HTTP
          Serial.print(F("[HTTPS] GET...\n"));
          // start connection and send HTTP header
          int httpCode = https.GET();
        
          // httpCode will be negative on error
          if (httpCode > 0) 
          {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            // file found at server
          }
          else 
          {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
          https.end();
          delay(1000);
        } 
        else 
        {
          Serial.printf("[HTTPS} Unable to connect\n");
        }
      }
   }
}
