// #include "Settings.ino"
// Analog input settings
  enum AnalogInputType {
    Voltage_0_10V, // 0-10V
    Current_4_20mA, // 4-20mA
    NTC // NTC thermistor
  };

  // For now, we fix is our inputType:
  AnalogInputType AIType = Voltage_0_10V;

  int nAI = 3; // total number of analog input pins
  int AIStartPin = 0; // pin from which you want to start the series

  int nAO = 4; // total number of analog outs
  int AOStartPin = 0; // pin from which you want to start the series


// for processing the analog inputs in 0-10V mode. 
float res_divider = 0.28057;
float reference = 3.3;
// and in 4-20mA mode: 
#define SENSE_RES 120
// and NTC mode: 
#define REFERENCE_RES 100000
float lowest_voltage = 2.7;

// Analog.ino
void initAnalogIO() {
  // Initialize analog I/O pins
    // set the analog inputs: 
  analogReadResolution(16);
  switch (AIType) {
    case Voltage_0_10V:
      Serial.println("Setting AI to 0-10V input");
      analog_in.set0_10V();
      break;
    case Current_4_20mA:
      Serial.println("Setting AI to 4-20mA input");
      analog_in.set4_20mA();
      break; 
    case NTC:
      Serial.println("Setting AI to NTC input");
      analog_in.setNTC();
      break;
  }    

  // initialize analog out:
  for (int thisPin = AOStartPin; thisPin < AOStartPin + nAO; thisPin++) {
    analog_out.period_ms(thisPin, 4); // 4 ms cycle time 
  }
}

float getA(int pin) {
  Serial.println("getA called for pin "+String(pin));
  if (pin>(AIStartPin + nAI) || pin<AIStartPin) return -1.0;

  int rawValue = analogRead(pin);
  float processedValue = 0.0;

  switch (AIType) {
    case Voltage_0_10V:
      processedValue = (rawValue * reference) / 65535 / res_divider;
      return processedValue;
    case Current_4_20mA:
      processedValue = map(rawValue, 0, 1023, 4, 20);
      Serial.print("Current (4-20mA): ");
      Serial.println(processedValue);
      break;
    case NTC:
      // Assuming a specific NTC thermistor calculation
      // This will need to be replaced with the actual calculation for your NTC thermistor
      float voltage_ch0 = (rawValue * reference) / 65535;
      float resistance_ch0;
      if (voltage_ch0 < lowest_voltage) {
        resistance_ch0 = ((-REFERENCE_RES) * voltage_ch0) / (voltage_ch0 - reference);
      } else {
        resistance_ch0 = -1;
      }
      return resistance_ch0;
  }
}

bool setA(int IONumber, float value){
  Serial.println("setA called for pin: " + String(IONumber) + " value: " + String(value));
  if ((IONumber>(AOStartPin + nAO) || IONumber<AOStartPin)) return false;
  if (value<0 || value>10.5) return false; // maximum 10.5V output
  analog_out.write(IONumber, value);
  return true; // if successful
}