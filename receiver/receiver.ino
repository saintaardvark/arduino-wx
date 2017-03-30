#include <PinChangeInterruptHandler.h>
#include <RFReceiver.h>

// Listen on digital pin 2
#define PULSE_LENGTH     500
#define LEDPIN           13

RFReceiver receiver(2, PULSE_LENGTH);
String payload;
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
        index = payload.indexOf(endOfMessage);
        payload.remove(index);
        Serial.println(payload);
        flashyflashy();
}
