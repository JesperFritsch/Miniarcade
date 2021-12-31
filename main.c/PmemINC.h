/*
 * PmemINC.h
 *
 * Created: 2021-03-12 19:13:52
 *  Author: Jes_p
 */ 


#ifndef PMEMINC_H_
#define PMEMINC_H_

#define F_CPU 16000000UL
#define BTNL 5    //4st defines för vart knapparna är kopplade.
#define BTNM 4
#define BTNR 3
#define ENTER 2
#define ledL PORTB2 //3st defines för vart LEDs är kopplade och 2st defines för enable pinnar till display.
#define ledM PORTB1
#define ledR PORTB0
#define digit1 PORTD0
#define digit2 PORTD1
#define segA 11   //7st defines för pinnarna till alla segment på 7segmets display.
#define segB 14
#define segC 17
#define segD 16
#define segE 15
#define segF 12
#define segG 18
#define BUZZ PORTD6 
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/atomic.h>
#include <avr/interrupt.h>


void init_DDR(void);
void ledBlink(unsigned char *sequence, uint8_t size, uint8_t *check, bool posedge_enter);
void init_ports(void);
int init_srand(void);
void assign_sequence(unsigned char* sequence, uint8_t size, uint8_t difficulty);
void cmp_sequence(unsigned char *sequence, uint8_t size, uint8_t *check, bool btn1, bool btn2, bool btn3, uint8_t *sound, bool *go_buzz);
void is_success(uint8_t *check);
void init_millis(unsigned long f_cpu);
unsigned long millis();
bool button_pressed(uint8_t button, uint8_t count);
void writePort(uint8_t pin, bool value);
void drawDigit(int8_t num);
void printDigit(int8_t num);
void button_state(uint8_t *count, uint8_t button, bool *btn_state, bool *posedge, bool *negedge);
void button_longpress(bool *btn_state, bool *longpress);
void button_2xclick(bool *btn_2x, bool *btn_state);
void displayDigit(uint8_t *seqsize, uint8_t *high_score, uint8_t *display_digit, bool *btn_enter_state, uint8_t *check);
void show_pop_up(uint8_t *pop_up, bool *ledL_on, bool *ledM_on, bool *ledR_on, float pace);
void generate_pop_up(uint8_t *pop_up);
void check_if_score(bool btn_L, bool btn_M, bool btn_R, uint8_t *score, bool *ledL_on, bool *ledM_on, bool *ledR_on, bool posedge_L, bool posedge_M, bool posedge_R, uint8_t *sound, bool *go_buzz);
void leds_off(bool *ledL_on, bool *ledM_on, bool *ledR_on);
void reset_games(uint8_t *check, bool *run_game);
void confirm_start(bool run_game, bool *ledL_on, bool *ledM_on, bool *ledR_on);
void game_dynamic(float *pace, unsigned long gametime, bool *run_game);
void reset_pop_score(uint8_t *score, bool run_game);
void manage_leds(bool ledL_on, bool ledM_on, bool ledR_on);
void game_select_mode(bool *mode_sel, bool btn_L, bool btn_M, bool btn_R, bool pos_e, uint8_t *game_mode, uint8_t *memo_diffuculty, uint8_t *pop_diffuculty);
void set_gametime(unsigned long *t_game, uint8_t difficulty);
void buzzer(uint16_t sound[][3], uint8_t sound_size, bool *go);
void call_buzzer(uint8_t *sound, bool *go, uint8_t wanted_sound);
//void buzzer(uint16_t *sound[], uint8_t sound_size, bool *go);
//void call_buzzer(uint16_t *sound[], uint8_t *sound_size, bool *go, uint16_t tones[][3], uint8_t rows);


#endif /* PMEMINC_H_ */
