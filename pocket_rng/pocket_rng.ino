#include <Wire.h>
#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "OPTIGATrustM.h"

#define RND_MAXLENGTH     64
#define PIN_RESET 6
#define PIN_SDA 4
#define PIN_SCL 5
#define SUPPRESSCOLLORS
uint8_t *rnd = new uint8_t[RND_MAXLENGTH];

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int rngErrorState = 0;

int buttonPins[] = {2,1,0,7,8,9,10,3};
int buttonStates[] = {-1,-1,-1,-1,-1,-1,-1,-1};
int lastButtonStates[] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
unsigned long lastDebounceTimes[] = {0,0,0,0,0,0,0,0,};
int buttonValues[] = {2,4,6,8,10,12,20,100};
unsigned long debounceDelay = 50;
int specialIndex = 0;

char eightBallResponses[30][30] = { // weird spacing is to make the line breaks clean
                                   "It is certain.",
                                   "It is decidedly so.",
                                   "Without a doubt.",
                                   "Yes - definitely.",
                                   "You may rely on it.",
                                   "As I see it, yes.",
                                   "Most likely.",
                                   "Outlook good.",
                                   "Yes.",
                                   "Signs point to yes.",
                                   "Reply hazy, try      again.",
                                   "Ask again later.",
                                   "Better not tell you  now.",
                                   "Cannot predict now.",
                                   "Concentrate and ask  again.",
                                   "Don't count on it.",
                                   "My reply is no.",
                                   "My sources say no.",
                                   "Outlook not so good.",
                                   "Very doubtful."
};

void resetDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
}

void FAIL(bool abandonHope) {
  resetDisplay();
  display.println("=RNG ERROR=");
  display.println("Reboot me!");
  display.display();
  if (abandonHope) {for(;;);}
}

void hsmReset() {
  rngErrorState = 0;
  digitalWrite(PIN_SDA, HIGH);
  digitalWrite(PIN_SCL, HIGH);
  digitalWrite(PIN_RESET, LOW);
  delay(1);
  digitalWrite(PIN_RESET, HIGH);
}

int beginHSM(bool abandonHope) {
  if (rngErrorState) { goto hsm_error; }
  rngErrorState = trustM.begin();
  if (rngErrorState) { goto hsm_error; }
  rngErrorState = trustM.setCurrentLimit(8);
  if (rngErrorState) { goto hsm_error; }
  return 0;
 hsm_error:
  FAIL(abandonHope);
  return rngErrorState;
}

void getRandom() {
  memset(rnd, 0, RND_MAXLENGTH);
  rngErrorState = trustM.getRandom(RND_MAXLENGTH, rnd);
  if (rngErrorState) {FAIL(true);}
}

void screenOfHex() {
  getRandom();
  resetDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  int width = SCREEN_WIDTH/6/2; // Each byte takes 2 characters
  int height = SCREEN_HEIGHT/8;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      int b = rnd[j*width + i];
      if (b < 16) {display.print("0");}
      display.print(b, HEX);
    }
    display.println();
  }
  display.display();
}

void printFastTransition() {
  for (int i = 0; i < 21; i++) {
    resetDisplay();
    display.setTextSize(1);
    for (int l = 0; l < SCREEN_HEIGHT/8; l++) {
      for (int j = 0; j < i+1; j++) {
        display.print("?");
      }
      display.println();
    }
    display.display();
    delay(20);
  }
}

int randInt(int range) {
  // The following code assumes 32-byte ints.
  // As I'm writing this for the Seeeduino XIAO,
  // which uses the SAMD21, that works.
  unsigned int gap = 16777216 % range;
  unsigned int upper_bound = 16777216 - gap;
 roll:
  getRandom();
  // If we wanted, instead of generating a new random blob immediately,
  // we could first look deeper into the existing array of randomness.
  // However, I'm lazy, this is easier to code, and the performance hit doesn't matter.
  unsigned int rval = rnd[0]*65536 + rnd[1]*256 + rnd[2];
  if (rval >= upper_bound) {
    // Doing this ensures that the number we're mod-ing is a multiple of the range.
    // As a result, each number in the range is equally likely to be chosen.
    // However, it does mean that this is probabilistic, not deterministic. Such is life.
    goto roll;
  }
  return (rval%range)+1;
}

void magicEightBall() {
  int r = randInt(20);
  printFastTransition();
  resetDisplay();
  display.setTextSize(1);
  display.print("Magic 8 Ball says: ");
  //display.print(r);
  display.println();
  display.println();
  display.println(eightBallResponses[r-1]);
  display.display();
}

void rollDie(int range) {
  int r = randInt(range);
  printFastTransition();
  resetDisplay();
  if (range == 2) {
    display.setTextSize(4);
    if (r == 1) {display.print("HEADS");} else {display.print("TAILS");}
  } else {
    display.print("RNG 1-"); display.print(range); display.println(":");
    display.print(">>>");
    if (r < 100) {
      display.print(" ");
    }
    if (r < 10) {
      display.print(" ");
    }
    display.print(r);
    display.println(" <<<");
  }
  display.display();
}

int checkButton(int i) {
  int reading = digitalRead(buttonPins[i]);
  if (reading != lastButtonStates[i]) {
    lastDebounceTimes[i] = millis();
  }
  if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
    if (reading != buttonStates[i]) {
      buttonStates[i] = reading;
      if (buttonStates[i] == LOW) {
        if (i != specialIndex && buttonStates[specialIndex] == LOW) {
          // The special button (coin flip) is being held down, so the other buttons now do different stuff.
          // Right now it's just acting as a magic 8-ball regardless of the other button pressed.
          magicEightBall();
        } else {
          rollDie(buttonValues[i]);
        }
        lastButtonStates[i] = reading;
        return 0;
      }
    }
  }
  lastButtonStates[i] = reading;
  return 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RESET, OUTPUT);
  for (int i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Don't proceed, loop forever
  }
  resetDisplay();
  display.display();
  if (beginHSM(false)) {
    hsmReset();
    beginHSM(true);
  }
  screenOfHex();
}

void loop() {
  for (int i = 0; i < 8; i++) {
    checkButton(i);
  }
}
B