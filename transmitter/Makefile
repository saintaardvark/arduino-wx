# BOARD_TAG    = mega
# BOARD_SUB    = atmega2560
MONITOR_PORT = /dev/ttyUSB0

ifeq ($(USER), aardvark)
$(info I see user aardvark, adjusting accordingly)
ARDUINO_DIR  = /home/aardvark/arduino-1.8.12
ARDMK_DIR     = /home/aardvark/dev/arduino/Arduino-Makefile
else ifeq ($(USER), pi)
$(info I see user pi, adjusting accordingly)
ARDUINO_DIR = /home/pi/arduino-1.8.2
ARDMK_DIR   = /home/pi/external_src/Arduino-Makefile
MONITOR_PORT = /dev/weatherstation
endif
BOARD_TAG    = uno
ARDUINO_LIBS =  DHT-sensor-library-master \
		Adafruit_BMP085_Unified-master \
		Wire \
		Adafruit_Sensor-master \
		RF24-master \
		SPI \
		OneWire-master \
		Arduino-Temperature-Control-Library-master

include $(ARDMK_DIR)/Arduino.mk

# Needed for Arduino-JSON; see
# http://stackoverflow.com/questions/38779413/arduinojson-undefined-reference-to-cxa-guard-acquire
# and https://github.com/bblanchon/ArduinoJson/issues/356 and
# https://github.com/sudar/Arduino-Makefile/blob/master/arduino-mk-vars.md
CXXFLAGS += -fno-threadsafe-statics

ifeq ($(USER), pi)
include ../Arduino.mk
endif
