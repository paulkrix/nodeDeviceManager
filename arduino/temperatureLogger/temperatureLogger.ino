 /****
  * Table of contents
  * -------------------
  * 1. Includes
  * 2. Globals and configuration
  * 3. Initialisation
  * 4. The loop
  * 5. Unregisterd device state
  * 6. Normal device operation
  *******/


/****
 * 1. Includes
 *******/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/****
 * 2. Globals and configuration
 *******/

/*
 *  Wifi variables
 */
const char* ssid = "";
const char* password = "";

/*
 * One wire and temperature sensor variables
 */
// Data wire is plugged into digital pin 2 which is gpio 4 on the Wemos
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 9 // Lower resolution
const int interval = 1000 * 60 * 5; //milliseconds
unsigned long previousMillis = 0;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer; // We'll use this variable to store a found device address

/*
 * Branch server variables
 */
const char* branch = NULL;
const int branchPort = 8080;
WiFiUDP Udp;
const unsigned int UdpPort = 4210;
boolean UdpStarted = false;
const unsigned int HttpPort = 8080;
WiFiServer server( HttpPort );
unsigned int deviceId = 0;

enum State {
  unregistered,
  normal,
} state;

/****
 * 3. Initialisation
 *******/

void setup() {

  state = unregistered;
  Serial.begin(9600);

  // Initialise sensors
  sensors.begin();
  if (!sensors.getAddress(thermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(thermometer, TEMPERATURE_PRECISION);


  while ( WiFi.status() != WL_CONNECTED ) {
    delay( 500 );
    Serial.print( "." );
  }
  Serial.println("");
  Serial.println( "WiFi connected" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.localIP() );

  state = registering;
  if( branch[0] == '\0' ) {
    state = scanning;
    Serial.println( "Starting UDP" );
    Udp.begin( localUdpPort );
    Serial.print( "Local port: " );
    Serial.println( Udp.localPort() );
  }

}

/****
 * 4. The loop
 *******/

 unsigned long currentMillis = millis();

 switch( state ) {
   case scanning:
     scanForBranch();
     break;
   case scanning_waiting:
     if( currentMillis - previousScan >= scanInterval ) {
       previousScan = currentMillis;
       changeState( scanning );
     }
     checkForResponse();
     break;

   case registering:
     registerDevice();
     break;
   case normal:
     if( currentMillis - previousMillis >= interval ) {
       previousMillis = currentMillis;
       //Do normal stuff
     }

     readFromClient();
     break;
 }

}

/**********
 * Misc
 *********/

void changeState( State newstate ) {
  state = newstate;
  stateReported = false;
}

void reportState( String message ) {
  if( !stateReported ) {
    Serial.println( message );
    stateReported = true;
  }
}
/**********
 * 5. Unregisterd device state
 **********/

void scanForBranch() {
  reportState( "\r\nScanning for branch" );
  IPAddress broadcastIp;
  broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();

  Serial.println( broadcastIp );

  Udp.beginPacket( broadcastIp, broadcastUdpPort );
  Udp.write("branchscan");
  Udp.endPacket();
  changeState( scanning_waiting );
}

void checkForResponse() {

  reportState( "\r\nChecking for response" );
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    char packetBuffer[255];  //buffer to hold incoming packet,
    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    packetBuffer[len] = 0;
    String response = String( packetBuffer );
    int splitPoint = response.indexOf(':');
    if( splitPoint > 0 ) {
      response.substring(0, splitPoint).toCharArray(branch, splitPoint+1);
      branchPort = response.substring(splitPoint + 1).toInt();
      changeState( registering );
      Serial.println(branch);
      Serial.println(branchPort);
    }

    Serial.println("Contents:");
    Serial.println(packetBuffer);
  }
}

int registerDevice() {
  reportState( "\r\nRegistering device")
  // Serial.println( branch );
  // Serial.println( branchPort );
  WiFiClient client;
  if (!client.connect(branch, branchPort)) {

    Serial.println("connection failed");
    return 1;
  }

  // We now create a URI for the request
  String url = "/devices/register";

  String deviceData = "{\"schema\":{\"database\":\"brew\","
    "\"collection\":\"temperature\"},"
    "\"ip\":\"" + WiFi.localIP().toString() + "\","
    "\"port\":8085}";

  // This will send the request to the server
  client.print("POST " + url + " HTTP/1.1\r\n" +
               "Host: " + branch + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + deviceData.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               deviceData + "\r\n" );

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return 2;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

  return 0;
}

/**********
 * 6. Normal device operation
 **********/

 void readFromClient() {
   // Check if a client has connected
   WiFiClient client = server.available();
   if( !client ) {
     return;
   }

   // Wait until the client sends some data
   Serial.println("new client");
   while(!client.available()){
     delay(1);
   }

   // Read the first line of the request
   String req = client.readStringUntil('\r');
   Serial.println(req);
   client.flush();

   String message = "unknown";
   if (req.indexOf("/status") != -1) {
     message = "running";
   } else {
     client.stop();
     return;
   }

   client.print("HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n" + message + "\r\n" );
 }

 void sendTemperatureToBranch() {

   WiFiClient client;
   if (!client.connect(branch, branchPort)) {
     Serial.println("connection failed");
     return;
   }

   sensors.requestTemperatures();

   // We now create a URI for the request
   String url = "devices/" + deviceId + "/input";


   float temperature = sensors.getTempC( thermometer );
   Serial.println( temperature );

   String data = "{\"c\":\"";
   data += String( temperature, 1 );
   data += "\"}";

   Serial.println( url );

   // This will send the request to the server
   client.print("POST " + url + " HTTP/1.1\r\n" +

                "Host: " + branch + "\r\n" +
                "Content-Type: application/x-www-form-urlencoded\r\n" +
                "Content-Length: " + data.length() + "\r\n" +
                "Connection: close\r\n\r\n" +
                data + "\r\n" );
   unsigned long timeout = millis();
   while (client.available() == 0) {
     if (millis() - timeout > 5000) {
       Serial.println(">>> Client Timeout !");
       client.stop();
       return;
     }
   }

   // Read all the lines of the reply from server and print them to Serial
   while(client.available()){
     String line = client.readStringUntil('\r');
     Serial.print( line );
   }

   Serial.println();
   Serial.println("closing connection");
 }
