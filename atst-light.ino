
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>
#define PIN      5 
#define N_LEDS 1

const char* ssid = "WLAN_SSID";
const char* password = "WLAN_PASSWORD";



AsyncWebServer server(80);
const char* PARAM_STRING = "inputString";
const char* PARAM_INT = "inputInt";
const char* PARAM_FLOAT = "inputFloat";



// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head><title>ESP Input Form</title><meta name="viewport" content="width=device-width, initial-scale=1"></head><body><h1>RAINBOW</h1><h3>RED</h3><form action="/get"><table><tr><td><label for="volume">Frequency</label></td><td><input type="range" name="r_speed" min="0" max="1000"></td></tr><tr><td><label for="volume">Amplitude</label></td><td><input type="range" name="r_amp" min="0" max="255"></td></tr><tr><td><label for="volume">Offset</label></td><td><input type="range" name="r_offset" min="-1000" max="1000"></td></tr></table><h3>GREEN</h3><table><tr><td><label for="volume">Frequency</label></td><td><input type="range" name="g_speed" min="0" max="1000"></td></tr><tr><td><label for="volume">Amplitude</label></td><td><input type="range" name="g_amp" min="0" max="255"></td></tr><tr><td><label for="volume">Offset</label></td><td><input type="range" name="g_offset" min="-1000" max="1000"></td></tr></table><h3>BLUE</h3><table><tr><td><label for="volume">Frequency</label></td><td><input type="range" name="b_speed" min="0" max="1000"></td></tr><tr><td><label for="volume">Amplitude</label></td><td><input type="range" name="b_amp" min="0" max="255"></td></tr><tr><td><label for="volume">Offset</label></td><td><input type="range" name="b_offset" min="-1000" max="1000"></td></tr></table><h3>PULSE</h3><table><tr><td><label for="volume">Frequency</label></td><td><input type="range" name="p_speed" min="0" max="1000"></td></tr><tr><td><label for="volume">Amplitude</label></td><td><input type="range" name="p_amp" min="0" max="255"></td></tr><tr><td><label for="volume">Offset</label></td><td><input type="range" name="p_offset" min="-1000" max="1000"></td></tr></table><br><input type="submit" value="Submit"></form></body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "inputString"){
    return readFile(SPIFFS, "/inputString.txt");
  }
  else if(var == "inputInt"){
    return readFile(SPIFFS, "/inputInt.txt");
  }
  else if(var == "inputFloat"){
    return readFile(SPIFFS, "/inputFloat.txt");
  }
  return String();
}



Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(57600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      writeFile(SPIFFS, "/inputString.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(PARAM_INT)) {
      inputMessage = request->getParam(PARAM_INT)->value();
      writeFile(SPIFFS, "/inputInt.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_FLOAT)) {
      inputMessage = request->getParam(PARAM_FLOAT)->value();
      writeFile(SPIFFS, "/inputFloat.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
  
  
  strip.begin();
}

void loop() {
  float t = millis()*0.01;
  float r = sin(t * 0.01 + 124.248) + 1. ;
  float g = sin(t * 0.00317 + 52.523) + 1. ;
  float b = sin(t * 0.00125 + 22.912) + 1. ;
  
  
  
  chase(strip.Color(r*127, g*127, b*127)); // Red
}

static void chase(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
      strip.setPixelColor(i  , c); // Draw new pixel
      //strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
      strip.show();
      delay(5);
  }
}
