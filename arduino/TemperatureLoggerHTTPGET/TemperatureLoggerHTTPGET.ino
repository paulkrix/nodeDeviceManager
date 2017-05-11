/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* password = "";

const char* host = "data.sparkfun.com";
const char* streamId = "bGl0jYr1wgflYod32xpb";
const char* privateKey = "VpoqRJnbz1uMdb1xP92z";
const int httpPort = 80;


// Data wire is plugged into digital pin 2 which is gpio 4 on the Wemon
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 9 // Lower resolution

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer; // We'll use this variable to store a found device address

const int interval = 1000 * 60 * 5; //milliseconds
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  delay(10);

  // Start up the sensor library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");

  if (!sensors.getAddress(thermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(thermometer, TEMPERATURE_PRECISION);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {

  unsigned long currentMillis = millis();

  if( currentMillis - previousMillis >= interval ) {
    previousMillis = currentMillis;
    logTemperature();
  }

}

void logTemperature() {

  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  sensors.requestTemperatures();

  // We now create a URI for the request
  String url = "/input/";
  url += streamId;

  float temperature = sensors.getTempC( thermometer );
  Serial.println( temperature );

  String data = "temperature=";
  data += String( temperature, 1 );

  Serial.println( url );

  // This will send the request to the server
  client.print("POST " + url + " HTTP/1.1\r\n" +

               "Host: " + host + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + data.length() + "\r\n" +
               "Phant-Private-Key: " + privateKey + "\r\n" +
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
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

}
