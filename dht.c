#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

const char* ssid     = "xxx";
const char* password = "xxx";

#define MQTT_SERVER      "xxx.xxx.xxx.xxx"
#define MQTT_SERVERPORT  1883
#define MQTT_USERNAME    "xxx"
#define MQTT_PASSWORD    "xxx"

#define DHT_PIN           5
#define DHTTYPE           DHT11

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

Adafruit_MQTT_Publish topicTemperature = Adafruit_MQTT_Publish(&mqtt, "/attic/temperature");
Adafruit_MQTT_Publish topicHumidity = Adafruit_MQTT_Publish(&mqtt, "/attic/humidity");

void MQTT_connect();
DHT_Unified dht(DHT_PIN, DHTTYPE);

/*
 * Set Up
 */
void setup() {

  // Connect to WiFi Network
  connect2WiFi();

  setupDHT();
}

/* 
 *  Connects to WiFi Network
 */
void connect2WiFi()
{
  // set the baud rate
  Serial.begin(115200);

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
}

uint32_t delayMS;
void setupDHT()
{
  // Initialize device.
  dht.begin();
  Serial.println("DHT11 Unified Sensor Example");

  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");

}

void loop() {

  // Connect to MQTT
  MQTT_connect();

  // wait for 10 seconds
  delay(10000);

  // Get temperature event and print its value.
  sensors_event_t tevent;
  sensors_event_t hevent;

  dht.temperature().getEvent(&tevent);
  if (isnan(tevent.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(tevent.temperature);
    Serial.println(" *C");
  }

  // Get humidity event and print its value.
  dht.humidity().getEvent(&hevent);
  if (isnan(hevent.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(hevent.relative_humidity);
    Serial.println("%");
  }
  
  if (! topicTemperature.publish(CelsiusToFahrenheit(tevent.temperature)))
    Serial.println(F("Failed"));

  if (! topicHumidity.publish(hevent.relative_humidity))
    Serial.println(F("Failed"));
}

void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  Serial.println("MQTT Connected!");
}

int CelsiusToFahrenheit ( float celsius )
{
  float fahrenheit = 0.0;
  fahrenheit = (( celsius * 9) /5 ) + 32;
  return fahrenheit;
}

int FahrenheitToCelsius ( float fahrenheit )
{
  float celsius = 0.0;
  celsius = ( fahrenheit - 32 ) / 1.8;
  return celsius;
}
