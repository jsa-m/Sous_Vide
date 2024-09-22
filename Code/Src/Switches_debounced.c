/*
 * Switches_debounced.c
 *
 *  Created on: Aug 17, 2024
 *      Author: joaso
 */

#include "Switches_debounced.h"

void SM_button_debounce(Button *btn){

	switch (btn->state_button) {
	case 0: //Init
		btn->button_cnt = 0;
		btn->flag_button_pressed = 0;
		btn->flag_long_button_pressed = 0;
		btn->counter_10ms = 0;
		btn->state_button = 1;
		btn->counter_3s = 0;
		break;
	case 1: //Wait
		if(HAL_GPIO_ReadPin(btn->gpio, btn->gpio_pin) == 1)
		{
			btn->counter_10ms = 0;
			btn->state_button = 2;
		}

		break;
	case 2: //Button_low
		btn->counter_10ms += 1;
		if(HAL_GPIO_ReadPin(btn->gpio, btn->gpio_pin) == 0) //Bounce
		{
			btn->state_button = 1;
		}
		else if (btn->counter_10ms >= 10) //No bounce
		{
			btn->state_button = 3;
		}
		break;
	case 3: //Check for button release
		btn->counter_3s +=1;			//Low state count
		if(btn->counter_3s >  1500)
		{
			btn->counter_3s = 1500; //Saturate
		}


		if(HAL_GPIO_ReadPin(btn->gpio, btn->gpio_pin) == 0) //Bounce
		{
			btn->state_button = 4;
		}
		break;
	case 4: //Button pressed without bounces
		btn->button_cnt += 1;
		btn->state_button = 1;
		if(btn->counter_3s <  1500)
		{
			btn->flag_button_pressed = 1;
		}
		else{
			btn->flag_long_button_pressed = 1;
		}
		btn->counter_3s = 0;
		break;
	default:
		break;
	}
}

void rotary_debounced(Rotary_enc *enc)
{

	enc->CLKstate = HAL_GPIO_ReadPin(enc->gpio, enc->encApin);
	enc->DTstate = HAL_GPIO_ReadPin(enc->gpio, enc->encBpin);

	switch (enc->state) {
	case 0: //Idle state, encoder not turning
		if(enc->CLKstate == 0)
		{
			enc->state = 1;				//Turn clockwise and CLK goes low first
		}else if (enc->DTstate == 0) {
			enc->state = 4;			    //Turn clockwise and DT goes low first
		}
		break;
		//Clockwise rotation
	case 1:
		if(enc->DTstate == 0)
		{
			enc->state = 2;				//Continue clockwise and DT will go low after CLK
		}
		break;
	case 2:
		if(enc->CLKstate == 1)
		{
			enc->state = 3;				//Turn further and CLK will go high first
		}
		break;
	case 3:
		if(enc->CLKstate == 1 && enc->DTstate == 1){	//Both CLK and DT high -> encoder completes one step
			enc->state = 0;
			enc->rotary_cnt += 1;
		}
		break;
		//Anticlockwise rotation, as for clockwise but with inputs inverted
	case 4:
		if(enc->CLKstate == 0)
		{
			enc->state = 5;
		}
		break;
	case 5:
		if(enc->DTstate == 1)
		{
			enc->state = 6;
		}
		break;
	case 6:
		if(enc->CLKstate == 1 && enc->DTstate == 1)
		{
			enc->state = 0;
			enc->rotary_cnt -=1;
		}
		break;
	default:
		break;
	}

}

