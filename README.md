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

Open an .ino file in the subdirectory NetworkedPortenta (with all the ino files) in your Arduino IDE, this should automatically open all the associated ino files. 
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
    4. To use the DO or DIO pins, supply 24V to the "24V In" pins on their respective banks. After that, they work as expected. 
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

A second bank "TEMP", "TEMPERATURE", "TEMPERATUREPROBES" is available, this can read out the three temperature probe inputs (0-2) on the PMC. Configuration of the temperature probe types (PT100, PT1000 RTDs or K or J-type thermocouples) is done in the Sensors.ino segment.

## More example Commands:

SET DO 5 1 - Set digital output pin 5 to HIGH.
GET DI 2 - Get the state of digital input pin 2.
SET AO 3 10.0 - Set analog output pin 3 to 10V.
GET AI 0 - Get the reading from analog input pin 0.
GET SENSOR ENV TEMP - Get the temperature reading from a sensor named "temperature".

## Future upgrades:

One issue is that polling the I2C-connected environmental chip (and maybe the internal temperature probe readouts) takes quite a while, which can block other operations. As the Portenta has two cores which each can run their own Arduino code,
the idea is to have one core (the M4 core for example) regularly poll and update the sensor parameters, while the second (M7) core handles the communication. 
This shouldn't be too hard to implement, as long as we use the shared memory space for storing the sensor parameters and update interval. 

The second upgrade is to allow adjustment of settings on the fly. At least the update interval should be settable, other options include latching of pins, and perhaps switching of AI and TEMP input types. 

A third upgrade would be reading a configuration from an attached USB stick. Test code showed that it was possible to mount and read a file, but the JSON interpreter could not be made to work yet. 

## Checks:
* Check ethernet functionality - done.
* Check DI read - done
* Check DO write and read - done, remember to connect 24V to the "24V In" on the DO Terminal Block. Can do >10 ops/s.
* Check DIO write and read - done, remember to connect 24V to the "24V In" on the DIO Terminal Block. Can do >10 ops/s.
* Check AI write and read - no dice yet, connecting AO to an AI pin did not result in useful readings. Can do >10 ops/s
* Check AO write and read - write works 0-10.5V, checked with DMM, but read is nonsense. Can do >10 ops/s
* Check TEMP bank write and read with 3-wire RTDs
* Check TEMP bank write and read with 2-wire TCs
* Check ENV sensor bank read - done works. This one is super slow as the BMP680 is slow to respond, taking about 200-300 ms (blocking) to respond to a single request. Can be fixed by updating internal values at regular intervals and just returning the internal stored values
* Check long-term stability - running for several days, no problem detected.
 
