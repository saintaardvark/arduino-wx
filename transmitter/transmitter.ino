/* -*- c-basic-offset:8; indent-tabs-mode:nil -*- */

#include <SPI.h>
#include <Adafruit_Sensor.h>

/*
  Format of message:
  {Temp: 19.30 C}

  "{": start of data (1 char)
  "XXXX: ": Measurement type (6 char)
  "XXXX.XX ": Measurement (8 char)
  "XX": Unit (2 char)
  "}": end of data (1 char)

  Null term: 1 char (not sure if this is needed)

  Total: 19 chars

  This is left over from previous attempts to transmit this via
  radio.
*/
#define MAX_PAYLOAD_LEN 66

/* Uncomment if you have a BMP sensor */
#define HAVE_BMP 1

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

/* Uncomment if you have a precipitation *meter* */
#define HAVE_PRCPMTR 1

#ifdef HAVE_PRCPMTR
/* For Unos, only pins 2 and 3 can do interrupts */
#define PRCPMTR_PIN 3
int prcpPushCounter = 0;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 200;   // the debounce time; increase if the output flickers
volatile long PrcpMtrCount = 0;
void PrcpMtrISR() {
        if ((millis() - lastDebounceTime) > debounceDelay) {
                lastDebounceTime = millis();
                PrcpMtrCount++;
        }
}

#endif

/* Uncomment if you have the temperature sensors. */
#define HAVE_1WIRE_TEMP_SENSORS 1

#ifdef HAVE_1WIRE_TEMP_SENSORS
#include <OneWire.h>
#include <DallasTemperature.h>

/* Use pin 2 for data */
#define ONE_WIRE_BUS 4          /* FIXME: will need to move the
                                   humidity detector away from pin 2 -
                                   it doesn't need an interrupt. */

/* Set up a oneWire instance to communicte with any OneWire devices. */
OneWire oneWire(ONE_WIRE_BUS);

/* Pass our oneWire reference to Dallas Temperature */
DallasTemperature sensors(&oneWire);

String msg = "";
volatile short i;

/* https://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html */
void discoverOneWireDevices(void) {
        byte i;
        byte present = 0;
        /* byte data[12]; */
        byte addr[8];

        Serial.print("Looking for 1-Wire devices...\n\r");
        while(oneWire.search(addr)) {
                present++;
                Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
                for( i = 0; i < 8; i++) {
                        Serial.print("0x");
                        if (addr[i] < 16) {
                                Serial.print('0');
                        }
                        Serial.print(addr[i], HEX);
                        if (i < 7) {
                                Serial.print(", ");
                        }
                }
                if ( OneWire::crc8( addr, 7) != addr[7]) {
                        Serial.print("CRC is not valid!\n");
                        return;
                }
        }
        Serial.println("Final count:");
        Serial.println(present);
        Serial.print("\n\r\n\rThat's it.\r\n");
        oneWire.reset_search();
        return;
}
#endif  /* HAVE_1WIRE_TEMP_SENSORS */

// Humidity sensor
#include "DHT.h"

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

#define SLEEPYTIME 30000

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
        /* See https://forum.arduino.cc/index.php?topic=341963.0 */
        char payload[MAX_PAYLOAD_LEN];
        msg.toCharArray(payload, MAX_PAYLOAD_LEN);
        Serial.println(msg);
        flashyflashy();
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        Serial.println("Reboot");

        /* DHT init */
        Serial.println("dht.begin");
        dht.begin();

#ifdef HAVE_PRCPMTR
        pinMode(PRCPMTR_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(PRCPMTR_PIN), PrcpMtrISR, FALLING);
        Serial.println("prcpmtr.begin");
#endif

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

#ifdef HAVE_PRECIP
        /* Try the pullup pins */
        pinMode(PRECIP_PIN, INPUT_PULLUP);
#endif

#ifdef HAVE_1WIRE_TEMP_SENSORS
        sensors.begin();
        discoverOneWireDevices();
        Serial.println("1wire.begin");
#endif  /* HAVE_1WIRE_TEMP_SENSORS */


        /* Finally, ready to go! */
        Serial.println("Node ID: " + String(NODE_ID));
        Serial.println("LET'S DO THIS!");
        Serial.println("---");
}

String build_msg(SensorData reading) {
        String msg;
        msg = "{";
        msg += String(reading.name);
        msg += ": ";
        msg += String(reading.value);
        msg += " ";
        msg += String(reading.units);
        msg += "}";
        return msg;
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
        humid_data.name = "Humd";
        humid_data.units = "%";
        humid_data.value = humid;

        transmit(build_msg(humid_data));

        temp = dht.readTemperature();
        temp_data.name = "Temp";
        temp_data.units = "C";
        temp_data.value = dht.readTemperature();
        transmit(build_msg(temp_data));

        /* node.data[0] = &humid_data; */
        /* node.data[1] = &temp_data; */

#ifdef HAVE_BMP
        SensorData pres_data;
        bmp.getEvent(&event);
        pres_data.name = "Pres";
        pres_data.units = "hP";
        pres_data.value = event.pressure;
        /* node.data[2] = &pres_data; */
        transmit(build_msg(pres_data));
#endif  /* HAVE_BMP */

#ifdef HAVE_PRECIP
        SensorData precip_data;
        precip_data.name = "Prcp";
        precip_data.units = "NA";
        precip_data.value = 1023 - analogRead(PRECIP_PIN);
        /* node.data[3] = &precip_data;  */
        transmit(build_msg(precip_data));
#endif  /* HAVE_PRECIP */

#ifdef HAVE_PRCPMTR
        SensorData prcp_mtr;
        prcp_mtr.name = "PrcpMtr";
        prcp_mtr.units = "mm";
        prcp_mtr.value = PrcpMtrCount;
        transmit(build_msg(prcp_mtr));
        // Reset after we transmit.
        PrcpMtrCount = 0;
#endif

#ifdef HAVE_1WIRE_TEMP_SENSORS
        sensors.requestTemperatures();

        /* I happen to know I have 3 sensors. */
        for (i=0; i < 3; i++) {
                /* Yep, ugly; see https://www.e-tinkers.com/2020/01/do-you-know-arduino-sprintf-and-floating-point/ */
                msg = "{soil_temp_";
                /* Note: I'm marking the sensors with nail polish, and using
                   number of dots to show which sensor is which.  Zero dots won't
                   work, so I'm incrementing the display name by one. */
                msg += i + 1;
                msg += ": ";
                /* Why `byIndex`? Because you can have more than one DS18B20 on the same bus */
                msg += sensors.getTempCByIndex(i);
                msg += " C}";
                Serial.println(msg);
        }
#endif

        transmit(final_msg_string);
        Serial.println(final_msg_string);
        delay(SLEEPYTIME);
}
