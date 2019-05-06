/* 1- Definitions and Information


    Based on work by Petr Lukas
   @ https://maker.pro/arduino/projects/smart-home-automated-lights-with-esp8266-and-philips-hue-part-2
   and Ross Monroe for UW IOT DRG

   Info in function descriptions mostly sourced from https://developers.meethue.com/develop/hue-api/groupds-api/
   Please view test patterns in draw() loop

   Info from testing: Spamming the bridge with consecutive requests causing it to start
   throwing 901 Type Errors - may need very slight delay between api calls

   Benchmarking: 10 repetitions of testPattern7()
   Baud 9600 debug true        15.68s
   Baud 9600 debug false       3.75s
   Baud 115200 debug true      3.70s
   Baud 115200 debug false     3.86s

   Conclusion: Slow baud rate is bad if debug on (duh)
*/

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

unsigned long currentTime;

int buttonStates[] = {0, 0, 0};
int buttonToggles[] = {0, 0, 0};
int buttonPins[] = {12, 13, 14};



/****SPIETH HOME***:*/

#define BRIDGE "Spieth Home"


// My Network (Spieth)
// If I accidentally posted code with this in here, whoops
const char* ssid = ""; //WIFI Network Name
const char* password = ""; //Add WIFI password  

/*****************/

/***********************************/
/*****IMPORTANT DEFINITIONS*********/
/*****LOOK THRU THESE***************/

//Change to the number of lights connected to bridge
#define NUM_LIGHTS 4

#define SERIAL_BAUD_RATE 115200 //Serial rate - set your serial monitor to this number to get sensible output

/*Uncomment one of the lines below to switch which
   bridge you are connected to,
*/
//#define BRIDGE "Lab Green"
//#define BRIDGE "Lab Blue"
//#define BRIDGE "Lab Red"
//#define BRIDGE "Sieg Master"



//const char* ssid = "University of Washington"; //WIFI Network Name
//const char* password = ""; //Add WIFI password



#define DEBUG true //Do you want messages printed to serial monitor?

/***********************************/
/****END IMPORTANT DEFINITIONS******/



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
    http.end();
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
  else if (BRIDGE == "Spieth Home") {
    ip = "10.0.1.16"; //Home
    api_token = "efzstVYGsi1LQDdNF2N4GoR4pSBjCPOpGOX-qK1e"; //Home
  }

  Serial.begin(SERIAL_BAUD_RATE); //Serial output to computer
  WiFi.begin(ssid, password);
  //pinMode(button1Pin, INPUT);
  dbprint("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    dbprint(".");
  }
  dbprintln("");
  dbprintln("Connected to WiFi: " + String(ssid));

  for (int pin : buttonPins) {
    pinMode(pin, INPUT);
  }
}


#define NUM_FLASHES 4

int button_push_test_timer = 0;
bool dir_toggle = 0;

void loop() {
  //Do something for each button if it is pressed
  //Only on initial press - not while button is held down
  updateButtonStates();

  currentTime = millis();

  if (currentTime > button_push_test_timer) {
    buttonPressed(dir_toggle);
    dir_toggle = !dir_toggle;
    button_push_test_timer = currentTime + 5000;
  }

  updateFlashes();

  //  testPattern1();
  //  testPattern2();
  //  testPattern3();
  //  testPattern4();
  //  testPattern5();
  //  testPattern6();
  //  testPattern7();
}

void updateButtonStates() {
  for (int i = 0; i < 3; i++) {
    buttonStates[i] = digitalRead(buttonPins[i]);
    if (buttonToggles[i] and !buttonStates[i]) {
      buttonToggles[i] = 0;
    }
    if (buttonStates[i] and !buttonToggles[i]) {
      buttonPressed(i);
      buttonToggles[i] = 1;
    }
  }
}


void buttonPressed(int buttonNum) {
  dbprintln("Button " + String(buttonNum) + " pressed.");
  switch (buttonNum) {
    case 0:
      activateFlash(1, "50000", "20000"); //Start a purple pulse of light on a green background moving forward
      break;
    case 1:
      activateFlash(0, "50000", "20000"); //Start a purple pulse of light on a green background moving backward
      break;
  }
}
