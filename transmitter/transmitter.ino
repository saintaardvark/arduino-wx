#include <RFTransmitter.h>
#include <Wire.h>
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
#endif

// Humidity sensor
#include "DHT.h"

// Copied from RFReceiver.h
const byte MAX_PAYLOAD_SIZE = 80;
char VWMsgBuf[MAX_PAYLOAD_SIZE];
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

// Send on digital pin 11 and identify as node 1
/* Try setting delay to longer... */
#define PULSE_LENGTH     500
#define BACKOFFDELAY    1000
RFTransmitter transmitter(TRANSMITTER_PIN, NODE_ID, PULSE_LENGTH, BACKOFFDELAY);

float humid;
float temp;
int precip;

void flashyflashy() {
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
}

// Thanks, bblanchon!
// https://github.com/bblanchon/ArduinoJson/wiki/FAQ#whats-the-best-way-to-use-the-library

// Thanks, linhartr22!
// https://github.com/linhartr22/433_MHz_Wireless_TX-RX_Demo/blob/master/TX_Temp_Test/TX_Temp_Test.ino#L80-L88
void VWTX(String VWMsgStr) {
        VWMsgStr.toCharArray(VWMsgBuf, MAX_PAYLOAD_SIZE);
        uint8_t VWMsgBufLen = strlen(VWMsgBuf);
        digitalWrite(13, true); // Flash a light to show transmitting
        transmitter.send((byte *)VWMsgBuf, VWMsgBufLen);
        digitalWrite(13, false); // Flash a light to show transmitting
        delay(1000);
        digitalWrite(13, true); // Flash a light to show transmitting
        transmitter.resend((byte *)VWMsgBuf, VWMsgBufLen);
        digitalWrite(13, false); // Flash a light to show transmitting
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        Serial.println("Reboot");
        Serial.println("dht.begin");
        dht.begin();

        /* Initialise the sensor */

#ifdef HAVE_BMP
        Serial.println("Pressure Sensor Test");
        Serial.println("");
        if(!bmp.begin()) {
                /* There was a problem detecting the BMP085 ... check your connections */
                Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
                while(1);
        }
#endif
        Serial.println("Node ID: " + String(NODE_ID));
}

void loop() {

        humid = dht.readHumidity();
        temp = dht.readTemperature();

#ifdef HAVE_BMP
        bmp.getEvent(&event);
        precip = 1023 - analogRead(PRECIP_PIN);
        SensorData pres_data;
        SensorData precip_data;
#endif

        // Doesn't seem to be an easy way to get the NODE_ID out on
        // the receiving end...
        /* final_msg_string = "Node: " + String(NODE_ID) + " , "; */
        /* final_msg_string += "Temp: " + String(temp) + " C , "; */

        final_msg_string = "{";
        final_msg_string += "Tmp " + String(temp) + " C,";
#ifdef HAVE_BMP
        final_msg_string += "Prs " + String(event.pressure) + " hP,";
        /* AU == arbitrary units */
        final_msg_string += "Prc " + String(precip) + " AU,";
#endif
        final_msg_string += "Hmd " + String(humid) + " %,";
        final_msg_string += "}";


        VWTX(final_msg_string);
        Serial.println(final_msg_string);
        delay(SLEEPYTIME);
}
