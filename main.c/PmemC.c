/*
 * PmemC.c
 *
 * Created: 2021-03-12 19:14:16
 *  Author: Jes_p
 */ 
#include "PmemINC.h"

volatile unsigned long timer1_millis;

void init_DDR(void){ 
  DDRB = (1<<ledL)|(1<<ledM)|(1<<ledR)|(1<<PORTB3)|(1<<PORTB4);
  DDRD = (1<<digit1)|(1<<digit2)|(1<<BUZZ);
  DDRC = (1<<PORTC0)|(1<<PORTC3)|(1<<PORTC2)|(1<<PORTC1)|(1<<PORTC4);
}
void init_ports(void){
  PORTD = (1<<ENTER)|(1<<BTNL)|(1<<BTNM)|(1<<BTNR)|(1<<digit1)|(1<<digit2);
}

void ledBlink(unsigned char *sequence, uint8_t size, uint8_t *check, bool posedge_enter){ //Funktion som tänder LEDs enligt den sekvenser som är genererad.
  static bool done_blink = true; //Bool för att hålla ordning på om det är dags att visa sekvensen för användaren.
  static bool time_set = false; //Bool för att hålla ordning på om vi har tilldelat time_stamp tiden då fuktionen anropades.
  static bool is_on = 0;      //Bool för att hålla ordning på om vi skall tända eller släcka LEDs.
  static bool allow = 0;
  static uint8_t count = 0;   //räknare för att hålla reda på om vi har visat alla delar av sekvensen eller inte.
  static unsigned long time_stamp = 0; //variabel för att hålla reda på när vi skall tända eller släcka LEDs.
  if(*check > 3){ //Om check är större än 3, så är det dags att visa sekvensen för användaren.
    if(*check == 5){
      done_blink = false;
    }
    PORTB &= ~((1 << ledL) | (1 << ledM) | (1 << ledR));
    *check = 3;
    allow = 1;
  }
  
  if(((!done_blink)||(posedge_enter))&&(allow)){
    done_blink = false;
    
    if(!time_set){          //om inte time_stamp redan har blivit tilldelad den aktuella tiden så skall den bli det.
    time_stamp = (millis() + 700);  //Tilldela rådande tid + 700ms.
    time_set = true;
    PORTB &= ~((1<<ledL)|(1<<ledM)|(1<<ledR));
    }
    
    if(millis() >= time_stamp){ //Om 700 ms har passerat sedan vi tilldelade time_stamp så skall vi
      time_stamp += 250;   //visa första delen av sekvensen. Därefter så skall vi släcka och tända nästa del var 250/125e ms.
      if(is_on){
        PORTB &= ~(sequence[count]);
        count++;
        is_on = false;
      }
      else{
        PORTB |= sequence[count];
        is_on = true;
      }
    }
    
    if(count >= size){    //Om räknaren har nått storleken på sekvensen så har alla delar visats och
      is_on = false;    //funktionen skall inte längre göra något fram till att check är 3 eller större.
      done_blink = true;
      time_set = false;
      *check = 2;
      count = 0;
      allow = 0;
    }
  }
}

int init_srand(void){ //för att sekvensen skall vara annorlunda varje gång används rand(), därför behöver srand() initieras.
  uint8_t count = 0;  //räknar upp en variabel tills användaren trycker ner "Enter". Värdet returneras in i srand().
  while(!button_pressed(ENTER, count)){
    count++;
    _delay_ms(1);
  }
  return count;
}

void assign_sequence(unsigned char* sequence, uint8_t size, uint8_t difficulty){ //Tilldelar sekvensen slumpade värden mellan 1-6. 
  const uint8_t low = 1;
  const uint8_t high = 6;
  uint8_t next = 0;

  switch(difficulty){
    case 1:
      do{
        next = low + rand() / (RAND_MAX / (4 - low + 1) + 1);
      }while(next == 3);
      sequence[size-1] = next;
      break;
      
    case 2:
      sequence[size-1] = low + rand() / (RAND_MAX / (high - low + 1) + 1);
      break;

    case 3:
      for(int i = 0; i < size; i++){
        sequence[i] = low + rand() / (RAND_MAX / (high - low + 1) + 1);
      }
      break;
  }
}

  //funktion som kollar vilka knappar man trycker ned och jämför med den nyss visade sekvensen.
void cmp_sequence(unsigned char *sequence, uint8_t size, uint8_t *check, bool btn1, bool btn2, bool btn3, uint8_t *sound, bool *go_buzz){
  
  static uint8_t count = 0;   //räknare för att hålla ordning på vilken del av sekvensen som skall jämföras med knapptrycken.
  static bool is_pressed = false; //Bool för att hålla reda på om man har tryckt in sin gissning eller ej.
  static uint8_t mask = 0x00;   //mask för att registrera vilka knappar man trycker ned.
  if(*check == 2){    //kolla bara knapptryck om check = 2, ledBlink() är färdig då.
    if(btn1 || btn2 || btn3){
      is_pressed = true;
      mask |= ~((PIND & (1<<BTNL)) | (PIND & (1<<BTNM)) | (PIND & (1<<BTNR)));
    }
  }
  
  if(!(btn1 || btn2 || btn3)&&is_pressed){  //kolla bara om det stämmer när man har tryckt ned en/flera knappar
                        // och dessutom släppt alla igen.
    if((0x07 & (mask>>3)) == sequence[count]){ //om det man tryckte in stämmer med sekvensen, räkna upp count.
      mask = 0x00;
      count++;
      if(count >= size){  //Om count når storleken på sekvensen så skall check sättas till 1 och 
        *check = 1;   //funktionsvarieblerna skall återställas.
        is_pressed = false;
        count = 0;
      }
      is_pressed = false;
    }
    else{       //om man misslyckas med att efterlikna sekvensen så skall check sättas till 0 och 
      mask = 0x00;  //funktionsvarieblerna skall återställas.
      is_pressed = false;
      call_buzzer(sound, go_buzz, 3);
      *check = 0;
      count = 0;
      
    }
  }
}

void is_success(uint8_t *check){    //funktion som visar om man lyckats eller misslyckats med att efterlikna sekvensen.
  static unsigned long t_zero = 0;  //variabel för att hålla reda på när LEDs skall tändas/släckas.
  static uint8_t count = 0;     //räknare för att hålla koll på antalet blink om man lyckats.
  static bool is_on = false;      //Bool för att hålla reda på om LEDs skall tändas/släckas.
  static bool time_set = false;   //Bool för att hålla koll på om tiden för blink "delay" är satt.
  if(*check == 1){          //om check == 1 så skall LEDs blinka.
    if(!time_set){
      t_zero = millis();
      time_set = true;
    }
    if(millis() >= (t_zero + 100)){ //Tänd och släck LEDs var 100ms.
      t_zero += 100;
      if(!is_on){
        PORTB |= 0x07;
        is_on = true;
        count++;
      }
      else{
        PORTB &= ~(0x07);
        is_on = false;
        count++;
      }
    }
    if(count >= 6){ //När count når 6 så är funktionen färdig och check ställs till 5, variabler återställs.
      count = 0;
      *check = 5;
      time_set = false;
    }
  }
  else if(*check == 0){ //Om check == 0 så skall LEDs lysa konstant i 400ms.
    if(!time_set){
      t_zero = millis();
      time_set = true;
    }
    PORTB |= 0x07;
    if(millis() >= (t_zero + 1490)){
      PORTB &= ~(0x07);
      *check = 4;
      time_set = false;
    }
  }
}


ISR(TIMER1_COMPA_vect) //interrupt rutin som räknar upp timer1_millis varje ms.
{
  timer1_millis++;
}

void init_millis(unsigned long f_cpu){ //initierar timerkrets till funktionen millis()
  unsigned long ctc_match_overflow;
  
  ctc_match_overflow = ((f_cpu / 1000) / 8);
  
  TCCR1B |= (1 << WGM12) | (1 << CS11);
  
  OCR1AH = (ctc_match_overflow >> 8);
  OCR1AL = ctc_match_overflow;
  
  TIMSK1 |= (1 << OCIE1A);
}

unsigned long millis(){ //funktion som returnerar millisekunder sedan start(sedan init_millis())
  unsigned long millis_return;
  
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    millis_return = timer1_millis;
  }
  return millis_return;
}

bool button_pressed(uint8_t button, uint8_t count){ //Funktion som bara kollar om en knapp trycks ned
                          //ingen debounce används endast för init_srand().
  if(!(PIND & (1<<button))){
    return true;
  }
  return false;
}

void writePort(uint8_t pin, bool value){  //Funktion som efterliknad digitalWrite i Arduino IDE
                      //Beroende på vilken pinne man ger den så väljer den port.
  if((pin >= 8) && (pin <= 13 )){
    if(value){
      PORTB |= (1<<(pin-8));
    }
    else{
      PORTB &= ~(1<<(pin-8));
    }
  }
  else if((pin >= 0) && (pin <= 7)){
    if(value){
      PORTD |= (1<<pin);
    }
    else{
      PORTD &= ~(1<<pin);
    }
  }
  else{
    if(value){
      PORTC |= (1<<(pin-14));
    }
    else{
      PORTC &= ~(1<<(pin-14));
    }
  }
  
}

void drawDigit(int8_t num){ //funktion som skriver en siffra till 7segmets display.
  const uint8_t segPins[8] = {
    segA, segB, segC, segD, segE, segF, segG
  };
  
  const uint8_t segments[11][8] = {
    {1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 0, 0, 0, 0},
    {1, 1, 0, 1, 1, 0, 1},
    {1, 1, 1, 1, 0, 0, 1},
    {0, 1, 1, 0, 0, 1, 1},
    {1, 0, 1, 1, 0, 1, 1},
    {1, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0},
  };
  
  if(num < 0 || num > 10){
    num = 10;
  }
  
  for(uint8_t i = 0; i<7; i++){
    writePort(segPins[i], segments[num][i]);
  }
}

void printDigit(int8_t num){      //Funktion som tänder och släcker hela siffror på 7segments display 
  static unsigned char t_zero = 0;  //håller koll på tiden då siffrorna skall lysa eller ej.
  static bool time_set = false;   //håller koll på om t_zero är tilldelad en tid eller inte.
  static bool toggle = false;     //håller koll på om den ena eller den andra siffran skall lysa på display.
  unsigned long milli = millis();
  if(num >= 10){      //om siffran man vill visa är större än 10 skall denna metoden följas.
    if(!time_set){
      t_zero = milli;
      time_set = true;
    }
    if(milli >= t_zero){ //varje siffra skall lysa i 5ms var.
      t_zero += 5;
      if(toggle){
        drawDigit(-1);
        writePort(digit2, true);  //när pinnen till digit1/digit2 är hög så går ingen ström igenom
        
        writePort(digit1, false); //när pinnen är låg så agerar den som jord.
        drawDigit(num / 10);
        toggle = !toggle;
      }
      else{
        drawDigit(-1);
        writePort(digit1, true);
        
        writePort(digit2, false);
        drawDigit(num % 10);
        toggle = !toggle;
      }
    }
    time_set = false; //nollställ så att en ny tid kan sättas inför nästa varv.
  }
  else{         //om siffran man vill visa är mindre än 10 så behövs endast digit1 (till vänster på display)
    if(!time_set){
      t_zero = milli;
      time_set = true;
    }
    writePort(digit2, true);    //inaktiverar digit2 (till höger på display).
    if(milli >= t_zero + 5){   //blinkar siffran så att den lyser med samma styrka som om båda används.
      t_zero += 5;
      time_set = false;
      if(toggle){
        toggle = !toggle;
        writePort(digit1, false);
        drawDigit(num);
      }
      else{
        toggle = !toggle;
        drawDigit(-1);
        writePort(digit1, true);
      }
    }
  }
}
//debounce funktion för att bestämma knapparnas "state", anropas från main varje 2ms.
void button_state(uint8_t *count, uint8_t button, bool *btn_state, bool *posedge, bool *negedge){ 
  bool cur_state = ((PIND & (1<<button)) == 0); //knappens rådande "state"
  
  if(cur_state != *btn_state){  //om rådande "state" är annat än vad programmet vet så skall en räknare räknas upp.
    ++*count;
    if(*count >= 10){      //om räknaren når 10 så anses knappens state vara ändrat, btn_state uppdateras.
      *btn_state = cur_state;
      *count = 0;
      
      if(*btn_state){
        *posedge = 1;
      }
      else{
        *negedge = 1;
      }
    }
  }
  else{
    *count = 0;
    *posedge = 0;
    *negedge = 0;
  }
}


//funktion som avgör vilken siffra som skall visas på displayen.
void displayDigit(uint8_t *seqsize, uint8_t *high_score, uint8_t *display_digit, bool *btn_enter_state, uint8_t *check){
  if((*seqsize == 1) && (*check == 3)){
    *display_digit = *high_score;
  }
  else{
    *display_digit = (*seqsize-1);
  }
}

//funktion som känner igen om man håller inne en knapp i en sekund och ändrar i så fall värdet på bool longpress. 
void button_longpress(bool *btn_state, bool *longpress){
  static unsigned long t_zero = 0;  //5st variabler för att holla koll på hur funktionen skall utföras.
  static bool is_done = false;
  static bool time_set = false;    //variabel för att hålla koll på om tiden är satt för att känna igen ett longpress
  if((*btn_state)&&(!is_done)){
    if(!time_set){
      t_zero = millis();
      time_set = true;
    }
    if((millis() >= t_zero + 1000)){
      is_done = true;
      *longpress = !(*longpress);
    }
  }
  else if(!(*btn_state)){
    time_set = false;
    is_done = false;
  }
} 

void button_2xclick(bool *btn_2x, bool *btn_state){
  static bool is_done = 1;
  static unsigned long timer;
  static bool check_state = 0;
  if(*btn_2x){
    *btn_2x = 0;
  }
  if((is_done) && (*btn_state)){
    is_done = 0;
    timer = (millis() + 500);
  }
  if((millis() <= timer) && (!is_done)){
    if(!*btn_state){
      check_state = 1;
    }
    if((*btn_state) && (check_state)){
      *btn_2x = 1;
      check_state = 0;
      is_done = 1;
    }
  }
  else{
    is_done = 1;
    check_state = 0;
  }
}

/*void call_buzzer(uint16_t *sound[], uint8_t *sound_size, bool *go, uint16_t tones[][3], uint8_t rows){
  free(sound);
  *sound_size = rows;
  *sound = malloc(sizeof(uint16_t[rows][3]));

  for(uint8_t i = 0; i < *sound_size; i++){
    for(uint8_t j = 0; j < 3; j++){
      sound[i][j] = tones[i][j];
    }
  }
  *go = 1;
  
}*/

void call_buzzer(uint8_t *sound, bool *go, uint8_t wanted_sound){
  *sound = wanted_sound;
  *go = 1;
}

void buzzer(uint16_t sound[][3], uint8_t sound_size, bool *go){
  static uint8_t counter = 0;
  uint8_t buzz_high = sound[counter][0];
  uint8_t buzz_low = sound[counter][1];
  uint16_t tone_dur = sound[counter][2];
  unsigned long milli = millis();
  static bool time_set = 0;
  static unsigned long timer;
  static unsigned long tone_timer;
  if(*go){
    if(!time_set){
      timer = (milli + tone_dur);
      tone_timer = milli;
      time_set = 1;
    }
    if(milli <= timer){
      if(milli < (tone_timer + buzz_high)){
        PORTD |= (1<<BUZZ);
      }
      else if(milli < (tone_timer + buzz_low)){
        PORTD &= ~(1<<BUZZ);
      }
      else{
        tone_timer = milli;
      }
    }
    else{
      counter++;
      if(counter >= sound_size){
        counter = 0;
        *go = 0; 
      }
      timer = milli + tone_dur;
    }
    
  }
  else{
    counter = 0;
    time_set = 0;
    PORTD &= ~(1<<BUZZ);
    
  }
  
}

void generate_pop_up(uint8_t *pop_up){

  //function just to generate the next LED to light up.
  const uint8_t low = 1;
  const uint8_t high = 3;
  if(*pop_up == 0){
    *pop_up = low + rand() / (RAND_MAX / (high - low + 1) + 1);
  }
}

void game_dynamic(float *pace, unsigned long gametime, bool *run_game){

  //Function to make the pop_up game dynamic, based on the gametime it progressively 
  //increses a float(pace) that can be used to ramp up the gameplay.

  unsigned long milli = millis();
  static bool time_set = 0;
  static unsigned long t_start = 0;
  gametime *= 1000;
  
  if(*run_game){
    if(!time_set){
      t_start = milli;
      time_set = 1;
    }
    *pace = (((float)milli - t_start) / gametime);
  }
  else{
    time_set = 0;
    *pace = 0;
  }
  if((milli - t_start) >= gametime){
    *run_game = 0;
  }
}

void set_gametime(unsigned long *t_game, uint8_t difficulty){
  switch(difficulty){
    case 1:
      *t_game = 30;
      break;
    
    case 2:
      *t_game = 60;
      break;

    case 3: 
      *t_game = 90;
      break;
  }
}

void show_pop_up(uint8_t *pop_up, bool *ledL_on, bool *ledM_on, bool *ledR_on, float pace){
  //function to show the player what buttons to press. 

  static unsigned long timer = 0;
  unsigned long milli = millis();

  static unsigned long ledL_timer = 0;
  static unsigned long ledM_timer = 0;
  static unsigned long ledR_timer = 0;
  static unsigned long ledL_offtimer = 0;
  static unsigned long ledM_offtimer = 0;
  static unsigned long ledR_offtimer = 0;
  const uint16_t default_low = 300;
  const uint16_t default_high = 2000;
  const uint16_t default_led_ton = 600;
  const uint16_t default_led_toff = 300;
  static uint8_t low;
  static uint16_t high;
  static uint16_t led_ton;
  static uint8_t led_toff;

  //Logic to ramp up the speed of the gameplay.

  low = default_low - (100 * pace);
  high = default_high - (1500 * pace);
  led_ton = default_led_ton - (400 * pace);
  led_toff = default_led_toff - (100 * pace);

  //randomgenerates the amout of time between every new LED pop up.
  //also makes sure that no LED is lit up immedietly after it has ben turned off.

  if(milli > timer){
    timer = milli + (low + rand() / (RAND_MAX / (high - low + 1) + 1));
    if((*pop_up == 1)&&(milli >= ledR_offtimer)){
      *ledR_on = 1;
      ledR_timer = (milli + led_ton);
    }
    else if((*pop_up == 2)&&(milli >= ledM_offtimer)){
      *ledM_on = 1;
      ledM_timer = (milli + led_ton);
    }
    else if((*pop_up == 3)&&(milli >= ledL_offtimer)){
      *ledL_on = 1;
      ledL_timer = (milli + led_ton);
    }
  }

  //turn off the LEDs if its button has not been pressed in time.

  if(milli >= ledR_timer){
    *ledR_on = 0;
    *pop_up = 0;
  }
  if(milli >= ledM_timer){
    *ledM_on = 0;
    *pop_up = 0;
  }
  if(milli >= ledL_timer){
    *ledL_on = 0;
    *pop_up = 0;
  }

  //if any LED is on, update its off-time timer.

  if(*ledR_on){
    ledR_offtimer = milli + led_toff;
  }
  if(*ledM_on){
    ledM_offtimer = milli + led_toff;
  }
  if(*ledL_on){
    ledL_offtimer = milli + led_toff;
  }
}

void manage_leds(bool ledL_on, bool ledM_on, bool ledR_on){
  //funtion just to manage the LEDs

  if(ledR_on){
    PORTB |= (1 << ledR);
  }
  else{
    PORTB &= ~(1 << ledR);
  }
  if(ledM_on){
    PORTB |= (1 << ledM);
  }
  else{
    PORTB &= ~(1 << ledM);
  }
  if(ledL_on){
    PORTB |= (1 << ledL);
  }
  else{
    PORTB &= ~(1 << ledL);
  }
}

void check_if_score(bool btn_L, bool btn_M, bool btn_R, uint8_t *score, bool *ledL_on, bool *ledM_on, bool *ledR_on, bool posedge_L, bool posedge_M, bool posedge_R, uint8_t *sound, bool *go_buzz){

  unsigned long milli = millis();
  static unsigned long timer = 0;
  static bool discard = 0;
  static uint16_t timeout = 1500;
  static uint8_t cheat_counter = 0;
  
  //some logick to make sure you cant gain points by rapidly hitting the buttons.

  if((posedge_L)&&(!*ledL_on)){
    cheat_counter++;
    call_buzzer(sound, go_buzz, 2);
  }
  if((posedge_M)&&(!*ledM_on)){
    cheat_counter++;
    call_buzzer(sound, go_buzz, 2);
  }
  if((posedge_R)&&(!*ledR_on)){
    cheat_counter++;
    call_buzzer(sound, go_buzz, 2);
  }
  if(*ledL_on || *ledM_on || *ledR_on){
    cheat_counter = 0;
  }
  if(cheat_counter >= 2){
      timer = (milli + timeout);
  }
  if(timer >= milli){
    discard = 1;
  }
  else{
    discard = 0;
  }

  //logic to check if a button is pressed when the corresponding LED is on. If so, turn off that LED and 
  //increment the score.

  if((posedge_L)&&(!discard)&&(*ledL_on)){
    *score += 1;
    *ledL_on = 0;
    call_buzzer(sound, go_buzz, 1);
  }
  if((posedge_M)&&(!discard)&&(*ledM_on)){
    *score += 1;
    *ledM_on = 0;
    call_buzzer(sound, go_buzz, 1);
  }
  if((posedge_R)&&(!discard)&&(*ledR_on)){
    *score += 1;
    *ledR_on = 0;
    call_buzzer(sound, go_buzz, 1);
  }
  
}

void leds_off(bool *ledL_on, bool *ledM_on, bool *ledR_on){
  *ledL_on = 0;
  *ledM_on = 0;
  *ledR_on = 0;
}

void reset_games(uint8_t *check, bool *run_game){
  *check = 4;
  *run_game = 0;
}

void reset_pop_score(uint8_t *score, bool run_game){
  //Function to reset the score just when a new game begins.
  static bool reset = 0;
  if(run_game){
    if(!reset){
      *score = 0;
      reset = 1;
    }
  }
  else{
    reset = 0;
  }
}

void game_select_mode(bool *mode_sel, bool btn_L, bool btn_M, bool btn_R, bool neg_e, uint8_t *game_mode, uint8_t *memo_difficulty, uint8_t *pop_difficulty){
  
  //Function to create a kind of lobby where you can pick what game to play and at what difficulty.

  if(btn_L){
    *game_mode = 1;
  }
  if(btn_M){
    *game_mode = 2;
  }

  if((*memo_difficulty == 0) || (*pop_difficulty == 0)){
    *pop_difficulty = 1;
    *memo_difficulty = 1;
  }
  
  switch(*game_mode){
    case 1:

      if(neg_e){
        ++*memo_difficulty;
        if(*memo_difficulty > 3){
          *memo_difficulty = 1;
        }
      }
      
      if(*memo_difficulty == 1){
        PORTB |= (1 << ledL);
        PORTB &= ~((1 << ledM) | (1 << ledR));
      }
      else if(*memo_difficulty == 2){
        PORTB |= (1 << ledL) | (1 << ledM);
        PORTB &= ~(1 << ledR);
      }
      else if(*memo_difficulty == 3){
        PORTB |= (1 << ledL) | (1 << ledM) | (1 << ledR);
      }
      break;
    case 2:
    
      if(neg_e){
        ++*pop_difficulty;
        if(*pop_difficulty > 3){
          *pop_difficulty = 1;
        }
      }

      if(*pop_difficulty == 1){
        PORTB |= (1 << ledL);
        PORTB &= ~((1 << ledM) | (1 << ledR));
      }
      else if(*pop_difficulty == 2){
        PORTB |= (1 << ledL) | (1 << ledM);
        PORTB &= ~(1 << ledR);
      }
      else if(*pop_difficulty == 3){
        PORTB |= (1 << ledL) | (1 << ledM) | (1 << ledR);
      }
      break;
      
    default:
      break;
  }
}


void confirm_start(bool run_game, bool *ledL_on, bool *ledM_on, bool *ledR_on){

  //Function to show the player that the game is about to begin.
  unsigned long milli = millis();
  static unsigned long timer = 0;
  const uint8_t duration = 100;
  static bool time_set = 0;
  static bool done = 0;

  if(!run_game){
    done = 0;
  }
  
  if(run_game && !done){
    if(!time_set){
      timer = milli;
      time_set = 1;
      *ledL_on = 1;
    }
    if(milli > timer + duration){
      *ledL_on = 0;
      *ledM_on = 1;
    }
    if(milli > timer + (2*duration)){
      *ledM_on = 0;
      *ledR_on = 1;
    }
    if(milli > timer + (3*duration)){
      *ledR_on = 0;
      *ledM_on = 1;
    }
    if(milli > timer + (4*duration)){
      *ledM_on = 0;
      *ledL_on = 1;
    }
    if(milli > timer + (5*duration)){
      *ledL_on = 0;
      done = 1;
      time_set = 0;
    }
  }
}
