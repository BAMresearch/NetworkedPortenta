// Digital.ino
#include <Arduino_MachineControl.h>
using namespace machinecontrol;
// #include "Settings.ino"

int nDO = 8; // total number of relays / digital output pins
int DOStartPin = 0; // pin number from which the relays are connected (i.e. for 4 relays, and start pin 5, relays are connected to pins 5, 6, 7, 8)

int nDI = 8; // total number of relays / digital output pins
int DIStartPin = 0; // pin number from which the relays are connected (i.e. for 4 relays, and start pin 5, relays are connected to pins 5, 6, 7, 8)

int nDIO = 12; // total number of digital I/O pins
int DIOStartPin = 0; // pin from which you want the series to start (as above)

int8_t DIOWrites[] = {
  IO_WRITE_CH_PIN_00,
  IO_WRITE_CH_PIN_01,
  IO_WRITE_CH_PIN_02,
  IO_WRITE_CH_PIN_03,
  IO_WRITE_CH_PIN_04,
  IO_WRITE_CH_PIN_05,
  IO_WRITE_CH_PIN_06,
  IO_WRITE_CH_PIN_07,
  IO_WRITE_CH_PIN_08,
  IO_WRITE_CH_PIN_09,
  IO_WRITE_CH_PIN_10,
  IO_WRITE_CH_PIN_11
};

int16_t DIOReads[] = {
  IO_READ_CH_PIN_00,
  IO_READ_CH_PIN_01,
  IO_READ_CH_PIN_02,
  IO_READ_CH_PIN_03,
  IO_READ_CH_PIN_04,
  IO_READ_CH_PIN_05,
  IO_READ_CH_PIN_06,
  IO_READ_CH_PIN_07,
  IO_READ_CH_PIN_08,
  IO_READ_CH_PIN_09,
  IO_READ_CH_PIN_10,
  IO_READ_CH_PIN_11
};

int16_t DIReads[] = {
  DIN_READ_CH_PIN_00,
  DIN_READ_CH_PIN_01,
  DIN_READ_CH_PIN_02,
  DIN_READ_CH_PIN_03,
  DIN_READ_CH_PIN_04,
  DIN_READ_CH_PIN_05,
  DIN_READ_CH_PIN_06,
  DIN_READ_CH_PIN_07,
};

bool setD(String IOType, int IONumber, int valueInt){
  // Serial.println("DSetter called");
  if (IOType=="DIO") {
    // make sure we are within range
    if (IONumber>11 || IONumber<0) return false;
    digital_programmables.set(DIOWrites[IONumber], valueInt);   // turn the LED on (HIGH is the voltage level)
  } else if (IOType=="DO" || IOType=="relay") {
    // make sure we are within range
    if (IONumber>7 || IONumber<0) return false;
    digital_outputs.set(IONumber, valueInt);
  } 
  return true; // if successful
}

int getD(String IOType, int IONumber){
  // Serial.println("DGetter called");
  if (IOType=="DIO") {
    // make sure we are within range
    if (IONumber>(DIOStartPin + nDIO) || IONumber<DIOStartPin) return false;
    int out = digital_programmables.read(DIOReads[IONumber]);   // turn the LED on (HIGH is the voltage level)
    return int(out);   // also for int/bool outputs
  } else if (IOType=="DO") {
    // make sure we are within range
    if (IONumber>(DOStartPin + nDO) || IONumber<DOStartPin) return false;
    int out = digital_outputs[IONumber].read();
    return int(out);   // also for int/bool outputs
  } else if (IOType=="DI") {
    // make sure we are within range
    if (IONumber>(DIStartPin + nDI) || IONumber<DIStartPin) return false;
    int out = digital_inputs.read(DIReads[IONumber]);
    return int(out);   // also for int/bool outputs
  } else { return (-1); }
}


void initDigitalIO() {
  // Initialize digital I/O pins
  // assuming the programmable I/O ports can also provide 0.5A output, we can use these. They have a write and read method.

  // if (!digital_programmables.init()) {
  //   Serial.println("GPIO expander initialization fail! ");
  // }
  // if (!digital_inputs.init()) {
  //   Serial.println("Digital input GPIO expander initialization fail!!");
  // }

  // //let's also use the digital_outputs...
  // digital_outputs.setLatch();
  // digital_programmables.setLatch();
  // Serial.println("GPIO expander initialization done ");

}
