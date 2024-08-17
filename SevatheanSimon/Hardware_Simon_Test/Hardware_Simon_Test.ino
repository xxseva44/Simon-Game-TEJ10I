/* BUILD PROPS */
#define DEBUG true//Enables serial debugging
#define DisplayI2C //Used to set display type
#define SoftwareVersion "0.1"
#define HardwareVersion "0.1"
/* END OF BUILD PROPS */

int BUTTON[4]={A1, A0, 3, 2}; //{B_RED, B_BLUE, B_GREEN, B_YELLOW}

int LED[4]={A3, A2, 5, 6}; //{RED, BLUE, GREEN, YELLOW}

//Piezo
#define Piezo 4
int PIEZO_TONE[4]={50, 70, 90, 100};

//debug
#define DEBUG true

//timer
unsigned long PrevTime = 0UL;

//display
#ifdef DisplayI2C
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);
#else
#include <LiquidCrystal.h>
#define Backlight 11
static uint8_t Brightness = 70;
LiquidCrystal lcd(7, 8, 9, 10, 12, 13);
#endif

void setup() {

  #ifndef DisplayI2C
  analogWrite(Backlight, Brightness);
  #endif
  #ifdef DisplayI2C
  lcd.init();
  lcd.backlight();
  #endif
  lcd.clear();
  

  //Buttons
  for(int b = 0; b < 4; b++){pinMode(BUTTON[b], INPUT);}
  //Piezo
  pinMode(Piezo, OUTPUT);
  //Leds
  for(int b = 0; b < 4; b++){pinMode(LED[b], OUTPUT);}

  Serial.begin(9600);
  if(DEBUG == true){
  DebugUtil("INIT DEBUG", "SOFTWARE VERSION", 'I' );
  DebugUtil("INIT DEBUG", SoftwareVersion, 'I' );
  DebugUtil("INIT DEBUG", "HARDWARE VERSION", 'I' );
  DebugUtil("INIT DEBUG", HardwareVersion, 'I' );
  }
}
void loop(){
  ButtonHandler();
  DisplayHandler("Hardware", 0, 0, 0);
  DisplayHandler("Test", 0, 1, 0);
}

void ButtonHandler(){
  char* F = "Button Handler";
  for(int c = 0; c < 4; c++){
    int Read = digitalRead(BUTTON[c]);
    if(Read == LOW){
      delay(5);
      if(Read == LOW){
        if(DEBUG == true){DebugUtil(F, "Button Hit!", 'I' );}
        tone(Piezo, PIEZO_TONE[c], 100);
        LedHandler(c, 60UL);
      }
    }
  }
}

/*
Led function to control leds.
Turns leds on and then after the inputted duration passes, it will turn the led off. 

LedHandler(COLOUR, DURATION);

BUG: When Duration is <= 50ms (50UL), function fails to clear leds.
*/
void LedHandler(int Colour, int Duration){
  char* F = "LED Handler";
  if(DEBUG == true){DebugUtil(F, "LED Requested!!", 'I' );}
  digitalWrite(LED[Colour], HIGH);
  if(DEBUG == true){
    switch(Colour){
      case 0:
      DebugUtil(F, "RED!", 'I' );
      break;
      case 1:
      DebugUtil(F, "BLUE!", 'I' );
      break;
      case 2:
      DebugUtil(F, "GREEN!", 'I' );
      break;
      case 3:
      DebugUtil(F, "YELLOW!", 'I' );
      break;
    }
  }
  if(Timer(Duration) == 1){
    digitalWrite(LED[Colour], LOW);
    if(DEBUG == true){DebugUtil(F, "LED Cleared.", 'I' );}
  }
}

/*
timer function to avoid complete system halts and
allows for simultaneous delays.
1000UL = 1s

if inputted time has passed, returns 1, if not, 0
*/
int Timer(int TimerDelay){
  char* F = "Timer";
  unsigned long CurrTime = millis();
  if(CurrTime - PrevTime >= TimerDelay){
    PrevTime = CurrTime;
    if(DEBUG == true){DebugUtil(F, "Timer Called", 'I' );}
    return 1;
    }
  else{return 0;}
}

void DisplayHandler(const char* msg, int x, int y, bool clear){
  char* F = "Display Handler";
  #ifndef DisplayI2C
  analogWrite(Backlight, Brightness);
  #endif
  if(clear == true){
    lcd.clear();
    //if(DEBUG == true){DebugUtil(F, "Display Cleared.", 'I' );}
  }
  //if(DEBUG == true){DebugUtil(F, "Drawing Display!", 'I' );}
  lcd.setCursor(x, y);
  lcd.print(msg);
}

/* Debug utility to simplify logging and debugging*/
void DebugUtil(char* Function, char* msg, char level){
  Serial.print(Function);
  switch(level){
    case 'I' :
    Serial.print(" | INFO: ");
    break;
    case 'W' :
    Serial.print(" | WARN: ");
    break;
    case 'E' :
    Serial.print(" | ERROR: ");
    break;
    case 'F' :
    while(level == 'f'){
      Serial.print(" | FATAL: ");
      Serial.println(msg);
      Serial.println("CRITIAL FAULT, HALTING!!!");
      Serial.println("PLEASE HARD RESET THE DEVICE TO RECOVER!!!");
    }
    break;
  }
  Serial.println(msg);
}