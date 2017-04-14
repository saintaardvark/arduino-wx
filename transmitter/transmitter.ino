#include <SPI.h>
#include "RF24.h"
#include <Adafruit_Sensor.h>

/* Uncomment if you have a BMP sensor */
/* #define HAVE_BMP 1 */

# ifdef HAVE_BMP
// Barometer / Temp
// Example code from https://learn.adafruit.com/bmp085?view=all
// Thanks, AdaFruit!
#include <Adafruit_BMP085_U.h>
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
sensors_event_t event;
#endif  /* HAVE_BMP */

/* Uncomment if you have precip sensor */
#define HAVE_PRECIP 1

// Humidity sensor
#include "DHT.h"
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);

byte addresses[][6] = {"trans","recv"};

/* Role: 0 == transmitter, 1 = receiver */
bool role = 0;

String final_msg_string;

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

#define TRANSMITTER_PIN  11
#define LEDPIN           13
#define PRECIP_PIN       A0

// Needed for JSON serialization
#define MAX_SENSORS 2

/* NODE_ID is needed for the transmitter. */
#define NODE_ID          2

#define SLEEPYTIME 10000

/* Uncomment for helpful debug messages */
/* #define DEBUGGING 1 */

#define DEBUGGING 1

float humid;
float temp;
int precip;

void flashyflashy() {
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
}

void transmit(String msg) {
        radio.stopListening();
        flashyflashy();
        if (!radio.write( &msg, sizeof(msg) )){
                Serial.println(F("failed"));
        }
        flashyflashy();
        radio.startListening();
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        Serial.println("Reboot");
        Serial.println("dht.begin");
        dht.begin();

        /* NRF24L01 init */
        Serial.println("radio.begin");
        radio.begin();
        // Set the PA Level low to prevent power supply related issues since this is a
        // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
        radio.setPALevel(RF24_PA_LOW);
        /* FIXME: Refactor, not needed; just keeping the copy-pasta simple for now.  */
        if (role == 0) {
                radio.openWritingPipe(addresses[1]);
                radio.openReadingPipe(1, addresses[0]);
        }
        radio.startListening();


#ifdef HAVE_BMP
        Serial.println("Pressure Sensor Test");
        Serial.println("");
        if(!bmp.begin()) {
                /* There was a problem detecting the BMP085 ... check your connections */
                Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
                while(1);
        }
#endif  /* HAVE_BMP */

        Serial.println("Node ID: " + String(NODE_ID));
}

void loop() {

        humid = dht.readHumidity();
        temp = dht.readTemperature();

#ifdef HAVE_BMP
        bmp.getEvent(&event);
        pres_data.name = "pressure";
        pres_data.units = "hPA";
        pres_data.value = event.pressure;
        node.data[2] = &pres_data;
#endif  /* HAVE_BMP */

#ifdef HAVE_PRECIP
        SensorData precip_data;
        precip_data.name = "precip";
        precip_data.units = "none";
        precip_data.value = 1023 - analogRead(PRECIP_PIN);
        node.data[3] = &precip_data;
#endif  /* HAVE_PRECIP */

        // Doesn't seem to be an easy way to get the NODE_ID out on
        // the receiving end...
        /* final_msg_string = "Node: " + String(NODE_ID) + " , "; */
        /* final_msg_string += "Temp: " + String(temp) + " C , "; */

        final_msg_string = "{";
        final_msg_string += "Tmp " + String(temp) + " C,";
#ifdef HAVE_BMP
        final_msg_string += "Prs " + String(event.pressure) + " hP,";
#endif  /* HAVE_BMP */

#ifdef HAVE_PRECIP
        /* AU == arbitrary units */
        final_msg_string += "Prc " + String(precip) + " AU,";
#endif  /* HAVE_PRECIP */

        final_msg_string += "}";


        transmit(final_msg_string);
        Serial.println(final_msg_string);
        delay(SLEEPYTIME);
}
