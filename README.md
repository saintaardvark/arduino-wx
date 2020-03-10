# Requirements

- [Arduino-makefile][0]
- python-serial
- avrdude
- [RFReceiver][1] and [RFTransmitter][2] libraries
- [PinChangeInterruptHandler library][4]
- Adafruit's [DHT Sensor library][3], [BMP085_Unified library][7] and [Unified Sensor Driver][5]
- [Benoit Blanchon's Arduino JSON library][6]
- [RF24][8]

# Building

```
make upload
```

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
