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
  bool btn_enter_long = 1;  //en bool för longpress på enterknappen.
  bool btn_enter_2x = false;
  bool posedge_e = 0;
  bool posedge_L = 0;
  bool posedge_M = 0;
  bool posedge_R = 0;
  bool negedge_e = 0;
  bool negedge_L = 0;
  bool negedge_M = 0;
  bool negedge_R = 0;

  //uint16_t (*sound)[3];
  //uint8_t sound_size = 0;
  uint8_t sound = 0;
  bool go_buzz;

  uint16_t looser_sound[3][3] = {{1, 2, 400}, {3, 6, 400}, {6, 12, 400}};
  uint16_t smack_sound[1][3] = {1,2,60};
  uint16_t miss_sound[1][3] = {6,12,60};

  uint8_t count1 = 0;   //en räknare för varje knapp, som räknas upp varje gång funktionen button_state() anropas
  uint8_t count2 = 0;   //om knappen är nedtryckt.
  uint8_t count3 = 0;
  uint8_t count4 = 0;

  uint8_t game_mode = 1;
  bool mode_select = 0;

  unsigned long milli;

  uint8_t check = 4;    //en variabel för att hålla koll på vad i programmet som skall utföras.
  uint8_t seqsize = 1;  //en variabel som innehåller storleken på sekvensen.
  uint8_t high_score = 0; //en variabel som innehåller sessionens högsta poäng.
  uint8_t display_digit = 0;  //en variabel som innehåller vilket nummer vi skall visa på displayen.
  uint8_t memo_difficulty = 0;
  unsigned char *sequence = (unsigned char*)malloc(seqsize * sizeof(unsigned char)); //allokerar minnet för våran sekvens.

  uint8_t pop_up = 0;
  uint8_t score = 0;
  bool run_game = 0;
  bool ledL_on = 0;
  bool ledM_on = 0;
  bool ledR_on = 0;
  unsigned long start_delay = 0;
  float game_pace = 0;
  unsigned long gametime = 0;
  uint8_t pop_difficulty = 0;

  while(1){

    milli = millis();

    if(btn_enter_long){   //om man håller ned enterknappen så skall man kunna växla mellan två spellägen.
      mode_select = 1;
      game_select_mode(&mode_select, btn_L_state, btn_M_state, btn_R_state, negedge_e, &game_mode, &memo_difficulty, &pop_difficulty);
      printDigit(game_mode);
      reset_games(&check, &run_game);
    }
    else{
      mode_select = 0;
      if(game_mode == 0){
        game_mode = 1;
      }
    }

    button_state(&count1, ENTER, &btn_enter_state, &posedge_e, &negedge_e); 
    button_state(&count2, BTNL, &btn_L_state, &posedge_L, &negedge_L);
    button_state(&count3, BTNM, &btn_M_state, &posedge_M, &negedge_M);
    button_state(&count4, BTNR, &btn_R_state, &posedge_R, &negedge_R);
    button_longpress(&btn_enter_state, &btn_enter_long);
    button_2xclick(&btn_enter_2x, &btn_enter_state);


    switch(sound){
      case 1:
        buzzer(smack_sound, 1, &go_buzz);
        break;
      case 2:
        buzzer(miss_sound, 1, &go_buzz);
        break;
      case 3:
        buzzer(looser_sound, 3, &go_buzz);
        break;
      default:
        break;
    }
    
    if(game_mode == 1 && !mode_select){
      if((check == 4) || (btn_enter_2x)){ //om check är 4 så betyder det att man har misslyckats med att trycka på rätt knappar.
        check = 4;
        free(sequence); //fria minnet och allokera nytt.
        seqsize = 1;
        sequence = (unsigned char*)malloc(seqsize * sizeof(unsigned char));
        assign_sequence(sequence, seqsize, memo_difficulty); 
      }
      
      if(check == 5){ //om check är 5 så betyder det att man har lyckats trycka in knapparna i rätt kombination.
        if(seqsize > high_score){
          high_score = seqsize;
        }
        seqsize++;  //öka storleken på sekvensen och allokera nytt.
        sequence = realloc(sequence, seqsize * sizeof(unsigned char));
        assign_sequence(sequence, seqsize, memo_difficulty);
      }
      
      ledBlink(sequence, seqsize, &check, posedge_e); 
      displayDigit(&seqsize, &high_score, &display_digit, &btn_enter_state, &check); 
      printDigit(display_digit);
      cmp_sequence(sequence, seqsize, &check, btn_L_state, btn_M_state, btn_R_state, &sound, &go_buzz);
      is_success(&check);
    }

    else if(game_mode == 2 && !mode_select){

      if(btn_enter_2x){
        run_game = 0;
        reset_pop_score(&score, run_game);
      }
      printDigit(score);
      confirm_start(run_game, &ledL_on, &ledM_on, &ledR_on);
      set_gametime(&gametime, pop_difficulty);
      game_dynamic(&game_pace, gametime, &run_game);
      reset_pop_score(&score, run_game);
      manage_leds(ledL_on, ledM_on, ledR_on);
      
      if(run_game){
        if(milli > start_delay){
          generate_pop_up(&pop_up);
          show_pop_up(&pop_up, &ledL_on, &ledM_on, &ledR_on, game_pace);
          check_if_score(btn_L_state, btn_M_state, btn_R_state, &score, &ledL_on, &ledM_on, &ledR_on, posedge_L, posedge_M, posedge_R, &sound, &go_buzz);
        }
      }
      else{
        leds_off(&ledL_on, &ledM_on, &ledR_on);
        if(posedge_e && !btn_enter_2x){
          run_game = 1;
          start_delay = (milli + 1500);
        }
      }
    }
  }
}

  
