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

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
byte addresses[][6] = {"1Node","2Node"};
// Used to control whether this node is sending or receiving
bool role = 0;

const char endOfMessage = '|';
int index;

/* Uncomment for helpful debug messages */
#define DEBUGGING 1

void debug(String msg) {
#ifdef DEBUGGING
        Serial.println(msg);
#endif  /* DEBUGGING */
}

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
        Serial.println("radio.begin");
        radio.begin();

        // Set the PA Level low to prevent power supply related issues since this is a
        // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
        radio.setPALevel(RF24_PA_LOW);

        // Open a writing and reading pipe on each radio, with opposite addresses
        debug("Setting up pipes");
        debug("Writing to 0, reading from 1");
        radio.openWritingPipe(addresses[0]);
        radio.openReadingPipe(1,addresses[1]);

        radio.startListening();
        Serial.println("Waiting...");
}

void loop() {

        unsigned long got_time;
        int len = 0;

        if( radio.available()){
                // Variable for the received timestamp
                while (radio.available()) {                                   // While there is data ready
                        len = radio.getDynamicPayloadSize();
                        Serial.println(len);
                        radio.read(&payload, len);              // Get the payload
                }
                Serial.print("Payload: |");
                Serial.print(payload);
                Serial.println("|");
                radio.stopListening();                                        // First, stop listening so we can talk
                radio.write( &got_time, sizeof(unsigned long));              // Send the final one back.
                radio.startListening();                                       // Now, resume listening so we catch the next packets.
                Serial.print(F("Sent response "));
                Serial.println(got_time);
        }

        /* char msg[MAX_PACKAGE_SIZE]; */
        /* byte senderId = 0; */
        /* byte packageId = 0; */
        /* byte len = receiver.recvPackage((byte *)msg, &senderId, &packageId); */
        /* payload = String(msg); */
        /* Serial.println(payload); */
}
