/**************************************************************************/
/*! 
    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.

*/
/**************************************************************************/

// choose to SPI or I2C or HSU
#if 0
  #include <SPI.h>
  #include <PN532_SPI.h>
  #include "PN532.h"

  PN532SPI pn532spi(SPI, 10);
  PN532 nfc(pn532spi);
#elif 0
  #include <PN532_HSU.h>
  #include <PN532.h>
      
  PN532_HSU pn532hsu(Serial);
  PN532 nfc(pn532hsu);
#else 
  #include <Wire.h>
  #include <PN532_I2C.h>
  #include <PN532.h>

  PN532_I2C pn532i2c(Wire);
  PN532 nfc(pn532i2c);
#endif
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void printHex(uint8_t* data, uint8_t len) {
    int i = 0;
    for (;i < len; i++) {
      Serial.print(data[i], HEX);
    }
}

void printHexChar(uint8_t* data, int numBytes) {
    for (uint8_t i = 0; i < numBytes; i++) {
        Serial.print(data[i], HEX);
    }
    Serial.print("        ");
    for (uint8_t i = 0; i < numBytes; i++) {
        char c = data[i];
        if (c <= 0x1f || c > 0x7f) {
            Serial.print('.');
        } else {
            Serial.print(c);
        }
    }
}

int debug = 0 ;
void setup(void) {
  if (debug) {
    Serial.begin(115200);
    Serial.println("Hello!");
  }
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata && debug) {
    Serial.print("Didn't find PN53x board");
  }
  // Got ok data, print it out!
  if (debug) {
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  }
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  if (debug) {
    Serial.println("Waiting for an ISO14443A Card ...");
  }
  // Transmitter is connected to Arduino Pin #2
  mySwitch.enableTransmit(2);
  mySwitch.setPulseLength(192);
  digitalWrite(13, LOW);
}

boolean testEqual(uint8_t* test_key, uint8_t* auth_key, uint8_t uidLength) {
  int i = 0;
  for (; i < uidLength; i++) {
    if (test_key[i] != auth_key[i]) {
      return false;
    }
  }
  return true;
}

int lightStatus = 0;

void on() {
  lightStatus = 1;
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  mySwitch.send(5248307, 24);
  mySwitch.send(5248451, 24);
  mySwitch.send(5248771, 24);
  mySwitch.send(5250307, 24);
  mySwitch.send(5256451, 24);
}

void off() {
  lightStatus = 0;
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  mySwitch.send(5248316, 24);
  mySwitch.send(5248460, 24);
  mySwitch.send(5248780, 24);
  mySwitch.send(5250316, 24);
  mySwitch.send(5256460, 24);
}

void loop(void) {

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  uint8_t nathan_key[] = { 4, 24, 253, 18, 255, 56, 129 };
  uint8_t jason_key[] = { 4, 7, 6, 18, 255, 56, 133 };
  uint8_t zella_key[] = { 4, 233, 254, 18, 255, 56, 128 }; //4E9FE12FF3880
  uint8_t blue_tag[] = { 243, 215, 214, 128 };
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    digitalWrite(13, HIGH);
    // Display some basic information about the card
    if (debug) {
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");printHex(uid, uidLength);
      Serial.println();
      Serial.println("");
    }
    if ((uidLength == 7 && (testEqual(uid, jason_key, 7) || testEqual(uid, nathan_key, 7) || testEqual(uid, zella_key, 7))) || (uidLength == 4 && testEqual(uid, blue_tag, 4))) {
      if (lightStatus == 0) {
        lightStatus = 1;
      }
      else {
        lightStatus = 0;
      }
    }
  }
  else {
    digitalWrite(13, LOW);
  }
  if (lightStatus == 1) {
    on();
  } else {
    off();
  }
  delay(1000);
}
