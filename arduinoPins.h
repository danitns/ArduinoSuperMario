// LCD Pins
const byte rsPin = 9;
const byte enPin = 8;
const byte d4Pin = 7;
const byte d5Pin = 6;
const byte d6Pin = 3;
const byte d7Pin = 4;
const byte backlightPin = 5;
LiquidCrystal lcd(rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin);

// Matrix Pins
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  //DIN, CLK, LOAD, No. DRIVER

// Joystick Pins
const byte pinSW = 2;  // digital pin connected to switch output
const byte pinX = A0;  // A0 - analog pin connected to X output
const byte pinY = A1;  // A1 - analog pin connected to Y output

// matrix images
const uint8_t IMAGES[][8] = {
  // welcome screen 0
  { 0b00011000,
    0b00111100,
    0b00011000,
    0b00111100,
    0b01011010,
    0b00011000,
    0b00100100,
    0b00100100 },
  // play game 1
  { 0b00111100,
    0b01000010,
    0b10011001,
    0b10010101,
    0b10011001,
    0b10010001,
    0b01000010,
    0b00111100 },
  // highscore 2
  { 0b10000001,
    0b10000001,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00100100,
    0b00100100,
    0b00011000 },
  // settings 3
  { 0b01100110,
    0b10001111,
    0b10100111,
    0b01011010,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001 },
  // about 4
  { 0b00011000,
    0b00100100,
    0b00100100,
    0b00000100,
    0b00011000,
    0b00010000,
    0b00000000,
    0b00010000 },
  // how to play 5
  { 0b01000010,
    0b10111101,
    0b10000001,
    0b10100101,
    0b10000001,
    0b10011001,
    0b10100101,
    0b01000010 },
  // end game 6
  { 0b00000000,
    0b10100101,
    0b01000010,
    0b10100101,
    0b00000000,
    0b00011000,
    0b00100100,
    0b01000010 },
  // win game 7
  { 0b01111110,
    0b01111110,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00111100 }
};

enum matrixImages {
  WELCOME_SCREEN_IMAGE,
  PLAY_GAME_IMAGE,
  HIGHSCORE_IMAGE,
  SETTINGS_IMAGE,
  ABOUT_IMAGE,
  HOW_TO_PLAY_IMAGE,
  END_GAME_IMAGE,
  WIN_GAME_IMAGE
};

uint8_t customChar[][8] = {
  // up arrow
  { B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100 },
  // down arrow
  { B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B10101,
    B01110,
    B00100 }
};

const int buzzerPin = A4;
