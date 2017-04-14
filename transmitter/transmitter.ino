#include <SPI.h>
#include "RF24.h"
#include <Adafruit_Sensor.h>


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

byte addresses[][6] = {"1Node","2Node"};

String final_msg_string;

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

#define LEDPIN           13
#define PRECIP_PIN       A0
#define MAX_SENSORS      4

struct SensorData {
        const char* name;
        const char* units;
        float value;
};

struct NodeData {
        const char* name;
        SensorData *data[MAX_SENSORS];
};

/* NODE_ID is needed for the transmitter. */
#define NODE_ID          3
NodeData node;

#define SLEEPYTIME 10000

/* Uncomment for helpful debug messages */
#define DEBUGGING 1

float humid;
float temp;
int precip;

void debug(String msg) {
#ifdef DEBUGGING
        Serial.println(msg);
#endif  /* DEBUGGING */
}

void flashyflashy() {
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
}

void transmit(String msg) {
        radio.stopListening();
        /* See https://forum.arduino.cc/index.php?topic=341963.0 */
        char payload[MAX_PAYLOAD_LEN];
        msg.toCharArray(payload, MAX_PAYLOAD_LEN);
        Serial.println(msg);
        flashyflashy();
        if (!radio.write(&payload, strlen(payload))){
                Serial.println(F("failed"));
        }
        flashyflashy();
        radio.startListening();
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        Serial.println("Reboot");

        /* DHT init */
        Serial.println("dht.begin");
        dht.begin();

        /* NRF24L01 init */
        Serial.println("radio.begin");
        radio.begin();
        /*
          Set the PA Level low to prevent power supply related issues.
          Since this is a getting_started sketch, and the likelihood
          of close proximity of the devices,  RF24_PA_MAX is default.

          FIXME: Refactor to make the addresses clearer.
        */

        radio.setPALevel(RF24_PA_LOW);
        debug("Setting up pipes");
        debug("Writing to 1, reading from 0");
        radio.openWritingPipe(addresses[1]);
        radio.openReadingPipe(1, addresses[0]);
        radio.startListening();

        /* BMP init */

#ifdef HAVE_BMP
        Serial.println("Pressure Sensor Test");
        Serial.println("");
        if(!bmp.begin()) {
                /* There was a problem detecting the BMP085 ... check your connections */
                Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
                while(1);
        }
#endif  /* HAVE_BMP */

        /* Finally, ready to go! */
        Serial.println("Node ID: " + String(NODE_ID));
        Serial.println("LET'S DO THIS!");
        Serial.println("---");
}

void loop() {

        SensorData humid_data;
        SensorData temp_data;

        /*
          You'd *think* you could just assign the result of
          dht.readHumidity() directly to humid_data.value.  Turns out
          that's not the case; if you do that, you just get 0 in
          there.  Same applies to dht.readTemperature.
        */
        humid = dht.readHumidity();
        humid_data.name = "humid";
        humid_data.units = "%";
        humid_data.value = humid;

        temp = dht.readTemperature();
        temp_data.name = "temp";
        temp_data.units = "C";
        temp_data.value = dht.readTemperature();

        node.data[0] = &humid_data;
        node.data[1] = &temp_data;

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

        /* node.name = (char*) NODE_ID_STR; */
        // Doesn't seem to be an easy way to get the NODE_ID out on
        // the receiving end...
        /* final_msg_string = "Node: " + String(NODE_ID) + " , "; */
        /* final_msg_string += "Temp: " + String(temp) + " C , "; */

        final_msg_string = "{";
        final_msg_string += "Tmp " + String(temp) + " C,";
        final_msg_string += "Hmd " + String(humid) + " %,";

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
