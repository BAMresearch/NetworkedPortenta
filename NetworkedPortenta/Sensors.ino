// #include "Settings.ino"

// Sensors.ino
#include <Adafruit_Sensor.h> // sensor libs
#include "Adafruit_BME680.h"

// Temperature bank settings: 
enum TBankInputType {
  PT100, // PT 100 sensor, can be used in two-or three-wire mode
  PT1000, // PT 1000 sensor, can be used in two-or three-wire mode
  K, // K-type thermocouple, two-wire
  J, // J-type thermocouple, two-wire
};

bool useBME680 = false;
// For now, we fix is our inputType to PT100:
TBankInputType TType = PT1000;
// guessing at the type here: 
int8_t TWire = THREE_WIRE; // in three-wire mode (as opposed to two-wire), only an option for RTDs, not thermocouples
float RREF = 430.0; // PT100 setting, will be updated in case PT1000 is selected. Documentation says 400 ohm, maybe..
float RNOMINAL = 100.0; // PT100 setting, will be updated in case PT1000 is selected

int nTBank = 3; // number of inputs on the temperature bank
int nTBankStartPin = 0; // start pin for the temperature bank

float T = 0; // Temperature HTU // , 3 = DS3231
float H = 0; // humidity from HTU, in rel.%
float P = 0; // pressure in hPa (mbar)
float G = 0; // Gas resistance (Ohm).
float dp = 0; // dewpoint

Adafruit_BME680 bme; // I2C

void initTemp(){
  if ((TType==K)||(TType==J)) {
    // Initialize temperature probes
    temp_probes.tc.begin();
    Serial.println("Temperature probes initialization done");
    // Enables Thermocouples chip select
    temp_probes.enableTC();
    Serial.println("Thermocouples enabled");
    return; // nothing more to do here
  }
  if (TType==PT1000) {
    RREF *= 10.0; // update for PT1000
    RNOMINAL *= 10.0; // update for PT1000
  }
  temp_probes.rtd.begin(TWire); // two or three wire
  temp_probes.enableRTD(); // for RTD sensors
  return;
}

void initBME() {
  if (!useBME680) {
    return; // nothing to do here. 
  }
  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid humidity sensor, check wiring!"));
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loopBME() {
  if (!useBME680) {
    return; // nothing to do here. 
  }
  if (! bme.performReading()) {
    Serial.println("Failed to perform BME680 reading :(");
    return;
  }
  T = bme.temperature; //C
  P = bme.pressure / 100.0; //hPa
  H = bme.humidity; // percent?
  G = bme.gas_resistance; // Ohms
  calcDewpoint();
  // Serial.println(String(T) + " deg C; " + String(P) + " mbar; " + String(H) + " %; " + String(G) + " AU; " + String(dp) + " deg C ");
}

float getEnvSensor(String Sensor) {
  if (!useBME680) {
    return (-1); // nothing to do here. 
  }
  loopBME();
  calcDewpoint();
  Sensor.toLowerCase(); // toLowerCase does inplace replacement, unlike toInt or toFloat.. 
  if (Sensor=="t" || Sensor=="temp"|| Sensor=="temperature") {
    return T;
  } else if (Sensor=="h" || Sensor=="humidity") {
    return H;
  } else if (Sensor=="p" || Sensor=="pressure") {
    return P;
  } else if (Sensor=="g" || Sensor=="gas"|| Sensor=="gasresistance") {
    return G;
  } else if (Sensor=="dp" || Sensor=="dewpoint") {
    return dp;
  } else { return (-1); }
}

float getSensorValue(String SensorBank, String Sensor) {
  Serial.println("getSensorValue called for Bank: " + SensorBank + " Sensor: " + Sensor);
  SensorBank.toLowerCase(); // toLowerCase does inplace replacement, unlike toInt or toFloat.. 
  if ((SensorBank == "env") || (SensorBank == "evironmental")) {
    return getEnvSensor(Sensor);
  }
  else if ((SensorBank == "temp") || (SensorBank == "temperature") || (SensorBank == "temperatureprobes")) {// on the PMC we have four inputs for temperature probes
    return getTempSensor(Sensor);
  }
  // add more sensor bank options here if you have them.
  else {return (-1); }  
}

float getTempSensor(String Sensor){
  float temperature = -999;
  int IONumber = Sensor.toInt();
  if ((IONumber>(AOStartPin + nAO) || IONumber<AOStartPin)) return -999;
  if ((TType==PT1000)||(TType==PT100)) {
    temp_probes.selectChannel(IONumber); // channel select has 150 ms delay
    uint16_t rtd = temp_probes.rtd.readRTD();
    Serial.println("Read RTD reading for sensor " + String(Sensor) + ": " + String(rtd));
    float ratio = rtd;
    ratio /= 32768;

    // Check and print any faults
    uint8_t fault = temp_probes.rtd.readFault();
    if (fault) {
      RTDCheckFault(fault); // we don't do anything with this at the moment. 
      return -998;
    } else {
      Serial.print("RTD value: "); Serial.println(rtd);
      Serial.print("Ratio = "); Serial.println(ratio, 8);
      Serial.print("Resistance = "); Serial.println(RREF * ratio, 8);
      temperature = temp_probes.rtd.readTemperature(RNOMINAL, RREF);
    }
  }
  else if ((TType==K)||(TType==J)) {
    temp_probes.selectChannel(IONumber); // channel select has 150 ms delay
    Serial.println("Getting TC reading for sensor " + String (Sensor));
    temperature = temp_probes.tc.readTemperature();
  }
  Serial.println("Temperature [°C] = " + String(temperature));
  return temperature;
}

void RTDCheckFault(uint8_t fault) {
  Serial.print("Fault 0x"); Serial.println(fault, HEX);
  if (temp_probes.rtd.getHighThresholdFault(fault)) {
    Serial.println("RTD High Threshold");
  }
  if (temp_probes.rtd.getLowThresholdFault(fault)) {
    Serial.println("RTD Low Threshold");
  }
  if (temp_probes.rtd.getLowREFINFault(fault)) {
    Serial.println("REFIN- > 0.85 x Bias");
  }
  if (temp_probes.rtd.getHighREFINFault(fault)) {
    Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
  }
  if (temp_probes.rtd.getLowRTDINFault(fault)) {
    Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
  }
  if (temp_probes.rtd.getVoltageFault(fault)) {
    Serial.println("Under/Over voltage");
  }
  temp_probes.rtd.clearFault();
  return; 
}

void initSensors() {
  // Initialize BME680 and other sensors
  initBME();
  initTemp();
}

void updateSensors() {
  // Read sensor data and process it. Not sure we need to do this at regular intervals.. 
  loopBME();
}