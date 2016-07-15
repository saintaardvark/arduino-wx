#include <PinChangeInterruptHandler.h>
#include <RFReceiver.h>

// Listen on digital pin 2
RFReceiver receiver(2);
String payload;
const char endOfMessage = '|';
int index;

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
}
