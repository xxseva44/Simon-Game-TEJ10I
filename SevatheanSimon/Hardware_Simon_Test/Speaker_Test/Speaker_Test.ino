int BUTTON[4]={A5, A4, 3, 2}; //{B_RED, B_BLUE, B_GREEN, B_YELLOW}
int LED[4]={A3, A2, 5, 6}; //{RED, BLUE, GREEN, YELLOW}

//Piezo
#define Piezo 4

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
  ButtonHandler();

}

void ButtonHandler(){
  for(int c = 0; c < 4; c++){
    int Read = digitalRead(BUTTON[c]);
    if(Read == LOW){
    delay(500);
      if(Read == LOW){
        if(digitalRead(BUTTON[c]) == HIGH){
          digitalWrite(LED[c], HIGH);
          //tone(Piezo, PIEZO_TONE[c], 2000);
          //delay(2000);
          //digitalWrite(LED[c], LOW);
          AudioHandler(0, c, 2000);
        }
      }
    }
  }
}

void AudioHandler(int type, int option, int timer){
  static int PIEZO_TONE[7]={60, 80, 100, 140, 180, 240, 260};
  switch(type){
    case 0:
    tone(Piezo, PIEZO_TONE[option], timer);
    break;
    case 1:
    Serial.println("INCOMPLETE");
    break;
  }

}
