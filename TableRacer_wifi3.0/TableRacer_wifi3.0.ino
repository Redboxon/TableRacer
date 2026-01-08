// make sure to upload with ESP32 Dev Module selected as the board under tools>Board>ESP32 Arduino

#include <Arduino.h>
#include <ESP32Servo.h> // by Kevin Harrington
#include <ESPAsyncWebSrv.h> // by dvarrel
#include <iostream>
#include <sstream>

#if defined(ESP32)
#include <AsyncTCP.h> // by dvarrel
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h> // by dvarrel
#endif

// defines

#define steeringServoPin 23
#define throttleServoPin 22
#define auxAttach0 26  // currently unused, will be implemented in future


// global constants
extern const char* htmlHomePage PROGMEM;
//=========================================================================================
const char* ssid = "TableRacer_1"; //this Value will change the name of you ESP32's network
//=========================================================================================
const int SteeringCenter = 86; 
volatile int targetSteering = SteeringCenter;
volatile int targetThrottle = SteeringCenter;
//Create Servo Objects
Servo steeringServo;
Servo throttleServo;
//Trims
int steeringTrim = 0; //change this value in case your racer is not going straight smaller nuber corrects to the right, larger to the left. DO NOT USE THE SteeringCenter variable for this
int throttleTrim = -4;
//Global Slider Variables
float throttleServoValue = SteeringCenter;
float steeringServoValue = SteeringCenter;
//Light variables
int lightTimer = 0;
bool lightIsOn = false;
//Webserver
AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");
unsigned long lastControlUpdate = 0;

void steeringControl(int steeringValue)
{
  steeringServoValue = 172- steeringValue - steeringTrim;
  if(steeringServoValue < 40)
  {
    steeringServoValue = 40;
  }
  else if(steeringServoValue > 132)
  {
    steeringServoValue = 132;
  }
  steeringServo.write(steeringServoValue);
}

void throttleControl(int throttleValue)
{
  throttleServoValue = throttleValue - throttleTrim;
  throttleServo.write(throttleServoValue);
}

void lightControl()
{
  if ((millis() - lightTimer) > 200) {
    if (lightIsOn) {
      digitalWrite(auxAttach0, LOW);
      lightIsOn = false;
    } else {
      digitalWrite(auxAttach0, HIGH);
      lightIsOn = true;
    }

    lightTimer = millis();
  }
}

void handleRoot(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str());
        int valueInt = atoi(value.c_str());
        if (key == "steering")
        {
          targetSteering = valueInt;
        }
        else if (key == "throttle")
        {
          targetThrottle = valueInt;
        }
        else if (key == "light")
        {
          lightControl();
        }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}

void setUpPinModes()
{
  steeringServo.attach(steeringServoPin);
  throttleServo.attach(throttleServoPin);
  steeringControl(steeringServoValue);
  throttleControl(throttleServoValue);
}

void setup(void)
{
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid );
  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP is ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);
  
  server.begin();
  Serial.println("Server online");
  Serial.end();
}

void loop()
{
  wsCarInput.cleanupClients();
  if(millis() - lastControlUpdate >= 20){ //50Hz
  steeringControl(targetSteering);
  throttleControl(targetThrottle);
  lastControlUpdate = millis();
  }
}
