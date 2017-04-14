#include <SPI.h>
#include "RF24.h"

#define LEDPIN           13

/* 
   Format of message: 
   {Tmp 19.30 C,Hmd 50.00 %,Prc 0 AU,}

   "{": start of data (1 char)

   Up to four of:
   "XXX ": Measurement type (4 char)
   "XXXX.XX ": Measurement (8 char)
   "XX ": Unit (3 char)
   ",": Separator (1 char)
   Total: 16 char

   "}": end of data (1 char)

   Null term: 1 char (not sure if this is needed)

   1 + 4 x 16 + 1 + 1= 66 chars

 */
#define MAX_PAYLOAD_LEN 66

char payload[MAX_PAYLOAD_LEN];

const char endOfMessage = '|';
int index;

void flashyflashy() {
        for (int i = 0 ; i < 2 ; i++ ) {
                digitalWrite(LEDPIN, HIGH);
                delay(125);
                digitalWrite(LEDPIN, LOW);
                delay(125);
        }
}

void setup() {
        Serial.begin(9600);
        Serial.println("Waiting...");
        receiver.begin();
}

void loop() {
        char msg[MAX_PACKAGE_SIZE];
        byte senderId = 0;
        byte packageId = 0;
        byte len = receiver.recvPackage((byte *)msg, &senderId, &packageId);
        payload = String(msg);
        Serial.println(payload);
}
