// Networking.ino

#include "EthernetInterface.h"

EthernetInterface net;
TCPSocket server;
TCPSocket* client = nullptr; // Dynamically assigned when a connection is established

bool useSerial = true; // can be set to false to disable controlling I/O via serial (debug messages only)

// No need for MAC in EthernetInterface, handled internally
// Network settings - for fixed IP
bool useEthernet = true; // can be set to false either externally or when ethernet checks fail
const char* ip = "172.17.1.124";
const char* netmask = "255.255.255.0";
const char* gateway = "172.17.1.1";
const int port = 1111;
bool useDHCP = false;

// // // // // // Non-user-adjustable variables // // // // // //
// max input size expected for one socket command, like 'set relay 0 1' or 'get temp'
#define INPUT_SIZE 100
// for commands
char* cmd = "";
char* element = "";
bool value = false;
String receivedString ; // used only for reading incoming HTTP packet

// EthernetServer serverS(1111); // socket port
void initEthernet() {
  // Bring up the ethernet interface
  if (!useEthernet) {
    Serial.println("setting useEthernet set to false, not initializing ethernet.");
    return;
  }
  SocketAddress addr;
    if (useDHCP) {
      net.set_dhcp(true);
      nsapi_error_t status = net.connect();
      if (status != 0) {
          Serial.println("Failed to connect with DHCP");
          useEthernet=false;
          return;
      }
    } else {
        net.set_dhcp(false);
        net.set_network(ip, netmask, gateway);
        nsapi_error_t status = net.connect();
        if (status != 0) {
            Serial.println("Failed to connect with static IP");
            return;
        }
    }

    net.get_ip_address(&addr);
    Serial.print("Connected with IP: ");
    Serial.println(addr.get_ip_address() ? addr.get_ip_address() : "None");
    
    // Start listening for clients
    nsapi_error_t socketStatus = server.open(&net);
    if (socketStatus == NSAPI_ERROR_OK) {
        socketStatus = server.bind(port);
        if (socketStatus != NSAPI_ERROR_OK) {
            Serial.println("Failed to bind socket");
            return;
        }
        nsapi_error_t listenStatus = server.listen(1); // Start listening for incoming connections
        
        if (listenStatus != NSAPI_ERROR_OK) {
          Serial.println("Failed to set up listener socket");
          return;
        }
        server.set_blocking(false); // set to non-blocking mode. 
        Serial.println("server listening");
    } else {
        Serial.println("Failed to open server");
    }

}

void listenForSocket() {
  if (!useEthernet) {
    return; // nothing to do. 
  }
  nsapi_error_t error;
  
  TCPSocket* client = server.accept(&error); // Accept a new connection
  if (error != NSAPI_ERROR_OK) {
      // Handle error or non-blocking mode indication
      if (error == NSAPI_ERROR_WOULD_BLOCK) {
          // Non-blocking mode and no incoming connection, handle accordingly
          // Serial.print("Accept error: ");
          // Serial.println(error);
      } else {
          // Some other error occurred
          Serial.print("Accept error: ");
          Serial.println(error);
      }
      return; // Exit the function, as there's no client to process
  }
  if (client != nullptr) {
      // Successfully accepted a new connection
      Serial.println("Socket client connected");

      // Example of reading data from the client
      char buffer[128];
      int len = client->recv(buffer, sizeof(buffer) - 1);
      if (len > 0) {
          buffer[len] = '\0'; // Null-terminate the string
          String input = String(buffer);
          String response = processCommand(input);
          
          // Send response back to client
          client->send(response.c_str(), response.length());
      }

      client->close(); // Close the client connection
      // this causes a hanggggg:
      // delete client; // Deallocate the TCPSocket instance for the client
      
  }
}

void listenForSerial() {
  if (!useSerial) {
    return; // nothing to do here. 
  }
  if (!Serial) {
    return; // nothing to do here. 
  }
  if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input.length() > 0) {
          String response = processCommand(input);
          Serial.println(response);
      }
  }
}

String processCommand(String command) {
  // Split the command into parts
  int firstSpace = command.indexOf(' ');
  String action = command.substring(0, firstSpace);
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  String pinType = command.substring(firstSpace + 1, secondSpace);
  int thirdSpace = command.indexOf(' ', secondSpace + 1);
  String pin = command.substring(secondSpace + 1, thirdSpace);
  String value = command.substring(thirdSpace + 1); // This will be empty for GET commands except for SENSOR get commands

  String response;
  // Dispatch to the appropriate handler
  if ((action == "SET" && pinType == "DO")||(action == "SET" && pinType == "DIO")) {
    setD(pinType, pin.toInt(), value.toInt());
    response = "OK";
  } else if (action == "GET" && (pinType == "DI"|| pinType == "DIO" || pinType == "DO")) {
    int pinValue = getD(pinType, pin.toInt());
    response = "OK: " + String(pinValue);
  } else if (action == "SET" && pinType == "AO") {
    setA(pin.toInt(), value.toFloat());
    response = "OK";
  } else if (action == "GET" && (pinType == "AI") || (pinType=="AO")) {
    float pinValue = getA(pin.toInt());
    response = "OK: " + String(pinValue);
  } else if (action == "GET" && pinType == "SENSOR") {
    float sensorValue = getSensorValue(pin, value); // here, 'pin' is the sensor bank nanme, 'value' here is the sensor name in the bank
    response = "OK: " + String(sensorValue);
  } else {
    response = "ERROR: Invalid command";
  }
  return response;
}