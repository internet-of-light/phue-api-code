/*
 * 
 * This code is ready for us to create custom patterns and accept sensor input. Try creating a pattern
 * using the examples of the testPattern functions at the bottom.
 * 
 * 
    Based on work by Petr Lukas
   @ https://maker.pro/arduino/projects/smart-home-automated-lights-with-esp8266-and-philips-hue-part-2
   and Ross Monroe for UW IOT DRG

   Info in function descriptions mostly sourced from https://developers.meethue.com/develop/hue-api/groupds-api/
   Please view test patterns in draw() loop

   Info from testing: Spamming the bridge with consecutive requests sometimes causes it to start
   throwing 901 Type Errors - may need very slight delay between api calls

   Benchmarking: 10 repetitions of testPattern7()
   Baud 9600 debug true        15.68s
   Baud 9600 debug false       3.75s
   Baud 115200 debug true      3.70s
   Baud 115200 debug false     3.86s

   Conclusion: Slow baud rate is bad if debug is on (duh)

   Index for LIGHTS starts at 1 - there is no light 0
   Index for GROUPS starts at 0 - group 0 is default group containing all of the bridge's lights
*/

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>


/***********************************/
/*****IMPORTANT DEFINITIONS*********/
/*****LOOK THRU THESE***************/

//Change to the number of lights connected to bridge
#define NUM_LIGHTS 4

#define SERIAL_BAUD_RATE 115200 //Serial rate - set your serial monitor to this number to get sensible output

/*Uncomment one of the lines below to switch which
   bridge you are connected to,
*/
#define BRIDGE "Lab Green"
//#define BRIDGE "Lab Blue"
//#define BRIDGE "Lab Red"
//#define BRIDGE "Sieg Master"


const char* ssid = "University of Washington"; //WIFI Network Name
const char* password = ""; //Add WIFI password

#define DEBUG true //Do you want messages printed to serial monitor?

/***********************************/
/****END IMPORTANT DEFINITIONS******/

//Use buttons to control lights
//Buttons currently unused
int button1Pin = 13;
int button1State = 0;

//Don't change these here, just change the definition above to switch bridges
//Look in setup() to see how these are assigned
String ip; //Hue Bridge IP Address
String api_token; //Hue Bridge Authentication api_token

/*
   Debugging Statement Functions
*/

//Easier than putting if(DEBUG) in front of every print statement
void dbprint(String in) {
  if (DEBUG) Serial.print(in);
}
void dbprintln(String in) {
  if (DEBUG) Serial.println(in);
}

/***********************************/
/********LIGHTING FUNCTIONS*********/

/* changeGroup: update a light group based on its index (groupNum).

   Up to 3 parameters can be changed in one function call
    - only the first parameter is required to run the function

   transitiontime must be specified:
      The duration of the transition from the light’s
      current state to the new state. This is given as
      a multiple of 100ms and defaults to 4 (400ms). For
      example, setting transitiontime:10 will make the
      transition last 1 second.

   Parameter options:
    "on" - true or false
    "bri" - 0 to 254
    "hue" . -both 0 and 65535 are red, 25500 is green and 46920 is blue
    "sat" - 254 is the most saturated (colored) and 0 is the least saturated (white)
    "effect" - The dynamic effect of the light, currently “none” and “colorloop” are supported. Other values will generate an error of type 7
    "xy" - [x,y]
    "ct" - Mired Color Temperature. Haven't used this.
    "alert" - The alert effect, which is a temporary change to the bulb’s state, and has one of the following values:
      -  “none” – The light is not performing an alert effect.
      -  “select” – The light is performing one breathe cycle.
      -  “lselect” – The light is performing breathe cycles for 15 seconds or until an "alert": "none"

    New values must be entered as string (include quote marks when calling this function)

    Examples:
      changeGroup(0, 0, "on", "true", "hue", "60000", "bri", "150");
        Change group 0, instantly, to state ON with HUE=60000 and BRIGHTNESS = 150
      changeGroup(2, 4, "hue", "25000");
        Change group 2, in 4/10ths of a second, to HUE=25000

    Group 0 is the default group containing all lights
    connected to the bridge. Other groups are created by user.
*/

void changeGroup(byte groupNum, byte transitiontime, String parameter, String newValue, String parameter2 = "",
                 String newValue2 = "", String parameter3 = "", String newValue3 = "") {

  String req_string = "http://" + ip + "/api/" + api_token + "/groups/" + groupNum + "/action";
  HTTPClient http;
  http.begin(req_string);

  String put_string = "{\"" + parameter + "\":" + newValue + ", \"transitiontime\": " +
                      transitiontime;
  if (!parameter2.equals("")) put_string += + ", \"" + parameter2 + "\": " + newValue2;
  if (!parameter3.equals("")) put_string += ", \"" + parameter3 + "\" : " + newValue3;
  put_string +=  + "}";

  dbprintln("Attempting PUT: " + put_string + " for GROUP: " + String(groupNum));

  int httpResponseCode = http.PUT(put_string);
  if (httpResponseCode == 200) {
    String response = http.getString();
    dbprintln("Response code: " + httpResponseCode);
    dbprintln("Response: " + response);
  } else {
    dbprint("Error on sending PUT Request: ");
    dbprintln(String(httpResponseCode));
  }
  http.end();
}

/* changeLight
   Update a single light based on its index (lightNum).
   transitiontime must be specified in deciseconds (tenths of a second)
   Parameter options:
    "on"
    "bri"
    "hue"
    "sat"
    "effect"
    "xy"
    "ct"
    "alert"

    Values must be entered as string (include quote marks when calling this function)
    
    Examples:
      changeLight(1, 0, "on", "false");
      changeLight(2, 4, "hue", "25000");
*/

void changeLight(byte lightNum, byte transitiontime, String parameter, String newValue, String parameter2 = "",
                 String newValue2 = "", String parameter3 = "", String newValue3 = "") {

  String req_string = "http://" + ip + "/api/" + api_token + "/lights/" + lightNum + "/state";
  HTTPClient http;
  http.begin(req_string);

  String put_string = "{\"" + parameter + "\":" + newValue + ", \"transitiontime\":" + transitiontime;
  if (!parameter2.equals("")) put_string += + ", \"" + parameter2 + "\": " + newValue2;
  if (!parameter3.equals("")) put_string += ", \"" + parameter3 + "\" : " + newValue3;
  put_string +=  + "}";

  dbprintln("Attempting PUT: " + put_string + " for LIGHT: " + String(lightNum));


  int httpResponseCode = http.PUT(put_string);
  if (httpResponseCode == 200) {
    String response = http.getString();
    dbprintln("Response code: " + httpResponseCode);
    dbprintln("Response: " + response);
  } else {
    dbprint("Error on sending PUT Request: ");
    dbprintln(String(httpResponseCode));
  }
  http.end();
}


//Simply toggle the on/off status of a light
//transitiontime must be specified
void toggleLight(byte lightNum, byte transitiontime) {
  bool newStatus = !checkLightStatus(lightNum);
  changeLight(lightNum, transitiontime, "on", newStatus ? "true" : "false");
}


//Return true/false when given light is on/off
bool checkLightStatus(byte lightNum) {
  dbprintln("Checking status of light " + String(lightNum));
  String req_string = "http://" + ip + "/api/" + api_token + "/lights/" + lightNum;
  HTTPClient http;
  http.begin(req_string);

  //char* get_string = "{\"on\": true}";
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    DynamicJsonBuffer jsonBuffer;
    String payload = http.getString();
    http.end();
    JsonObject& root = jsonBuffer.parse(payload);
    dbprintln("Response code" + String(httpResponseCode));
    //dbprintln("Payload: " + payload);
    String lightStatus = (const char*)root["state"]["on"];
    dbprintln("ON Status of light " + String(lightNum) + ": " + lightStatus);
    return lightStatus.equals("true");
  } else {
    dbprint("Error on sending GET Request: ");
    dbprintln(String(httpResponseCode));
    return false;
  }
}

void setup() {

  if (BRIDGE == "Lab Green") {
    ip = "172.28.219.225"; //Lab Green
    api_token = "lxjNgzhDhd0X-qhgM4lsgakORvWFZPKK70pE0Fja"; //Lab Green
  }
  else if (BRIDGE == "Lab Blue") {
    ip = "172.28.219.235"; //Lab Blue
    api_token = "qn41nLuAOgrvfOAPeNQCYB6qannsoS8uDtyBJtMc"; //Lab Blue
  }
  else if (BRIDGE == "Lab Red") {
    ip = "172.28.219.189"; //Lab Red
    api_token = "Lht3HgITYDN-96UYm5mkJ4CEjKj20d3siYidcSq-"; //Lab Red
  }
  else if (BRIDGE == "Sieg Master") {
    ip = "172.28.219.179"; //Sieg Master
    api_token = "rARKEpLebwXuW01cNVvQbnDEkd2bd56Nj-hpTETB"; //Sieg Master
  }

  Serial.begin(SERIAL_BAUD_RATE); //Serial output to computer
  WiFi.begin(ssid, password);
  pinMode(button1Pin, INPUT);
  dbprint("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    dbprint(".");
  }
  dbprintln("Connected to WiFi: " + String(ssid));
}

//Try these out
void loop() {
  //testPattern1();
  //testPattern2();
  //testPattern3();
  //testPattern4();
  //testPattern5();
  testPattern6();
  //testPattern7();
}

//Jump thru 7 colors on each light consecutively
void testPattern1() {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    changeLight(i, 0, "hue", "0", "on", "true", "bri", "200"); //Red
    delay(500);
    changeLight(i, 0, "hue", "10000"); //Yellow
    delay(500);
    changeLight(i, 0, "hue", "20000"); //Green
    delay(500);
    changeLight(i, 0, "hue", "30000"); //Blue / Cyan
    delay(500);
    changeLight(i, 0, "hue", "40000"); //Light Blue
    delay(500);
    changeLight(i, 0, "hue", "50000"); //Purple
    delay(500);
    changeLight(i, 0, "hue", "60000"); //Red
    delay(500);
  }
}

//Test how quickly we can push changes. Go from hue 0 to 65000 on each light consecutively
//First test, 10000 steps (going to 60000 by increments of 6) way way too slow
//Second test (this versio) - 100 steps (going t o 65000 by
//    increments of 650) = decent speed. Loops thru all colors in about 15 seconds
void testPattern2() {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    for (int h = 0; h < 100; h++) {
      changeLight(i, 0, "hue", String(h * 650));
    }
  }
}

//Color loop through Group 0 (all lights, default group) in 25 steps
//Sometimes: {"error":{"type":901,"address":"/groups/0/action/bri","description":"Internal error, 404"}}
//I think this comes from spamming API calls too quickly
void testPattern3() {
  for (int h = 0; h < 25; h++) {
    changeGroup(0, 5, "hue", String(h * 650 * 4), "bri", "100");
    delay(500);
  }
}

//Test toggleLight function
void testPattern4() {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    toggleLight(i, 0);
    delay(200);
    toggleLight(i, 0);
    //delay(1000);
  }
}

//Send purple flash through green lights, no transition time (a little choppy)
void testPattern5() {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    changeLight(i, 0, "hue", "60000", "bri", "200");
    delay(200);
    changeLight(i, 0, "hue", "20000");
  }
}

//Send blue flash through yellow lights, .2 second transition time, looks pretty nice
void testPattern6() {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    changeLight(i, 2, "hue", "45000", "bri", "200");
    delay(200);
    changeLight(i, 2, "hue", "10000");
  }
}

//Send red flash through green lights, .2 second transition time with no delay
//Fastest possible speed. Looks good at BAUD=9600 w/ debug on, too fast at BAUD = 115200
void testPattern7() {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    changeLight(i, 2, "hue", "0", "bri", "200");
    changeLight(i, 2, "hue", "30000");
  }
}
