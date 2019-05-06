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

//Send a pulse of light through all the lights
void lightPulse(char* pulseHue, char* backgroundHue, int delayms) {
  for (int i = 1; i <= NUM_LIGHTS; i++) {
    changeLight(i, 2, "hue", pulseHue, "bri", "200");
    delay(200);
    changeLight(i, 2, "hue", backgroundHue);
  }
}
