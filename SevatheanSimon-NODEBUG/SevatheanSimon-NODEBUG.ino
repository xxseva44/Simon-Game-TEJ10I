/* GENERIC INFO */
#define HardwareVersion "2.0"
#define BuildDate __DATE__ 

/* HARDWARE DEFINITION */
#define ButtonQuantity 4 //Number of buttons
#define LEDQuantity 4 //Number of LEDS

#define Piezo 4

int BUTTON[ButtonQuantity]={A1, A0, 3, 2}; //{B_RED, B_BLUE, B_GREEN, B_YELLOW}
int LED[LEDQuantity]={A3, A2, 5, 6}; //{RED, BLUE, GREEN, YELLOW}
int PIEZO_TONE[4]={100, 200, 300, 400};

//display
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);

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
const int MAX_RAND = 50; // Sets the max limit of possible pattern lengths
int CPU_SIMON_SEQUENCE[MAX_RAND];
int USER_SIMON_SEQUENCE[MAX_RAND];
int Step = 0;
int Index = 0;
bool USER_FAIL_FLAG = false;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();          
  //Buttons
  for(int b = 0; b < 4; b++){pinMode(BUTTON[b], INPUT);}
  //Piezo
  pinMode(Piezo, OUTPUT);
  //Leds
  for(int b = 0; b < 4; b++){pinMode(LED[b], OUTPUT);}

  randomSeed(analogRead(A2));
}

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
          tone(Piezo, PIEZO_TONE[c], 100);
          if(c != CPU_SIMON_SEQUENCE[Step]){
            USER_FAIL_FLAG = true;
          }
          Step++;
          return c;
        }
        break;
      }
    }
  }
}

/*
SoundFX: Class which melodies that can be used. 
Methods:
1. Winner: Winner tone.
2. Loser: Loser tone.
Dependancies: NONE
Params: NONE
Returns: NONE
*/

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

/*
GameSystem: Class which contains core game functions and variables. 
Methods:
1. IncrementLevel: Increases level count as user wins.
2. IncrementScore: Increases score count as user wins.
3. IncrementChallenge: modifies the variables increasing difficulty as user reaches higher levels.
Variables: 
1. Duration: Duration of the CPU play time.
2. Time: Time for player to input their pattern.
3. Difficulty: Difficulty used to set starting point for GamePatternLength.
4. GamePatternLength: Used to derive the length of patterns.
5. Time_Multiplier: Value used to multiply the subtracted value from time.
6. Duration_Multiplier: Value used to multiply the subtracted value from duration.
7. Lvl: Current level of the user.
8. Score: Score of the user.
9. Score_Multiplier: Value used to multiply the score as user reaches higher levels.
10. Disp_difficulty: Difficulty to display on displays.
Dependancies: NONE
Params: NONE
Returns: NONE
*/

class GameSystem {
  public:
  int Duration;
  int Time;
  int Difficulty;
  int GamePatternLength;
  int Time_Multiplier;
  int Duration_Multiplier;
  int Lvl = 1;
  int Score = 0;
  int Score_Multiplier;
  char* Disp_Difficulty;

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

/*
MenuSystems: Class which contains core menu interface methods and variables. All menus on the device are built upon this minus any lcd related functions. 
Methods:
1. NavigationSelect: When the "green button" is pushed, the bool Menu_Enter will become true, entering menus. If the "red button" is pressed, it goes false and exits.
2. OptionSelect: Functionally the same as NavigationSelect however intended to be used with options.
3. NavigationPosition: By pushing left(0,1) anf right(0,2) buttons, this will adjust Menu_Index which can be used to switch between available menus by moving left or right.
4. OptionPosition: Functionally the same as NavigationPosition however intended to be used with options.
Variables: 
1. Menu_Quantity: Defines amount of menus, important for NavigationPosition.
2. Menu_Index: Used in conjunction with a switch statement to actially switch between the menus.
3. Menu_Enter: Used to enter/exit menus when true/false.
4. Option_Select: Functionally the same as Menu_Enter however, meant for options.
5. Option_Index: Functionally the same as Menu_Index however, meant for options.
6. Option_Quantity: Functionally the same as Menu_Quantity however, meant for options.
Dependancies: ButtonHandler
Params: NONE
Returns: NONE
*/
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
      }
    else if(ButtonHandler(0,2)==1){
      Menu_Index++;
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
SystemUI: Function which is the main menu interface the user lands on upon boot.
When user adjusts menu index via buttons through the menu class, the sub menus will present a title. This will not enter
unless user selects while within the submenus. 
Params: NONE
Returns: NONE
*/

void SystemUI(){
  MenuSystems MainUI;
  MainUI.Menu_Quantity = 2;
  MainUI.NavigationPosition();
  MainUI.NavigationSelect();
  DisplayHandler(0, "              <>", 0, 0, 1);
  switch(MainUI.Menu_Index){
    case 0:
      SimonGame_Main();
    break;
    case 1:
    About();
    break;
    case 2:
    Stats();
    break;
  }
}

/*
About: Function which is the about menu code.
Params: NONE
Returns: NONE
*/
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

/*
Stats: Function which is the stats menu code.
Params: NONE
Returns: NONE
*/

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
          SimonGame_Scoreboard(true, SimonGame.Score, SimonGame.GamePatternLength);
          SimonGame.IncrementLevel();
          SimonGame.IncrementScore();
          SimonGame.IncrementChallenge();
          SimonGame_Sanitize(SimonGame.GamePatternLength, true);
        }
        else if((SimonGame_User(SimonGame.Time, SimonGame.GamePatternLength)==false) || (SimonGame_Check(SimonGame.GamePatternLength)==false)){
          SimonGame_Scoreboard(false, SimonGame.Score, SimonGame.GamePatternLength);
          SimonSFX.Loser();
          SimonGame_ScoreManager(SimonGame.Score, SimonGame.Lvl, SimonGame.Difficulty);
          SimonGame_Sanitize(SimonGame.GamePatternLength, false);
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
    }
    break;
    case 2:
    if(Score > MED_PR_Score){
      MED_PR_Score = Score;
      MED_PR_Level = lvl;
      NewHighScore = true;
    }
    break;
    case 3:
    if(Score > HARD_PR_Score){
      HARD_PR_Score = Score;
      HARD_PR_Level = lvl;
      NewHighScore = true;
    }
    break;
    case 4:
    if(Score > CRY_PR_Score){
      CRY_PR_Score = Score;
      CRY_PR_Level = lvl;
      NewHighScore = true;
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

void SimonGame_Scoreboard(bool Result, int Score, int PatternLength){
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
    delay(800);
    for(int y = 0; y<PatternLength; y++){
      LedHandler(CPU_SIMON_SEQUENCE[y],300);
      tone(Piezo, PIEZO_TONE[CPU_SIMON_SEQUENCE[y]], 400);
    }
    delay(800);
  }
}

bool SimonGame_CPU_Generator(int PatternLength){
  while(Index<PatternLength){
    CPU_SIMON_SEQUENCE[Index] = random(0,4);;
    if(Index > 0){
      while(CPU_SIMON_SEQUENCE[Index] == CPU_SIMON_SEQUENCE[Index-1]){
      CPU_SIMON_SEQUENCE[Index] = random(0,4); //Make sure there are no multiples in a row.
      }
    }
  Index++;
  }
  return true;
}

bool SimonGame_CPU(int Duration, int PatternLength){
  while(SimonGame_CPU_Generator(PatternLength) != true){} //Make sure pattern is ready
  for(int p = 0; p<PatternLength; p++){
    LedHandler(CPU_SIMON_SEQUENCE[p],Duration);
    tone(Piezo, PIEZO_TONE[CPU_SIMON_SEQUENCE[p]], Duration);
    delay(100);
  }
  return true; //Notify when completed
}

bool SimonGame_User(int Time, int PatternLength){
  unsigned long currentTime = millis();
  
  while(Step < PatternLength){
    USER_SIMON_SEQUENCE[Step] = ButtonHandler(1,0);
    if(USER_FAIL_FLAG == true){
      return false;
    }
  }
  return true; //Notify when completed
}


bool SimonGame_Check(int PatternLength){
  for(int x = 0; x<PatternLength; x++){
    if(USER_SIMON_SEQUENCE[x] != CPU_SIMON_SEQUENCE[x]){
      return false;
    }
  }
  return true;
}

void SimonGame_Sanitize(int PatternLength, bool GameStatus){
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
  digitalWrite(LED[Colour], HIGH);
  delay(Duration);
  digitalWrite(LED[Colour], LOW);
}

/* 
Function to put together the different lcd functions into one.
Bypasses LiquidCrystal clearing as it is slow.

Also sets backlight when called. (non i2c interface displays only)
*/
int DisplayHandler(bool mode, char* msg, int value, int x, int y){
  static bool cleared = false;
  lcd.setCursor(x, y);
  if(mode==0){
    lcd.print(msg);
  }
  else if(mode == 1){
    lcd.print(value);
  }
} 