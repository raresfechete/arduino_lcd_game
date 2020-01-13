#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int lcd_key = 0;
int adc_key_in = 0;

#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

byte drum_left[8] = {
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b10101
};
byte drum_right[8] = {
  0b10101,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
};
byte player_left[8] = {
  0b11111,
  0b00000,
  0b10000,
  0b11111,
  0b11111,
  0b10000,
  0b00000,
  0b10101
};
byte player_right[8] = {
  0b10101,
  0b00000,
  0b10000,
  0b11111,
  0b11111,
  0b10000,
  0b00000,
  0b11111
};
byte inamic1_stanga[8] = {
  0b11111,
  0b10000,
  0b01001,
  0b00110,
  0b00110,
  0b01001,
  0b10000,
  0b10101
};
byte inamic1_drapta[8] = {
  0b10101,
  0b00000,
  0b11111,
  0b00110,
  0b00110,
  0b11111,
  0b00000,
  0b11111
};

byte crash[8] = {
  0b00110,
  0b10100,
  0b01111,
  0b00100,
  0b10111,
  0b01010,
  0b10010,
  0b01001
};

int game_state; //0-start menu, 1-game, 2-game over screen
int rows[2][17];//cele doua laneuri, 0 - drum gol, 10 - jucatorul, <10 - inamici, >10 - crash
int crash_event; //se pune pe 1 in caz de accident

int player_pos;

unsigned long timp;
unsigned long timp_start;
unsigned long score;

const int VITEZAMAX = 300;
const int VITEZAMIN = 100;
int viteza;
int pasi=0;

int glont;
int rezerva_gloante;

void setup() {
  lcd.begin(16, 2);
  
  lcd.createChar(0, drum_left);
  lcd.createChar(1, drum_right);
  lcd.createChar(2, player_left);
  lcd.createChar(3, player_right);
  lcd.createChar(4, inamic1_stanga);
  lcd.createChar(5, inamic1_drapta);
  lcd.createChar(7, crash);

  crash_event=0;
  game_state = 0; // setam gamestatetul initial si afisam ecranul de inceput
  lcd.setCursor(0, 0);
  lcd.print("PRESS SELECT TO");
  lcd.setCursor(0, 1);
  lcd.print("START THE GAME");
}

void loop() {
  lcd_key = read_LCD_buttons();
  switch (game_state) {
    case 0: {
        if (lcd_key == 4) {
          start_setup();
          game_state = 1;
          viteza= VITEZAMAX;
          score=0;
          timp_start=millis();
        }
        break;
    }

    case 1: {
        timp=millis();
        if (crash_event==1){
          game_state=2;
          for (int i = 0; i < 16; i++) {
            lcd.setCursor(i, 0);
            lcd.write(7);
            rows[0][i]=0;

            lcd.setCursor(i, 1);
            lcd.write(7);
            rows[1][i]=0;
            delay(25);
          }
          delay(1000);
          break;
        }
        pasi++;
        if (lcd_key == 0){ //trage si primul inamic din rand cu jucatorul explodeaza
          if (rezerva_gloante){
            glont=1;
            while(glont<16 && rows[player_pos][glont]==0)
              glont++;
            rows[player_pos][glont]=2;
            score+=(2-player_pos);
            rezerva_gloante--;
          }
        }
        if (lcd_key == 1) {
          rows[0][0]=10;
          rows[1][0]=0;
          player_pos=0;
        }
        if (lcd_key == 2) {
          rows[1][0]=10;
          rows[0][0]=0;
          player_pos=1;
        }
        draw_frame();
        delay(viteza);
        if (pasi%3==0)
          spawn();
        if(pasi%50==0)
          rezerva_gloante++;  
        if (viteza>VITEZAMIN)
          viteza--;
        break;
    }

    case 2: {
      lcd.setCursor(0,0);
      lcd.print("GAME OVER       ");
      lcd.setCursor(0,1);
      lcd.print("SCORE ");
      lcd.print(score); 
      lcd.print("         ");
    }

    if (lcd_key == 4) {
      start_setup();
      game_state = 1;
      viteza= VITEZAMAX;
      timp_start=millis();
      score=0;
      crash_event=0;
    }
    break;
  }
}

void spawn(){
  if (timp%2==0 || timp%5==0 || timp&7==0 || timp%11==0 || timp%13==0){
    if(pasi%2==0)
      rows[0][16]=1;
    else 
      rows[1][16]=1;
  }
}

void draw_frame(){
  for (int i = 0; i < 16; i++) {
    rows[0][i]+=rows[0][i+1];
    rows[0][i+1]=0;
    rows[1][i]+=rows[1][i+1];
    rows[1][i+1]=0;

    swap_position(i, 0);
    swap_position(i, 1);
  }
  
  rows[1-player_pos][0]=0;
}


void swap_position(int i, int row){ //muta caracterul din i+1 de pe raundul row in i
  lcd.setCursor(i, row);
  switch (rows[row][i]) {
    case 0: {
      lcd.write(byte(0)+row);
      break;
    }
    case 1: {
      lcd.write(4+row);
      lcd.setCursor(i+1,row);
      lcd.write(byte(0)+row);
      break;
    }
    case 2:
      lcd.write(7);
      rows[row][i]=0;
      break;
    case 10:{
      lcd.write(2+row);
      break;
    }
    default:{
      lcd.write(7);
      score+= (timp-timp_start)/1000;
      crash_event=1;
    }
  }
}

void start_setup() { // seteaza starea initiala cand se da start sau restart. pune drumul si jucatorul pe pozitia stanga

  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 0);
    lcd.write(byte(0));
    rows[0][i]=0;

    lcd.setCursor(i, 1);
    lcd.write(1);
    rows[1][i]=0;
  }

  lcd.setCursor(0, 0);
  lcd.write(2);
  rezerva_gloante=1;
  rows[0][0]=10;
  player_pos=0;
}

int read_LCD_buttons() { // read the buttons
  adc_key_in = analogRead(0); // read the value from the sensor
  
  if (adc_key_in > 1000) return btnNONE;

  if (adc_key_in < 60) return btnRIGHT;
  if (adc_key_in < 200) return btnUP;
  if (adc_key_in < 400) return btnDOWN;
  if (adc_key_in < 600) return btnLEFT;
  if (adc_key_in < 800) return btnSELECT;
  
  return btnNONE; // when all others fail, return this.
}
