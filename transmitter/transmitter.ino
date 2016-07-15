#include <RFTransmitter.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

// Barometer / Temp
// Example code from https://learn.adafruit.com/bmp085?view=all
// Thanks, AdaFruit!
#include <Adafruit_BMP085_U.h>
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
sensors_event_t event;

// Humidity sensor
#include "DHT.h"

// Copied from RFReceiver.h
const byte MAX_PAYLOAD_SIZE = 80;
char VWMsgBuf[MAX_PAYLOAD_SIZE];
String final_msg_string;
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

#define NODE_ID          1
#define TRANSMITTER_PIN  11
#define LEDPIN           13

// Send on digital pin 11 and identify as node 1
RFTransmitter transmitter(TRANSMITTER_PIN, NODE_ID);

float h;
float t;

int counter = 0;

void flashyflashy() {
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
}

// Thanks, linhartr22!
// https://github.com/linhartr22/433_MHz_Wireless_TX-RX_Demo/blob/master/TX_Temp_Test/TX_Temp_Test.ino#L80-L88
void VWTX(String VWMsgStr) {
        VWMsgStr.toCharArray(VWMsgBuf, MAX_PAYLOAD_SIZE);
        uint8_t VWMsgBufLen = strlen(VWMsgBuf);
        digitalWrite(13, true); // Flash a light to show transmitting
        transmitter.send((byte *)VWMsgBuf, VWMsgBufLen);
        digitalWrite(13, false); // Flash a light to show transmitting
        Serial.println(VWMsgBufLen);
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        dht.begin();
        Serial.println("Pressure Sensor Test");
        Serial.println("");
        /* Initialise the sensor */
        if(!bmp.begin()) {
                /* There was a problem detecting the BMP085 ... check your connections */
                Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
                while(1);
        }
}

void loop() {
        h = dht.readHumidity();
        t = dht.readTemperature();

        bmp.getEvent(&event);

        final_msg_string = "Temp: " + String(t) + " C , ";
        final_msg_string += "Pres: " + String(event.pressure) + " hPA , ";
        final_msg_string += "Humid: " + String(h) + "%|";
        Serial.println(final_msg_string);

        VWTX(final_msg_string);

        Serial.print("Loop #");
        Serial.println(counter);
        //transmitter.send(textmsg, textmsg.length());
        flashyflashy();
        delay(1000);
        // transmitter.resend((byte *)msg, strlen(msg) + 1);
        // transmitter.resend((byte *)final_msg, strlen(final_msg) + 1);
}
