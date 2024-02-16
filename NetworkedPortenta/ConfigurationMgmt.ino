// ConfigurationMgmt.ino
// this holds all the methods used to load and use the PortentaConfig.json file on an attached USB drive. 
#include "Arduino_POSIXStorage.h"
#include <ArduinoJson.h> // Make sure to add this library to your project

volatile bool usbAttached = false;

void usbCallback()
{
  usbAttached = true;
}

// const char* filename = "PConf.txt"; // this is what we'll be looking for. 
const char* filename = "/usb/PortentaConfig.json"; // this is what we'll be looking for. /usb/ is just the internal indicator for USB drive
constexpr int bufferSize = 3000;
char config[bufferSize]; 

void readConfig() {
  // largely cribbed from the example code from Arduino_POSIXStorage:
  if (-1 == register_hotplug_callback(DEV_USB, usbCallback))
  {
    if (ENOTSUP == errno)
    {
      Serial.println("Hotplug registration isn't supported on this board");
      for ( ; ; ) ;
    }
  }
  Serial.println("Please insert a thumb drive with the " + String(filename) + " configuration file.");
  while (false == usbAttached) {
    delay(500);
  }
  Serial.println("Thank you!");

  // now we can try reading the file
  FILE *fp;
  // Allocate a temporary JsonDocument
  JsonDocument doc;
  if (0 == mount(DEV_USB, FS_FAT, MNT_DEFAULT))
  {
    fp = fopen(filename, "r");
    if (nullptr != fp)
    {
      // if (NULL != fgets(config, bufferSize, fp))
      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, fp);
      if (error)
        Serial.println(F("Failed to read file, using default configuration"));
      else
      {
        // Serial.println(String(doc["network"]["staticIP"]));
      }
    }
    else
    {
      Serial.println("Error opening file for reading");
      Serial.println(errno);
    }
    fclose(fp);
  }
  else
  {
    Serial.println("Error mounting USB thumb drive");
    Serial.println(errno);
  }
  umount(DEV_USB);

}


// JsonDocument loadConfig() {
//   // Assuming a document size of 1024 bytes; adjust based on your config's complexity
//   JsonDocument doc(1024);

//   // Mount the USB drive
//   if (0 == mount(DEV_USB, FS_FAT, MNT_DEFAULT)) {
//     // Open the configuration file for reading
//     FILE *fp = fopen("/usb/PortentaConfig.json", "r");
//     if (nullptr != fp) {
//       // Parse the JSON configuration file
//       DeserializationError error = deserializeJson(doc, fp);
//       if (error) {
//         Serial.print(F("Failed to read file, using default configuration: "));
//         Serial.println(error.c_str());
//       }
//       fclose(fp); // Close the file
//     } else {
//       Serial.println("Failed to open config file for reading");
//     }
//     umount(DEV_USB); // Unmount the USB drive
//   } else {
//     Serial.println("Failed to mount USB drive for config loading");
//   }

//   return doc; // Return the loaded (or empty) configuration document
// }

