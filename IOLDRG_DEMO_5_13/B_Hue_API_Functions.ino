/*

    Tab 2 - Hue API Funcion bodies

*/

void changeGroup(byte groupNum, byte transitiontime, String parameter, String newValue, String parameter2,
                 String newValue2, String parameter3, String newValue3,
                 String parameter4, String newValue4) {

  String req_string = "http://" + ip + "/api/" + api_token + "/groups/" + groupNum + "/action";
  HTTPClient http;
  http.begin(req_string);

  String put_string = "{\"" + parameter + "\":" + newValue + ", \"transitiontime\": " +
                      transitiontime;
  if (!parameter2.equals("")) put_string += + ", \"" + parameter2 + "\": " + newValue2;
  if (!parameter3.equals("")) put_string += ", \"" + parameter3 + "\" : " + newValue3;
  if (!parameter4.equals("")) put_string += ", \"" + parameter4 + "\" : " + newValue4;
  put_string +=  + "}";

  dbprintln("Attempting PUT: " + put_string + " for GROUP: " + String(groupNum));

  int httpResponseCode = http.PUT(put_string);
  if (httpResponseCode == 200) {
    String response = http.getString();
    dbprintln("Response: " + response);
  } else {
    dbprint("Error on sending PUT Request: ");
    dbprintln(String(httpResponseCode));
  }
  http.end();
}

void changeLight(byte lightNum, byte transitiontime, String parameter, String newValue, String parameter2,
                 String newValue2, String parameter3, String newValue3,
                 String parameter4, String newValue4) {

  String req_string = "http://" + ip + "/api/" + api_token + "/lights/" + lightNum + "/state";
  HTTPClient http;
  http.begin(req_string);

  String put_string = "{\"" + parameter + "\":" + newValue + ", \"transitiontime\":" + transitiontime;
  if (!parameter2.equals("")) put_string += + ", \"" + parameter2 + "\": " + newValue2;
  if (!parameter3.equals("")) put_string += ", \"" + parameter3 + "\" : " + newValue3;
  if (!parameter4.equals("")) put_string += ", \"" + parameter4 + "\" : " + newValue4;
  put_string +=  + "}";

  dbprintln("Attempting PUT: " + put_string + " for LIGHT: " + String(lightNum));


  int httpResponseCode = http.PUT(put_string);
  if (httpResponseCode == 200) {
    String response = http.getString();
    dbprintln("Response: " + response);
  } else {
    dbprint("Error on sending PUT Request: ");
    dbprintln(String(httpResponseCode));
  }
  http.end();
}

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





// Unused in this demo
// These arrays contain the light numbers for controlling the Sieg Master lights
/*
  int LOWER_FLOOR_LIGHT_SPIRAL[] = {22, 15, 10, 23, 11, 14, 16, 21, 7};
  int LOWER_FLOOR_LIGHT_LOOP_NO_MIDDLE[] = {22, 15, 10, 23, 11, 14, 16, 21};

  #define LOWER_FLOOR_MIDDLE 7

  int UPPER_FLOOR_LIGHT_SPIRAL[] = {18, 20, 12, 5, 13, 17, 9, 24, 8, 25, 26, 19};
*/
