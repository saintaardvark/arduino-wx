#include <RFTransmitter.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

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
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

#define TRANSMITTER_PIN  11
#define LEDPIN           13
#define PRECIP_PIN       A0

// Needed for JSON serialization
// Not sure why -- but MAX_SENSORS needs to be number of sensors + 1.
#define MAX_SENSORS 3
#define NODEDATA_JSON_SIZE (JSON_OBJECT_SIZE(1 + MAX_SENSORS * 3))

struct SensorData {
        const char* name;
        const char* units;
        float value;
};

struct NodeData {
        const char* name;
        SensorData *data[MAX_SENSORS];
};

#define NODE_ID          1
NodeData node;

#define SLEEPYTIME 10000

#define DEBUGGING 1

// Send on digital pin 11 and identify as node 1
RFTransmitter transmitter(TRANSMITTER_PIN, NODE_ID);

float humid;
float temp;
int precip;

int counter = 0;

void flashyflashy() {
        digitalWrite(LEDPIN, HIGH);
        delay(125);
        digitalWrite(LEDPIN, LOW);
        delay(125);
}

// Thanks, bblanchon!
// https://github.com/bblanchon/ArduinoJson/wiki/FAQ#whats-the-best-way-to-use-the-library
void serialize(const NodeData& node, char* json, size_t maxSize) {
        StaticJsonBuffer<NODEDATA_JSON_SIZE> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["name"] = node.name;
        JsonArray& data = root.createNestedArray("data");
        int i;
        for (i=0; i < MAX_SENSORS; i++) {
                JsonObject& measurement = data.createNestedObject();
                measurement["name"] = node.data[i]->name;
                measurement["value"] = node.data[i]->value;
                measurement["units"] = node.data[i]->units;
#ifdef DEBUGGING
                Serial.println("node.data[" + String(i) + "]");
                Serial.println("Name: " + String(node.data[i]->name));
                Serial.println("Value: " + String(node.data[i]->value));
                Serial.println("Units: " + String(node.data[i]->units));
#endif
        }
        root.printTo(json, maxSize);
}

// Thanks, linhartr22!
// https://github.com/linhartr22/433_MHz_Wireless_TX-RX_Demo/blob/master/TX_Temp_Test/TX_Temp_Test.ino#L80-L88
void VWTX(String VWMsgStr) {
        VWMsgStr.toCharArray(VWMsgBuf, MAX_PAYLOAD_SIZE);
        uint8_t VWMsgBufLen = strlen(VWMsgBuf);
        digitalWrite(13, true); // Flash a light to show transmitting
        transmitter.send((byte *)VWMsgBuf, VWMsgBufLen);
        digitalWrite(13, false); // Flash a light to show transmitting
}

void setup() {
        Serial.begin(9600);
        pinMode(LEDPIN, OUTPUT);
        Serial.println("Reboot");
        Serial.println("dht.begin");
        dht.begin();

        /* Initialise the sensor */

        Serial.println("Pressure Sensor Test");
        Serial.println("");
        if(!bmp.begin()) {
                /* There was a problem detecting the BMP085 ... check your connections */
                Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
                while(1);
        }
        node.name = (char*) NODE_ID;
}

void loop() {
        humid = dht.readHumidity();
        temp = dht.readTemperature();
        bmp.getEvent(&event);
        precip = 1023 - analogRead(PRECIP_PIN);

        SensorData humid_data;
        SensorData temp_data;
        SensorData pres_data;
        SensorData precip_data;

        humid_data.name = "humid";
        humid_data.units = "%";
        humid_data.value = humid;

        temp_data.name = "temp";
        temp_data.units = "C";
        temp_data.value = temp;

        pres_data.name = "pressure";
        pres_data.units = "hPA";
        pres_data.value = event.pressure;

        precip_data.name = "precip";
        precip_data.units = "none";
        precip_data.value = precip;

        node.data[0] = &humid_data;
        node.data[1] = &temp_data;
        node.data[2] = &pres_data;
        node.data[3] = &precip_data;

        char json_for_serial[NODEDATA_JSON_SIZE];
        serialize(node, json_for_serial, NODEDATA_JSON_SIZE);
        // Doesn't seem to be an easy way to get the NODE_ID out on
        // the receiving end...
        final_msg_string = "Node: " + String(NODE_ID) + " , ";
        final_msg_string += "Temp: " + String(temp) + " C , ";
        final_msg_string += "Pres: " + String(event.pressure) + " hPA , ";
        final_msg_string += "Precip: " + String(precip) + " , ";
        final_msg_string += "Humid: " + String(humid) + " %|";

        VWTX(final_msg_string);

        /* Serial.println(final_msg_string); */
        /* Serial.print("Message length: "); */
        /* Serial.println(final_msg_string.length()); */
        /* Serial.print("Loop #"); */
        /* Serial.println(counter); */

        Serial.println(json_for_serial);
        counter++;
        delay(SLEEPYTIME);
}
