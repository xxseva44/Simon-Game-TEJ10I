int LED[4]={A3, A2, 5, 6}; //{RED, BLUE, GREEN, YELLOW}
void setup() {for(int b = 0; b < 4; b++){pinMode(LED[b], OUTPUT);}}
void loop(){for(int c = 0; c < 4; c++){digitalWrite(LED[c], HIGH);}}