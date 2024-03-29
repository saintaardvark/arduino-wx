# Requirements

- [Arduino-makefile][0]
- python-serial
- avrdude
- [RFReceiver][1] and [RFTransmitter][2] libraries
- [PinChangeInterruptHandler library][4]
- Adafruit's [DHT Sensor library][3], [BMP085_Unified library][7] and [Unified Sensor Driver][5]
- [Benoit Blanchon's Arduino JSON library][6]
- [RF24][8]

- [Arduino Temperature Control Library][9]
- [OneWire][10] (though I should look at https://github.com/pstolarz/OneWireNg)

# Building

```
make upload
```

# Measurements

- Temp & humidity come from the DHT22
- Air pressure comes from the BMP85


# Diagram

List of pins in use:


- Precipitation Meter: 3 (PRCPMTR_PIN 3)
- Onewire bus: 4 (ONE_WIRE_BUS)
- Humidity/temp pin: 5 (DHTPIN)
- Transmitter: Pin 11

- Precipitation detector: A0 (PRECIP_PIN)
- Barometer: A4 & A5 (SDA, SCL)

**Note: only pins 2 and 3 support interrupts.**

# Temp sensor

 white: pos
 grey: data
 mauve: negative

[0]: https://github.com/sudar/Arduino-Makefile.git
[1]: https://github.com/zeitgeist87/RFReceiver
[2]: https://github.com/zeitgeist87/RFTransmitter
[3]: https://github.com/adafruit/DHT-sensor-library
[4]: https://github.com/zeitgeist87/PinChangeInterruptHandler
[5]: https://github.com/adafruit/Adafruit_Sensor
[6]: https://github.com/bblanchon/ArduinoJson
[7]: https://github.com/adafruit/Adafruit_BMP085_Unified
[8]: https://github.com/nRF24/RF24
[9]: https://github.com/milesburton/Arduino-Temperature-Control-Library
[10]: https://github.com/PaulStoffregen/OneWire
