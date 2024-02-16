// Network module for the MOUSE and the new robot hardware.
// Makes the digital and analog ports readable and (for output ports) writeable over the network. 
// Also reads environment variables (pressure, temperature, humidity) through connected i2c BME/BMP280 chip
// IP address by default gets from DHCP server, but can be hardcoded
// todo: find something for the on-board temp connection readouts
// todo: find something for the on-board CAN and RS485 buses
// todo: find something to read out separate nicla sense boards.
// todo: enable logging of env and inputs to on-board memory
// todo: enable NTP-supplied time-stamping

// Future upgrade options could include using a json config file on the SD card reader
// integrated on the arduino network shield.
// Additionally, other pins could be exposed for both input and output, digital and analog as required

// Checks:
// Check ethernet functionality
// Check DI write and read
// Check DO write and read
// Check DIO write and read
// Check AI write and read
// Check AO write and read
// Check TEMP bank write and read with 3-wire RTDs
// Check TEMP bank write and read with 2-wire TCs
// Check ENV sensor bank read

// // // // // // Load libraries // // // // // //

#include <Arduino_MachineControl.h>
using namespace machinecontrol;

#include <SPI.h>
#include <Wire.h> // I2C
#include <Ethernet.h> // for ethernet

// No need for these includes as the Arduino automatically includes all other ino files in the folder
// #include "Networking.ino"
// #include "Digital.ino"
// #include "Analog.ino"
// #include "Sensors.ino"
// #include "Utilities.ino"

void setup() {
  Serial.begin(115200);
  // while (!Serial); // wait for serial port to connect. Needed for native USB port only

  // Load configuration
  // readConfig();
  // useDHCP = configDoc["network"]["useDHCP"];

  initNetworking();
  initSensors();
  initDigitalIO();
  initAnalogIO();
}

void loop() {
  listenForSocketClients();
  listenForSerialCommands();
  updateSensors();
  // ethernetKeepalive();
}