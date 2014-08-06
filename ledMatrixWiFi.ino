/*
 * Uses FrequencyTimer2 library to
 * constantly run an interrupt routine
 * at a specified frequency. This
 * refreshes the display without the
 * main loop having to do anything.
 *
 *
 *
 */

//#include <FrequencyTimer2.h>
#include <TimerOne.h>
#include <avr/pgmspace.h>
#include "Symbols.h"

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>



byte row = 0;//********** row col change
byte leds[8][8];

// pin[xx] on led matrix connected to nn on Arduino (-1 is dummy to make array start at pos 1)
byte pins[17]= {-1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0 ,A1 ,A2 ,A3};

// col[xx] of leds = pin yy on led matrix
byte cols[8] = {pins[2], pins[8], pins[16], pins[1], pins[12], pins[15], pins[11], pins[14]};
//
// row[xx] of leds = pin yy on led matrix
byte rows[8] = {pins[7], pins[6], pins[4], pins[13], pins[3], pins[9], pins[10], pins[5]};
/*
// col[xx] of leds = pin yy on led matrix
int cols[8] = {pins[5], pins[10], pins[9], pins[3], pins[13], pins[4], pins[6], pins[7]};
//
// row[xx] of leds = pin yy on led matrix
int rows[8] = {pins[2], pins[8], pins[16], pins[1], pins[11], pins[15], pins[12], pins[14]};
*/
byte symbols[][8][8] PROGMEM = 
{ 
  FULLBRIGHT8x8,
  SPACE8x8,
  SMILE8x8,
  FROWN8x8,
  CATFACE8x8,
  SMILE08x8,
  SMILE18x8,
  SMILE28x8,
  SMILE38x8,
  SMILE48x8,
  EXCLAIM8x8,
  QUESTION8x8,
  FULLSTOP8x8,
  A8x8,
  B8x8,
  C8x8,
  D8x8,
  E8x8,
  F8x8,
  G8x8,
  H8x8,
  I8x8,
  J8x8,
  K8x8,
  L8x8,
  M8x8,
  N8x8,
  O8x8,
  P8x8,
  Q8x8,
  R8x8,
  S8x8,
  T8x8,
  U8x8,
  V8x8,
  W8x8,
  X8x8,
  Y8x8,
  Z8x8
};

//byte currentSymbol[8][8];

enum symbolLib
{
  FULLBRIGHT,
  SPACE,
  SMILE,
  FROWN,
  CATFACE,
  SMILE0,
  SMILE1,
  SMILE2,
  SMILE3,
  SMILE4,
  EXCLAIM,
  QUESTION,
  FULLSTOP,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z
};



byte patternHelloWorld [] =//start with space
{
  H,E,L,L,O,SPACE,W,O,R,L,D,EXCLAIM
};


byte patternTest [] =//start with space
{
  A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,SPACE
};

byte patternFrown2Smile [] =
{
  SMILE0,SMILE1,SMILE2,SMILE3,SMILE4
};

byte *patternCompendium[] =
{
  patternHelloWorld,patternTest,patternFrown2Smile
};

enum patternLib
{
  numHelloWorld,
  numTest,
  numFrown2Smile
};
byte patternSize[] = //manually set until elegant solution is found
{
  sizeof(patternHelloWorld),
  sizeof(patternTest),
  sizeof(patternFrown2Smile)
};
int input = 0;
int symbol = 0;
int pattern = 0;
int sequence = 0;
bool boolAnimate = false;
int style;
enum animateStyle
{
  aniSlideLeft,
  aniSlideUp,
  aniAnimate
};

int numCycles = -1;
int lingeringImage;

int timeUpdate = 0;
bool commandProcessed;

YunServer server;

void setup() {
  // sets the pins as output
  for (int i = 1; i <= 16; i++) {
    pinMode(pins[i], OUTPUT);
  }

  // set up cols and rows
  for (int i = 1; i <= 8; i++) {
    digitalWrite(cols[i - 1], LOW);
  }

  for (int i = 1; i <= 8; i++) {
    digitalWrite(rows[i - 1], LOW);
  }

  clearLeds();
  
/*
  // Turn off toggling of pin 11
  FrequencyTimer2::disable();
  // Set refresh rate (interrupt timeout period)
  FrequencyTimer2::setPeriod(2000);
  // Set interrupt routine to be called
  FrequencyTimer2::setOnOverflow(display);
  */
  
  Timer1.initialize(2000);
  Timer1.attachInterrupt(display);

  setSymbol(SMILE);//smile
  
  Bridge.begin();
  Serial.begin(9600);
  
  
  server.listenOnLocalhost();
  server.begin();

  //Serial.println(sizeof(&patternCompendium[0]));
  //while (!Serial);

}

void loop() 
{
  
  
  YunClient client = server.accept();
  
  if(client)
  {
    process(client);
    client.stop();
  }
  
  //while(Serial.available() > 0)
  {
    
    //Serial.print("loop");
    //Serial.println(input);
    
    //if (Serial.read() == '/n')
    {
      switch(input)
      {
        case 1:
        {
          sequence = numHelloWorld;
          boolAnimate = true;
          style = aniSlideLeft;
          numCycles = -1;
          input = 0;
          break;
        }
        case 2:
        {
          sequence = numTest;
          boolAnimate = true;
          style = aniSlideLeft;
          numCycles = 2;
          lingeringImage = QUESTION;
          input = 0;
          break;
        }
        case 3:
        {
          setSymbol(SMILE);
          //Serial.println("set smile");
          boolAnimate = false;
          input = 0;
          break;
        }
        case 4:
        {
          setSymbol(FROWN);
          //Serial.println("set frown");
          boolAnimate = false;
          input = 0;
          break;
        }
        case 5:
        {
          setSymbol(CATFACE);
          //Serial.println("set catface");
          boolAnimate = false;
          input = 0;
          break;
        }
        case 6:
        {
          sequence = numFrown2Smile;
          boolAnimate = true;
          style = aniAnimate;
          numCycles = 1;
          lingeringImage = SMILE4;
          input = 0;
          break;
        }
        case 7:
        {
          sequence = numHelloWorld;
          boolAnimate = true;
          style = aniSlideUp;
          numCycles = -1;
          lingeringImage = SMILE4;
          input = 0;
          break;
        }
        case 8:
        {
          sequence = numHelloWorld;
          boolAnimate = true;
          style = aniAnimate;
          numCycles = 1;
          lingeringImage = QUESTION;
          input = 0;
          break;
        }
        default:
        {
          //pattern = 0;
        }
      }
    }
    
  }
  if(numCycles > 0 || numCycles == -1)
  {
    if(boolAnimate)
    {

      switch(style)
      {
        case aniSlideLeft:
        {
          slidePatternLeft(sequence, pattern, 60);
          break;
        }
        case aniSlideUp:
        {
          slidePatternUp(sequence, pattern, 60);
          break;
        }
        case aniAnimate:
        {
          animate(sequence, pattern, 200,0);
          break;
        }
        
        default:
        {
          
        }
        
      }
      pattern = ++pattern;
      if(pattern >= patternSize[sequence])//end of hello world!//-1 for ghetto fix sequences so I don't access some random memory...
      {
        pattern = 0;//beginning of hello world
        if(numCycles != -1)
        {
          numCycles--;
        }
      }  
      
      
    }
  }
  else if(boolAnimate)
  {
    setSymbol(lingeringImage);
  }
  //Serial.println(numCycles);
}

void clearLeds() {
  // Clear display array
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      leds[i][j] = 0;
    }
  }
}

void setSymbol(int symbol) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      leds[i][j] = pgm_read_byte(&(symbols[symbol][i][j]));
      //pgm_read_byte(&(symbols[symbol]
    }
  }
}

void slidePatternLeft(int sequence, int pattern, int del) {
  //***** 1 col buffer
  /*
  for (int i = 0; i < 7; i++) 
  {
    for (int j = 0; j < 8; j++) 
    {
      leds[j][i] = leds[j][i+1]; //#1
    }
  }
  for (int j = 0; j < 8; j++) 
  {  
    leds[j][7] = 0;
  } 
  delay(del);
  */
  //*****
  for (int l = 0; l < 8; l++) //L
  {
    for (int i = 0; i < 7; i++) 
    {
      for (int j = 0; j < 8; j++) 
      {
        leds[j][i] = leds[j][i+1]; //#1
      }
    }
    for (int j = 0; j < 8; j++) 
    {
      //pgm_read_byte(&(symbols[1][2][0]))
      leds[j][7] = pgm_read_byte(&(symbols[patternCompendium[sequence][pattern]][j][0 + l]));
      //leds[j][7] = symbols[patternCompendium[sequence][pattern]][j][0 + l];//L
    }
    delay(del);
  }
}

void slidePatternUp(int sequence, int pattern, int del)
{
  //***** 1 row buffer
  for (int i = 0; i < 8; i++) 
  {
    for (int j = 0; j < 7; j++) 
    {
      leds[j][i] = leds[j+1][i]; //#1
    }
  }
  for (int j = 0; j < 8; j++) 
  {
    leds[7][j] = 0;//L
  }
  delay(del);
  //*****
  for (int l = 0; l < 8; l++) //L
  {
    for (int i = 0; i < 8; i++) 
    {
      for (int j = 0; j < 7; j++) 
      {
        leds[j][i] = leds[j+1][i]; //#1
      }
    }
    for (int j = 0; j < 8; j++) 
    {
      leds[7][j] = pgm_read_byte(&(symbols[patternCompendium[sequence][pattern]][0+l][j]));//L
    }
    delay(del);
  }
}  

void animate(int sequence, int pattern, int del, int offDel)
{
  clearLeds();
  delay(offDel);
  for (int i = 0; i < 8; i++) 
  {
    for (int j = 0; j < 8; j++) 
    {
      leds[j][i] = pgm_read_byte(&(symbols[patternCompendium[sequence][pattern]][j][i]));
    }
  }
  delay(del);
}


// Interrupt routine
void display() {
  digitalWrite(rows[row], HIGH);// Turn whole previous column off
  row++;
  if (row == 8) {
    row = 0;
  }
  for (int col = 0; col < 8; col++) {
    if (leds[col][7 - row] == 1) {
        digitalWrite(cols[col], HIGH); // Turn on this led 
    }
    else {
      digitalWrite(cols[col], LOW); // Turn off this led
    }
  }
  digitalWrite(rows[row], LOW); // Turn whole column on at once (for equal lighting times)
}

void process(YunClient client)
{
  String command = client.readStringUntil('/');
  input = command.toInt();
  pattern = 0;
  
}
