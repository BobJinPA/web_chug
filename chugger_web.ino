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
#define NOSTART 0
#define FAULT 1
#define COMPLETE 2
#define INFO 3
int brightness = 150;
int forceReading;
//ESP8266WebServer server ( 80 );
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NeoPIN, NEO_RGB + NEO_KHZ800);
bool sequence = false;
int duration = 200;
String macAddress;
const String chug_server = "http://192.168.1.161:5000";
unsigned long start_sequence_timer;
unsigned long drink_up_time;
unsigned long drink_down_time;
 
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

bool pad_load(){
  delay(1); // get rid of this later. for now, needed to keep chip from flipping out
  forceReading = analogRead(forcePin);
  if (forceReading > threshold){
    return true;
  } else {
    return false;  
  }
}

void set_all_lights(int r,int g,int b){
  for(int i=0; i<NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(g, r, b)); 
  } 
  strip.show();
}

void fault_sequence(){ // this can be blocking. No timing and web post alread happened
  int count = 0;
  int led;
  do {
    for(int i=0; i<NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(0, 10, 0)); 
    } 
    led = count % 4;
    if (led == 3) {led = 1;}
    strip.setPixelColor(led, strip.Color(0, 255, 0)); 
    strip.show();
    count +=1; 
    delay(duration/4);
  } while (count < 70);
}

void winning_sequence(){ // this can be blocking. No timing and web post alread happened
  int count = 0;
  int led;
  do {
    for(int i=0; i<NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(10, 0, 0)); 
    } 
    led = count % 4;
    if (led == 3) {led = 1;}
    strip.setPixelColor(led, strip.Color(255, 0, 0)); 
    strip.show();
    count +=1; 
    delay(duration/5);
    } while (count < 80);
}

bool send_message(int messageType, String messageText){
  // send to server
  Serial.println("GET call from " + macAddress + ", " + String(messageType) + ", " + messageText);
  return true;
}

bool check_for_contest(){
  // add call here for the server to see if a game is happening
  // for now, just default to true
  delay (1000);
  return true;
}

bool check_for_drink(){
  Serial.println("check_for_drink()");
  bool ready = false;
  unsigned long timer = millis();
  do {
    if (pad_load()==true){
      ready = true;
      set_all_lights(102,204,0);
    } else {
      ready = false;
      if ((millis()-timer) % ((duration * 2))< (duration * 2)/2){
        set_all_lights(255, 255, 0);
      } else {
        set_all_lights(0,0,0);
      } 
    }
    } while ((millis()-timer)< 10000);
    
  Serial.println("done checking for drinks");
  if (ready==false){
    set_all_lights(255, 128, 0);
    }
  return ready;
}

bool start_race_sequence(){
  String color = "amber";
  String state = "on";
  unsigned long flashCount = 0;
  set_all_lights(0,0,0); //default all lights off
  Serial.println("start_race_sequence()");
  unsigned long timer = millis();
  do {
    if (pad_load()==true){
      unsigned long timer_diff = millis();
      if (timer_diff-timer >= (duration*3)) {
        if (color=="amber" && state=="on" && flashCount <= 2) {
          set_all_lights(155, 210, 0);
          //set_all_lights(128,70,128);
          state = "off";
        } else if (color=="amber" && state=="off" && flashCount <= 2) {
          set_all_lights(0,0,0);
          state = "on";
          flashCount += 1;
        } else if (flashCount >= 2) {
          set_all_lights(0,255,0);
          Serial.println("GREEN");
          Serial.println("done with start sequence");
          return true;
        }
        timer = timer_diff;
      }
    } else {
      return false; //
    }
  } while (true);
}

bool check_for_drink_up(){
  // to do: add timeout logic that returns false when time exceeded
  do {} while (pad_load()==true);
  drink_up_time = millis();
  return true;
}

bool check_for_drink_down(){
  // to do: add timeout logic that returns false when time exceeded
  do {} while (pad_load()==false);
  drink_down_time = millis();
  return true;
}

void setup(void) 
{
  strip.setBrightness(brightness);
  strip.begin();
  set_all_lights(204, 255, 255);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200); 
  Serial.println();
  Serial.println("NeoPixel started");
  WiFiManager wm;
  //wm.resetSettings(); //reset settings - wipe credentials for testing
  bool res;
  res = wm.autoConnect("Chugger"); // no password
  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } 
  else {       
    Serial.println("connected"); //if you get here you have connected to the WiFi  
  }
  set_all_lights(0, 128, 255);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  macAddress = WiFi.macAddress();
  Serial.println(macAddress);
}

void loop(){
  Serial.println("check server for game");
  if (check_for_contest()==true){
    set_all_lights(255, 0, 255);
    Serial.println("checking for drink on pad");
    if (check_for_drink() == true) {
      Serial.println("drink on pad, starting the drag light sequence");
      if (start_race_sequence() == true){
        Serial.println("Start timing");
        start_sequence_timer = millis();
        if (check_for_drink_up() == true){
          Serial.println("drink is up off pad");
          Serial.println("reaction time: " + String(drink_up_time - start_sequence_timer) );
          if (check_for_drink_down()==true){
            Serial.println("drink is back on pad");
            Serial.println("chug time: " + String(drink_down_time - start_sequence_timer) );
            send_message(INFO, String(drink_up_time - start_sequence_timer));
            bool response = send_message(COMPLETE, String(drink_down_time - start_sequence_timer));
            if (response == true) {
              winning_sequence();
            } else {
              fault_sequence();
            }
          }
        } 
      } else {
        Serial.println("false start");
        send_message(FAULT, "Fault. Play jump started");
        fault_sequence();
      }
    }
  } else {
    send_message(NOSTART, "No drink for this game");
    Serial.println("No drink for this game");
    delay (1000);
  } 
}
