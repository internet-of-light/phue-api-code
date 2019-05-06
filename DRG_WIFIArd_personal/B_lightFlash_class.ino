/*
   FLASH CLASS
*/
class flash {
  private:
    int pos;
    bool activeStatus;
    bool forward;
    String mainHue, backgroundHue;
    unsigned long waitTime;
    unsigned long lastUpdate;
    void displayFlash();

  public:
    flash();
    void updateFlash();
    void activate(int forward, String mainHue, String backgroundHue);
    bool isActive();
};

flash::flash()
{
  this->waitTime = 1000;
  this->activeStatus = false;
}

bool flash::isActive()
{
  return this->activeStatus;
}

void flash::activate(int forward, String mainHue, String backgroundHue)
{
  this->activeStatus = true;
  this->pos = 1;
  this->forward = forward;
  this->mainHue = mainHue;
  this->backgroundHue = backgroundHue;

  this->displayFlash();
}

void flash::updateFlash() {
  if (currentTime - this->lastUpdate > this-> waitTime) {
    this->pos++;
    this->lastUpdate = currentTime;
    this->displayFlash();
  }

  if (this->pos > NUM_LIGHTS) {
    this->activeStatus = false;
  }
}

void flash::displayFlash() {
  if (this->forward) { //If it is moving forward thru lights
    changeLight(this->pos, 0, "hue", this->mainHue); //Immediately switch to main color
    changeLight(this->pos, 20, "hue", this->backgroundHue); //Slowly switch back to background color
  }
  else { //If it is moving backward thru lights
    changeLight(NUM_LIGHTS + 1 - this->pos, 0, "hue", this->mainHue); //Immediately switch to main color
    changeLight(NUM_LIGHTS + 1 - this->pos, 20, "hue", this->backgroundHue); //Slowly switch back to background color
  }
}
/*
   END FLASH CLASS
*/

flash flashes[NUM_FLASHES];

//Activates the next flash with a defined color
void activateFlash(int forward, String mainHue, String backgroundHue) {
  for (int i = 0; i < NUM_FLASHES; i++) {
    if (!flashes[i].isActive()) {
      dbprintln("Activated: " + String(i));
      flashes[i].activate(forward, mainHue, backgroundHue);
      break;
    }
  }
}


void updateFlashes() {
  for (int i = 0; i < NUM_FLASHES; i++) {
    if (flashes[i].isActive()) {
      flashes[i].updateFlash();
    }
  }
}
