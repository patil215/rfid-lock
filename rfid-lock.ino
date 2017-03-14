#include <AddicoreRFID.h>
#include <SPI.h>
#include <Servo.h>
#include "AllowedCards.h"

#define uchar unsigned char

AddicoreRFID rfid;

Servo lockServo;
boolean locked;

const int POSITION_UNLOCK = 142;
const int POSITION_LOCK = 52;

const int PIN_CHIPSELECT = 10;
const int PIN_NRST = 5;
const int PIN_TRANSISTOR = 9;
const int PIN_SERVO = 8;
const int PIN_BUZZER = 7;


void initializePins() {
  pinMode(4, OUTPUT);
  pinMode(PIN_TRANSISTOR, OUTPUT);
  pinMode(PIN_CHIPSELECT, OUTPUT);
  pinMode(PIN_NRST, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  
  SPI.begin();

  initializePins();

  digitalWrite(PIN_TRANSISTOR, HIGH);
  lockServo.write(POSITION_UNLOCK);
  
  lockServo.attach(PIN_SERVO);
  locked = false;
  
  digitalWrite(PIN_TRANSISTOR, LOW);

  digitalWrite(PIN_CHIPSELECT, LOW);
  digitalWrite(PIN_NRST, HIGH);

  rfid.AddicoreRFID_Init();
}

void setServo(boolean enabled) {
  if(enabled) {
    digitalWrite(PIN_TRANSISTOR, HIGH);
  } else {
    digitalWrite(PIN_TRANSISTOR, LOW);
  }
}

void lockBeep() {
    tone(PIN_BUZZER, 800); // Send 1KHz sound signal...
    delay(200);
    noTone(PIN_BUZZER);
    tone(PIN_BUZZER, 1000); // Send 1KHz sound signal...
    delay(200);
    noTone(PIN_BUZZER);
}

void unlockBeep() {
    tone(PIN_BUZZER, 1000); // Send 1KHz sound signal...
    delay(200);
    noTone(PIN_BUZZER);
    tone(PIN_BUZZER, 800); // Send 1KHz sound signal...
    delay(200);
    noTone(PIN_BUZZER);
}

void toggleLock() {
    if (!locked) {
      lockBeep();
      
      setServo(true);

      lockServo.attach(PIN_SERVO);
      delay(15);

      lockServo.write(POSITION_LOCK);
      delay(500);

      setServo(false);

      locked = true;
    } else {
      unlockBeep();
      
      setServo(true);

      lockServo.attach(PIN_SERVO);
      delay(15);

      lockServo.write(POSITION_UNLOCK);
      delay(500);


      setServo(false);

      locked = false;
    }
}

uchar * readRfid() {
  uchar str[16];
  //Find tags, return tag type
  uchar status = rfid.AddicoreRFID_Request(PICC_REQIDL, str);
  //Anti-collision, return tag serial number 4 bytes
  status = rfid.AddicoreRFID_Anticoll(str);
  if(status == MI_OK) {
    return str;
  }
  return NULL;
}

void loop() {
  lockServo.detach();

  uchar * str = readRfid();
  if(str != NULL) {
    Serial.print("Tag number:\t");
    Serial.print(str[0]);
    Serial.print(" ,");
    Serial.print(str[1]);
    Serial.print(" ,");
    Serial.print(str[2]);
    Serial.print(" ,");
    Serial.print(str[3]);
    Serial.print("\n");
    
    uchar checksum = str[0] ^ str[1] ^ str[2] ^ str[3];
    Serial.print("Calculated checksum:\t");
    Serial.println(checksum);

    toggleLock();
    
    Serial.println();
    delay(1000);
  }

  rfid.AddicoreRFID_Halt();
}
