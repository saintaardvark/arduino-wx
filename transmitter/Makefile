# BOARD_TAG    = mega
# BOARD_SUB    = atmega2560
# MONITOR_PORT = /dev/ttyACM0
ARDUINO_DIR  = /home/aardvark/arduino-1.8.12
BOARD_TAG    = uno
MONITOR_PORT = /dev/ttyACM1
ARDUINO_LIBS =  DHT-sensor-library-master Adafruit_BMP085_Unified-master Wire Adafruit_Sensor-master RF24-master SPI
ARDMK_DIR     = /home/aardvark/dev/arduino/Arduino-Makefile

include $(ARDMK_DIR)/Arduino.mk

# Needed for Arduino-JSON; see
# http://stackoverflow.com/questions/38779413/arduinojson-undefined-reference-to-cxa-guard-acquire
# and https://github.com/bblanchon/ArduinoJson/issues/356 and
# https://github.com/sudar/Arduino-Makefile/blob/master/arduino-mk-vars.md
CXXFLAGS += -fno-threadsafe-statics

include ../Arduino.mk
