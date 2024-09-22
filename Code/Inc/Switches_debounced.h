/*
 * Switches_debounced.h
 *
 *  Created on: Aug 17, 2024
 *      Author: joaso
 */

#include "stm32f1xx_hal.h"

typedef struct{
	//State machine variables
	uint8_t counter_10ms; //Counter for button state machine
	uint16_t counter_3s; //Counter to decide between short or long press
	uint8_t state_button;
	//Button related variables
	uint16_t button_cnt;
	uint8_t flag_button_pressed;
	uint8_t flag_long_button_pressed;
	//Physical button
	GPIO_TypeDef *gpio;
	uint16_t gpio_pin;

}Button;

typedef struct{
	//State machine variables
	uint8_t state;
	uint8_t CLKstate;
	uint8_t DTstate;
	//Encoder related variables
	int16_t rotary_cnt;
	//Physical encoder
	GPIO_TypeDef *gpio;	//Assumed encoder pins are in same port
	uint16_t encApin;
	uint16_t encBpin;

}Rotary_enc;

void SM_button_debounce(Button *btn);
void rotary_debounced(Rotary_enc *enc);
