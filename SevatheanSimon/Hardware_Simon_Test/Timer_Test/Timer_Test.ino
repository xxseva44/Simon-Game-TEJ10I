int BUTTON[4]={A5, A4, 3, 2}; //{B_RED, B_BLUE, B_GREEN, B_YELLOW}
int LED[4]={A3, A2, 5, 6}; //{RED, BLUE, GREEN, YELLOW}
//Piezo
#define Piezo 4
//timer
unsigned long PrevTime = 0UL;
//debug
#define DEBUG true

void setup() {
  //Buttons
  for(int b = 0; b < 4; b++){pinMode(BUTTON[b], INPUT);}
  //Piezo
  pinMode(Piezo, OUTPUT);
  //Leds
  for(int b = 0; b < 4; b++){pinMode(LED[b], OUTPUT);}

  Serial.begin(9600);
}

void loop() {
  if(Timer(1000UL)==1){}
}

void ButtonHandler(){
  char* F = "Button Handler";
  for(int c = 0; c < 4; c++){
    int Read = digitalRead(BUTTON[c]);
    if(Read == LOW){
      if((Timer(500UL) == 1) && (Read == LOW)){
        if(DEBUG == true){DebugUtil(F, "Button Hit!", 'I' );}
        digitalWrite(LED[c], HIGH);
      }
    }
  }
}

/*
Custom timer/delay to avoid complete system halts amd
allows for simultaneous delays.
1000UL = 1s

if inputted time has passed, returns 1, if not, 0
*/
int Timer(int TimerDelay){
  char* F = "Timer";
  unsigned long CurrTime = millis();
  if(CurrTime - PrevTime >= TimerDelay){
    PrevTime = CurrTime;
    if(DEBUG == true){DebugUtil(F, "Timer Called!", 'I' );}
    return 1;
    }
  else{return 0;}
}

/* Debug utility*/
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
