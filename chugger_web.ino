//set up actions for all routes including the race sequence

#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h> 
#include <DNSServer.h>  
#include <ESP8266WebServer.h> 
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#define NeoPIN D5
#define forcePin A0
#define NUM_LEDS 3
#define threshold 15
int brightness = 150;
int forceReading;
//ESP8266WebServer server ( 80 );
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NeoPIN, NEO_RGB + NEO_KHZ800);
bool sequence = false;
unsigned long start_sequence_timer;
int duration = 200;
const String chug_server = "http://192.168.1.161:5000";

void connect_color(){
  for(int i=0; i<NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(211, 255, 0)); 
  } 
  strip.show(); 
 }


// just a red white blue sequence for fun. might delete later
void sequence_lights(){
  long diff;
  int r,g,b;
  int lite;
  unsigned long current = millis();
  diff = millis() - start_sequence_timer;

  if (diff/duration < 1){
    r = 255;
    g = 0;
    b = 0;
    lite = 0;
  } else if (diff/duration < 2) {
    r = 85;
    g = 85;
    b = 85;
    lite = 1; 
  } else if (diff/duration < 3){
    r = 0;
    g = 0;
    b = 255;
    lite = 2;
  } else {
   start_sequence_timer = millis(); 
   sequence_lights();
  }
 
  for(int i=0; i<NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); 
  } 
  strip.setPixelColor(lite, strip.Color(g, r, b)); 
  strip.show(); 
}

void handleReady() {
  Serial.println("Ready");
  // blue lights in a sparkling pattern to indicate a chug
  }
void handleStart() {
  Serial.println("Start");
  // start light sequence, false start and timer. Send result to chug_server
  // do not return control to loop() until done
  }
void handleResultWin() {
  Serial.println("Win");
  // flash green or something
  }
void handleResultLose() {
  Serial.println("Lose");
  // flash some lower sequennce
}
void handleNotFound(){
  Serial.println("Not Found");
  //server.send ( 404, "text/plain", "Unexpected request" );
}
 
void setup(void) 
{
  WiFi.mode(WIFI_STA);
  Serial.begin(115200); 
  Serial.println();
  strip.setBrightness(brightness);
  strip.begin();
  Serial.println("NeoPixel started");
  WiFiManager wm;
  //wm.resetSettings(); //reset settings - wipe credentials for testing
  bool res;
  connect_color();
  res = wm.autoConnect("Chugger"); // no password
  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } 
  else {       
    Serial.println("connected"); //if you get here you have connected to the WiFi  
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  //server.on ( "/ready", handleReady );
  //server.on ( "/start", handleStart );
  //server.on ( "/result/win", handleResultWin );
  //server.on ( "/result/lose", handleResultLose );
  //server.onNotFound ( handleNotFound );
  //server.begin();
}

String httpGETRequest(String serverName) {
  HTTPClient http;
  Serial.print("Making http GET request to ");
  Serial.println(serverName);
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName.c_str());
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void test_get() { // delete this after a while
  String jsonBuffer;
  jsonBuffer = httpGETRequest(chug_server.c_str()); 
    Serial.println(jsonBuffer);
  JSONVar myObject = JSON.parse(jsonBuffer);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  } 
}

void loop() {
  String ipAddr;
  ipAddr = httpGETRequest("http://ifconfig.me");
  Serial.println("asked the internet for my IP");
  Serial.println(ipAddr);
  delay(30000);
}

void loop_() 
{
  forceReading = analogRead(forcePin);

  if (forceReading < threshold){
    for(int i=0; i<NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(0, 10, 0)); 
    } 
    strip.show();
    sequence==false;
   } else { // cup on

   if (sequence==false){
    start_sequence_timer = millis();
    sequence = true;
    }
    sequence_lights();
  }
}
