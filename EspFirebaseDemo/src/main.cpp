/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <PubSubClient.h>
//#include <Adafruit_Sensor.h>
#include <OneWire.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <ESP_Mail_Client.h>
#include <DallasTemperature.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Gimp"
#define WIFI_PASSWORD "FC7KUNPX"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDkJinA0K6NqBLGR4KYnX8AdDNgXp2-FDI"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp32-heater-controler-6d11f-default-rtdb.europe-west1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// put your ssid and password here
// const char *ssid = "Gimp";
const char *ssid = "Gimp_EXT";
const char *password = "FC7KUNPX";

// put your mqtt server here:

const char *mqtt_server = "c846e85af71b4f65864f7124799cd3bb.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Secure MQTT port
const char *mqtt_user = "Tortoise";
const char *mqtt_password = "Hea1951Ter";

// Initialize the pubsub client
WiFiClientSecure espClient;
//PubSubClient client(espClient);

// put global variables here:
// Define pins and other constants
#define Relay_Pin D5 // active board
// #define builtInLED_Pin 13    // on board LED_Pin
#define LED_Pin 13 // LED_Pin  //change when debuged
// Data wire is connected to D7
#define ONE_WIRE_BUS D7 // active board  // on pin 10 (a 4.7K resistor is necessary)

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass the OneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Array to hold addresses of the DS18B20 sensors
DeviceAddress redSensor, greenSensor, blueSensor;
// Define pins and other constants
byte i, present = 0, type_s, data[12], addr[8];

float celsius, BlueSensor, GreenSensor, RedSensor, stores1, stores2, stores3, prevS1 = -1, prevS2 = -1, prevS3 = -1;

int adr;
uint_fast8_t amTemperature, pmTemperature, amTemp, pmTemp; // is set by the sliders
uint_fast8_t AMtime, PMtime, Day, Hours, Minutes, seconds, amHours, amMinutes, pmHours, pmMinutes, prevHours = -true, prevMinutes = -1;
bool Am, AmFlag, heaterStatus = false, publishStartUp = true, StartUp = true, heaterOn = false, prevHeaterStatus = false;
// Timer-related variables
unsigned long heaterOnTime = 0;
const unsigned long heaterTimeout = 3600000;

char prevTargetTemperature[23], targetTemperature[23];

/********************************************
      settup the time variables start
 * ******************************************/

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char sensor[50];
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
// Time zone and DST offsets
const long utcOffsetInSeconds = 0;    // Standard time offset (e.g., UTC+0)
const long dstOffsetInSeconds = 3600; // Daylight saving time offset (e.g., +1 hour)

// Function to check if DST is in effect
bool isDST(int day, int month, int hour, int weekday)
{
  // Example logic for DST in Europe (last Sunday in March to last Sunday in October)
  if (month < 3 || month > 10)
    return false; // No DST in Jan, Feb, Nov, Dec
  if (month > 3 && month < 10)
    return true;                  // DST in Apr, May, Jun, Jul, Aug, Sep
  int lastSunday = day - weekday; // Calculate the last Sunday of the month
  if (month == 3)
    return (lastSunday >= 25 && (hour >= 2)); // DST starts at 2 AM on the last Sunday in March
  if (month == 10)
    return (lastSunday < 25 || (lastSunday == 25 && hour < 2)); // DST ends at 2 AM on the last Sunday in October
  return false;
}
/********************************************
      settup the time variables end
 * ******************************************/
/********************************************
      wifi and pubSup credentials start
 * ******************************************/

// the sender email credentials
#define SENDER_EMAIL "esp8266heaterapp@gmail.com";
#define SENDER_PASSWORD "xhjh djyf roxm sxzh";
#define RECIPIENT_EMAIL "mac5y4@talktalk.net"
#define SMTP_HOST "smtp.gmail.com";
#define SMTP_PORT 587;

SMTPSession smtp;

WiFiClient Temp_Control;
// PubSubClient client(Temp_Control);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
long int value = 0;

// Function prototypes
// void setup_wifi();
void callback(char *topic, uint8_t *payload, unsigned int length);
// void callback(char *topic, byte *payload, unsigned int length);
int reconnect(int index);
void publishTempToMQTT();
void relay_Control();
void sendSensor();
void startHeaterTimer();
void checkHeaterTimeout();
void smtpCallback(SMTP_Status status);
void gmail_send(String subject, String message);
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

/********************************************
      wifi and pubSup credentials end
 * ******************************************/

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  sendSensor();
   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    Firebase.RTDB.setInt(&fbdo, "RED", (int)redSensor);
    Firebase.RTDB.setInt(&fbdo, "GREEN", (int)greenSensor);
    Firebase.RTDB.setInt(&fbdo, "BLUE", (int)blueSensor);
    sendDataPrevMillis = millis();
    Serial.println("loop " + String(count));
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;

    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0, 100)))
    {
      Serial.println("loop " + String(count));
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

void sendSensor()
{
  /**************************
       DS18B20 Sensor
         Starts Here
  **************************/
  char sensVal[50]; // Declare sensVal here
                    // Request temperatures from all sensors
  sensors.requestTemperatures();

  // Read temperatures from each sensor
  float RedSensor = sensors.getTempC(redSensor);
  float GreenSensor = sensors.getTempC(greenSensor);
  float BlueSensor = sensors.getTempC(blueSensor);

  // Print temperatures to the serial monitor
  Serial.print("red: ");
  Serial.print(RedSensor);
  Serial.println(" °C");

  Serial.print("Green: ");
  Serial.print(GreenSensor);
  Serial.println(" °C");

  Serial.print("blue: ");
  Serial.print(BlueSensor);
  Serial.println(" °C");

  Serial.println("-----------------------");

  delay(2000);

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
      amTemp = amTemperature;
      // int myTemp = amTemp;
      snprintf(targetTemperature, sizeof(targetTemperature), "%d", amTemp);
    }
  }
  else
  {
    if (pmHours == Hours && pmMinutes == Minutes)
    { // set pmTemp for the Night time setting
      Serial.println("line 602 Am == false");
      AmFlag = false;
      pmTemp = pmTemperature;
      int myTemp = pmTemp;
      snprintf(targetTemperature, sizeof(targetTemperature), "%d", pmTemp);
    }
  }
}
/*************************************************************
                             Heater Control
                                    End
 *************************************************************/