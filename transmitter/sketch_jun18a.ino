#include <RFTransmitter.h>

#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

#define NODE_ID          1
#define OUTPUT_PIN       11
#define LEDPIN           13

// Send on digital pin 11 and identify as node 1
RFTransmitter transmitter(OUTPUT_PIN, NODE_ID);

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        dht.begin();

}

void loop() {
  //char *msg = "Hello World!        ";
  char *msg;
  sprintf((char *)msg, "Hello, world!  T: %d H: %f", t, h);
  //String textmsg = "Hello, world! Temp: ";

  transmitter.resend((byte *)msg, strlen(msg) + 1);
        digitalWrite(LEDPIN, HIGH);
        delay(500);
        digitalWrite(LEDPIN, LOW);
        float h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        float t = dht.readTemperature();
        //char *msg = malloc(strlen(printf("Hello, world! T: %f H: %f", t, h)));
        Serial.println("I'm here...");
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print("Temp: ");
        Serial.print(t);
        Serial.println();
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
        delay(1000);

}
