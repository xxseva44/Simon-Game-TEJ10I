/*  SUBSYSTEM DEBUG  */
//#define DEBUG_Util
//#define DEBUG_Device_Info
//#define DEBUG_LED
//#define DEBUG_Button
//#define DEBUG_Display
//#define DEBUG_Menu_Systems
//#define DEBUG_Timer
//#define DEBUG_SimonGame_User
//#define DEBUG_SimonGame_Check
//#define DEBUG_SimonGame_Main
//#define DEBUG_SimonGame_CPU
//#define DEBUG_SimonGame_CPU_Generator
//#define DEBUG_System_Navigation_Select
//#define DEBUG_System_Navigation_Position
/*  END OF SUBSYSTEM DEBUG  */

/* ################# BUILD PROPS ################# */
/* GENERIC INFO */
#define HardwareVersion "2.0"
#define BuildDate __DATE__ 

/* HARDWARE DEFINITION */
#define DisplayI2C //Used to set display type
#define ButtonQuantity 4
#define Piezo 4

//#define EEPROMEX  //Broken
/* ################# BUILD PROPS ################# */
int BUTTON[ButtonQuantity]={A1, A0, 3, 2}; //{B_RED, B_BLUE, B_GREEN, B_YELLOW}

int LED[4]={A3, A2, 5, 6}; //{RED, BLUE, GREEN, YELLOW}

//Piezo
int PIEZO_TONE[4]={100, 200, 300, 400};

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

#ifdef EEPROM
//EEPROM
#include <EEPROMex.h>
#endif

//Simon
#define SimonPatternMax 4
int EASY_PR_Score = 0;
int EASY_PR_Level = 0;
int MED_PR_Score = 0;
int MED_PR_Level = 0;
int HARD_PR_Score = 0;
int HARD_PR_Level = 0;
int CRY_PR_Score = 0;
int CRY_PR_Level = 0;
#ifdef EERPOM
#define AddrEasyScorePR 0
#define AddrEasyLvlPR 1
#define AddrMedScorePR 2
#define AddrMedLvlPR 3
#define AddrHardScorePR 4
#define AddrHardLvlPR 5
#define AddrCryScorePR 6
#define AddrCryLvlPR 7
#endif
const int MAX_RAND = 50; // Sets the max limit of possible pattern lengths
int CPU_SIMON_SEQUENCE[MAX_RAND];
int USER_SIMON_SEQUENCE[MAX_RAND];
int Step = 0;
int Index = 0;
bool USER_FAIL_FLAG = false;

//timer
unsigned long InitialTime = 0UL;

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

  randomSeed(analogRead(A2));

  Serial.begin(9600);
  #ifdef DEBUG_Device_Info
  DebugUtil(0, "INIT DEBUG", "Build Date:", 0, 'I' );
  DebugUtil(0, "INIT DEBUG", BuildDate, 0, 'I' );
  DebugUtil(0, "INIT DEBUG", "HARDWARE VERSION", 0, 'I' );
  DebugUtil(0, "INIT DEBUG", HardwareVersion, 0, 'I' );
  #endif

  //Restore previous PRs
  #ifdef EEPROM
  #ifdef DEBUG_Device_Info
  DebugUtil(0, "INIT", "Restoring Data From EEPROM!", 0, 'I' );
  #endif
  EASY_PR_Score = readLong(AddrEasyScorePR);
  EASY_PR_Level = readInt(AddrEasyLvlPR);
  MED_PR_Score = readLong(AddrMedScorePR);
  MED_PR_Level = readInt(AddrMedLvlPR);
  HARD_PR_Score = readLong(AddrHardScorePR);
  HARD_PR_Level = readInt(AddrHardLvlPR);
  CRY_PR_Score = readLong(AddrCryScorePR);
  CRY_PR_Level = readInt(AddrCryLvlPR);
  #endif 
}

#ifdef DEBUG_Util
/* 
Debug utility to simplify logging and debugging
Mode 0 = Text Only
Mode 1 = Text + Number

DEFAULTS TO MODE 1

*/
void DebugUtil(bool mode, char* Function, char* msg, int val, char level){
  Serial.print(Function);
  Serial.print(" | "); //divider
  switch(level){
    case 'I' :
    Serial.print("INFO:");
    break;
    case 'W' :
    Serial.print("WARN:");
    break;
    case 'E' :
    Serial.print("ERROR:");
    break;
    case 'F' :
    while(level == 'f'){
      Serial.print("FATAL: ");
      Serial.println(msg);
      Serial.println("CRITIAL FAULT, HALTING!!!");
      Serial.println("PLEASE HARD RESET THE DEVICE TO RECOVER!!!");
    }
    break;
  }
  Serial.print(" "); //spacer
  if(mode == 0){Serial.println(msg);}
  else if(mode == 1){
    Serial.print(msg); 
    Serial.print(" ");
    Serial.println(val);
  }
} 
#endif

void loop(){
  SystemUI();
}

/*
Button Function
Mode determines whether the function will check all buttons(Mode 1) or a specific button( Mode 0). 

COMMENT:
I like the behavior with the button acting on press rather than release. However i recognize that this means multiple presses can be registered in a row.
So, I added states to ensure that a button can't be abused and register multiple times -- either on purpose or by accident. 
I have only done this for the second mode(game mode) of the button function as I like this behaviour within menus.

NOTE: 
When mode 1, set button to 0(doesn't really matter but for the sake of neatness just do it.)
*/
int ButtonHandler(bool mode, int button){
  #ifdef DEBUG_Button
  char* F = "Button Handler";
  #endif
  static bool PrevButtonStates[ButtonQuantity] = {HIGH};
  for(int c = 0; c < ButtonQuantity; c++){
    int Read = digitalRead(BUTTON[c]);
    if(Read == LOW){
      switch(mode){
        case 0:
        if(button == c){
          delay(300);
          if(Read == LOW){
              tone(Piezo, 320, 50);
              return 1;
          }
        }
        break;
        case 1:
        delay(5);
        if((Read == LOW) && (digitalRead(BUTTON[c]) != PrevButtonStates[c])){
          PrevButtonStates[c]=LOW;
          LedHandler(c, 50);
          tone(Piezo, PIEZO_TONE[c], 50);
          if(c != CPU_SIMON_SEQUENCE[Step]){
            USER_FAIL_FLAG = true; //I'd rather not do this in here but this was the best i could think of for now. 
          }
          Step++;
          return c;
        }
        break;
      }
    }
  }
}

class SoundFX {
  public:
  void Winner(){
    tone(Piezo, 120);
    tone(Piezo, 160);
    tone(Piezo, 220);
    tone(Piezo, 90);
    noTone(Piezo);
  }
  void Loser(){
    tone(Piezo, 220);
    delay(500);
    tone(Piezo, 180);
    delay(500);
    tone(Piezo, 80);
    delay(500);
    tone(Piezo, 60);
    delay(500);
    noTone(Piezo);
  }
};

class GameSystem {
  public:
  int Duration; //Duration of the CPU play time
  int Time; //Time for player to input their pattern
  int Difficulty; //Difficulty used to set starting point for GamePatternLength
  int GamePatternLength; //Used to derive the length of patterns
  int Time_Multiplier;
  int Duration_Multiplier;
  int Lvl = 1; //Current level of the user
  int Score = 0; // Score of the user
  int Score_Multiplier;
  char* Disp_Difficulty; //Difficulty to display on displays
  /*
  Increases the difficulty as the player succeeds.
  */
  void IncrementLevel(){
    Lvl++;
  }

  void IncrementScore(){
    Score = Score + Duration*2;
  }
  void IncrementChallenge(){
    if(Time > 0){
      Time = Time - 1*Time_Multiplier;
    }
    if(Duration > 0){
      Duration = Duration - 1*Duration_Multiplier;
    }

    GamePatternLength++;
  }
};


//Note: If possible, avoid using OptionSelect and NavigationSelect simultaineously. This causes delayed button responses.
class MenuSystems {
  private:
  char* F = "MenuSystem";
  public:
  static int Menu_Quantity;
  static int Menu_Index;
  static bool Menu_Enter;
  static bool Option_Select;
  static int Option_Index;
  static int Option_Quantity;
  

  void NavigationSelect(){
    if(ButtonHandler(0,3)==true){
      Menu_Enter = true;
    }
    if(ButtonHandler(0,0)==true){
      Menu_Enter = false;
    }
  }

  void OptionSelect(){
    if(ButtonHandler(0,3)==true){
      Option_Select = true;
    }
    else if(ButtonHandler(0,0)==true){
      Option_Select = false;
    }
  }

  void NavigationPosition(){
    if(ButtonHandler(0,1)==1){
      Menu_Index--;
      #ifdef DEBUG_Menu_Systems
      DebugUtil(1, F, "Menu Moved Left:", Menu_Index, 'I' );
      #endif
      }
    else if(ButtonHandler(0,2)==1){
      Menu_Index++;
      #ifdef DEBUG_Menu_Systems
      DebugUtil(1, F, "Menu Moved Right:", Menu_Index, 'I' );
      #endif
      }
    if(Menu_Index < 0){Menu_Index = Menu_Quantity;}
    else if(Menu_Index > Menu_Quantity){Menu_Index = 0;}
  }

  void OptionPosition(){
    if(ButtonHandler(0,1)==1){
      Option_Index--;
      }
    else if(ButtonHandler(0,2)==1){
      Option_Index++;
      }
    if(Option_Index < 0){Option_Index = Option_Quantity;}
    else if(Option_Index > Option_Quantity){Option_Index = 0;}
  }
};
int MenuSystems::Menu_Quantity;
int MenuSystems::Menu_Index;
bool MenuSystems::Menu_Enter;
int MenuSystems::Option_Quantity;
int MenuSystems::Option_Index;
bool MenuSystems::Option_Select;

/* 
As the name suggests, this function Increases or decreases the menuposition.
When called, declare the number of menu items being used.
*/

int SystemNavigationMenuPosition(int MenuItems){
  #ifdef DEBUG_System_Navigation_Position
  char* F = "SystemUI_NavigationPosition";
  #endif
  static int MenuPosition = 0;
  if(ButtonHandler(0,1)==1){MenuPosition = MenuPosition - 1;}
  else if(ButtonHandler(0,2)==1){MenuPosition = MenuPosition + 1;}
  if(MenuPosition < 0){MenuPosition = MenuItems;}
  else if(MenuPosition > MenuItems){MenuPosition = 0;}

  return MenuPosition;
}

/* 
As the name suggests, this function handles the selection of items
*/

bool SystemMainSelect(){
  #ifdef DEBUG_System_Navigation_Select
  char* F = "SystemUI_NavigationSelect";
  #endif
  static bool Enter;
  if(ButtonHandler(0,3)==true){
    Enter = true;
    #ifdef DEBUG_System_Navigation_Select
    DebugUtil(0, F, "Program Enter", 0, 'I' );
    #endif
    LedHandler(3, 70);
  }
  else if(ButtonHandler(0,0)==true){
    Enter = false;
    #ifdef DEBUG_System_Navigation_Select
    DebugUtil(0, F, "Program Exit", 0, 'I' );
    #endif
    LedHandler(0, 70);
  }
  return Enter;
}

void SystemUI(){
  MenuSystems MainUI;
  MainUI.Menu_Quantity = 2;
  MainUI.NavigationPosition();
  MainUI.NavigationSelect();
  DisplayHandler(0, "              <>", 0, 0, 1);
  switch(MainUI.Menu_Index){
    case 0:
    //while(SystemMainSelect() == true){
      SimonGame_Main();
    //}
    break;
    case 1:
    About();
    break;
    case 2:
    Stats();
    break;
  }
}

void About(){
  MenuSystems About;
  About.NavigationSelect();
  DisplayHandler(0 ,"  About Device  ", 0, 0, 0);
  while(About.Menu_Enter==true){
    About.Menu_Quantity = 2;
    About.NavigationPosition();
    About.NavigationSelect();
    switch(About.Menu_Index){
      case 0:
      DisplayHandler(0 ,"Developed By:   ", 0, 0, 0);
      DisplayHandler(0 ,"Geel.S        <>", 0, 0, 1);
      break;
      case 1:
      DisplayHandler(0, "Hardware version", 0, 0, 0);
      DisplayHandler(0, HardwareVersion, 0, 0, 1);
      DisplayHandler(0, "           <>", 0, 3, 1);
      break;
      case 2:
      DisplayHandler(0, "Build Date:     ", 0, 0, 0);
      DisplayHandler(0, BuildDate, 0, 0, 1);
      DisplayHandler(0, "<>", 0, 14, 1);
      break;
    }
  }
}

void Stats(){
  MenuSystems Stats;
  Stats.NavigationSelect();
  DisplayHandler(0 ,"   Statistics   ", 0, 0, 0);
  while(Stats.Menu_Enter==true){
    Stats.NavigationSelect();
    Stats.Menu_Quantity = 3;
    Stats.NavigationPosition();
    Stats.OptionSelect();
    switch(Stats.Menu_Index){
      case 0:
      DisplayHandler(0, "Easy P.Rs    ", 0, 0, 0);
      DisplayHandler(0, "Hold Enter   ", 0, 0, 1);
      break;
      case 1:
      DisplayHandler(0, "Med  P.Rs   ", 0, 0, 0);
      DisplayHandler(0, "Hold Enter   ", 0, 0, 1);
      break;
      case 2:
      DisplayHandler(0, "Hard P.Rs     ", 0, 0, 0);
      DisplayHandler(0, "Hold Enter   ", 0, 0, 1);
      break;
      case 3:
      DisplayHandler(0, "Cry  P.Rs     ", 0, 0, 0);
      DisplayHandler(0, "Hold Enter   ", 0, 0, 1);
      break;
    }
    while(Stats.Option_Select == true){
      Stats.OptionSelect();
      DisplayHandler(0, "PR Score:", 0, 0, 0);
      DisplayHandler(0, "PR Level:", 0, 0, 1);
      if(Stats.Menu_Index == 0){
        DisplayHandler(1, "N/A", EASY_PR_Score, 9, 0);
        DisplayHandler(1, "N/A", EASY_PR_Level, 9, 1);
      }
      else if(Stats.Menu_Index == 1){
        DisplayHandler(1, "N/A", MED_PR_Score, 9, 0);
        DisplayHandler(1, "N/A", MED_PR_Level, 9, 1);
      }
      else if(Stats.Menu_Index == 2){
        DisplayHandler(1, "N/A", HARD_PR_Score, 9, 0);
        DisplayHandler(1, "N/A", HARD_PR_Level, 9, 1);
      }
      else if(Stats.Menu_Index == 3){
        DisplayHandler(1, "N/A", CRY_PR_Score, 9, 0);
        DisplayHandler(1, "N/A", CRY_PR_Level, 9, 1);
      }
    }
  }
}

void SimonGame_Main(){
  #ifdef DEBUG_SimonGame_Main
  char* F = "SimonGame_Main";
  #endif
  MenuSystems SimonUI;
  GameSystem SimonGame;
  SoundFX SimonSFX;
  SimonUI.NavigationSelect();
  bool GameEnter = false;
  DisplayHandler(0, "   Simon Game   ", 0, 0, 0);
  while(SimonUI.Menu_Enter==true){
    SimonUI.OptionPosition();
    SimonUI.Option_Quantity = 3;
    SimonUI.NavigationSelect();
    DisplayHandler(0, "Select Level:   ", 0, 0, 0);
    switch(SimonUI.Option_Index){
    case 0:
      DisplayHandler(0, "Easy          <>", 0, 0, 1);
      SimonGame.Difficulty = 1;
      SimonGame.Disp_Difficulty = "Easy";
      SimonGame.Duration = 300;
      SimonGame.Time = 5000;
      SimonGame.GamePatternLength = 1;
      SimonGame.Time_Multiplier = 2;
      SimonGame.Duration_Multiplier = 4;
      SimonGame.Score_Multiplier = 1;
    break;
    case 1:
      DisplayHandler(0, "Med           <>", 0, 0, 1);
      SimonGame.Difficulty = 2;
      SimonGame.Disp_Difficulty = "Med ";
      SimonGame.Duration = 220;
      SimonGame.Time = 4000;
      SimonGame.GamePatternLength = 1;
      SimonGame.Time_Multiplier = 4;
      SimonGame.Duration_Multiplier = 10;
      SimonGame.Score_Multiplier = 2;
    break;
    case 2:
      DisplayHandler(0, "Hard          <>", 0, 0, 1);
      SimonGame.Difficulty = 3;
      SimonGame.Disp_Difficulty = "Hard";
      SimonGame.Duration = 180;
      SimonGame.Time = 3500;
      SimonGame.GamePatternLength = 2;
      SimonGame.Time_Multiplier = 6;
      SimonGame.Duration_Multiplier = 15;
      SimonGame.Score_Multiplier = 3;
    break;
    case 3:
      DisplayHandler(0, "Cry           <>", 0, 0, 1);
      SimonGame.Difficulty = 4;
      SimonGame.Disp_Difficulty = "Cry";
      SimonGame.Duration = 150;
      SimonGame.Time = 2000;
      SimonGame.GamePatternLength = 4;
      SimonGame.Time_Multiplier = 10;
      SimonGame.Duration_Multiplier = 20;
      SimonGame.Score_Multiplier = 5;
    break;
    }
    if(ButtonHandler(0,3)==true){
      GameEnter = true;
    }
    Index = 0; //Reset Index when starting game.
    while(GameEnter == true){
      DisplayHandler(0, "Score:          ", 0, 0, 0);
      DisplayHandler(1, "N/A", SimonGame.Score, 12 , 0);
      DisplayHandler(0, "Level:          ", 0, 0, 1);
      DisplayHandler(1, "N/A", SimonGame.Lvl, 12 , 1);
      if(SimonGame_CPU(SimonGame.Duration, SimonGame.GamePatternLength)==true){
        if((SimonGame_User(SimonGame.Time, SimonGame.GamePatternLength)==true) && (SimonGame_Check(SimonGame.GamePatternLength)==true)){
          SimonGame_Scoreboard(true, SimonGame.Score);
          SimonGame.IncrementLevel();
          SimonGame.IncrementScore();
          SimonGame.IncrementChallenge();
          SimonGame_Sanitize(SimonGame.GamePatternLength, true);
        }
        else if((SimonGame_User(SimonGame.Time, SimonGame.GamePatternLength)==false) || (SimonGame_Check(SimonGame.GamePatternLength)==false)){
          SimonGame_Sanitize(SimonGame.GamePatternLength, false);
          SimonGame_Scoreboard(false, SimonGame.Score);
          SimonGame_ScoreManager(SimonGame.Score, SimonGame.Lvl, SimonGame.Difficulty);
          SimonSFX.Loser();
          return;
        }
      }
    }
  }
}

void SimonGame_ScoreManager(int Score, int lvl, int difficulty){
  bool NewHighScore = false;
  switch(difficulty){
    case 1:
    if(Score > EASY_PR_Score){
      EASY_PR_Score = Score;
      EASY_PR_Level = lvl;
      NewHighScore = true;
      #ifdef EEPROM
      updateLong(AddrEasyScorePR, EASY_PR_Score);
      updateInt(AddrEasyLvlPR, EASY_PR_Level);
      #endif
    }
    break;
    case 2:
    if(Score > MED_PR_Score){
      MED_PR_Score = Score;
      MED_PR_Level = lvl;
      NewHighScore = true;
      #ifdef EEPROM
      updateLong(AddrMedScorePR, MED_PR_Score);
      updateInt(AddrMedLvlPR, MED_PR_Level);
      #endif
    }
    break;
    case 3:
    if(Score > HARD_PR_Score){
      HARD_PR_Score = Score;
      HARD_PR_Level = lvl;
      NewHighScore = true;
      #ifdef EEPROM
      updateLong(AddrHardScorePR, HARD_PR_Score);
      updateInt(AddrHardLvlPR, HARD_PR_Level);
      #endif
    }
    break;
    case 4:
    if(Score > CRY_PR_Score){
      CRY_PR_Score = Score;
      CRY_PR_Level = lvl;
      NewHighScore = true;
      #ifdef EEPROM
      updateLong(AddrCryScorePR, CRY_PR_Score);
      updateInt(AddrCryLvlPR, CRY_PR_Level);
      #endif
    }
    break;
  }
  if(NewHighScore == true){
    DisplayHandler(0, "NEW HIGH SCORE! ", 0, 0, 0);
    DisplayHandler(0, "                ", 0, 0 , 1);
    DisplayHandler(1, "N/A", Score, 0 , 1);
  }
  delay(3000);
}

void SimonGame_Scoreboard(bool Result, int Score){
  if(Result==true){
    DisplayHandler(0, "WIN!            ", 0, 0, 0);
    DisplayHandler(0, "Score:          ", 0, 0 , 1);
    DisplayHandler(1, "N/A", Score, 11 , 1);
    delay(300);
  }
  else if(Result==false){
    DisplayHandler(0, "LOSE!            ", 0, 0, 0);
    DisplayHandler(0, "Score:          ", 0, 0 , 1);
    DisplayHandler(1, "N/A", Score, 11 , 1);
    delay(400);
  }
}

bool SimonGame_CPU_Generator(int PatternLength){
  #ifdef DEBUG_SimonGame_CPU_Generator
  char* F = "SimonGame_CPU_Generator";
  DebugUtil(0, F, "CPU Pattern Creation Starting!", 0, 'I' );
  DebugUtil(1, F, "CPU Pattern Length Set To:", PatternLength-1, 'I' );
  #endif
  while(Index<PatternLength){
    CPU_SIMON_SEQUENCE[Index] = random(0,4);;
    if(Index > 0){
      while(CPU_SIMON_SEQUENCE[Index] == CPU_SIMON_SEQUENCE[Index-1]){
      CPU_SIMON_SEQUENCE[Index] = random(0,4); //Make sure there are no multiples in a row.
      #ifdef DEBUG_SimonGame_CPU_Generator
      DebugUtil(0, F, "Repeated Value Found!", 0, 'I' );
      DebugUtil(0, F, "Regenerating Values!", 0, 'I' );
      #endif
      }
    }
  #ifdef DEBUG_SimonGame_CPU
  DebugUtil(1, F, "CPU Sequence Position:", Index, 'I' );
  DebugUtil(1, F, "CPU Set it to button", CPU_SIMON_SEQUENCE[Index], 'I' );
  #endif
  Index++;
  }
  return true;
}

bool SimonGame_CPU(int Duration, int PatternLength){
  #ifdef DEBUG_SimonGame_CPU
  char* F = "SimonGame_CPU";
  DebugUtil(0, F, "CPU Turn Started!", 0, 'I' );
  #endif
  while(SimonGame_CPU_Generator(PatternLength) != true){} //Make sure pattern is ready
  for(int p = 0; p<PatternLength; p++){
    LedHandler(CPU_SIMON_SEQUENCE[p],Duration);
    tone(Piezo, PIEZO_TONE[CPU_SIMON_SEQUENCE[p]], Duration);
    delay(100);
  }
  #ifdef DEBUG_SimonGame_CPU
  DebugUtil(0, F, "CPU's Turn Done!", 0, 'I' );
  #endif
  return true; //Notify when completed
}

bool SimonGame_User(int Time, int PatternLength){
  #ifdef DEBUG_SimonGame_User
  char* F = "SimonGame_User";
  DebugUtil(0, F, "User's Turn", 0, 'I' );
  #endif
  unsigned long currentTime = millis();
  
  while(Step < PatternLength){
    USER_SIMON_SEQUENCE[Step] = ButtonHandler(1,0);
    if(USER_FAIL_FLAG == true){
      return false;
    }
  }

  #ifdef DEBUG_SimonGame_User
  DebugUtil(0, F, "User Input Period Done!", 0, 'I' );
  #endif
  return true; //Notify when completed
}

bool SimonGame_Check(int PatternLength){
  #ifdef DEBUG_SimonGame_Check
  char* F = "SimonGame_Check";
  DebugUtil(0, F, "Checking...", 0, 'I' );
  #endif
  for(int x = 0; x<PatternLength; x++){
    if(USER_SIMON_SEQUENCE[x] != CPU_SIMON_SEQUENCE[x]){
      #ifdef DEBUG_SimonGame_Check
      DebugUtil(1, F, "DISCREPANCY AT INDEX:", x, 'I' );
      DebugUtil(0, F, "CHECK FAILED!", 0, 'I' );
      #endif
      return false;
    }
  }
  return true;
}

void SimonGame_Sanitize(int PatternLength, bool GameStatus){
  #ifdef DEBUG_SimonGame_Check
  char* F = "SimonGame_Sanitize";
  DebugUtil(0, F, "Sanitizing...", 0, 'I' );
  #endif
  for(int x = 0; x<PatternLength; x++){ //Only reset what was used rather than the entire array. 
    USER_SIMON_SEQUENCE[x] = 0;
    if(GameStatus == false){
      CPU_SIMON_SEQUENCE[x] = 0;
    }
  }
  if(GameStatus == false){
    USER_FAIL_FLAG = false;
    Index = 0;
  }
  Step = 0;
}

static void LedHandler(int Colour, int Duration){
  #ifdef DEBUG_LED
  char* F = "LED Handler";
  DebugUtil(0, F, "LED Requested!", 0, 'I' );
  #endif
  digitalWrite(LED[Colour], HIGH);
  #ifdef DEBUG_LED
  switch(Colour){
    case 0:
    DebugUtil(1, F, "RED:", Duration, 'I' );
    break;
    case 1:
    DebugUtil(1, F, "BLUE:", Duration, 'I' );
    break;
    case 2:
    DebugUtil(1, F, "GREEN:", Duration, 'I' );
    break;
    case 3:
    DebugUtil(1, F, "YELLOW:", Duration, 'I' );
    break;
  }
  #endif
  delay(Duration);
  digitalWrite(LED[Colour], LOW);
  #ifdef DEBUG_LED
  DebugUtil(0, F, "LED Cleared.", 0, 'I' );
  #endif
}

/* 
Function to put together the different lcd functions into one.
Bypasses LiquidCrystal clearing as it is slow.

NOTE: Forget clearing, just clear previous characters with white space

Also sets backlight when called. (non i2c interface displays only)
*/
int DisplayHandler(bool mode, char* msg, int value, int x, int y){
  static bool cleared = false;
  #ifdef DEBUG_Display
  char* F = "Display Handler";
  #endif
  #ifndef DisplayI2C
  analogWrite(Backlight, Brightness);
  #endif
  lcd.setCursor(x, y);
  if(mode==0){
    lcd.print(msg);
  }
  else if(mode == 1){
    lcd.print(value);
  }
} 