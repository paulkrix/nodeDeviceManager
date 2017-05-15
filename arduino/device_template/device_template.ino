/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "";
const char* password = "";

const int interval = 1000 * 60 * 5; //milliseconds
unsigned long previousMillis = 0;

char branch[15];
int branchPort = 8084;

WiFiUDP Udp;
const unsigned int localUdpPort = 4210; //Do I need to send and receive on different ports?
const unsigned int broadcastUdpPort = 4211;

const int scanInterval = 1000; //milliseconds
unsigned long previousScan = 0;

const unsigned int HttpPort = 8080;
WiFiServer server( HttpPort );

#define STATUS_LED_PIN 2
unsigned int blinkInterval = 1000;
unsigned long previousBlink = 0;
int blinkState = LOW;

enum State {
  scanning,
  scanning_waiting,
  registering,
  normal,
  nothing
};
State state;
boolean stateReported = false;

void setup() {

  Serial.begin( 9600 );

  // Connect to WiFi network
  Serial.print( "Connecting to " );
  Serial.println( ssid );
  WiFi.begin( ssid, password );

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
  pinMode( STATUS_LED_PIN, OUTPUT );

}

void loop() {

  unsigned long currentMillis = millis();

  switch( state ) {
    case scanning:
      scanForBranch();
      blinkLed();
      break;
    case scanning_waiting:
      if( currentMillis - previousScan >= scanInterval ) {
        previousScan = currentMillis;
        changeState( scanning );
      }
      blinkLed();
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

void blinkLed() {
  unsigned long currentMillis = millis();
  if( currentMillis - previousBlink >= blinkInterval ) {
    previousBlink = currentMillis;
    digitalWrite( STATUS_LED_PIN, blinkState );
    if (blinkState == LOW) {
      blinkState = HIGH;
    } else {
      blinkState = LOW;
    }
  }
}

/**********
 * Unregisterd device state
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
    String remoteString = "";
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      remoteString += String( remote[i] );
      if (i < 3) {
        Serial.print(".");
        remoteString += ".";
      }

    }
    remoteString.toCharArray( branch, 15 );
    Serial.println( branch );
    Serial.println( remoteString );
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    char packetBuffer[255];  //buffer to hold incoming packet,
    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    packetBuffer[len] = 0;
    branchPort = atoi( packetBuffer );
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    Serial.println( branchPort );
    changeState( registering );
  }
}

int registerDevice() {
  reportState( "\r\nRegistering device");
  // Serial.println( branch );
  // Serial.println( branchPort );
  WiFiClient client;
  if (!client.connect(branch, branchPort)) {

    Serial.println("connection failed");
    return 1;
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ip"] = WiFi.localIP().toString();
  root["port"] = HttpPort;
  JsonObject& schema = root.createNestedObject();
  schema["database"] = "brew";
  schema["collection"] = "temperature";

  json.printTo( Serial );

  // We now create a URI for the request
  String url = "/devices";

  // String deviceData = "{\"schema\":{\"database\":\"brew\","
  //   "\"collection\":\"temperature\"},"
  //   "\"ip\":\"" + WiFi.localIP().toString() + "\","
  //   "\"port\":}";

  // This will send the request to the server
  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: " + branch);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Content-Length: " + json.measureLength());
  client.println("Connection: close");
  client.println();
  json.printTo( client );

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

  digitalWrite( STATUS_LED_PIN, LOW );
  changeState( normal );
}


/**********
 * Normal device state
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
  String reqestLine = client.readStringUntil('\n');
  reqestLine.trim();
  Serial.println(reqestLine);
  client.flush();

  //Check what protocol the request is
  if( reqestLine.substring(0, 3) == "GET" ) {
    handleGetRequest( requestLine, client );
  }
  if( reqestLine.substring(0, 4) == "POST" ) {
    handlePostRequest( requestLine, client );
  }
  //Unhandled protocol, just ignore it.
  client.stop();
  return;

  // String message = "unknown";
  // if (reqestLine.indexOf("/status") != -1) {
  //   message = "running";
  // } else {
  //   client.stop();
  //   return;
  // }
  //
  // client.print("HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n" + message + "\r\n" );


  // Match the request
  //Replace with actual request stuff
//  int val;
//  if (req.indexOf("/gpio/0") != -1)
//    val = 0;
//  else if (req.indexOf("/gpio/1") != -1)
//    val = 1;
//  else {
//    Serial.println("invalid request");
//    client.stop();
//    return;
//  }

}


void handleGetRequest( String lineOne, WiFiClient client ) {
  int endOfPath = lineOne.substring( 4 ).indexOf(' ');
  String path = lineOne.substring( 4, endOfPath );
  String message = "Sweet get request brah.";
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n" + message + "\r\n" );
  //Check if there is a query string
  int queryStringDelimiter = path.indexOf('?');
  String queryString = "";
  if( queryStringDelimiter >= 0 && path.length() > queryStringDelimiter ) {
    queryString = path.substring( queryStringDelimiter + 1 );
  }
  if( queryString.length() > 0 ) {
    //parse query string
  }
  //Do the action
}

void handlePostRequest( String lineOne, WiFiClient client ) {
  int endOfPath = lineOne.substring( 5 ).indexOf(' ');
  String path = lineOne.substring( 5, endOfPath );
  String message = "Sweet post request brah.";
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n" + message + "\r\n" );
  //Find blank line, the body starts after that.
  //Parse the body
  //Do the action
}
