/*
 * PmemC.c
 *
 * Created: 2021-03-12 19:14:16
 *  Author: Jes_p
 */ 
#include "PmemINC.h"

volatile unsigned long timer1_millis;

void init_DDR(void){ 
  DDRB = (1<<ledL)|(1<<ledM)|(1<<ledR)|(1<<PORTB3)|(1<<PORTB4)|(1<<BUZZ);
  DDRD = (1<<digit1)|(1<<digit2);
  DDRC = (1<<PORTC0)|(1<<PORTC3)|(1<<PORTC2)|(1<<PORTC1)|(1<<PORTC4);
}
void init_ports(void){
  PORTD = (1<<ENTER)|(1<<BTNL)|(1<<BTNM)|(1<<BTNR)|(1<<digit1)|(1<<digit2);
}

void ledBlink(unsigned char *sequence, uint8_t size, uint8_t *check, uint8_t period, bool posedge_enter){ //Funktion som tänder LEDs enligt den sekvenser som är genererad.
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
      time_stamp += period;   //visa första delen av sekvensen. Därefter så skall vi släcka och tända nästa del var 250/125e ms.
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

void assign_sequence(unsigned char* sequence, uint8_t size){ //Tilldelar sekvensen slumpade värden mellan 1-6. 
  const uint8_t low = 1;
  const uint8_t high = 6;
  for(uint8_t i = size-1; i < size; i++){
    sequence[i] = low + rand() / (RAND_MAX / (high - low + 1) + 1);
  }
}

  //funktion som kollar vilka knappar man trycker ned och jämför med den nyss visade sekvensen.
void cmp_sequence(unsigned char *sequence, uint8_t size, uint8_t *check, bool btn1, bool btn2, bool btn3){
  
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
    if(count >= 6){ //När count när 6 så är funktionen färdig och check ställs till 5, variabler återställs.
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
  if(num >= 10){      //om siffran man vill visa är större än 10 skall denna metoden följas.
    if(!time_set){
      t_zero = millis();
      time_set = true;
    }
    if(millis() >= t_zero){ //varje siffra skall lysa i 5ms var.
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
      t_zero = millis();
      time_set = true;
    }
    writePort(digit2, true);    //inaktiverar digit2 (till höger på display).
    if(millis() >= t_zero + 5){   //blinkar siffran så att den lyser med samma styrka som om båda används.
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
void button_state(uint8_t *count, uint8_t button, bool *btn_state, bool *posedge){ 
  bool cur_state = ((PIND & (1<<button)) == 0); //knappens rådande "state"
  
  if(cur_state != *btn_state){  //om rådande "state" är annat än vad programmet vet så skall en räknare räknas upp.
    ++*count;
    if(*count >= 4){      //om räknaren når 4 så anses knappens state vara ändrat, btn_state uppdateras.
      *btn_state = cur_state;
      *count = 0;
    }
    *posedge = 1;
    
  }
  else{
    *count = 0;
    *posedge = 0;
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
  static uint8_t counter = 0;
  static bool is_on = false;
  static bool is_done = false;
  static bool go_blink = false;
  static bool time1_set = false;    //variabel för att hålla koll på om tiden är satt för att känna igen ett longpress
  static bool time2_set = false;    //variabel för att hålla koll på om tiden för eventuell blinkning är satt.
  if((*btn_state)&&(!is_done)){
    if(!time1_set){
      t_zero = millis();
      time1_set = true;
    }
    if((millis() >= t_zero + 1000)){
      is_done = true;
      go_blink = true;
      *longpress = !(*longpress);
    }
  }
  else if(!(*btn_state)){
    time1_set = false;
    is_done = false;
  }
  if(go_blink){   
    if(!time2_set){
      t_zero = millis();
      time2_set = true;
    }
    if(millis() >= (t_zero + 100)){ //Tänd och släck LEDs var 100ms.
      t_zero += 100;
      if(!is_on){
        PORTB |= (1 << ledL);
        is_on = true; 
        counter++;
      }
      else{
        PORTB &= ~(1 << ledL);
        is_on = false;
        counter++;
      }
      if(counter >= 6){
        counter = 0;
        time2_set = false;
        go_blink = false;
        is_done = true;
      }
    }
  }
} 

void button_2xclick(uint8_t *check, bool *btn_2x, bool *btn_state){
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
    }
  }
  else{
    is_done = 1;
    check_state = 0;
  }
}

void buzzer(uint8_t check){
  static bool go = 0;
  static unsigned long time_stamp3 = 0, timer = 0;
  if((check == 0)||(go)){
    if(!go){
      go = 1;
      time_stamp3 = millis();
      timer = millis();
    }
    if(time_stamp3 < timer+500){
      if(millis() > time_stamp3+1){
        PORTB |= (1<<BUZZ);
      }
      if(millis() > time_stamp3+2){
        time_stamp3 = millis();
        PORTB &= ~(1<<BUZZ);
      }
    
    }
    else if(time_stamp3 < timer+1000){
      if(millis() > time_stamp3+3){
        PORTB |= (1<<BUZZ);
      }
      if(millis() > time_stamp3+6){
        time_stamp3 = millis();
        PORTB &= ~(1<<BUZZ);
      }
      
    }
    else if(time_stamp3 < timer+1500){
      if(millis() > time_stamp3+6){
        PORTB |= (1<<BUZZ);
      }
      if(millis() > time_stamp3+12){
        time_stamp3 = millis();
        PORTB &= ~(1<<BUZZ);
      }
      
    }
    if(millis() > timer+1500){
      go = 0;
    }
  
  }
}

void generate_pop_up(uint8_t *pop_up){
  static uint8_t last_pop = 0;
  const uint8_t low = 1;
  const uint8_t high = 4;
  if(*pop_up == 0){
    *pop_up = low + rand() / (RAND_MAX / (high - low + 1) + 1);
    if(*pop_up == 3){
      *pop_up == 0;
    }
  }
}

void show_pop_up(uint8_t *pop_up){
  static unsigned long timer = 0;
  static uint8_t low = 100;
  static uint8_t high = 1000;

  static bool ledL_on = 0;
  static bool ledM_on = 0;
  static bool ledR_on = 0;

  static unsigned long ledL_timer = 0;
  static unsigned long ledM_timer = 0;
  static unsigned long ledR_timer = 0;

  if(millis() > timer){
    timer += low + rand() / (RAND_MAX / (high - low + 1) + 1);

    if(*pop_up == 1){
      PORTB |= (1 << ledR);
      ledR_on = 1;
      ledR_timer = (millis() + 300);
    }
    else if(*pop_up == 2){
      PORTB |= (1 << ledM);
      ledM_on = 1;
      ledM_timer = (millis() + 300);
    }
    else if(*pop_up == 4){
      PORTB |= (1 << ledL);
      ledL_on = 1;
      ledL_timer = (millis() + 300);
    }

    if(millis() >= ledR_timer){
      PORTB &= ~(1 << ledR);
      ledR_on = 0;
    }
    if(millis() >= ledM_timer){
      PORTB &= ~(1 << ledM);
      ledM_on = 0;
    }
    if(millis() >= ledL_timer){
      PORTB &= ~(1 << ledL);
      ledL_on = 0;
    }
  }
}