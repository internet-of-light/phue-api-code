/*  Sam Spieth

    For IOL DRG 5/13/19

    Demo of button remote prototype for wall mounting

    Blue and Green buttons light flash the corresponding color
    and fade back to warm white

    Hue API framework based on work by Petr Lukas
   @ https://maker.pro/arduino/projects/smart-home-automated-lights-with-esp8266-and-philips-hue-part-2
   and Ross Monroe for UW IOT DRG

    CONTENTS
    Tab 1 (This tab)
      Definitions - set number of buttons, button pins, number of lights, bridge
      Debug print functions
      Hue API Function headers - function bodies in Tab 2
      Setup - Initialize variables
      buttonPressed - actions when buttons are pressed
      updateButtonStates - check if buttons are presse
      Draw - just checking buttons in this sketch

    Tab 2
      Hue API Funcion bodies

*/

//Libraries used in this project
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>


/*****IMPORTANT DEFINITIONS*********/


#define NUM_BUTTONS 2
byte buttonPins[] = {4, 5};

//Light indexes for Sieg lower floor
int LOWER_FLOOR_LIGHT_SPIRAL[] = {22, 15, 10, 23, 11, 14, 16, 21, 7};


//Change to the number of lights connected to bridge
//#define NUM_LIGHTS 26
#define NUM_LIGHTS 9

#define SERIAL_BAUD_RATE 115200 //Serial rate - set your serial monitor to this number to get sensible output

/*Uncomment one of the lines below to switch which
   bridge you are connected to,
*/
//#define BRIDGE "Lab Green"
//#define BRIDGE "Lab Blue"
//#define BRIDGE "Lab Red"
#define BRIDGE "Sieg Master"



const char* ssid = "University of Washington"; //WIFI Network Name
const char* password = ""; //No password for UW Wifi - leave as ""



#define DEBUG true //Do you want messages printed to serial monitor?

/****END IMPORTANT DEFINITIONS******/

//Don't change these here, just change the definition above to switch bridges
//Look in setup() to see how these are assigned
String ip; //Hue Bridge IP Address
String api_token; //Hue Bridge Authentication api_token

//Tracking buttons and debouncing
byte buttonStates[NUM_BUTTONS], buttonToggles[NUM_BUTTONS];
unsigned long buttonLastUpdates[NUM_BUTTONS];

//Tracking time for debouncing and timed functions
unsigned long currentTime;

unsigned long blueLightTimer, greenLightTimer, lastLightUpdate;
bool button_state_changed_since_last_draw_loop; //The lights are pushed new commands every 500 ms. This boolean tells the program
// to push an update immediately instead if one of the buttons was pushed. This was implemented because
// writing new API calls every draw() loop was causing the bridge to throw error 901s again (Internal 404)

bool lightOffset; //Make dual color lights shift/rotate
/*
   Debugging Print Functions
   Easier than putting if(DEBUG) in front of every print statement
*/
void dbprint(String in) {
  if (DEBUG) Serial.print(in);
}
void dbprintln(String in) {
  if (DEBUG) Serial.println(in);
}

/***********************************/
/********LIGHTING FUNCTIONS*********/

/* changeGroup: update a light group based on its index (groupNum).

   Up to 4 parameters can be changed in one function call
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
                 String newValue2 = "", String parameter3 = "", String newValue3 = "",
                 String parameter4 = "", String newValue4 = "");

/* changeLight
   Update a single light based on its index (lightNum)

   Up to 4 parameters can be changed in one function call
    - only the first parameter is required to run the function

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
                 String newValue2 = "", String parameter3 = "", String newValue3 = "",
                 String parameter4 = "", String newValue4 = "");


/*  checkLightStatus
    returns true or false (1 or 0)
    checks if light is on or off
    parameter: # of light
*/
bool checkLightStatus(byte lightNum);


/*  toggleLight
    Simply toggle the on/off status of a light
    transitiontime must be specified
    parameter: # of light
*/
void toggleLight(byte lightNum, byte transitiontime) {
  bool newStatus = !checkLightStatus(lightNum);
  changeLight(lightNum, transitiontime, "on", newStatus ? "true" : "false");
}



/*  setup
    runs once at beginning
    initialize variables
    assign IP and Hue API token depending on Bridge definition
*/
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

  WiFi.begin(ssid, password); //Connect to WiFi Network
  dbprintln("Connecting to WiFi: " + String(ssid));
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    dbprint(".");
  }
  dbprintln("");
  dbprintln("Connected to WiFi: " + String(ssid));

  //Initialize button arrays
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttonStates[i] = 0; //Current state of button: 1=pressed, 0=not pressed
    buttonToggles[i] = 0; //For debouncing
    buttonLastUpdates[i] = 0; //For debouncing
  }

  blueLightTimer = 0;
  greenLightTimer = 0;
  lastLightUpdate = 0;
  button_state_changed_since_last_draw_loop = false;
  lightOffset = false;
}



/*  buttonPressed
    Called when a button is pressed by the updateButtonStates() function
    Defines an action for each button
*/
void buttonPressed(int buttonNum) {
  dbprintln("Button " + String(buttonNum) + " pressed.");
  switch (buttonNum) {
    case 0: //Green Button
      /*changeGroup(0, 5, "on", "true", "bri", "254", "hue", "24000", "sat", "254");
        delay(2000);
        changeGroup(0, 20, "on", "true", "bri", "180", "hue", "10000");
      */
      greenLightTimer = currentTime;
      button_state_changed_since_last_draw_loop = true;
      break;
    case 1: //Blue Button
      /*changeGroup(0, 5, "on", "true", "bri", "254", "hue", "44000", "sat", "254");
        delay(2000);
        changeGroup(0, 20, "on", "true", "bri", "180", "hue", "10000");
      */
      blueLightTimer = currentTime;
      button_state_changed_since_last_draw_loop = true;
      break;
  }
}


/*  updateButtonStates
    Called each draw loop
    Checks if each button is pressed
    uses the "buttonToggles" and "buttonLastUpdates" arrays to avoid debouncing issues
*/
void updateButtonStates() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonStates[i] = !digitalRead(buttonPins[i]); //Invert to get 1=pressed and 0=not pressed
    if (buttonToggles[i] and !buttonStates[i]) {
      buttonToggles[i] = 0;
    }
    if (buttonStates[i] and !buttonToggles[i] and currentTime - buttonLastUpdates[i] > 50) {
      buttonPressed(i);
      buttonToggles[i] = 1;
      buttonLastUpdates[i] = currentTime;
    }
  }
}

/*  loop
    contains code to run every cycle
    check buttons and do actions if buttons are pressed
*/
void loop() {
  currentTime = millis();
  updateButtonStates();

  greenBlueButtonResponse();
}


#define BUTTON_PUSH_DURATION 5000
/*  greenBlueButtonResponse
    The main feature of this demo (5/12)
    blueLightActive and greenLightActive booleans check if the corresponding buttons
    have been pushed in the last BUTTON_PUSH_DURATION milliseconds

    If only green or blue buttons, lights go that color
    if both, moving alternate pattern of green/blue with some purple randomly added
    if neither, slow fade to warm white
*/
//
//
void greenBlueButtonResponse() {
  
  bool blueLightActive = currentTime - blueLightTimer < BUTTON_PUSH_DURATION;
  bool greenLightActive = currentTime - greenLightTimer < BUTTON_PUSH_DURATION;

  //Push the commands to the bridge every 3 seconds to make sure everything goes through
  //but avoid overloading the bridge. Twice a second was too fast
  //Commands are also sent immediately when a button is pressed
  
  if (currentTime - lastLightUpdate > 2000 or button_state_changed_since_last_draw_loop) {
    if (blueLightActive and !greenLightActive) { //If blue and not green
      changeGroup(0, 10, "on", "true", "bri", "254", "hue", "44000", "sat", "250"); //Fade to blue

    } else if (greenLightActive and !blueLightActive) { //If green and not blue
      changeGroup(0, 10, "on", "true", "bri", "254", "hue", "24000", "sat", "250"); //Fade to green

    } else if (greenLightActive and blueLightActive) { //If both
      for (int i = 0; i < NUM_LIGHTS; i++) { //Alternate between blue and green
        //Pick a color
        //Make the blue and green move using lightOffset
        String color = i % 2 == 0 ? lightOffset ? "24000" : "44000" : lightOffset ? "44000" : "24000"; //44000 = blue, 24000 = green
        if(random(0,20) < 1) color = "54000"; //Randomly throw in some purple; 54000 = purple
        
        //Pick the correct light index
        int lightNum = BRIDGE == "Sieg Master" ? LOWER_FLOOR_LIGHT_SPIRAL[i] : i + 1;\

        //Change the light
        changeLight(lightNum, 10, "on", "true", "bri", "254", "hue", color, "sat", "200");
      }

    } else { //If neither blue or green
      changeGroup(0, 30, "on", "true", "bri", "180", "hue", "10000"); //Fade slowly back to warm white
    }
    lastLightUpdate = currentTime;
    lightOffset = !lightOffset;
    button_state_changed_since_last_draw_loop = false;
  }
}
