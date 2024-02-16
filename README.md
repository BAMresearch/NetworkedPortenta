# NetworkedPortenta
Arduino IDE code to expose the I/O of an Arduino Portenta Machine Control to an ethernet network.

License: MIT

## Installation:

Installing this requires the Arduino IDE, tested on version 2.3.1, with (off the top of my head) the following libraries:
  - Arduino_Machinecontrol 1.1.1
  - Arduino_PosixStorage 1.2.0: (for future configuration loading from a USB drive)
  - Adafruit BME680 2.0.4: for interaction with the I2C environmental sensor
  - Adafruit Unified Sensor 1.1.14: Same as above
  - Arduino_USBHostMbed5 0.3.1: for USB storage
Configuration of some sections can be done at the top of each .ino file. Upon compilation, 
the Arduino IDE will automatically include all .ino files in the directory (in alphabetical order). 
When upload to the board fails with a DFUError, sometimes it's helpful to double-click the reset button on the Arduino. 

## Communication: 

Communication can be achieved using USB serial at 115200 KBps. In that case, be aware that there are also debug and 
supplementary messages being sent on this bus. The reply to a command should always be preceded by an "OK" or "OK:" message for 
non-returning and value-returning messages, respectively. 

A second communication option is via the built-in ethernet interface. This can use either a fixed IP or DHCP. 
The ethernet settings configuration at the moment is specified at the top of the Networking.ino file. When connected, 
the same command structure specified below is available via a socket interface running on port 1111. 
For example, to set DO pin 3 high, you can execute (on another unix/linux/macos computer), note: probably \n is not necessary: 
> printf 'SET DO 3 1 \n' | nc 192.168.178.199 1111  
which should respond with "OK". The state can be verified with: 
> printf 'GET DO 3 \n' | nc 192.168.178.199 1111

to read out the temperature from the environmental sensor, use: 
printf 'GET SENSOR ENV TEMP' | nc 192.168.178.199 1111
which should return "OK: 19.83" if the temperature is 19.83 degrees C
to read out the temperature from a PT100 attached in three-wire mode to temperature probe input 2: 
printf 'GET SENSOR TEMP 2' | nc 192.168.178.199 1111
This should return a temperature ("OK: 22.50"). However, if there is an error, it reads -999 or -998, probably due to a wrong connection of the temperature probe. 

Some commands take up to a few hundred ms to respond, due to the time it takes to switch to the relevant port. 

## Connection notes: 
    1. you can only set one type of temperature probe input for the temperature probe bank, either all RTD or all TC, not both
    2. Same with the analog inputs, they're either all 0-10V, 4-20mA or NTC
    3. Configuration for the different components can be found in their respective sections. Future upgrades will load configuration from inserted USB drive PortentaConfig.json
    4. DIO pins do not seem to successfully read state (writing state seems fine). Best stick to DI and DO pins for these.
    5. External sensor bank is at the moment configured to be a BME680 chip connected via the 4-pin i2c connector on the front

## Command structure: 
The command format follows the description: ACTION PIN_TYPE PIN_NUMBER VALUE for setting and ACTION PIN_TYPE PIN_NUMBER for getting values.

To set a digital output pin: SET DO <pin> <value>
To get a digital input pin state: GET DI <pin>
To set an analog output pin: SET AO <pin> <value>
To get an analog input pin reading: GET AI <pin>
To get a sensor value: GET SENSOR <bank_name> <sensor_name>

Available sensor banks include "ENV" or "ENVIRONMENTAL", which reads the attached BME680 chip. sensor names include:
  - H, HUMIDITY: the relative humidity in percent
  - T, TEMP, TEMPERATURE: the ambient temperature in degree C
  - P, PRESSURE: the ambient pressure in mbar
  - dp, DEWPOINT: the dewpoint in degree C
  - G, GAS, GASRESISTANCE: the amount of volatile compounds in the gas stream in arbitrary units
A second bank "TEMP", "TEMPERATURE", "TEMPERATUREPROBES" is available, this can read out the three temperature probe inputs on the PMC. 

## More example Commands:

SET DO 5 1 - Set digital output pin 5 to HIGH.
GET DI 2 - Get the state of digital input pin 2.
SET AO 3 10.0 - Set analog output pin 3 to 10V.
GET AI 0 - Get the reading from analog input pin 0.
GET SENSOR ENV temperature - Get the temperature reading from a sensor named "temperature".

// Checks:
// Check ethernet functionality - done. 
// Check DI write and read
// Check DO write and read - done
// Check DIO write and read - done, read does not return a high value when set high, not sure what's the issue
// Check AI write and read
// Check AO write and read 
// Check TEMP bank write and read with 3-wire RTDs
// Check TEMP bank write and read with 2-wire TCs
// Check ENV sensor bank read - done
// Check long-term stability