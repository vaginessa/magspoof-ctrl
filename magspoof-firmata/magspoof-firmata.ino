#include <Firmata.h>

void stringCallback(char *myString) {
  Firmata.sendString(myString);
}


void sysexCallback(byte command, byte argc, byte *argv) {
  Firmata.sendSysex(command, argc, argv);
}

void setup() {
  Firmata.setFirmwareVersion(FIRMATA_MAJOR_VERSION, FIRMATA_MINOR_VERSION);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.begin(57600);
}

void loop() {
  while (Firmata.available()) {
    Firmata.processInput();
  }
}


