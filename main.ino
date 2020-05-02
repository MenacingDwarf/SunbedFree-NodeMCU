#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EasyButton.h>

#define BUTTON_PIN 0

// WiFi connection settings
const char *ssid = "FamilyRoom";
const char *password = "wilddoghere";
 
//Web/Server address to read/write from 
String host = "193.187.175.147:8082";

void WiFiConnect() {
  WiFi.disconnect();
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("NodeMcu connected to wifi...");
}

void SendStatus(int buttonStatus) {
  HTTPClient http;
 
  int x, y;
  String type, postData;

  // Board coordinates on map of sunbeds
  x = 1;
  y = 2;

  // Specify type of action according to buttonStatus
  if (buttonStatus == 0) {
    type = "\"start\"";
  } else {
    type = "\"end\"";
  }
 
  // Post Data in json format
  postData = "{\"x\": " + String(x) + ", \"y\": " + String(y) + ", \"type\": " + type + "}";
  
  http.begin("http://" + host + "/api/v1/event/");  //Specify request destination
  http.addHeader("Content-Type", "application/json");    //Specify content-type header
  http.addHeader("accept", "application/json");    //Specify accept header
 
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
 
  Serial.println(httpCode);
  Serial.println(payload);
 
  http.end();  //Close connection
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);
  Serial.println("");
  Serial.println("Wake up!");
  delay(1000);

  // Read current button status
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  int buttonStatus = digitalRead(BUTTON_PIN);

  // Read previous button status from rtc memory
  byte prevButtonStatus[1];
  system_rtc_mem_read(64, prevButtonStatus, 1);

  // Print statuses
  Serial.println("Current status: " + String(buttonStatus));
  Serial.println("Prev status: " + String(prevButtonStatus[0]));

  // If status has changed need to send data to server
  if (buttonStatus != prevButtonStatus[0]) {
    WiFiConnect();
    SendStatus(buttonStatus);
  }

  // Save current status into rtc memory for the next wake up
  prevButtonStatus[0] = buttonStatus;
  system_rtc_mem_write(64, prevButtonStatus, 1);
  
  while (!Serial) { }
  Serial.println("Deep sleep for 10sec");
  ESP.deepSleep(10e6);
}

void loop() {
}
