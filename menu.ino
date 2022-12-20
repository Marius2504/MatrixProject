/*
EEPROM
--------------------------------------------------
byte        Desc
0           byte for the length of highscore
1           difficulty level
2           LCD contrast
3           LCD brightness
4           Matrix brightness
5           Sounds on/off
...         Free space for game variables
128         Highscore
9->13
8->2
--------------------------------------------------
*/
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "LedControl.h"

struct
{
  char name[7];
  int score;
}listHighScore[5];

#define NOTE_E4 330
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587

int notesWin[] = {
    NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
};

// Durations (in ms) of each music note of the song
// Quarter Note is 250 ms when songSpeed = 1.0

int durationsWin[] = {
    125, 125, 250, 125, 125
};

const byte matrixSize = 8;
//EEPROM
const byte lengthHighScore = 0;
const byte difficulty = 1;
const byte lcdContrast = 2;
const byte lcdBrightness = 3;
const byte matrixBrightness = 4;
const byte sounds = 5;
const byte startHighScore = 128;
//

const byte dinPin = 12;
const byte loadPin = 11;
const byte clockPin = 10;
const byte rs = 13;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 2;
const byte d6 = 5;
const byte d7 = 4;
const byte buzzerPin = 3;
const byte randomPin = 0;


const int pinSW = A2;  // digital pin connected to switch output
const int pinX = A1;  // A0 - analog pin connected to X output
const int pinY = A0;
const int LCD_Contrast = 9;
const int LCD_Backlight = 6;

int xValue = 0;
int yValue = 0;
int swValue = 0;
int lastSwValue = 0;
byte buttonPressed = 0;
bool joyMoved = false;
int buzzerTone1 = 1000;
int buzzerTone2 = 700;
int buzzerTone3 = 500;
int buzzerTone4 = 300;

//byte buttonPressed = 0;
unsigned long lastDebounceTimeForButton = 0;
unsigned long timeForLetter = 300;
unsigned long lastTimeForLetter = 0;
unsigned long timeForPause = 1500;
unsigned long lastTimeForPause = 0;
unsigned long debounceDelay = 100;
unsigned long lastShown = 0;
unsigned long debounceDelayShown = 20000;
unsigned long TimeInteraction = 300;
unsigned long lastTimeInteraction = 0;
unsigned long TimeInteractionSnake = 100;
unsigned long lastTimeInteractionSnake = 0;

int difficultyLevel = 0;
int lcdContrastLevel = 0;
int lcdBrightnessLevel = 0;
int matrixBrightnessLevel = 0;
bool soundLevel = true;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

char name[] = "aaaaaa";
char nameCopy[]="aaaaaa";//a copy of the name
int letterPosition = 0;
int letterValue = (int)name[letterPosition];

const float songSpeed = 1.0;
byte messageShown = 0;       // if the initial message was shown
int timeForExposure = 3000;  // the time in ms for message
int timeExposed;
bool clear = false;  // if the lcd should be cleared
int state = -1;       // 0-menu, 1-startgame, 2-highScore, 3-settings, 4-about, 5-howtoplay
String options[] = { "Start game", "High score", "Settings", "About", "How to play", "Reset" };
int position = 0;  // the index of options[] that will be displayed
int state2 = 0;
int position2 = 0;
String options2[] = { "Enter name", "Difficulty", "LCD contrast", "LCD brightness", "Matrix brightness", "Sound on/off" };
char msg[] = "Game created by Marius Dumitrescu, University of Bucharest githubLink: https://github.com/Marius2504";

char msg1[] = "Move the snake with joystick and collect apples to increase your score";
bool shown = false;  //is the variable that says if the text should be displayed or not
bool read = false;   //if the data was read
int value;           //value read from EEPROM
int buzzInterval = 100;
int lowerOffset = 400;
int higherOffest = 600;
bool blink = true;
//highscore variables
int numberOfChr = 0;
bool generated = false;  //if the apple was generated

//game variables
int xSnake = 2;
int ySnake = 0;
struct pos {
  int x;
  int y;
};
pos length[64];
int score = 0;
int lengthSnake = 3;
int xApple = 4;
int yApple = 4;
int randomNumber = 0;
int nextMove = 1;  //0-sus,1-dreapta, 2-jos,3-stanga
bool GameOver = false;

//characters 

byte upArrow[8] = {
	0b00000,
	0b00100,
	0b00100,
	0b01110,
	0b01110,
	0b11111,
	0b11111,
	0b00000
};
byte downArrow[8] = {
	0b00000,
	0b11111,
	0b11111,
	0b01110,
	0b01110,
	0b00100,
	0b00100,
	0b00000
};
byte block[8] = {
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};

//String options2[] = {"Enter name","Difficulty","LCD contrast","LCD brightness","Matrix brightness","Sound on/off"};

byte playFigure[8]={0x00,0x60,0x78,0x7C,0x7C,0x78,0x60,0x00};
byte highScoreFigure[8] = {0x00,0x7E,0x42,0x3C,0x18,0x18,0x3C,0x00};
byte settingsFigure[8] = { 0x00,0x3E,0x7E,0x0C,0x0C,0x0C,0x0C,0x00 };
byte aboutFigure[8] = {0x00,0x8F,0x8C,0xDF,0xAC,0x8F,0x00,0x00};
byte howToPlayFigure[8]={0x00,0x1C,0x22,0x02,0x04,0x08,0x00,0x08};
byte resetFigure[8]={0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81};
byte nameFigure[8]={0x00,0x24,0x34,0x2C,0x24,0x00,0x7E,0x00};
byte difficultyFigure[8]={0x00,0x30,0x28,0x28,0x30,0x00,0x7E,0x00};
byte lcFigure[8]={0x00,0x46,0x48,0x48,0x76,0x00,0x7E,0x00};
byte lbFigure[8]={0x4C,0x4A,0x4E,0x4A,0x6C,0x00,0x7E,0x00};
byte mbFigure[8]={0x44,0x6C,0x54,0x44,0x44,0x00,0x7C,0x00};
byte soundFigure[8]={0x04,0x22,0x69,0xE5,0xE5,0x69,0x22,0x04};


//Upper and lower limits for map
byte lowerLimitSettingsLevelLCD = 0;
byte upperLimitSettingsLevelLCD = 16;
byte lowerLimitSettingsLevelANALOG = 0;
byte upperLimitSettingsLevelANALOG = 255;
void setup() {
  Serial.begin(9600);
 // String cc = "marius:1000, ion:2000           \n";
 // EEPROM.update(0, cc.length());
 // EEPROM.put(128, "marius:1000, ion:2000           \n");
  //listHighScore[0].name = "1asda";
  /*
  strcpy(listHighScore[0].name,"marius");
  listHighScore[0].score = 8;
  
  strcpy(listHighScore[1].name,"mihaii");
  listHighScore[1].score = 5;

  strcpy(listHighScore[2].name,"robert");
  listHighScore[2].score = 3;

  strcpy(listHighScore[3].name,"george");
  listHighScore[3].score = 2;

  strcpy(listHighScore[4].name,"ancaaa");
  listHighScore[4].score = 1;

  EEPROM.put(startHighScore,listHighScore);
*/
  setLCD();  //set the initial values for contrast and brightness
  length[0].x = 0;
  length[0].y = 0;
  length[1].x = 1;
  length[1].y = 0;
  length[2].x = 2;
  length[2].y = 0;

  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  lcd.begin(16, 2);
  lc.shutdown(0, false);
  lc.setIntensity(0, 1);
  lc.clearDisplay(0);
  initializeMatrix(true);
  randomSeed(analogRead(randomPin));

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  lcd.createChar(2, block);
}

void loop() {
  if (state == -1 && messageShown == 0)
    message();

  if (state == -1 && clear == true && millis() - timeExposed > timeForExposure) {
    lcd.clear();
    clear = false;
    state = 0;
  }
  
  switch (state) {
    case 0: menu(); break;
    case 1: startGame(); break;
    case 2: highscore(); break;
    case 3: settings(); break;
    case 4: about(); break;
    case 5: howToPlay(); break;
    case 6: reset(); break;
  }
}
void menu() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (joyMoved == false) {
    if (yValue < lowerOffset && position > 0) {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone1, buzzInterval);
      position--;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && position < 5) {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone1, buzzInterval);
      position++;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
    int i;
    if(position == 0)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,playFigure[i]);
    if(position == 1)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,highScoreFigure[i]);
    if(position == 2)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,settingsFigure[i]);
    if(position == 3)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,aboutFigure[i]);
    if(position == 4)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,howToPlayFigure[i]);
    if(position == 5)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,resetFigure[i]);
    
    lcd.clear();
    waitInteraction();
    lcd.print(options[position]);
    if(position>0)
    {
      lcd.setCursor(15, 0);
      lcd.write((byte)0);
    }
    if(position<5)
    {
      lcd.setCursor(15, 1);
      lcd.write((byte)1);
    }
    shown = true;
  }
  if (xValue < lowerOffset) {
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone2, buzzInterval);
    state = position + 1;  //because state = 0 is for menu
                           //  writeToRow2("High scores: marius:1000, ion:2000           ");
    joyMoved = true;
    shown = false;
    waitInteraction();
  }
}

void startGame() {

  for (int i = 0; i < lengthSnake; i++){
    Serial.print(length[i].x);
    Serial.print("-");
    Serial.print(length[i].y);
    Serial.print(" ");
  }
  Serial.println(" ");
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  swValue = !digitalRead(pinSW);
  
  if (GameOver == false) {
    if (millis() - lastTimeInteractionSnake > TimeInteractionSnake) {
      lastTimeInteractionSnake = millis();
      shown = false;

      //fruit generator
      if (generated == false) {
        //xApple = 3;
        //yApple = 3;
        bool ok = false;
        while (ok == false) {
          ok = true;
          for (int i = 0; i < lengthSnake; i++)
            if (length[i].x == xApple && length[i].y == yApple)
              ok = false;
          if (ok == false) {
            xApple = random(0, 7);
            yApple = random(0, 7);
          }
        }
        generated = true;
      }

      //movement logic
      if (joyMoved == false) {
        if (yValue < lowerOffset && length[lengthSnake - 2].y != ySnake - 1) {
          if (soundLevel == true)
            tone(buzzerPin, buzzerTone1, buzzInterval);
          //ySnake--;
          nextMove = 0;
          joyMoved = true;
          //shown = false;
        }
        if (yValue > higherOffest && length[lengthSnake - 2].y != ySnake + 1) {
          if (soundLevel == true)
            tone(buzzerPin, buzzerTone1, buzzInterval);
          //ySnake++;
          nextMove = 2;
          joyMoved = true;
          //shown = false;
        }
        if (xValue < lowerOffset && length[lengthSnake - 2].x != xSnake + 1) {
          if (soundLevel == true)
            tone(buzzerPin, buzzerTone1, buzzInterval);
          //xSnake++;
          nextMove = 1;
          joyMoved = true;
          // shown = false;
        }
        if (xValue > higherOffest && length[lengthSnake - 2].x != xSnake - 1) {
          if (soundLevel == true)
            tone(buzzerPin, buzzerTone1, buzzInterval);
          //xSnake--;
          nextMove = 3;
          joyMoved = true;
          // shown = false;
        }
        //lc.setLed(0, ySnake, xSnake, true);
      }
      if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest && xValue > lowerOffset && xValue < higherOffest)
        joyMoved = false;

      //timer for next move
      switch (nextMove) {
        case 0: ySnake--; break;
        case 1: xSnake++; break;
        case 2: ySnake++; break;
        case 3: xSnake--; break;
      }
      if (ySnake == matrixSize)
        ySnake = 0;
      if (ySnake == -1)
        ySnake = matrixSize-1;

      if (xSnake == matrixSize)
        xSnake = 0;
      if (xSnake == -1)
        xSnake = matrixSize-1;


      //refrshing screen
      if (shown == false) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("    Score : ");
        lcd.print(score);
        lcd.setCursor(0, 1);
        lcd.print("Name:");
        lcd.print(name);
        for (int i = 0; i < lengthSnake; i++)
          if (xSnake == length[i].x && ySnake == length[i].y)
            GameOver = true;
        if (GameOver == true) {
          int okk=0;
          char str[100]="Congratulations on reaching ";
          char buf[10] = {};
          int nm = score;
          int ct1 = 0;
          while(nm!=0)
          {
            nm=nm/10;
            ct1++;
          }
          playWinSong();
          snprintf(buf, 10, "%d", score);
          strcat(str,buf);
          strcat(str," score");
          writeToRow2(str,29+ct1);
          EEPROM.get(startHighScore,listHighScore);
          for(int i = 0;i<5;i++)
            if(listHighScore[i].score <score){
              okk =1;
              if(i==1)
                okk =2;
              for(int j = 4;j>i;j--)
              {
                listHighScore[i].score = listHighScore[i-1].score;
                strcpy(listHighScore[i].name,listHighScore[i-1].name);
              }
              listHighScore[i].score =score;
              strcpy(listHighScore[i].name,name);
              EEPROM.put(startHighScore,listHighScore);
              i=5; // stop
            }
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Game over");
          if(okk==1)
            lcd.print(" Top 5");
          if(okk==2)
            lcd.print(" BEST");

          lcd.setCursor(0, 1);
          lcd.print("Score:");
          lcd.print(score);
          lcd.print(" ");
          lcd.print(name);

          xSnake = 2;
          ySnake = 0;
          score = 0;
          lengthSnake = 3;
          generated = false;
          nextMove = 1;
          length[0].x = 0;
          length[0].y = 0;
          length[1].x = 1;
          length[1].y = 0;
          length[2].x = 2;
          length[2].y = 0;
        }
        
        if (xSnake == xApple && ySnake == yApple) {
          tone(buzzerPin, buzzerTone1, buzzInterval);
          tone(buzzerPin, buzzerTone2, buzzInterval);
          tone(buzzerPin, buzzerTone3, buzzInterval);
          score++;
          if(difficultyLevel >1)
            score ++;
          lengthSnake++;
          for (int i = lengthSnake - 1; i > 0; i--)
            length[i] = length[i - 1];
          generated = false;  // the positions of fruit will be changed
        }
        initializeMatrix(false);
        for (int i = 0; i < lengthSnake - 1; i++)
          length[i] = length[i + 1];
        length[lengthSnake - 1].x = xSnake;
        length[lengthSnake - 1].y = ySnake;

        for (int i = 0; i < lengthSnake; i++)
          lc.setLed(0, length[i].y, length[i].x, true);
        lc.setLed(0, yApple, xApple, true);
        shown = true;
      }
    }
  }

  //exit
  
  if(swValue != lastSwValue&&  millis() - lastDebounceTimeForButton > debounceDelay)
    {
      GameOver = false;
      lastDebounceTimeForButton = millis();
      state = 0;//because state = 0 is for menu
      shown = false;
      waitInteraction();
      lcd.clear();
    }
}

void highscore() {
  xValue = analogRead(pinX);
  if (read == false) {
    EEPROM.get(startHighScore,listHighScore);
    read = true;
  }
  if (shown == false) {
    numberOfChr = 40;
    lcd.clear();
    char str[100]="";
    char buf[10] = {};
    int nm = 0;
    int ct1 = 0;
    for(int i =0;i<5;i++)
    {
      strcat(str,listHighScore[i].name);
      strcat(str,":");
      nm = listHighScore[i].score;
      while(nm!=0)
      {
        nm=nm/10;
        ct1++;
      }
      numberOfChr +=ct1;
      snprintf(buf, 10, "%d", listHighScore[i].score);  
      strcat (str, buf);
      strcat(str," ");
    }
    strcat(str,"\0");
    writeToRow2(str, strlen(str));
    lastShown = millis();
    shown = true;
  }
  if (shown == true && millis() - lastShown > debounceDelayShown)
    shown = false;

  if (xValue > higherOffest) {
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    state = 0;  //because state = 0 is for menu
    shown = false;
    waitInteraction();
  }
}
//String options2[] = {"Enter name","Difficulty","LCD contrast","LCD brightness","Matrix brightness","Sound on/off"};
void settings() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  
  if (state2 != 0) {
    // read = false;
    switch (state2) {
      case 1: EnterName(); break;
      case 2: Difficulty(); break;
      case 3: LCD_contrast(); break;
      case 4: LCD_brightness(); break;
      case 5: Matrix_brightness(); break;
      case 6: Sound(); break;
    }
  }
  if (joyMoved == false && state2 == 0) {
    if (yValue < lowerOffset && position2 > 0) {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone1, buzzInterval);
      position2--;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && position2 < 5) {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone1, buzzInterval);
      position2++;
      joyMoved = true;
      shown = false;
    }
  }

  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest && xValue > lowerOffset && xValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
    waitInteraction();
    lcd.clear();
    lcd.print(options2[position2]);

    int i;
    if(position2 == 0)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,nameFigure[i]);
    if(position2 == 1)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,difficultyFigure[i]);
    if(position2 == 2)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,lcFigure[i]);
    if(position2 == 3)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,lbFigure[i]);
    if(position2 == 4)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,mbFigure[i]);
    if(position2 == 5)
      for(i=0;i<matrixSize;i++)
        lc.setRow(0,i,soundFigure[i]);

    if(position2>0)
    {
      lcd.setCursor(15, 0);
      lcd.write((byte)0);
    }
    if(position2<5)
    {
      lcd.setCursor(15, 1);
      lcd.write((byte)1);
    }
    shown = true;
  }
  if (xValue < lowerOffset && joyMoved == false) {
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone2, buzzInterval);
    state2 = position2 + 1;
    joyMoved = true;
    shown = false;
    waitInteraction();
  }
  if (xValue > higherOffest && joyMoved == false) {
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    state = 0;  //because state = 0 is for menu
    shown = false;
    joyMoved = true;
    waitInteraction();
  }
}

void about() {
  xValue = analogRead(pinX);
  if (shown == false) {
    lcd.clear();
    writeToRow2(msg, strlen(msg));
    lastShown = millis();
    shown = true;
  }
  if (shown == true && millis() - lastShown > debounceDelayShown)
    shown = false;

  if (xValue > higherOffest) {
    state = 0;  //because state = 0 is for menu
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
void howToPlay() {
  xValue = analogRead(pinX);
  if (shown == false) {
    
    lcd.clear();
    writeToRow2(msg1, strlen(msg1));
    lastShown = millis();
    shown = true;
  }
  if (shown == true && millis() - lastShown > debounceDelayShown)
    shown = false;

  if (xValue > higherOffest) {
    state = 0;  //because state = 0 is for menu
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
void reset() {
  xValue = analogRead(pinX);
  if (shown == false) {
    EEPROM.get(lengthHighScore,numberOfChr);
    if (numberOfChr == 0) {
      lcd.clear();
      lcd.print("Null score");
      shown = true;
    } else {
      lcd.clear();
      lcd.print("Reset highscores?");
      shown = true;
      
    }
  }
  if(xValue<lowerOffset)
  {
    for(int i=0;i<6;i++){
      strcpy(listHighScore[i].name,"");
      listHighScore[i].score = 0;
    }
    EEPROM.put(startHighScore,listHighScore);
    EEPROM.put(lengthHighScore,0);
  }
  
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest && xValue > lowerOffset && xValue < higherOffest)
    joyMoved = false;

  if (xValue > higherOffest) {
    state = 0;  //because state = 0 is for menu
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
void message() {
  lcd.clear();
  char cuv[] = "Welcome to the game                ";
  writeToRow2(cuv,strlen(cuv));
  messageShown = 1;
  timeExposed = millis();
  clear = true;
}

//A function that will write on the second row a given word
void writeToRow2(char cuv[], int number) {
  lcd.setCursor(0, 1);
  char ecran[] = "                ";
  int ct = 15;
  int index = 0;
  int index2 = 1;
  bool finish = false;
  for (int i = 0; i < number; i++) {
    xValue = analogRead(pinX);

    if (xValue > higherOffest) {
      break;
    }
    if (finish == false)
      index = 0;
    else {
      index = index2;
      index2++;
    }
    for (int j = ct; j < 16; j++) {
      ecran[j] = cuv[index];
      index++;
    }
    
    lcd.clear();
    lcd.print(ecran);
    while (millis() - lastTimeForLetter < timeForLetter) {
    }
    lastTimeForLetter = millis();
    if (ct == 0 && finish == false)
      finish = true;

    if (ct > 0)
      ct--;
  }
  //delay(1500);
  if (xValue < higherOffest) {
    while (millis() - lastTimeForPause < timeForPause) {
    }
  }
  lastTimeForPause = millis();
}

void EnterName() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  

  if (joyMoved == false) {
    if (yValue < lowerOffset && letterValue < (int)'z') {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone1, buzzInterval);
      letterValue++;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && letterValue > (int)'a') {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone1, buzzInterval);
      letterValue--;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == false) {
    if (xValue < lowerOffset && letterPosition < 6) {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone4, buzzInterval);
      letterPosition++;
      letterValue = name[letterPosition];
      joyMoved = true;
      shown = false;
    }
    if (xValue > higherOffest && letterPosition == 0) {
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone4, buzzInterval);
      state2 = 0;  //because state = 0 is for menu
      joyMoved = true;
      shown = false;
      waitInteraction();
    }
    if (xValue > higherOffest && letterPosition > 0) {
      letterPosition--;
      letterValue = name[letterPosition];
      joyMoved = true;
      shown = false;
      if (soundLevel == true)
        tone(buzzerPin, buzzerTone3, buzzInterval);
    }
  }
  if (shown == false && state2 != 0) {
    name[letterPosition] = (char)letterValue;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("      Name");
    lcd.setCursor(0, 1);
    lcd.print(name);
    shown = true;
    waitInteraction();
    strcpy(nameCopy,name);
  }
  
  if(millis() - lastTimeForLetter > timeForLetter)
  {
    switch(blink)
    {
      case true:nameCopy[letterPosition] = (char)letterValue;break;
      case false:nameCopy[letterPosition] = ' ';break;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("      Name");
    lcd.setCursor(0, 1);
    lcd.print(nameCopy);
    
    lastTimeForLetter = millis();
    blink = !blink;
  }

  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest && xValue > lowerOffset && xValue < higherOffest)
    joyMoved = false;
}


void Difficulty() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (read == false) {
    difficultyLevel = EEPROM.read(difficulty);
    read = true;
  }

  if (joyMoved == false) {
    if (yValue < lowerOffset && difficultyLevel < 3) {
      difficultyLevel++;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && difficultyLevel > 0) {
      difficultyLevel--;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Dificulty");
    lcd.setCursor(0, 1);
    switch (difficultyLevel) {
      case 0: lcd.print("Easy"); break;
      case 1: lcd.print("Medium"); break;
      case 2: lcd.print("Hard"); break;
      case 3: lcd.print("Impossible"); break;
    }
    shown = true;
  }

  if (xValue > higherOffest) {
    EEPROM.update(difficulty, difficultyLevel);
    switch (difficultyLevel) {
      case 0: TimeInteractionSnake = 200;break;
      case 1: TimeInteractionSnake = 130;break;
      case 2: TimeInteractionSnake = 80;break;
      case 3: TimeInteractionSnake = 50;break;
    }

    state2 = 0;  //because state = 0 is for menu
    joyMoved = true;
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
void LCD_contrast() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (read == false) {
    lcdContrastLevel = EEPROM.read(lcdContrast);
    read = true;
  }

  if (joyMoved == false) {
    if (yValue < lowerOffset && lcdContrastLevel < 16) {
      lcdContrastLevel++;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && lcdContrastLevel > 0) {
      lcdContrastLevel--;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
    lcd.clear();
    analogWrite(LCD_Contrast, map(lcdContrastLevel, lowerLimitSettingsLevelLCD, upperLimitSettingsLevelLCD, lowerLimitSettingsLevelANALOG, upperLimitSettingsLevelANALOG));
    lcd.setCursor(0, 0);
    lcd.print("   LCD Contrast");
    lcd.setCursor(0, 1);
    for (int i = 0; i < lcdContrastLevel; i++)
      lcd.write((byte)2);
    shown = true;
  }

  if (xValue > higherOffest) {
    EEPROM.update(lcdContrast, lcdContrastLevel);
    state2 = 0;  //because state = 0 is for menu
    joyMoved = true;
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
/*
const int LCD_Contrast = 11;
const int LCD_Backlight= 10;
*/
void LCD_brightness() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (read == false) {
    lcdBrightnessLevel = EEPROM.read(lcdBrightness);
    read = true;
  }

  if (joyMoved == false) {
    if (yValue < lowerOffset && lcdBrightnessLevel < 16) {
      lcdBrightnessLevel++;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && lcdBrightnessLevel > 0) {
      lcdBrightnessLevel--;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   LCD Brightness");
    lcd.setCursor(0, 1);
    for (int i = 0; i < lcdBrightnessLevel; i++)
      lcd.write((byte)2);
    shown = true;

    analogWrite(LCD_Backlight, map(lcdBrightnessLevel, lowerLimitSettingsLevelLCD, upperLimitSettingsLevelLCD, lowerLimitSettingsLevelANALOG, upperLimitSettingsLevelANALOG));
  }

  if (xValue > higherOffest) {
    EEPROM.update(lcdBrightness, lcdBrightnessLevel);
    state2 = 0;  //because state = 0 is for menu
    joyMoved = true;
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
void Matrix_brightness() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (read == false) {
    matrixBrightnessLevel = EEPROM.read(matrixBrightness);
    read = true;
  }

  if (joyMoved == false) {
    if (yValue < lowerOffset && matrixBrightnessLevel < 16) {
      matrixBrightnessLevel++;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && matrixBrightnessLevel > 0) {
      matrixBrightnessLevel--;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
  
    lc.setIntensity(0, matrixBrightnessLevel);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Matrix Brightness");
    lcd.setCursor(0, 1);
    for (int i = 0; i < matrixBrightnessLevel; i++)
      lcd.write((byte)2);
    shown = true;
    shown = true;
  }

  if (xValue > higherOffest) {
    EEPROM.update(matrixBrightness, matrixBrightnessLevel);
    state2 = 0;  //because state = 0 is for menu
    joyMoved = true;
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}


void Sound() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (read == false) {
    soundLevel = EEPROM.read(sounds);
    read = true;
  }

  if (joyMoved == false) {
    if (yValue < lowerOffset && soundLevel == false) {
      soundLevel = true;
      joyMoved = true;
      shown = false;
    }
    if (yValue > higherOffest && soundLevel == true) {
      soundLevel = false;
      joyMoved = true;
      shown = false;
    }
  }
  if (joyMoved == true && yValue > lowerOffset && yValue < higherOffest)
    joyMoved = false;

  if (shown == false) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("     Sound");
    lcd.setCursor(0, 1);
    if (soundLevel == false)
      lcd.print("OFF");
    else
      lcd.print("ON");
    shown = true;
  }

  if (xValue > higherOffest) {
    EEPROM.update(sounds, soundLevel);
    state2 = 0;  //because state = 0 is for menu
    joyMoved = true;
    shown = false;
    if (soundLevel == true)
      tone(buzzerPin, buzzerTone3, buzzInterval);
    waitInteraction();
  }
}
void initializeMatrix(bool valueFor) {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, valueFor);  // turns on LED at col, row
    }
  }
}

void waitInteraction() {
  lastTimeInteraction = millis();
  while (millis() - lastTimeInteraction < TimeInteraction) {}
}

void setLCD() {
  lcdContrastLevel = EEPROM.read(lcdContrast);
  lcdBrightnessLevel = EEPROM.read(lcdBrightness);
  soundLevel = EEPROM.read(sounds);
  difficultyLevel = EEPROM.read(difficulty);
  matrixBrightnessLevel = EEPROM.read(matrixBrightness);
  analogWrite(LCD_Contrast, map(lcdContrastLevel, lowerLimitSettingsLevelLCD, upperLimitSettingsLevelLCD, lowerLimitSettingsLevelANALOG, upperLimitSettingsLevelANALOG));
  analogWrite(LCD_Backlight, map(lcdBrightnessLevel, lowerLimitSettingsLevelLCD, upperLimitSettingsLevelLCD, lowerLimitSettingsLevelANALOG, upperLimitSettingsLevelANALOG));
  
  switch (difficultyLevel) {
      case 0: TimeInteractionSnake = 200;break;
      case 1: TimeInteractionSnake = 130;break;
      case 2: TimeInteractionSnake = 80;break;
      case 3: TimeInteractionSnake = 50;break;
    }
}

void playWinSong()
{
  const int totalNotes = sizeof(notesWin) / sizeof(int);
  // Loop through each note
  for (int i = 0; i < totalNotes; i++)
  {
    const int currentNote = notesWin[i];
    float wait = durationsWin[i] / songSpeed;
    // Play tone if currentNote is not 0 frequency, otherwise pause (noTone)
    if (currentNote != 0)
      tone(buzzerPin, notesWin[i], wait); // tone(pin, frequency, duration)
    else
      noTone(buzzerPin);
    // delay is used to wait for tone to finish playing before moving to next loop
    waitInteraction();
  }
}
