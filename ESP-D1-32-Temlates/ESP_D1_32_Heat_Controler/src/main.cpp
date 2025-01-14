// ESP32 Blink Example
#include <Arduino.h>
#include <WiFi.h>
#include <pubsubclient.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP_Mail_Client.h>

// put global variables here:
// Define pins and other constants
#define Relay_Pin D5 // active board
//#define builtInLED_Pin 13    // on board LED_Pin
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
uint_fast8_t amTemperature, pmTemperature, amTemp, pmTemp; // is set by the sliders
uint_fast8_t AMtime, PMtime, Day, Hours, Minutes, seconds, amHours, amMinutes, pmHours, pmMinutes, prevHours = -true, prevMinutes = -1;
bool Am, AmFlag, heaterStatus = false, StartUp = true, sensorReadStartUp = true, heaterOn = false, prevHeaterStatus = false;

// Timer-related variables
unsigned long heaterOnTime = 0;
const unsigned long heaterTimeout = 3600000;// 1 hour

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

// the sender email credentials
#define SENDER_EMAIL "esp8266heaterapp@gmail.com";
#define SENDER_PASSWORD "xhjh djyf roxm sxzh";
#define RECIPIENT_EMAIL "mac5y4@talktalk.net"
#define SMTP_HOST "smtp.gmail.com";
#define SMTP_PORT 587;

SMTPSession smtp;

#define WIFI_SSID "Gimp"
#define WIFI_PASSWORD "FC7KUNPX"

// put your mqtt server here:

const char *mqtt_server = "ea53fbd1c1a54682b81526905851077b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883; // Secure MQTT port
const char *mqtt_user = "ESP32Tortiose";
const char *mqtt_password = "Hea1951TerESP32";

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
void relay_Control();
void sendSensor();
void startHeaterTimer();
void checkHeaterTimeout();
void smtpCallback(SMTP_Status status);
void gmail_send(String subject, String message);

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
   pinMode(Relay_Pin, OUTPUT);
  pinMode(LED_Pin, OUTPUT);      // digitalWrite (LED_Pin, LOW);//LED_Pin off
  pinMode(LED_BUILTIN, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
  relay_Control();
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  // Convert epoch time to struct tm
  time_t rawTime = epochTime;
  struct tm *timeInfo = localtime(&rawTime);

  // Extract time components
  int currentDay = timeInfo->tm_mday;
  int currentMonth = timeInfo->tm_mon + 1; // tm_mon is 0-based
  int currentHour = timeInfo->tm_hour;
  int currentWeekday = timeInfo->tm_wday; // tm_wday is 0-based (Sunday)

  // Adjust for DST
  if (isDST(currentDay, currentMonth, currentHour, currentWeekday))
  {
    epochTime += dstOffsetInSeconds;
    rawTime = epochTime;
    timeInfo = localtime(&rawTime);
  }
  Hours = (timeInfo->tm_hour);
  Minutes = (timeInfo->tm_min);
  Serial.print("******line 234 Minutes: ");
  Serial.println(Minutes);
  seconds = (timeInfo->tm_sec);
  Am = true;
  Am = (Hours < 12);
  // if StartUp == 1 and Am true then targetTemperature must ==  amTemperature else  targetTemperature must == pmTemperature
  if (StartUp && Am == true)
  {
    sprintf(targetTemperature, "%d", amTemperature);
    Serial.println("StartUp == true and Am == true");
    StartUp = false;
  }
  else if (StartUp && Am == false)
  {
    sprintf(targetTemperature, "%d", pmTemperature);
    Serial.println("StartUp == true and Am == false");
    StartUp = false;
  }
}

void handleWiFiConnection(const char *ssid, const char *password, bool &isConnected)
{
  static bool isStaticIPConfigured = false;

  // Configure static IP once
  if (!isStaticIPConfigured)
  {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
      Serial.println("Failed to configure Static IP");
    }
    else
    {
      Serial.println("Static IP configured");
    }
    isStaticIPConfigured = true; // Avoid reconfiguring every call
  }

  // Handle WiFi connection
  if (WiFi.status() == WL_CONNECTED && !isConnected)
  {
    Serial.println("Connecting to WiFi");
    digitalWrite(LED_BUILTIN, HIGH); // Turn LED on
    isConnected = true;              // Update connection status
  }
  else if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting");                           // Debug message
    WiFi.begin(ssid, password);                           // Attempt connection
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED state
    delay(1000);                                          // Wait before retrying
    isConnected = false;                                  // Update connection status
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
  /**************************
       DS18B20 Sensor
         Ends Here
  **************************/

  /*************************************************************
                              Heater Control
                                   start
  ************************************************************/
Serial.print("line 319 pmHours: ");
Serial.print(pmHours);
Serial.print("  pmMinutes: ");
Serial.println(pmMinutes);
Serial.print("line 323 Hours: ");
Serial.print(Hours);
Serial.print(" Minutes: ");
Serial.println(Minutes);

  if (Am)
  {
    if (amHours == Hours && amMinutes == Minutes)
    { // set amTemp for the Night time setting
      Serial.println("line 591 Am == true");
      AmFlag = true;
      amTemp = amTemperature;
      snprintf(targetTemperature, sizeof(targetTemperature), "%d", amTemp);
    }
  }
  else
  {
    if (pmHours == Hours && pmMinutes == Minutes)
    { // set pmTemp for the Night time setting
      AmFlag = false;
      pmTemp = pmTemperature;
      snprintf(targetTemperature, sizeof(targetTemperature), "%d", pmTemp);
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
  Serial.print("*********&&&&&&&&******* Message arrived [");
  Serial.println(topic);
  Serial.print("*********&&&&&&&&******* Message: "); // Print the message
  Serial.println((char *)payload);
  if (topic == nullptr || payload == nullptr)
  {
    Serial.println("Error: Null topic or payload");
    return;
  }
  // Null-terminate the payload to treat it as a string
  payload[length] = '\0';

  if (strstr(topic, "amTemperature"))
  {
    sscanf((char *)payload, "%d", &amTemperature);
    Serial.print("line 391 ******** amTemperature:******  ");
    Serial.println(String(amTemperature));
    if (StartUp == true)
    {
      amTemp = amTemperature;
      Serial.print("line 392 ******** ESP32amTemp:**********  ");
      Serial.println(String(amTemp));
    }
  }
  if (strstr(topic, "pmTemperature"))
  {
    sscanf((char *)payload, "%d", &pmTemperature);
    Serial.print("line 398 *********pmTemperature:********* ");
    Serial.println(String(pmTemperature));
    if (StartUp == true)
    {
      pmTemp = pmTemperature;
      StartUp = false;
    }
  }
  if (strstr(topic, "AMtime"))
  {
    sscanf((char *)payload, "%d:%d", &amHours, &amMinutes);
    Serial.print("line 405 ********* AMtime ********* ");
    Serial.println(String((char *)payload));
  }
  if (strstr(topic, "PMtime"))
  {
    sscanf((char *)payload, "%d:%d", &pmHours, &pmMinutes);
    Serial.print("line 410 ********** PMtime b******** ");
    Serial.println(String((char *)payload));
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
      client.subscribe("amTemperature");
      client.subscribe("pmTemperature");
      client.subscribe("AMtime");
      client.subscribe("PMtime");
      client.subscribe("HeaterStatus");
      client.subscribe("outSide");
      client.subscribe("coolSide");
      client.subscribe("heater");
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


/*************************************************************
                            Relay Control
                                 start
*************************************************************/

void relay_Control()
{
  int targetTemp = AmFlag ? amTemp : pmTemp;
  Serial.print("line 466 targetTemp: ");
  Serial.println(targetTemp);
  Serial.print("line 468 AmFlag: ");
  Serial.println(AmFlag);
  Serial.print("line 470 amTemp: ");
  Serial.println(amTemp);
  Serial.print("line 472 pmTemp: ");
  Serial.println(pmTemp); 
  Serial.print("line 474 RedSensor: ");
  Serial.println(RedSensor);
  Serial.print("line 476 targetTemp: ");
  Serial.println(targetTemp);
  if (RedSensor < targetTemp)
  {
    Serial.println("line 480 RedSensor < targetTemp");
    digitalWrite(Relay_Pin, HIGH);
    digitalWrite(LED_Pin, HIGH);    // LED_Pin on
    digitalWrite(LED_BUILTIN, HIGH); // LED_Pin on
    heaterStatus = true;
    if (!heaterOn)
    {
      startHeaterTimer();
    }
  }
  else if (RedSensor > targetTemp)
  {
    digitalWrite(Relay_Pin, LOW);
    digitalWrite(LED_Pin, LOW);      // LED_Pin off
    digitalWrite(LED_BUILTIN, LOW); // LED_Pin off
    heaterStatus = false;
    heaterOn = false;
  }
}
/*************************************************************
                            Relay Control
                                 End
*************************************************************/
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
    client.publish("heater", sensVal, true);
    prevS1 = RedSensor;
    Serial.print("line 352 ,,,,,, Publishing redSensor ,,,,,, ");
    Serial.println(RedSensor);
    Serial.print("line 353 ,,,,,, prevS1 ,,,,,, ");
    Serial.println(String(prevS1));
  }

  // publish greenSensor
  if (fabs(GreenSensor - prevS2) > threshold)
  {
    snprintf(sensVal, sizeof(sensVal), "%.2f", GreenSensor);
    client.publish("coolSide", sensVal, true);
    prevS2 = GreenSensor;
    Serial.println("line 362 Publishing greenSensor");
    Serial.print("line 363 ,,,,,, greenSensor ,,,,,,, ");
    Serial.println(String(sensVal));
  }
  // publish blueSensor
  if (fabs(BlueSensor - prevS3) > threshold)
  {

    snprintf(sensVal, sizeof(sensVal), "%.2f", BlueSensor);
    client.publish("outSide", sensVal, true);
    prevS3 = BlueSensor;
    Serial.print("line 366 ,,,,,,, Publishing blueSensor ,,,,,,,");
    Serial.println(BlueSensor);
  }

  // publish Hours
  if (Hours != prevHours)
  {
    Serial.println("line 375 ,,,,,, Publishing Hours ,,,,,, ");
    snprintf(sensVal, sizeof(sensVal), "%d", Hours);
    client.publish("gaugeHours", sensVal, true);
    prevHours = Hours;
  }

  // publish Minutes
  if (Minutes != prevMinutes)
  {
    Serial.println("line 384 ,,,,,,, Publishing Minutes ,,,,,,, ");
    snprintf(sensVal, sizeof(sensVal), "%d", Minutes);
    client.publish("gaugeMinutes", sensVal, true);
    prevMinutes = Minutes;
  }

  // publish Heater Status
  if (heaterStatus != prevHeaterStatus)
  {
    const char *heaterStatusStr = heaterStatus ? "true" : "false";
    client.publish("HeaterStatus", heaterStatusStr, true);
    prevHeaterStatus = heaterStatus;
  }

  char targetTemperatureStr[23];
  if (strcmp(targetTemperature, prevTargetTemperature) != 0)
  {
   
    snprintf(targetTemperatureStr, sizeof(targetTemperatureStr), "%s", targetTemperature);
    client.publish("TargetTemperature", targetTemperatureStr, true);
     Serial.println("line 464 Publishing target temperature");
    strcpy(prevTargetTemperature, targetTemperature); // Copy the new value to prevTargetTemperature
  }
}

/********************************************
         send temperature value
             to server for
          temperature monitor
                to receive
                   end
* ******************************************/


void startHeaterTimer()
{
  heaterOnTime = millis();
  heaterOn = true;
}

/********************************************
                Check Heater Timeout start
 ******************************************/

void checkHeaterTimeout()
{
  if (heaterOn && (millis() - heaterOnTime > heaterTimeout))
  {
    if (RedSensor < amTemp || RedSensor < pmTemp) // Check if the temperature is still below the threshold
    {
      client.publish("HeaterStatus", "Temperature did not rise within the expected time.");

      // Prepare the email subject and message
      String subject = "Heater Alert";
      String message = "Temperature did not rise within the expected time. The heater has been turned off.";

      // Send the email
      gmail_send(subject, message);
    }
    heaterOn = false;
  }
}

/********************************************
                Check Heater Timeout end
 ******************************************/

/********************************************
                Email send start
 ******************************************/

void gmail_send(String subject, String message)
{
  // set the network reconnection option
  MailClient.networkReconnect(true);

  smtp.debug(1);

  smtp.callback(smtpCallback);

  // set the session config
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = SENDER_EMAIL;
  config.login.password = "xhjh djyf roxm sxzh";
  config.login.user_domain = F("127.0.0.1");
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  // declare the message class
  SMTP_Message emailMessage;

  // set the message headers
  emailMessage.sender.name = "ESP8266 Heater App";
  emailMessage.sender.email = SENDER_EMAIL;
  emailMessage.subject = subject;
  emailMessage.addRecipient(F("To Whom It May Concern"), RECIPIENT_EMAIL);

  emailMessage.text.content = message;
  emailMessage.text.charSet = "utf-8";
  emailMessage.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  // set the custom message header
  emailMessage.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

  // connect to the server
  if (!smtp.connect(&config))
  {
    Serial.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn())
  {
    Serial.println("Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      Serial.println("Successfully logged in.");
    else
      Serial.println("Connected with no Auth.");
  }

  // start sending Email and close the session
  if (!MailClient.sendMail(&smtp, &emailMessage))
    Serial.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}

// callback function to get the Email sending status
void smtpCallback(SMTP_Status status)
{
  if (status.success())
  {
    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      // get the result item
      SMTP_Result result = smtp.sendingResult.getItem(i);
    }
    // free the memory
    smtp.sendingResult.clear();
  }
}
/********************************************
                Email send End
 ******************************************/