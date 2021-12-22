/*
 * project_memorytest.c
 *
 * Created: 2021-03-12 19:12:41
 * Author : Jes_p
 */ 

#include "PmemINC.h"

int main(void)
{
  init_millis(F_CPU);
  srand(init_srand());  //initiera srand med funktionen init_srand();
  rand();rand();      //kalla på rand 2 gånger för att bli av med eventuella förutsägbara tal i början.
  init_DDR();
  init_ports();
  bool btn_enter_state = false; //Booleans för att representera knapparnas tillstånd.
  bool btn_L_state = false;
  bool btn_M_state = false;
  bool btn_R_state = false;
  bool btn_enter_long = false;  //en bool för longpress på enterknappen.
  bool btn_enter_2x = false;
  bool posedge_enter = 0;
  bool posedge_L = 0;
  bool posedge_M = 0;
  bool posedge_R = 0;
  uint8_t count1 = 0;   //en räknare för varje knapp, som räknas upp varje gång funktionen button_state() anropas
  uint8_t count2 = 0;   //om knappen är nedtryckt.
  uint8_t count3 = 0;
  uint8_t count4 = 0;
  unsigned long time_btn = 0;   //en variabel för att kunna räkna ut när vi skall anropa button_state() för alla knappar.
  uint8_t blink_period = 250;
  uint8_t game_mode;
  bool mode_select;
  uint8_t check = 6;    //en variabel för att hålla koll på vad i programmet som skall utföras.
  uint8_t seqsize = 1;  //en variabel som innehåller storleken på sekvensen.
  uint8_t high_score = 0; //en variabel som innehåller sessionens högsta poäng.
  uint8_t display_digit = 0;  //en variabel som innehåller vilket nummer vi skall visa på displayen.
  unsigned char *sequence = (unsigned char*)malloc(seqsize * sizeof(unsigned char)); //allokerar minnet för våran sekvens.
  assign_sequence(sequence, seqsize); //tilldela sekvensen första värdet.
  while(1){
    
    
    if(btn_enter_long){   //om man håller ned enterknappen så skall man kunna vexla mellan två spellägen.
      mode_select = 0; //snabba läget då sekvensen blinkar med 100ms period, och vanlig.
    }
    else{
      mode_select = 1;
    }

    if(mode_select){
      if(btn_L_state){
        game_mode = 1;
      }
      if(btn_M_state){
        game_mode = 2;
      }
      printDigit(game_mode);
    }
    
    if(millis() >= (time_btn + 2)){ //en gång varje 2 ms vill vi kolla om någon knapp är nedtryckt.
      time_btn += 2;
      button_state(&count1, ENTER, &btn_enter_state, &posedge_enter); 
      button_state(&count2, BTNL, &btn_L_state, &posedge_L);
      button_state(&count3, BTNM, &btn_M_state, &posedge_M);
      button_state(&count4, BTNR, &btn_R_state, &posedge_R);
      button_longpress(&btn_enter_state, &btn_enter_long);
      
    }
    
    if(game_mode == 1 && !mode_select){
      if((check == 4) || (btn_enter_2x)){ //om check är 4 så betyder det att man har misslyckats med att trycka på rätt knappar.
        check = 4;
        free(sequence); //fria minnet och allokera nytt.
        seqsize = 1;
        sequence = (unsigned char*)malloc(seqsize * sizeof(unsigned char));
        assign_sequence(sequence, seqsize); 
      }
      
      if(check == 5){ //om check är 5 så betyder det att man har lyckats trycka in knapparna i rätt kombination.
        if(seqsize > high_score){
          high_score = seqsize;
        }
        seqsize++;  //öka storleken på sekvensen och allokera nytt.
        sequence = realloc(sequence, seqsize * sizeof(unsigned char));
        assign_sequence(sequence, seqsize);
      }

      button_2xclick(&check, &btn_enter_2x, &btn_enter_state);
      ledBlink(sequence, seqsize, &check, blink_period, posedge_enter); 
      displayDigit(&seqsize, &high_score, &display_digit, &btn_enter_state, &check); 
      printDigit(PORTD0);
      cmp_sequence(sequence, seqsize, &check, btn_L_state, btn_M_state, btn_R_state);
      is_success(&check);
      buzzer(check);
    }
    else if(game_mode == 2 && !mode_select){
      printDigit(69);
    }
  }
}

  
