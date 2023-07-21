/* -*- c-basic-offset:8; indent-tabs-mode:nil -*- */

#include <SPI.h>
#include <Adafruit_Sensor.h>

/*
  The Arduino Makefile does not support multiple ino
  files (https://github.com/sudar/Arduino-Makefile/issues/49), and I'm
  not familiar enough with C/C++ to break things out into separate
  c/cpp/header files.  For now, I'm sticking with this.
*/

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

/*

  For a list of pins in use, see README.md
  Note: only pins 2 and 3 support interrupts.

*/
#define MAX_PAYLOAD_LEN 66

/*
******************
BMP sensor begin
******************
*/

#define HAVE_BMP 1

# ifdef HAVE_BMP
// Barometer / Temp
// Example code from https://learn.adafruit.com/bmp085?view=all
// Thanks, AdaFruit!
#include <Adafruit_BMP085_U.h>
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
sensors_event_t event;
#endif  /* HAVE_BMP */

/*
**************
BMP sensor end
**************
*/

/*
**************************
Precipitation sensor begin
**************************
*/

/* Uncomment if you have precip sensor */
#define HAVE_PRECIP 1
#define PRECIP_PIN       A0

/*
************************
Precipitation sensor end
************************
*/

/*
*************************
Precipitation meter begin
*************************
*/


/* Uncomment if you have a precipitation *meter* */
#define HAVE_PRCPMTR 1

#ifdef HAVE_PRCPMTR
/* For Unos, only pins 2 and 3 can do interrupts */
#define PRCPMTR_PIN 3
int prcpPushCounter = 0;

unsigned long lastPrcpMtrDebounceTime = 0;  // the last time the output pin was toggled
unsigned long prcpMtrDebounceDelay = 200;   // the debounce time; increase if the output flickers
volatile long PrcpMtrCount = 0;
void PrcpMtrISR() {
        if ((millis() - lastPrcpMtrDebounceTime) > prcpMtrDebounceDelay) {
                lastPrcpMtrDebounceTime = millis();
                PrcpMtrCount++;
        }
}

#endif

/*
***********************
Precipitation meter end
***********************
*/

/*
******************
1wire sensor begin
******************
*/

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
volatile float soil_temp;
volatile short num_onewire_devices = 0;

/* https://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html */
void discoverOneWireDevices(void) {
        byte i;
        /* byte data[12]; */
        byte addr[8];

        Serial.print(F("Looking for 1-Wire devices...\n\r"));
        while(oneWire.search(addr)) {
                num_onewire_devices++;
                Serial.print(F("\n\rFound \'1-Wire\' device with address:\n\r"));
                for( i = 0; i < 8; i++) {
                        Serial.print(F("0x"));
                        if (addr[i] < 16) {
                                Serial.print(F("0"));
                        }
                        Serial.print(addr[i], HEX);
                        if (i < 7) {
                                Serial.print(F(", "));
                        }
                }
                if ( OneWire::crc8( addr, 7) != addr[7]) {
                        Serial.println(F("CRC is not valid!"));
                        return;
                }
        }
        Serial.print(F("Final count: "));
        Serial.println(num_onewire_devices);
        oneWire.reset_search();
        return;
}
#endif  /* HAVE_1WIRE_TEMP_SENSORS */

/*
****************
1wire sensor end
****************
*/

/*
****************
DHT sensor begin
****************
*/

// Humidity sensor
#include "DHT.h"

byte addresses[][6] = {"1Node","2Node"};

String final_msg_string;

#define DHTPIN 5     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

/*
**************
DHT sensor end
**************
*/

/*
***********************
Anemometer sensor begin
***********************
*/

#define HAVE_ANEMOMETER 1

#ifdef HAVE_ANEMOMETER

#define HALLPIN 2

volatile float lastAnemometerDebounceTime = 0.0;  // the last time the output pin was toggled
float anemometerDebounceDelay = 200.0;   // the debounce time; increase if the output flickers
volatile float avgAnemometerTime = 0.0;

volatile float anemometerTriggerCount = 0.0;
volatile float anemometerLastDetectorCheckTime = 0.0;
volatile float anemometerRPM = 0.0;
volatile float anemometerElapsedTimeSinceLastInterrupt = 0.0;

void anemometerISR() {
        anemometerElapsedTimeSinceLastInterrupt = (float)millis() - lastAnemometerDebounceTime;
        if (anemometerElapsedTimeSinceLastInterrupt > anemometerDebounceDelay) {
                lastAnemometerDebounceTime = millis();
                anemometerTriggerCount++;
                /* anemometerDetectorTime += anemometerElapsedTimeSinceLastInterrupt; */
        }
}

#endif

/*
*********************
Anemometer sensor end
*********************
*/



#define LEDPIN           13
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

void init_dht() {
        dht.begin();
        Serial.println(F("DHT sensor initialized!"));
}

void init_prcpmtr() {
#ifdef HAVE_PRCPMTR
        pinMode(PRCPMTR_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(PRCPMTR_PIN), PrcpMtrISR, FALLING);
        Serial.println(F("Precipitation meter initialized!"));
#endif
}

void init_bmp() {
#ifdef HAVE_BMP
        Serial.println(F("Pressure Sensor Test..."));
        if(!bmp.begin()) {
                /* There was a problem detecting the BMP085 ... check your connections */
                Serial.print(F("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!"));
        }
        Serial.println(F("BMP sensor initialized!"));
#endif  /* HAVE_BMP */
}

void init_precip() {
#ifdef HAVE_PRECIP
        /* Try the pullup pins */
        pinMode(PRECIP_PIN, INPUT_PULLUP);
        Serial.println(F("Precipitation detector initialized!"));
#endif
}

void init_1wire_temp() {
#ifdef HAVE_1WIRE_TEMP_SENSORS
        sensors.begin();
        discoverOneWireDevices();
        Serial.println(F("1wire temp sensors initalized!"));
#endif  /* HAVE_1WIRE_TEMP_SENSORS */
}

void init_anemometer() {
#ifdef HAVE_ANEMOMETER
        pinMode(HALLPIN, INPUT);
        attachInterrupt(digitalPinToInterrupt(HALLPIN), anemometerISR, FALLING);
        Serial.println(F("Anemometer initialized!"));
#endif
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        Serial.println(F("Reboot"));

        init_dht();
        init_prcpmtr();
        init_bmp();
        init_precip();
        init_1wire_temp();
        init_anemometer();

        /* Finally, ready to go! */
        Serial.println("Node ID: " + String(NODE_ID));
        Serial.println(F("LET'S DO THIS!"));
        Serial.println(F("---"));
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

void read_and_log_humidity() {
        SensorData humid_data;
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
}

void read_and_log_temp() {
        SensorData temp_data;
        /*
          You'd *think* you could just assign the result of
          dht.readHumidity() directly to humid_data.value.  Turns out
          that's not the case; if you do that, you just get 0 in
          there.  Same applies to dht.readTemperature.
        */
        temp = dht.readTemperature();
        temp_data.name = "Temp";
        temp_data.units = "C";
        temp_data.value = dht.readTemperature();
        transmit(build_msg(temp_data));
}

void read_and_log_bmp() {
#ifdef HAVE_BMP
        SensorData pres_data;
        bmp.getEvent(&event);
        pres_data.name = "Pres";
        pres_data.units = "hP";
        pres_data.value = event.pressure;
        /* node.data[2] = &pres_data; */
        transmit(build_msg(pres_data));

        /*
          "A man with one watch knows what time it is.  A man with two
          watches is never sure."

          https://en.wikipedia.org/wiki/Segal%27s_law
        */
        Sensordata bmp_temp;
        bmp_temp.name = "BMPTemp";
        bmp_temp.units = "C";
        float bmp_temp;
        bmp.getTemperature(&bmp_temp);
        bmp_temp.value = bmp_temp;
        transmit(build_msg(bmp_temp));
#endif  /* HAVE_BMP */
}

void read_and_log_prcp() {
#ifdef HAVE_PRECIP
        SensorData precip_data;
        precip_data.name = "Prcp";
        precip_data.units = "NA";
        precip_data.value = 1023 - analogRead(PRECIP_PIN);
        /* node.data[3] = &precip_data;  */
        transmit(build_msg(precip_data));
#endif  /* HAVE_PRECIP */
}

void read_and_log_prcpmtr() {
#ifdef HAVE_PRCPMTR
        SensorData prcp_mtr;
        prcp_mtr.name = "PrcpMtr";
        prcp_mtr.units = "mm";
        prcp_mtr.value = PrcpMtrCount;
        transmit(build_msg(prcp_mtr));
        // Reset after we transmit.
        PrcpMtrCount = 0;
#endif
}

void read_and_log_soiltemp() {
#ifdef HAVE_1WIRE_TEMP_SENSORS
        sensors.requestTemperatures();

        for (i=0; i < num_onewire_devices; i++) {
                /* Yep, ugly; see https://www.e-tinkers.com/2020/01/do-you-know-arduino-sprintf-and-floating-point/ */
                msg = "{soil_temp_";
                /* Note: I'm marking the sensors with nail polish, and using
                   number of dots to show which sensor is which.  Zero dots won't
                   work, so I'm incrementing the display name by one. */
                msg += i + 1;
                msg += ": ";
                /* Why `byIndex`? Because you can have more than one DS18B20 on the same bus */
                soil_temp = sensors.getTempCByIndex(i);
                if (int(soil_temp) == -127) {
                        msg += "NAN";
                } else {
                        msg += soil_temp;
                }
                msg += " C}";
                Serial.println(msg);
        }
#endif
}

void read_and_log_anemometer() {
#ifdef HAVE_ANEMOMETER
        SensorData anemometer_rpm;
        anemometer_rpm.name = "AnemometerRPM";
        anemometer_rpm.units = "rpm";
        /* Set value to 0 right away -- otherwise, if nothing happens
           & we don't recalculate, we end up retransmitting the value
           calculated from last time. */
        anemometer_rpm.value = 0.0;

        SensorData anemometer_rot_count;
        anemometer_rot_count.name = "AnemometerRotCount";
        anemometer_rot_count.units = "rot";
        anemometer_rot_count.value = 0.0;

        // Including this mainly for debugging
        SensorData anemometer_trigger_count;
        anemometer_trigger_count.name = "AnemometerTriggerCount";
        anemometer_trigger_count.units = "tr";
        anemometer_trigger_count.value = anemometerTriggerCount;

        if (anemometerTriggerCount > 0) {
                /* Each trigger represents 1/3rd of a rotation */
                anemometer_rot_count.value = anemometerTriggerCount / 3.0;
                /* avgAnemometerTime is time for 1/3rd of a rotation */
                avgAnemometerTime = ((float)millis() - anemometerLastDetectorCheckTime) / anemometerTriggerCount;

                /* millis() can wrap around, so make sure our answer above is positive */
                if (avgAnemometerTime > 0) {
                        /*
                          Math:

                          - we have three arms, and there's a magnet on each arm

                          - avgAnemometerTime is the time in msec between
                          triggers (ie, when we see a magnetic field go by)

                          - "time between detections" is time for *one-third*
                          of a rotation, because we have magnets on three
                          arms

                          - so average time *per rotation* is avgAnemometerTime * 3

                          - one minute = 60 s * 1000 ms/s

                          - revolutions per minute therefore is one minute /
                          (3 * avgAnemometerTime)
                        */
                        anemometerRPM = (60.0 * 1000.0) / (3.0 * avgAnemometerTime);
                        anemometer_rpm.value = anemometerRPM;
                }
        }

        transmit(build_msg(anemometer_rpm));
        transmit(build_msg(anemometer_rot_count));
        transmit(build_msg(anemometer_trigger_count));

        // Reset after we transmit.
        anemometerTriggerCount = 0.0;
        anemometerLastDetectorCheckTime = (float) millis();
        anemometerRPM = 0.0;
#endif
}

void loop() {
        read_and_log_humidity();
        read_and_log_temp();
        read_and_log_bmp();
        read_and_log_prcp();
        read_and_log_prcpmtr();
        read_and_log_soiltemp();
        read_and_log_anemometer();
        Serial.println();
        delay(SLEEPYTIME);
}
