ARDMK_VENDOR = archlinux-arduino

USER_LIB_PATH += ./lib
ARDUINO_LIBS = \
	SPI \
	SoftwareSerial \
	Wire \
	BusIO \
	INA219
OBJDIR = ./build

ARDUINO_PORT = /dev/ttyACM0
BOARD_TAG = uno

CFLAGS_STD = -std=gnu11 -fno-fat-lto-objects -flto -DDEBUG=1 -O1
CXXFLAGS_STD = -std=gnu++11 -fno-threadsafe-statics -flto -DDEBUG=1 -O1

include /usr/share/arduino/Arduino.mk
