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
#include <string.h>

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

#define NODE_ID          1
#define TRANSMITTER_PIN  11
#define LEDPIN           13

// Send on digital pin 11 and identify as node 1
RFTransmitter transmitter(TRANSMITTER_PIN, NODE_ID);


char *hlegend = "H: ";
char *tlegend = "T: ";
char *plegend = "P: ";

float h;
float t;

char hmsg[6];
char tmsg[6];
char pmsg[8];
sensors_event_t event;

int counter = 0;

void flashyflashy() {
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
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
        // 12.45\0
        hmsg[6];
        // args: (src, width, precision, dest)
        dtostrf(h, 4, 2, hmsg);
        // Read temperature as Celsius (the default)
        t = dht.readTemperature();
        // 12.45\0
        tmsg[6];
        // args: (src, width, precision, dest)
        dtostrf(t, 4, 2, tmsg);
        /* Get a new sensor event */
        //
        bmp.getEvent(&event);
        // if (event.pressure) {
        //         /* Display atmospheric pressure in hPa */
        //         Serial.print("Pressure: ");
        //         Serial.print(event.pressure);
        //         Serial.println(" hPa");
        // }
        // else {
        //         Serial.println("Sensor error");
        // }

        // 1234.67\0
        dtostrf(event.pressure, 6, 2, pmsg);
        // now combine.  See https://www.reddit.com/r/arduino/comments/2zzd41/need_help_combining_2_char_arrays/
        char final_msg[sizeof(tlegend) +
                       sizeof(tmsg) +
                       1 +      // " "
                       sizeof(hlegend) +
                       sizeof(hmsg) +
                       1 +      // " "
                       sizeof(plegend) +
                       sizeof(pmsg)
                       ];
        strcat(final_msg, tlegend);
        strcat(final_msg, tmsg);
        strcat(final_msg, " ");
        strcat(final_msg, hlegend);
        strcat(final_msg, hmsg);
        strcat(final_msg, " ");
        strcat(final_msg, plegend);
        strcat(final_msg, pmsg);
        // char *msg = "Hello World!";
        //char *msg = malloc(strlen(printf("Hello, world! T: %f H: %f", t, h)));
        // char *msg;
        // sprintf(msg, "Hello, world!  T: %f H: %f", t, h);
        //String textmsg = "Hello, world! Temp: ";
        Serial.println(final_msg);
        // transmitter.send((byte *)msg, strlen(msg) + 1);
        transmitter.send((byte *)final_msg, strlen(final_msg) + 1);
        Serial.print("That was message #");
        Serial.println(counter);
        //transmitter.send(textmsg, textmsg.length());
        flashyflashy();
        delay(1000);
        // transmitter.resend((byte *)msg, strlen(msg) + 1);
        // transmitter.resend((byte *)final_msg, strlen(final_msg) + 1);
}
