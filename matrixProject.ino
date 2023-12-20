#include <EEPROM.h>
#include "LedControl.h"
#include <LiquidCrystal.h>
#include "arduinoPins.h"
#include "pitches.h"

// enums
enum joystickDirections {
  LEFT_OR_UP,
  CENTER,
  RIGHT_OR_DOWN
};

enum menuType {
  WELCOME_SCREEN,
  MAIN_MENU,
  START_GAME,
  CHANGE_NAME,
  HIGHSCORE,
  SETTINGS,
  ABOUT,
  HOW_TO_PLAY,
  END_GAME,
  WIN_GAME,
  GAME_SCORE,
};

enum customChars {
  UP_ARROW,
  DOWN_ARROW
};

enum blockTypes {
  EMPTY_SPACE,
  WALL,
  TRAP,
  COIN
};

enum soundEffects {
  COIN_SOUND,
  JUMP_SOUND,
  END_GAME_SOUND,
  PRESSED_BUTTON_SOUND,
  WIN_GAME_SOUND
};

// letters
const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// structs
struct Coordinates {
  int row = 0;
  int col = 0;
};

struct Player {
  const byte jumpHeight = 3;
  const byte jumpIncrementTime = 150;
  long lastJumpIncrement = 0;
  const byte fallTime = 125;
  long lastFall = 0;
  bool onGround = 1;
  byte jumpNumberOfIncrements = 0;
  Coordinates pastPosition;
  Coordinates position;
};

struct HighscoreRecord {
  byte name[3];
  int score;
};

// sound variables
byte soundToPlay = 0;
byte shouldPlay = 0;
byte noteIndex = 0;
byte songSize = 0;
unsigned long lastNotePlayed = 0;
unsigned int duration = 0;
bool isSoundOn = true;

byte gameTimer = 60;
long lastTimerDecrement = 0;

int currentScore = 0;

byte easyGameMultiplier = 1;
byte mediumGameMultiplier = 2;
byte hardGameMultiplier = 3;

const byte playerBlinkInterval = 20;
long lastPlayerBlink = 0;
bool playerState = 1;
const int movementSpeed = 300;
long lastMove = 0;

const byte trapBlinkInterval = 200;
long lastTrapBlink = 0;
byte trapState = 1;

const unsigned int coinBlinkInterval = 500;
long lastCoinBlink = 0;
byte coinState = 1;

HighscoreRecord highscoreRecords[3];
byte username[3] = { 0, 0, 0 };
byte letterIndex = 0;

Player player;

const byte displayLen = 16;
const byte displayHeight = 2;
const byte maxLcdBrightness = 5;
byte lcdBrightness = 5;

volatile bool selectButtonPressed = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

enum menuType currentMenu = WELCOME_SCREEN;
bool inGame = false;
bool shouldRerender = false;

bool joyMoved = false;
const int minThreshold = 400;
const int maxThreshold = 600;

const byte mainMenuLen = 5;
const char* mainMenu[mainMenuLen] = { "Start", "Highscore", "Settings", "About", "How to play" };
const byte settingsMenuLen = 5;
const char* settingsMenu[settingsMenuLen] = { "LCD Bright", "Game Bright", "Reset score", "Music", "Back" };
const byte difficultyMenuLen = 3;
const String difficultyMenu[difficultyMenuLen] = { "Easy", "Medium", "Hard" };
const char* aboutMenu = "Super Mario platformer game.  Github: danitns";
const char* howToPlayMenu = "Use joystick to move player.  Your goal is to get to the left side of the map. Slow blink = coin, fast blink = trap";
byte currentMenuIndex = 0;
byte currentMenuLen = 0;
byte currentDifficulty = 0;


// Matrix variables
Coordinates bottomLeftViewCoordinates;
const byte matrixSize = 8;
byte matrixBrightness = 5;
byte maxMatrixBrightness = 5;
const byte mapRows = 12;
const byte mapCols = 32;
const byte easyLevel[mapRows][mapCols] PROGMEM = {
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 3, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 3, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 3, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

const byte mediumLevel[mapRows][mapCols] PROGMEM = {
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 3, 0, 0 },
  { 0, 0, 0, 3, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 3, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

const byte hardLevel[mapRows][mapCols] PROGMEM = {
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 3, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 3, 2, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 2, 3, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

byte matrix[mapRows][mapCols];

//song
int divider = 0, noteDuration = 0;

void setup() {
  pinMode(buzzerPin, OUTPUT);

  getUsernameFromEeprom(3);
  getBrightnessAndSoundFromEeprom();
  Serial.begin(9600);
  initLcd();
  initMatrix();
  pinMode(pinSW, INPUT_PULLUP);

  //initGameVariables(0);

  initHighscore();

  attachInterrupt(digitalPinToInterrupt(pinSW), handleButtonPress, FALLING);
}

void loop() {
  if (inGame) {
    play();
  } else {
    renderMenu();
  }
  if(isSoundOn && shouldPlay) {
    if(millis() - lastNotePlayed > duration){
      playNote();
    }
  }
}

// utils
void loadMatrixFromProgmem(const byte src[][mapCols], byte dest[][mapCols]) {
  for (int row = 0; row < mapRows; row++) {
    for (int col = 0; col < mapCols; col++) {
      dest[row][col] = pgm_read_byte(&src[row][col]);
    }
  }
}

void writeStringOnCenter(String str, int row = 0, bool displayAvailableDirections = false) {
  byte cursorPos = (displayLen / 2) - (str.length() / 2);
  lcd.setCursor(cursorPos, row);
  lcd.print(str);
  if (displayAvailableDirections) {
    displayMenuArrows();
  }
}

void displayMenuArrows() {
  if (currentMenuIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write(UP_ARROW);
  }
  if (currentMenuIndex < currentMenuLen - 1) {
    lcd.setCursor(15, 1);
    lcd.write(DOWN_ARROW);
  }
}

void handleButtonPress() {
  static unsigned long interruptTime = 0;  // Timestamp for the current interrupt, retains its value between ISR calls
  interruptTime = micros();                // Capture the current time in microseconds
  // Debounce logic: If interrupts come in faster succession than the debounce delay, they're ignored
  if (interruptTime - lastInterruptTime > debounceDelay * 1000) {  // Convert debounceDelay to microseconds for comparison
    selectButtonPressed = true;
  }
  // Update the last interrupt timestamp
  lastInterruptTime = interruptTime;
}

void displayImage(const uint8_t* image) {
  for (int row = 0; row < matrixSize; row++) {
    lc.setRow(0, row, image[matrixSize - row - 1]);
  }
}

void getBrightnessAndSoundFromEeprom() {
  lcdBrightness = EEPROM.read(0);
  matrixBrightness = EEPROM.read(1);
  isSoundOn = EEPROM.read(2);
}

void saveValuesToEeprom() {
  EEPROM.update(0, lcdBrightness);
  EEPROM.update(1, matrixBrightness);
  EEPROM.update(2, isSoundOn);
}

joystickDirections getJoystickPosition(byte pin) {
  unsigned int value = analogRead(pin);
  if (value > maxThreshold) {
    return RIGHT_OR_DOWN;
  } else if (value < minThreshold) {
    return LEFT_OR_UP;
  } else return CENTER;
}

// init functions
void initLcd() {
  lcd.begin(16, 2);
  byte mappedValue = map(lcdBrightness, 0, maxLcdBrightness, 0, 254);

  lcd.createChar(UP_ARROW, customChar[UP_ARROW]);
  lcd.createChar(DOWN_ARROW, customChar[DOWN_ARROW]);

  analogWrite(backlightPin, mappedValue);
  writeStringOnCenter(F("SUPER MARIO"));
  writeStringOnCenter(F("press button"), 1);
}

void initMatrix() {
  lc.shutdown(0, false);                 // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness);  // sets brightness (0~15 possible values)
  lc.clearDisplay(0);                    // clear screen
  displayImage(IMAGES[currentMenu]);
}

void initGameVariables(byte option) {
  player.position.row = 1;
  player.position.col = 2;

  if (option == 0) {
    loadMatrixFromProgmem(easyLevel, matrix);
  } else if (option == 1) {
    loadMatrixFromProgmem(mediumLevel, matrix);
  } else {
    loadMatrixFromProgmem(hardLevel, matrix);
  }

  gameTimer = 60;
  currentScore = 0;
}

void initHighscore() {
  EEPROM.get(6, highscoreRecords);
}

void playNote() {
  int note = 0;
  switch(soundToPlay) {
    case COIN_SOUND:
      note = coinSound[noteIndex];
      duration = coinSound[noteIndex + 1];
      break;
    case JUMP_SOUND:
      note = jumpSound[noteIndex];
      duration = jumpSound[noteIndex + 1];
      break;
    case END_GAME_SOUND:
      note = endOfGameSound[noteIndex];
      duration = endOfGameSound[noteIndex + 1];
      break;
    case PRESSED_BUTTON_SOUND:
      note = pressedButtonSound[noteIndex];
      duration = pressedButtonSound[noteIndex + 1];
      break;
    case WIN_GAME_SOUND:
      note = winGameSound[noteIndex];
      duration = winGameSound[noteIndex + 1];
  };
  tone(buzzerPin, note, duration);
  noteIndex = noteIndex + 2;
  lastNotePlayed = millis();
  
  if(note == END_OF_SONG) {
    shouldPlay = false;
    noTone(buzzerPin);
  }
}

void playSong(soundEffects soundType) {
  shouldPlay = true;
  noteIndex = 0;
  soundToPlay = soundType;
}

void renderMenu() {
  handleJoystickMovementInMenu();
  if (selectButtonPressed) {
    playSong(PRESSED_BUTTON_SOUND);

    executeChosenAction();
    currentMenuIndex = 0;
    shouldRerender = true;
    selectButtonPressed = false;
  }
  if (shouldRerender) {
    lcd.clear();
    displayMenu();
    shouldRerender = false;
  }
}

void executeChosenAction() {
  switch (currentMenu) {
    case WELCOME_SCREEN:
      currentMenu = MAIN_MENU;
      currentMenuLen = mainMenuLen;
      break;
    case MAIN_MENU:
      executeMainMenuAction();
      break;
    case SETTINGS:
      if (currentMenuIndex == 2) {
        resetHighscore();
      }
      if (currentMenuIndex == 4) {
        currentMenu = MAIN_MENU;
        currentMenuLen = mainMenuLen;
      }
      break;
    case ABOUT:
      currentMenu = MAIN_MENU;
      currentMenuLen = mainMenuLen;
      break;
    case START_GAME:
      currentDifficulty = currentMenuIndex;
      initGameVariables(currentDifficulty);
      currentMenu = CHANGE_NAME;
      currentMenuLen = strlen(alphabet);
      lcd.cursor();
      break;
    case CHANGE_NAME:
      saveNameToEeprom(3);
      inGame = true;
      break;
    case WIN_GAME:
      currentMenu = GAME_SCORE;
      currentMenuLen = 1;
      break;
    default:
      currentMenu = MAIN_MENU;
      currentMenuLen = mainMenuLen;
      break;
  }
}

void executeMainMenuAction() {
  switch (currentMenuIndex) {
    case 0:  // start game
      currentMenu = START_GAME;
      currentMenuLen = difficultyMenuLen;
      break;
    case 1:  //highscore
      currentMenu = HIGHSCORE;
      currentMenuLen = 3;
      break;
    case 2:  // settings
      currentMenu = SETTINGS;
      currentMenuLen = settingsMenuLen;
      break;
    case 3:  // about
      currentMenu = ABOUT;
      currentMenuLen = ceil(strlen(aboutMenu) / float(((displayLen - 1) * displayHeight)));
      break;
    case 4:
      currentMenu = HOW_TO_PLAY;
      currentMenuLen = ceil(strlen(howToPlayMenu) / float(((displayLen - 1) * displayHeight)));
    default:
      break;
  }
}

void resetHighscore() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      highscoreRecords[i].name[j] = 0;
    }
    highscoreRecords[i].score = 0;
  }
  EEPROM.put(6, highscoreRecords);
}

void displayMenu() {
  switch (currentMenu) {
    case MAIN_MENU:
      writeStringOnCenter(mainMenu[currentMenuIndex], 0, true);
      displayImage(IMAGES[currentMenuIndex + 1]);
      if (inGame == true) {
        displayMap();
      }
      break;
    case HIGHSCORE:
      displayHighscore();
      break;
    case SETTINGS:
      displaySettings();
      break;
    case ABOUT:
      writeStringWithScroll(aboutMenu);
      break;
    case HOW_TO_PLAY:
      writeStringWithScroll(howToPlayMenu);
      break;
    case START_GAME:
      writeStringOnCenter(difficultyMenu[currentMenuIndex], 0, true);
      break;
    case CHANGE_NAME:
      displayUsername();
      break;
    case END_GAME:
      writeStringOnCenter(F("END GAME"));
      writeStringOnCenter(F("press button"), 1);
      displayImage(IMAGES[END_GAME_IMAGE]);
      break;
    case WIN_GAME:
      writeStringOnCenter("YOU WIN!");
      writeStringOnCenter(F("press button"), 1);
      displayImage(IMAGES[WIN_GAME_IMAGE]);
      break;
    case GAME_SCORE:
      writeStringOnCenter("SCORE: " + String(currentScore));
      writeStringOnCenter(F("press button"), 1);
      displayImage(IMAGES[WIN_GAME_IMAGE]);
      break;
    default:
      break;
  }
}

void displayUsername() {
  char userName[6];
  userName[0] = '<';
  for (int i = 0; i < 3; i++) {
    userName[i + 1] = alphabet[username[i]];
    Serial.println(username[i]);
  }

  userName[4] = '>';
  userName[5] = '\0';
  writeStringOnCenter(userName, 0, true);
  writeStringOnCenter("press to save", 1);

  byte cursorPos = (displayLen / 2) - (3 / 2) + letterIndex;
  lcd.setCursor(cursorPos, 0);
}


void getUsernameFromEeprom(int address) {
  EEPROM.get(address, username);
}

void saveNameToEeprom(int address) {
  for (int i = 0; i < 3; i++) {
    EEPROM.update(address + i, username[i]);
  }
}

void displayHighscore() {
  writeStringOnCenter("NAME SCORE", 0, true);
  String nameAndScore;
  for (int i = 0; i < 3; i++) {
    nameAndScore += alphabet[highscoreRecords[currentMenuIndex].name[i]];
  }
  nameAndScore += "  ";
  nameAndScore += String(highscoreRecords[currentMenuIndex].score);
  writeStringOnCenter(nameAndScore, 1);
}


void displaySettings() {
  writeStringOnCenter(settingsMenu[currentMenuIndex], 0, true);
  switch (currentMenuIndex) {
    case 0:
      displayCurrentSettings(lcdBrightness, maxLcdBrightness, 1);
      break;
    case 1:
      displayCurrentSettings(matrixBrightness, maxMatrixBrightness, 1);
      break;
    case 3:
      displayCurrentSettings(isSoundOn, 1, 1);
  }
}

void displayCurrentSettings(int currentSettingValue, int maxSettingValue, int screenRowIndex) {
  String str;
  if(currentSettingValue > 0) {
    str = "< ";
  } else {
    str = "  ";
  }
  for (int i = 0; i < maxSettingValue; i++) {
    if (i < currentSettingValue)
      str += 'X';
    else
      str += '-';
  }
  if(currentSettingValue < maxSettingValue) {
    str += " >";
  } else {
    str += " ";
  }
  writeStringOnCenter(str, screenRowIndex);
}

void handleJoystickMovementInMenu() {
  if (currentMenu == CHANGE_NAME) {
    currentMenuIndex = username[letterIndex];
  }

  if (getJoystickPosition(pinY) == LEFT_OR_UP && joyMoved == false && currentMenuIndex > 0) {
    currentMenuIndex--;
    if (currentMenu == CHANGE_NAME) {
      username[letterIndex] = currentMenuIndex;
    }
    shouldRerender = true;
    joyMoved = true;
  }

  if (getJoystickPosition(pinY) == RIGHT_OR_DOWN && joyMoved == false && currentMenuIndex < (currentMenuLen - 1)) {
    currentMenuIndex++;
    if (currentMenu == CHANGE_NAME) {
      username[letterIndex] = currentMenuIndex;
    }
    shouldRerender = true;
    joyMoved = true;
  }

  if (currentMenu == SETTINGS) {
    adjustSettings();
  }

  if (currentMenu == CHANGE_NAME) {
    adjustName();
  }

  if (getJoystickPosition(pinY) == CENTER && getJoystickPosition(pinX) == CENTER) {
    joyMoved = false;
  }
}

void adjustSettings() {
  if (getJoystickPosition(pinX) == LEFT_OR_UP && joyMoved == false) {
    if (currentMenuIndex == 0 && lcdBrightness > 0) {
      lcdBrightness--;
      byte mappedValue = map(lcdBrightness, 0, maxLcdBrightness, 0, 254);
      analogWrite(backlightPin, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    } else if (currentMenuIndex == 1 && matrixBrightness > 0) {
      matrixBrightness--;
      byte mappedValue = map(matrixBrightness, 0, maxMatrixBrightness, 0, 15);
      lc.setIntensity(0, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    } else if(currentMenuIndex == 3 && isSoundOn == true) {
      isSoundOn = false;
      shouldRerender = true;
      joyMoved = true;
    }

    saveValuesToEeprom();
  }

  if (getJoystickPosition(pinX) == RIGHT_OR_DOWN && joyMoved == false) {
    if (currentMenuIndex == 0 && lcdBrightness < maxLcdBrightness) {
      lcdBrightness++;
      byte mappedValue = map(lcdBrightness, 0, maxLcdBrightness, 0, 254);
      analogWrite(backlightPin, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    } else if (currentMenuIndex == 1 && matrixBrightness < maxMatrixBrightness) {
      matrixBrightness++;
      byte mappedValue = map(matrixBrightness, 0, maxMatrixBrightness, 0, 15);
      lc.setIntensity(0, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    } else if(currentMenuIndex == 3 && isSoundOn == false) {
      isSoundOn = true;
      shouldRerender = true;
      joyMoved = true;
    }
  }

  saveValuesToEeprom();
}

void adjustName() {
  if (getJoystickPosition(pinX) == LEFT_OR_UP && joyMoved == false && letterIndex >= 1) {
    letterIndex--;
    shouldRerender = true;
    joyMoved = true;
  }

  if (getJoystickPosition(pinX) == RIGHT_OR_DOWN && joyMoved == false && letterIndex <= 1) {
    letterIndex++;
    shouldRerender = true;
    joyMoved = true;
  }
}

void writeStringWithScroll(String message) {
  byte numberOfCharsToPrint = (displayLen - 1) * displayHeight;
  for (int i = 0; i < numberOfCharsToPrint; i++) {
    if (i == displayLen - 1) {
      lcd.setCursor(0, 1);
    }
    byte index = (currentMenuIndex * numberOfCharsToPrint) + i;
    if (index >= message.length())
      break;
    lcd.print(message[index]);
  }
  displayMenuArrows();
}

void play() {
  handleLcdDisplayInGame();
  handleMapState();
  handlePlayerState();
}

void handleLcdDisplayInGame() {
  if (shouldRerender) {
    lcd.clear();
    switch (currentDifficulty) {
      case 0:
        writeStringOnCenter("Easy. Sc: " + String(currentScore));
        break;
      case 1:
        writeStringOnCenter("Med. Sc: " + String(currentScore));
        break;
      case 2:
        writeStringOnCenter("Hard. Sc: " + String(currentScore));
        break;
    }
    writeStringOnCenter("Time: " + String(gameTimer), 1);
    shouldRerender = false;
  }
  if (millis() - lastTimerDecrement > 1000) {
    gameTimer--;
    shouldRerender = true;
    lastTimerDecrement = millis();
  }
}

void handleMapState() {
  if (player.position.col < (matrixSize / 2)) {
    bottomLeftViewCoordinates.col = 0;
  } else if (player.position.col > mapCols - (matrixSize / 2)) {
    bottomLeftViewCoordinates.col = mapCols - matrixSize;
  } else {
    bottomLeftViewCoordinates.col = player.position.col - 4;
  }

  if (player.position.row < (matrixSize / 2)) {
    bottomLeftViewCoordinates.row = 0;
  } else if (player.position.row > mapRows - (matrixSize / 2)) {
    bottomLeftViewCoordinates.row = mapRows - matrixSize;
  } else {
    bottomLeftViewCoordinates.row = player.position.row - 4;
  }

  if (millis() - lastTrapBlink > trapBlinkInterval) {
    trapState = !trapState;
    lastTrapBlink = millis();
  }

  if (millis() - lastCoinBlink > coinBlinkInterval) {
    coinState = !coinState;
    lastCoinBlink = millis();
  }

  displayMap();
}

void displayMap() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      byte matrixValue = matrix[row + bottomLeftViewCoordinates.row][col + bottomLeftViewCoordinates.col];
      byte state = 0;
      switch (matrixValue) {
        case EMPTY_SPACE:
          state = 0;
          break;
        case WALL:
          state = 1;
          break;
        case TRAP:
          state = trapState;
          break;
        case COIN:
          state = coinState;
          break;
      };
      lc.setLed(0, row, col, state);
    }
  }
}

void handlePlayerState() {
  displayPlayer();
  movePlayer();
  checkForBonusPoints();
  checkForGameResult();
}

void displayPlayer() {
  lc.setLed(0, player.pastPosition.row - bottomLeftViewCoordinates.row, player.pastPosition.col - bottomLeftViewCoordinates.col, LOW);
  if (millis() - lastPlayerBlink > playerBlinkInterval) {
    playerState = !playerState;
    lastPlayerBlink = millis();
  }
  lc.setLed(0, player.position.row - bottomLeftViewCoordinates.row, player.position.col - bottomLeftViewCoordinates.col, playerState);
  player.pastPosition = player.position;
}

void movePlayer() {
  applyHorizontalMoving();
  applyVerticalMoving();
}

void applyHorizontalMoving() {
  if (getJoystickPosition(pinX) == LEFT_OR_UP && matrix[player.position.row][player.position.col - 1] != 1) {
    if (millis() - lastMove > movementSpeed) {
      player.position.col--;
      lastMove = millis();
    }
  }
  if (getJoystickPosition(pinX) == RIGHT_OR_DOWN && matrix[player.position.row][player.position.col + 1] != 1) {
    if (millis() - lastMove > movementSpeed) {
      player.position.col++;
      lastMove = millis();
    }
  }
}

void applyVerticalMoving() {
  if (matrix[player.position.row - 1][player.position.col] == WALL) {
    player.onGround = true;
  }

  if (getJoystickPosition(pinY) == LEFT_OR_UP && matrix[player.position.row - 1][player.position.col] == WALL) {
    playSong(JUMP_SOUND);
    player.jumpNumberOfIncrements = player.jumpHeight;
  }

  // jump logic
  if (player.jumpNumberOfIncrements > 0) {
    if (millis() - player.lastJumpIncrement > player.jumpIncrementTime) {
      if (player.position.row + 1 < mapRows && matrix[player.position.row + 1][player.position.col] != WALL) {
        player.position.row++;
      }

      player.lastJumpIncrement = millis();
      player.jumpNumberOfIncrements--;
    }
    if (player.jumpNumberOfIncrements == 0) {
      player.lastFall = millis();
    }
  } else if (matrix[player.position.row - 1][player.position.col] != WALL) {
    // added onGround so the player wouldn t fall instantly from the current row to row-1
    if (player.onGround == true) {
      player.lastFall = millis();
      player.onGround = false;
    }
    if (millis() - player.lastFall > player.fallTime) {
      player.position.row--;
      player.lastFall = millis();
    }
    if (matrix[player.position.row - 1][player.position.col] != WALL) {
      player.lastJumpIncrement = millis();
    }
  }
}

void checkForBonusPoints() {
  if (matrix[player.position.row][player.position.col] == COIN) {
    playSong(COIN_SOUND);
    currentScore += 20;
    matrix[player.position.row][player.position.col] = EMPTY_SPACE;
  }
}

void checkForGameResult() {
  if (player.position.row <= 0 || gameTimer == 0 || matrix[player.position.row][player.position.col] == TRAP) {
    inGame = false;
    currentMenu = END_GAME;
    shouldRerender = true;
    playSong(END_GAME_SOUND);
  } else if (player.position.col >= mapCols) {
    playSong(WIN_GAME_SOUND);
    inGame = false;
    currentMenu = WIN_GAME;
    shouldRerender = true;
    calculateScore();
  }
}

void calculateScore() {
  switch (currentDifficulty) {
    case 0:
      currentScore += easyGameMultiplier * gameTimer;
      break;
    case 1:
      currentScore += mediumGameMultiplier * gameTimer;
      break;
    case 2:
      currentScore += hardGameMultiplier * gameTimer;
      break;
  };

  checkForHighscore();
}

void checkForHighscore() {
  bool isChanged = false;
  HighscoreRecord currentRecord;
  for (int i = 0; i < 3; i++) {
    currentRecord.name[i] = username[i];
  }
  currentRecord.score = currentScore;

  for (int i = 0; i < 3; i++) {
    if (currentRecord.score > highscoreRecords[i].score) {
      isChanged = true;

      HighscoreRecord tempRecord = highscoreRecords[i];

      for (int letterIndex = 0; letterIndex < 3; letterIndex++) {
        highscoreRecords[i].name[letterIndex] = currentRecord.name[letterIndex];
      }

      highscoreRecords[i].score = currentRecord.score;

      currentRecord = tempRecord;
    }
  }

  if (isChanged) {
    EEPROM.put(6, highscoreRecords);
  }
}
