// ESP32 Blink Example
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <pubsubclient.h>
#include <WiFiUdp.h>
// #include <NTPClient.h>

// put global variables here:
// Define pins and other constants
#define Relay_Pin D5 // active board
// #define builtInLED_Pin 13    // on board LED_Pin
#define LED_Pin D6 // LED_Pin  //change when debuged
// Data wire is connected to D7
#define ONE_WIRE_BUS D7 // active board  // on pin 10 (a 4.7K resistor is necessary)

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass the OneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Array to hold addresses of the DS18B20 sensors
DeviceAddress redSensor, greenSensor, blueSensor;

float celsius, BlueSensor, GreenSensor, RedSensor, stores1, stores2, stores3, prevS1, prevS2, prevS3;

int adr;
uint_fast8_t ESP32amTemperature, ESP32pmTemperature, amTemp, pmTemp; // is set by the sliders
uint_fast8_t ESP32AMtime, ESP32PMtime, Day, Hours, Minutes, seconds, amHours, amMinutes, pmHours, pmMinutes, prevHours = -true, prevMinutes = -1;
bool Am, AmFlag, heaterStatus = false, StartUp = true, sensorReadStartUp = true, heaterOn = false, prevHeaterStatus = false;

char prevTargetTemperature[23], ESP32targetTemperature[23];

// Define pins and other constants
// #include <Firebase_ESP_Client.h>
// //Provide the token generation process info.
// #include "addons/TokenHelper.h"
// //Provide the RTDB payload printing info and other helper functions.
// #include "addons/RTDBHelper.h"

#define WIFI_SSID "Gimp"
#define WIFI_PASSWORD "FC7KUNPX"

// put your mqtt server here:

const char *mqtt_server = "c846e85af71b4f65864f7124799cd3bb.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Secure MQTT port
const char *mqtt_user = "Tortoise";
const char *mqtt_password = "Hea1951Ter";

// Initialize the pubsub client
WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
long int value = 0;

// prototypes
void setup();
void loop();
void callback(char *topic, uint8_t *payload, unsigned int length);
int reconnect(int index);
void publishTempToMQTT();
// void relay_Control();
void sendSensor();

/********************************************
  Static IP address and wifi conection Start
********************************************/

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional
/********************************************
     Static IP address and wifi conection end
 ********************************************/

bool isConnected = false; // flag to check if the device is connected to the wifi

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(921600);
  delay(1000);
  Serial.println("Hello World!");
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Initialize Firebase
  // config.api_key = "AIzaSyA81MAjPXCO5fYpzsrhXP6t3XQ4oVNrpAo";
  // auth.user.email = "your_email";
  // auth.user.password = "your_password";
  // Firebase.begin(&config, &auth);
  // Firebase.reconnectWiFi(true);

  //  Firebase.begin(&config, &auth);
  // Firebase.reconnectWiFi(true);
  // Start the DallasTemperature library
  sensors.begin();

  // Check if at least 3 sensors are connected
  if (sensors.getDeviceCount() < 3)
  {
    Serial.println("Error: Less than 3 DS18B20 sensors detected!");
    while (true)
      ; // Halt execution
  }

  // Assign addresses to sensors
  if (!sensors.getAddress(redSensor, 0))
    Serial.println("Error: Sensor 1 not found!");
  if (!sensors.getAddress(greenSensor, 1))
    Serial.println("Error: Sensor 2 not found!");
  if (!sensors.getAddress(blueSensor, 2))
    Serial.println("Error: Sensor 3 not found!");

  // Set MQTT server and callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Set insecure mode for WiFiClientSecure
  espClient.setInsecure();
}

void loop()
{
  /***********************
   *   WIFI Conection     *
   *       Start          *
   ***********************/

  if (WiFi.status() == WL_CONNECTED && !isConnected)
  {
    Serial.println("Connecting to WiFi");
    digitalWrite(LED_BUILTIN, HIGH);
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
  }
  /***********************
   *   WIFI Conection     *
   *         End          *
   ***********************/

  if (!client.connected())
  {
    Serial.println("void loop() line 175");
    // Serial.println("Calling Reconnect to MQTT server");
    // Attempt to reconnect and check the return value
    if (reconnect(2) == 1)
    {
      Serial.println("BBB loop Successfully connected and subscribed in setup");
    }
    else
    {
      Serial.println("Failed to connect and subscribe in setup");
    }
  }
  client.loop();
  sendSensor();
  publishTempToMQTT();
}

void handleWiFiConnection(const char* ssid, const char* password, bool &isConnected) {
  static bool isStaticIPConfigured = false;

  // Configure static IP once
  if (!isStaticIPConfigured) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("Failed to configure Static IP");
    } else {
      Serial.println("Static IP configured");
    }
    isStaticIPConfigured = true; // Avoid reconfiguring every call
  }

  // Handle WiFi connection
  if (WiFi.status() == WL_CONNECTED && !isConnected) {
    Serial.println("Connecting to WiFi");
    digitalWrite(LED_BUILTIN, HIGH); // Turn LED on
    isConnected = true;             // Update connection status
  } else if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting"); // Debug message
    WiFi.begin(ssid, password); // Attempt connection
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED state
    delay(1000); // Wait before retrying
    isConnected = false; // Update connection status
  }
}

void sendSensor()
{
  /**************************
       DS18B20 Sensor
         Starts Here
  **************************/
  // Serial.println("line 169");
  char sensVal[50]; // Declare sensVal here
                    // Request temperatures from all sensors
  sensors.requestTemperatures();

  // Read temperatures from each sensor
  RedSensor = sensors.getTempC(redSensor);
  GreenSensor = sensors.getTempC(greenSensor);
  BlueSensor = sensors.getTempC(blueSensor);
  if (sensorReadStartUp)
  {
    Serial.println("line 178 sensorReadStartUp == true");
    prevS1 = RedSensor - 2;
    prevS2 = GreenSensor - 2;
    prevS3 = BlueSensor - 2;
    sensorReadStartUp = false;
  }
  // Serial.println("line 182 sensorReadStartUp " + String(sensorReadStartUp));
  // Serial.print("line 183  prevS1: ");
  // Serial.println(prevS1);
  // Serial.print("line 183  RedSensor: ");
  // Serial.println(RedSensor);
  // Serial.print("line 185  prevS2: ");
  // Serial.println(prevS2);
  // Serial.print("line 185  GreenSensor: ");
  // Serial.println(GreenSensor);
  // Serial.print("line 187  prevS3: ");
  // Serial.println(prevS3);
  // Serial.print("line 187  BlueSensor: ");
  // Serial.println(BlueSensor);
  // Serial.println("Delaying for 9 seconds");
  // Serial.println("After delay");
  // // Print temperatures to the serial monitor
  // Serial.print("line 189 red: ");
  // Serial.print(RedSensor);
  // Serial.println(" °C");

  // Serial.print("Green: ");
  // Serial.print(GreenSensor);
  // Serial.println(" °C");

  // Serial.print("blue: ");
  // Serial.print(BlueSensor);
  // Serial.println(" °C");

  // Serial.println("-----------------------");



  /**************************
       DS18B20 Sensor
         Ends Here
  **************************/

  /*************************************************************
                              Heater Control
                                   start
  ************************************************************/

  if (Am)
  {
    if (amHours == Hours && amMinutes == Minutes)
    { // set amTemp for the Night time setting
      Serial.println("line 591 Am == true");
      AmFlag = true;
      amTemp = ESP32amTemperature;
      // int myTemp = amTemp;
      snprintf(ESP32targetTemperature, sizeof(ESP32targetTemperature), "%d", amTemp);
    }
  }
  else
  {
    if (pmHours == Hours && pmMinutes == Minutes)
    { // set pmTemp for the Night time setting
      Serial.println("line 602 Am == false");
      AmFlag = false;
      pmTemp = ESP32pmTemperature;
      int myTemp = pmTemp;
      snprintf(ESP32targetTemperature, sizeof(ESP32targetTemperature), "%d", pmTemp);
    }
  }
}
/*************************************************************
                             Heater Control
                                    End
 *************************************************************/

/*************************************************************
 *                     call back Start                       *
 *************************************************************/
void callback(char *topic, uint8_t *payload, unsigned int length)
{
  if (topic == nullptr || payload == nullptr)
  {
    Serial.println("Error: Null topic or payload");
    return;
  }
  // Null-terminate the payload to treat it as a string
  payload[length] = '\0';

  if (strstr(topic, "ESP32amTemperature"))
  {
    sscanf((char *)payload, "%d", &ESP32amTemperature);
    Serial.println("line 391 ******** ESP32amTemperature:****** " + String(ESP32amTemperature));
    if (StartUp == true)
    {
      amTemp = ESP32amTemperature;
      Serial.println("line 392 ******** ESP32amTemp:********** " + String(amTemp));
    }
  }
  if (strstr(topic, "ESP32pmTemperature"))
  {
    sscanf((char *)payload, "%d", &ESP32pmTemperature);
    Serial.println("line 398 *********ESP32pmTemperature:********* " + String(ESP32pmTemperature));
    if (StartUp == true)
    {
      pmTemp = ESP32pmTemperature;
      StartUp = false;
    }
  }
  if (strstr(topic, "ESP32AMtime"))
  {
    sscanf((char *)payload, "%d:%d", &amHours, &amMinutes);
    Serial.println("line 405 ********* ESP32AMtime *********"+String((char *)payload));
  }
  if (strstr(topic, "ESP32PMtime"))
  {
    sscanf((char *)payload, "%d:%d", &pmHours, &pmMinutes);
    Serial.println("line 410 ********** ESP32PMtime b******** "+String((char *)payload));
  }
  if (amTemp != 0 && pmTemp != 0)
  {
    StartUp = 0;
  }
}

/*************************************************************
 *                     call back End                         *
 ************************************************************/

/*************************************************************
 *                       Reconnect Start                      *
 *************************************************************/
int reconnect(int index)
{
  // Serial.println("reconnect index: " + String(index));
  while (!client.connected())
  {
    Serial.println("line 282 reconnect");
    //  Set a longer keep-alive interval (e.g., 60 seconds)
    client.setKeepAlive(60);
    if (client.connect("WemosD1Client", mqtt_user, mqtt_password))
    {
      Serial.println("Successfully connected to MQTT server");
      client.subscribe("Temp_Control/sub");
      client.subscribe("control");
      client.subscribe("ESP32amTemperature");
      client.subscribe("ESP32pmTemperature");
      client.subscribe("ESP32AMtime");
      client.subscribe("ESP32PMtime");
      client.subscribe("ESP32HeaterStatus");
      client.subscribe("ESP32ESP32outSide");
      client.subscribe("ESP32coolSide");
      client.subscribe("ESP32heater");
      return 1;
    }
    else
    {
      Serial.print("line 258 failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
  return 0;
}
/*************************************************************
 *                      Reconnect End                        *
 * ***********************************************************/

/********************************************
         send temperature value
             to server for
          temperature monitor
                to receive.
                  start
* ******************************************/
void publishTempToMQTT(void)
{
  // Serial.println("line 331 publishTempToMQTT");
  if (!client.connected())
  {

    // Attempt to reconnect and check the return value
    if (reconnect(3) == 1)
    {
      Serial.println("CCC publish Successfully connected and subscribed in setup");
    }
    else
    {
      Serial.println("Failed to connect and subscribe in setup");
    }
  }
  // Serial.println("line 373 ...... RedSensor .... " + String(RedSensor));
  // Serial.println("line 374 ...... prevS1 .... " + String(prevS1));
  // Serial.println("line 375 ...... GreenSensor .... " + String(GreenSensor) + " " + String(prevS2));
  // Serial.println("line 376 ...... BlueSensor .... " + String(BlueSensor) + " " + String(prevS3));
  char sensVal[50];
  const float threshold = 0.2; // Define a threshold for significant change
  // Publish redSensor
  if (fabs(RedSensor - prevS1) > threshold)
  {
    snprintf(sensVal, sizeof(sensVal), "%.2f", RedSensor);
    client.publish("ESP32heater", sensVal, true);
    prevS1 = RedSensor;
    Serial.print("line 352 ,,,,,, Publishing redSensor ,,,,,, ");
    Serial.println(RedSensor);
    Serial.println("line 353 ,,,,,, prevS1 ,,,,,, " + String(prevS1));
  }

  // publish greenSensor
  if (fabs(GreenSensor - prevS2) > threshold)
  {
    snprintf(sensVal, sizeof(sensVal), "%.2f", GreenSensor);
    client.publish("ESP32coolSide", sensVal, true);
    prevS2 = GreenSensor;
    Serial.println("line 362 Publishing greenSensor");
    Serial.println("line 363 ,,,,,, greenSensor ,,,,,,, " + String(sensVal));
  }
  // publish blueSensor
  if (fabs(BlueSensor - prevS3) > threshold)
  {

    snprintf(sensVal, sizeof(sensVal), "%.2f", BlueSensor);
    client.publish("ESP32outSide", sensVal, true);
    prevS3 = BlueSensor;
    Serial.print("line 366 ,,,,,,, Publishing blueSensor ,,,,,,,");
    Serial.println(BlueSensor);
  }

  // publish Hours
  if (Hours != prevHours)
  {
    Serial.println("line 375 ,,,,,, Publishing Hours ,,,,,, ");
    snprintf(sensVal, sizeof(sensVal), "%d", Hours);
    client.publish("ESP32gaugeHours", sensVal, true);
    prevHours = Hours;
  }

  // publish Minutes
  if (Minutes != prevMinutes)
  {
    Serial.println("line 384 ,,,,,,, Publishing Minutes ,,,,,,, ");
    snprintf(sensVal, sizeof(sensVal), "%d", Minutes);
    client.publish("ESP32gaugeMinutes", sensVal, true);
    prevMinutes = Minutes;
  }

  // publish Heater Status
  if (heaterStatus != prevHeaterStatus)
  {
    const char *heaterStatusStr = heaterStatus ? "true" : "false";
    client.publish("ESP32HeaterStatus", heaterStatusStr, true);
    prevHeaterStatus = heaterStatus;
  }

  char targetTemperatureStr[23];
  if (strcmp(ESP32targetTemperature, prevTargetTemperature) != 0)
  {
    Serial.println("line 464 Publishing target temperature");
    snprintf(targetTemperatureStr, sizeof(targetTemperatureStr), "%s", ESP32targetTemperature);
    client.publish("ESP32TargetTemperature", targetTemperatureStr, true);
    strcpy(prevTargetTemperature, ESP32targetTemperature); // Copy the new value to prevTargetTemperature
  }
}

/********************************************
         send temperature value
             to server for
          temperature monitor
                to receive
                   end
* ******************************************/
