ESP_ADDR=esp32cam
# default anyway: ESP_PORT=3232

#FIXME: if BOARD = esp32cam, we get a compile error:
#/home/tconnors/.arduino15/packages/esp32/hardware/esp32/1.0.6/cores/esp32/esp32-hal-misc.c:199:29: error: expected expression before '/' token
#     setCpuFrequencyMhz(F_CPU/1000000);
# We sort of hack around that by defining -DF_CPU=240000000L manually below:

#BOARD = lolin32
BOARD = esp32cam
# ttgo-lora32-v1
# esp32
CHIP = esp32
# on the lolin32 board (but not my other esp32 boards), flashing at full 921600 doesn't start - with communication error.  likely hardware fix is this, or just upload slower: https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header/
UPLOAD_SPEED = 460800

# EXCLUDE_DIRS=$(wildcard $(ARDUINO_LIBS)/*/tests) $(ARDUINO_LIBS)/ESP8266SdFat $(ARDUINO_LIBS)/SDFS
EXCLUDE_DIRS=XXXXXXXXXXXXXXXX

include $(HOME)/Arduino/template/Makefile
BUILD_EXTRA_FLAGS:=$(BUILD_EXTRA_FLAGS) -DF_CPU=240000000L

UPLOAD_PORT = /dev/ttyUSB0
.DEFAULT_GOAL := ota
#.DEFAULT_GOAL := flash
