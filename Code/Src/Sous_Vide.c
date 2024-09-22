/*
 * Sous_Vide.c
 *
 *  Created on: Aug 17, 2024
 *      Author: joaso
 */

#include "Sous_Vide.h"


void HMI_stMachine(HMI *hmi, OLED_SIMPLE *oled, Button *b1, Button *b2, Rotary_enc *enc1, Rotary_enc *enc2, DS18B20_TSens *Tsen, QueueHandle_t hmiToCntrlQueue, QueueHandle_t CntrlToHmiQueue, RTC_HandleTypeDef *hrtc){

	char tempStr[5]; //4 digits + null terminator
	uint16_t whole, fraction, temperature;
	static ControlEvent queueHMI;
	static RTC_TimeTypeDef rtcTime;

	switch (hmi->state) {
	case 0: //Init
		Oled_Simple_Init(oled, oled->i2cHandle);
		Oled_Simple_Fill(oled, 0x00);
		Oled_Simple_Update_Screen(oled, oled->i2cHandle);
		hmi->state = 1;
		hmi->ref_hours = 0;
		hmi->ref_min = 0;
		hmi->ref_temperature = 0;
		hmi->ref_temperature_prev = 99; //Reset timer to update temperature
		hmi->ref_hours_prev = 25; //Reset timer to update temperature
		hmi->ref_min_prev = 65;
		hmi->internal_menu = 1;
		hmi->timer_hours = 0;
		hmi->timer_hours_prev = 25;
		hmi->timer_min = 0;
		hmi->timer_min_prev = 61;
		break;
	case 1: //Initial menu
		if(hmi->state != hmi->prev_state) //Execute only once
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,30,10);
			Oled_Simple_Write_STRING(oled,"SOUS", Font_11x18,0);
			Oled_Simple_Set_cursor(oled,50,40);
			Oled_Simple_Write_STRING(oled,"VIDE", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			Oled_Simple_Fill(oled, 0x00);
			oled->OLED_HALF = 2;
			Oled_Simple_Set_cursor(oled,0,30);
			Oled_Simple_Write_STRING(oled,"Pulse para iniciar", Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
			queueHMI = EVENT_IDDLE;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}

		//Check if button menu was pressed to change to next menu
		if(b1->flag_button_pressed == 1)
		{
			b1->flag_button_pressed = 0;
			enc1->rotary_cnt = 0; //Reset counter to capture temperature
			Oled_Simple_Fill(oled, 0x00);
			Oled_Simple_Update_Screen(oled, oled->i2cHandle);
			hmi->state = 2;
		}
		break;
	case 2: //Select temperature 90ºC max
		//update menu only once
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,5,5);
			Oled_Simple_Write_STRING(oled,"SELECCIONE", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			Oled_Simple_Set_cursor(oled,0,28);
			Oled_Simple_Write_STRING(oled,"TEMPERATURA", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}

		//Check for reference temperature changes
		//Read rotary cnt
		if(enc1->rotary_cnt < 0)
		{
			enc1->rotary_cnt = 0;			//Check for negative values
		}
		else if (enc1->rotary_cnt >90) {
			enc1->rotary_cnt = 90;			//max operating temperature
		}
		hmi->ref_temperature = enc1->rotary_cnt;

		//Only if temperature changes update lower part of screen
		if(hmi->ref_temperature != hmi->ref_temperature_prev)
		{
			hmi->ref_temperature_prev = hmi->ref_temperature;
			oled->OLED_HALF = 2;
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			Oled_Simple_Set_cursor(oled,55,20);
			//Add zero if number less than zero
			if(hmi->ref_temperature < 10)
			{
				Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
			}
			sprintf(tempStr, "%u", hmi->ref_temperature);
			Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
		}

		//Check if button menu was pressed to change to next menu
		if(b1->flag_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 3;
			enc1->rotary_cnt = 0;
		}
		break;
	case 3: //Select time
		//Update menu only once
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,5,5);
			Oled_Simple_Write_STRING(oled,"SELECCIONE", Font_11x18,0);
			Oled_Simple_Set_cursor(oled,25,28);
			Oled_Simple_Write_STRING(oled,"TIEMPO", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}

		//Change between hours and minutes
		if(b2->flag_button_pressed == 1)
		{
			b2->flag_button_pressed = 0;

			//Menu 1 change to minutes

			if(hmi->internal_menu == 1)
			{
				enc1->rotary_cnt = hmi->ref_min;
				hmi->internal_menu = 2;
			}
			//Change to hours
			else{
				enc1->rotary_cnt = hmi->ref_hours;
				hmi->internal_menu = 1;
			}
		}

		//Check for reference temperature changes
		if(hmi->internal_menu == 1)
		{
			//Read rotary cnt
			if(enc1->rotary_cnt < 0)
			{
				enc1->rotary_cnt = 0;			//Check for negative values
			}
			else if (enc1->rotary_cnt >24) {
				enc1->rotary_cnt = 24;			//Limit hours counter
			}
			hmi->ref_hours = enc1->rotary_cnt;

			//Only if hours changes update lower part of screen
			if(hmi->ref_hours != hmi->ref_hours_prev)
			{
				hmi->ref_hours_prev = hmi->ref_hours;
				oled->OLED_HALF = 2;
				Oled_Simple_Fill(oled, 0x00); //Clean buffer
				Oled_Simple_Set_cursor(oled,30,20);
				if(hmi->ref_hours < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				sprintf(tempStr, "%u", hmi->ref_hours);
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Write_STRING(oled,":", Font_11x18,0);
				sprintf(tempStr, "%u", hmi->ref_min);
				if(hmi->ref_min < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			}

		}
		else if (hmi->internal_menu == 2) {
			//Read rotary cnt
			if(enc1->rotary_cnt < 0)
			{
				enc1->rotary_cnt = 0;			//Check for negative values
			}
			else if (enc1->rotary_cnt >60) {
				enc1->rotary_cnt = 60;			//Limit minutes counter
			}
			hmi->ref_min = enc1->rotary_cnt;

			//Only if minutes changes update lower part of screen
			if(hmi->ref_min != hmi->ref_min_prev)
			{
				hmi->ref_min_prev = hmi->ref_min;
				oled->OLED_HALF = 2;
				Oled_Simple_Fill(oled, 0x00); //Clean buffer
				Oled_Simple_Set_cursor(oled,30,20);
				if(hmi->ref_hours < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				sprintf(tempStr, "%u", hmi->ref_hours);
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Write_STRING(oled,":", Font_11x18,0);
				sprintf(tempStr, "%u", hmi->ref_min);
				if(hmi->ref_min < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			}
		}


		//Check if button menu was pressed to change to next menu
		if(b1->flag_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 4;
		}
		break;

	case 4: //Init pre-heat
		//update menu only once
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,10,5);
			Oled_Simple_Write_STRING(oled," PULSE", Font_11x18,0);
			Oled_Simple_Set_cursor(oled,10,40);
			Oled_Simple_Write_STRING(oled," PARA", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			oled->OLED_HALF = 2;
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			Oled_Simple_Set_cursor(oled,10,20);
			Oled_Simple_Write_STRING(oled," INICIAR", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}
		//Check if button menu was pressed to change to next menu
		//If temperature is greater than set point then surpass pre-heating state
		//otherwise enter pre-heating state
		if(b1->flag_button_pressed == 1 && (Tsen->Temperature >= hmi->ref_temperature) && (hmi->ref_temperature != 0.0))
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 6;
		}
		else if (b1->flag_button_pressed == 1 && (Tsen->Temperature < hmi->ref_temperature) && (hmi->ref_temperature > 0.0))
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 5;
			queueHMI = EVENT_PREHEAT;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}
		//No input go to iddle
		else if((hmi->ref_temperature == 0.0) && b1->flag_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 1;
		}
		break;

	case 5: //Pre-heating state
		if(hmi->state != hmi->prev_state) //Execute only once
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,10,0);
			Oled_Simple_Write_STRING(oled,"Precalentando", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,10,20);
			Oled_Simple_Write_STRING(oled,"recipiente", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,0,50);
			Oled_Simple_Write_STRING(oled,"Consigna Actual", Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			Oled_Simple_Fill(oled, 0x00);
			oled->OLED_HALF = 2;
			sprintf(tempStr, "%u", hmi->ref_temperature);
			Oled_Simple_Set_cursor(oled,0,20);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			//Convert float temperature
			temperature = (uint16_t)(Tsen->Temperature * 10);
			whole = (uint16_t)(temperature/10);
			fraction = (uint16_t)(temperature - whole*10);
			sprintf(tempStr,"%d.%d", whole, fraction);
			Oled_Simple_Set_cursor(oled,60,20);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->last_temp = Tsen->Temperature;
			hmi->prev_state = hmi->state; //Execute only once
		}
		//If temperature has change update
		if(hmi->last_temp != Tsen->Temperature)
		{
			hmi->last_temp = Tsen->Temperature;
			Oled_Simple_Fill(oled, 0x00);
			oled->OLED_HALF = 2;
			sprintf(tempStr, "%u", hmi->ref_temperature);
			Oled_Simple_Set_cursor(oled,0,20);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			//Convert float temperature
			temperature = (uint16_t)(hmi->last_temp * 10);
			whole = (uint16_t)(temperature/10);
			fraction = (uint16_t)(temperature - whole*10);
			sprintf(tempStr,"%d.%d", whole, fraction);
			Oled_Simple_Set_cursor(oled,60,20);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);

		}

		//If long button press reset, else wait to the other state machine
		if(b1->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_long_button_pressed = 0;
			hmi->state = 1;
		}
		//Here we check if sous-vide has been preheated
		else if (b1->flag_button_pressed == 1 && (Tsen->Temperature >= hmi->ref_temperature)) //Todo se puede dar el caso de que se tarde mucho en apagar la alarma y no cambie de estado
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 6;
			queueHMI = EVENT_IDDLE;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}
		else if(b1->flag_button_pressed == 1)
		{
			b1->flag_button_pressed = 0;
		}

		break;
	case 6: //Wait to start cooking
		if(hmi->state != hmi->prev_state) //Execute only once
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,10,0);
			Oled_Simple_Write_STRING(oled,"  Recipiente", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,10,20);
			Oled_Simple_Write_STRING(oled," precalentado", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,0,50);
			Oled_Simple_Write_STRING(oled,"Pulse para iniciar", Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			Oled_Simple_Fill(oled, 0x00);
			oled->OLED_HALF = 2;
			Oled_Simple_Set_cursor(oled,0,20);
			Oled_Simple_Write_STRING(oled,"     coccion", Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}

		//If long button press reset, else wait to the other state machine
		if(b1->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_long_button_pressed = 0;
			hmi->state = 1;
		}
		else if (b1->flag_button_pressed == 1) {
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 7;
			//Configure and start RTC alarm from captured data
			set_RTC_time(hrtc, 0, 0);
			if(!(hmi->ref_hours == 0 && hmi->ref_min == 0))
			{
				set_alarm(hrtc, hmi->ref_hours, hmi->ref_min);
			}
		}
		break;
	case 7: //Cooking state
		if(hmi->state != hmi->prev_state) //Execute only once
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,0,0);
			Oled_Simple_Write_STRING(oled,"Coccion en proceso", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,10,20);
			Oled_Simple_Write_STRING(oled,"Tiempo restante", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,30,40);
			if(hmi->timer_hours < 10)
			{
				Oled_Simple_Write_STRING(oled,"0", Font_7x10,0);
			}
			sprintf(tempStr, "%u", (hmi->ref_hours - hmi->timer_hours) < 0 ? 0 : (hmi->ref_hours - hmi->timer_hours));
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Write_STRING(oled,":", Font_7x10,0);
			sprintf(tempStr, "%u", (hmi->ref_min - hmi->timer_min) < 0 ? 0 : (hmi->ref_min - hmi->timer_min));
			if(hmi->timer_min < 10)
			{
				Oled_Simple_Write_STRING(oled,"0", Font_7x10,0);
			}
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			Oled_Simple_Fill(oled, 0x00);
			oled->OLED_HALF = 2;
			sprintf(tempStr, "%u", hmi->ref_temperature);
			Oled_Simple_Set_cursor(oled,0,20);
			Oled_Simple_Write_STRING(oled,"Consigna Actual", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,0,40);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			//Convert float temperature
			temperature = (uint16_t)(hmi->last_temp * 10);
			whole = (uint16_t)(temperature/10);
			fraction = (uint16_t)(temperature - whole*10);
			sprintf(tempStr,"%d.%d", whole, fraction);
			Oled_Simple_Set_cursor(oled,60,40);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
			//Enter in cooking mode
			if(hmi->ref_min == 0 && hmi->ref_hours == 0)
			{
				queueHMI = EVENT_IDDLE;
				Oled_Simple_Fill(oled, 0x00);
				hmi->state = 1;
			}
			else
			{
				queueHMI = EVENT_COOK;

			}
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full

		}
		//If time has change update
		HAL_RTC_GetTime(hrtc, &rtcTime, RTC_FORMAT_BIN);
		hmi->timer_hours = rtcTime.Hours;
		hmi->timer_min = rtcTime.Minutes;


		if((hmi->timer_hours != hmi->timer_hours_prev) || (hmi->timer_min != hmi->timer_min_prev))
		{
			hmi->timer_hours_prev = hmi->timer_hours;
			hmi->timer_min_prev = hmi->timer_min;
			oled->OLED_HALF = 1;
			Oled_Simple_Fill(oled, 0x00);
			Oled_Simple_Set_cursor(oled,0,0);
			Oled_Simple_Write_STRING(oled,"Coccion en proceso", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,10,20);
			Oled_Simple_Write_STRING(oled,"Tiempo restante", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,30,40);
			if(hmi->timer_hours < 10)
			{
				Oled_Simple_Write_STRING(oled,"0", Font_7x10,0);
			}
			sprintf(tempStr, "%u", (hmi->ref_hours - hmi->timer_hours) < 0 ? 0 : (hmi->ref_hours - hmi->timer_hours));
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Write_STRING(oled,":", Font_7x10,0);
			sprintf(tempStr, "%u", (hmi->ref_min - hmi->timer_min) < 0 ? 0 : (hmi->ref_min - hmi->timer_min));
			if(hmi->timer_min < 10)
			{
				Oled_Simple_Write_STRING(oled,"0", Font_7x10,0);
			}
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);


		}
		//If temperature has change update
		if(hmi->last_temp != Tsen->Temperature)
		{
			hmi->last_temp = Tsen->Temperature;
			Oled_Simple_Fill(oled, 0x00);
			oled->OLED_HALF = 2;
			sprintf(tempStr, "%u", hmi->ref_temperature);
			Oled_Simple_Set_cursor(oled,0,20);
			Oled_Simple_Write_STRING(oled,"Consigna Actual", Font_7x10,0);
			Oled_Simple_Set_cursor(oled,0,40);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			//Convert float temperature
			temperature = (uint16_t)(hmi->last_temp * 10);
			whole = (uint16_t)(temperature/10);
			fraction = (uint16_t)(temperature - whole*10);
			sprintf(tempStr,"%d.%d", whole, fraction);
			Oled_Simple_Set_cursor(oled,60,40);
			Oled_Simple_Write_STRING(oled,tempStr, Font_7x10,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);

		}

		//If long button press reset, else wait to the other state machine
		if(b1->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_long_button_pressed = 0;
			b1->flag_button_pressed = 0;
			hmi->state = 1;
			queueHMI = EVENT_IDDLE;
		    xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}

		//If long button press other button pause state
		if(b2->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b2->flag_long_button_pressed = 0;
			b2->flag_button_pressed = 0;
			hmi->state = 9;
			queueHMI = EVENT_IDDLE;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}
		//Comprobamos que se ha cumplido el tiempo
		if ((hmi->timer_hours == hmi->ref_hours) && (hmi->timer_min == hmi->ref_min) && !((hmi->timer_min == 0) && hmi->timer_hours == 0))
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			hmi->state = 8;
			queueHMI = EVENT_ALARM;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}

		break;
	case 8: //END STATE
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Fill(oled, 0x00);
			Oled_Simple_Set_cursor(oled,5,5);
			Oled_Simple_Write_STRING(oled,"COCCION", Font_11x18,0);
			Oled_Simple_Set_cursor(oled,0,28);
			Oled_Simple_Write_STRING(oled,"FINALIZADA", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			oled->OLED_HALF = 2;
			Oled_Simple_Fill(oled, 0x00);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}
		//If long button press reset, else wait to the other state machine
		if(b1->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_long_button_pressed = 0;
			hmi->state = 1;
			queueHMI = EVENT_IDDLE;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}
		else if (b1->flag_button_pressed == 1) {
			Oled_Simple_Fill(oled, 0x00); //Clean buffer

			b1->flag_button_pressed = 0;
			hmi->state = 1;
			queueHMI = EVENT_IDDLE;
			xQueueSend(hmiToCntrlQueue,&queueHMI,0);  //For now doesn't matter if queue its full
		}
		break;
	case 9: //Pause state
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Fill(oled, 0x00);
			Oled_Simple_Set_cursor(oled,30,5);
			Oled_Simple_Write_STRING(oled,"EN", Font_11x18,0);
			Oled_Simple_Set_cursor(oled,10,28);
			Oled_Simple_Write_STRING(oled,"PAUSA", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			oled->OLED_HALF = 2;
			Oled_Simple_Fill(oled, 0x00);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}
		//If long button press exit pause state
		if(b2->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b2->flag_long_button_pressed = 0;
			b2->flag_button_pressed = 0;
			hmi->state = 7;
		}
		else if (b1->flag_button_pressed == 1) { //Move to modify temperature menu
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_long_button_pressed = 0;
			b1->flag_button_pressed = 0;
			hmi->state = 10;
		}
	case 10: //Modify temperature
		//update menu only once
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,10,5);
			Oled_Simple_Write_STRING(oled,"MODIFICA", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			Oled_Simple_Set_cursor(oled,0,28);
			Oled_Simple_Write_STRING(oled,"TEMPERATURA", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}

		//Check for reference temperature changes
		//Read rotary cnt
		if(enc1->rotary_cnt < 0)
		{
			enc1->rotary_cnt = 0;			//Check for negative values
		}
		else if (enc1->rotary_cnt >90) {
			enc1->rotary_cnt = 90;			//max operating temperature
		}
		hmi->ref_temperature = enc1->rotary_cnt;

		//Only if temperature changes update lower part of screen
		if(hmi->ref_temperature != hmi->ref_temperature_prev)
		{
			hmi->ref_temperature_prev = hmi->ref_temperature;
			oled->OLED_HALF = 2;
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			Oled_Simple_Set_cursor(oled,55,20);
			//Add zero if number less than zero
			if(hmi->ref_temperature < 10)
			{
				Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
			}
			sprintf(tempStr, "%u", hmi->ref_temperature);
			Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
		}

		//Check if button menu was pressed to change to next menu
		if(b1->flag_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			hmi->state = 11;
			enc1->rotary_cnt = 0;
		}
		if(b2->flag_long_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b2->flag_long_button_pressed = 0;
			b2->flag_button_pressed = 0;
			hmi->state = 7;
		}
		break;

	case 11: //Change time
		//Update menu only once
		if(hmi->state != hmi->prev_state)
		{
			oled->OLED_HALF = 1;
			Oled_Simple_Set_cursor(oled,5,5);
			Oled_Simple_Write_STRING(oled,"SELECCIONE", Font_11x18,0);
			Oled_Simple_Set_cursor(oled,25,28);
			Oled_Simple_Write_STRING(oled,"TIEMPO", Font_11x18,0);
			Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			hmi->prev_state = hmi->state; //Execute only once
		}

		//Change between hours and minutes
		if(b2->flag_button_pressed == 1)
		{
			b2->flag_button_pressed = 0;

			//Menu 1 change to minutes

			if(hmi->internal_menu == 1)
			{
				enc1->rotary_cnt = hmi->ref_min;
				hmi->internal_menu = 2;
			}
			//Change to hours
			else{
				enc1->rotary_cnt = hmi->ref_hours;
				hmi->internal_menu = 1;
			}
		}

		//Check for reference temperature changes
		if(hmi->internal_menu == 1)
		{
			//Read rotary cnt
			if(enc1->rotary_cnt < 0)
			{
				enc1->rotary_cnt = 0;			//Check for negative values
			}
			else if (enc1->rotary_cnt >24) {
				enc1->rotary_cnt = 24;			//Limit hours counter
			}
			hmi->ref_hours = enc1->rotary_cnt;

			//Only if hours changes update lower part of screen
			if(hmi->ref_hours != hmi->ref_hours_prev)
			{
				hmi->ref_hours_prev = hmi->ref_hours;
				oled->OLED_HALF = 2;
				Oled_Simple_Fill(oled, 0x00); //Clean buffer
				Oled_Simple_Set_cursor(oled,30,20);
				if(hmi->ref_hours < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				sprintf(tempStr, "%u", hmi->ref_hours);
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Write_STRING(oled,":", Font_11x18,0);
				sprintf(tempStr, "%u", hmi->ref_min);
				if(hmi->ref_min < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			}

		}
		else if (hmi->internal_menu == 2) {
			//Read rotary cnt
			if(enc1->rotary_cnt < 0)
			{
				enc1->rotary_cnt = 0;			//Check for negative values
			}
			else if (enc1->rotary_cnt >60) {
				enc1->rotary_cnt = 60;			//Limit minutes counter
			}
			hmi->ref_min = enc1->rotary_cnt;

			//Only if minutes changes update lower part of screen
			if(hmi->ref_min != hmi->ref_min_prev)
			{
				hmi->ref_min_prev = hmi->ref_min;
				oled->OLED_HALF = 2;
				Oled_Simple_Fill(oled, 0x00); //Clean buffer
				Oled_Simple_Set_cursor(oled,30,20);
				if(hmi->ref_hours < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				sprintf(tempStr, "%u", hmi->ref_hours);
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Write_STRING(oled,":", Font_11x18,0);
				sprintf(tempStr, "%u", hmi->ref_min);
				if(hmi->ref_min < 10)
				{
					Oled_Simple_Write_STRING(oled,"0", Font_11x18,0);
				}
				Oled_Simple_Write_STRING(oled,tempStr, Font_11x18,0);
				Oled_Simple_Update_Half_screen(oled, oled->i2cHandle);
			}
		}


		//Check if button menu was pressed to change to next menu
		if(b2->flag_long_button_pressed == 1 || b1->flag_button_pressed == 1)
		{
			Oled_Simple_Fill(oled, 0x00); //Clean buffer
			b1->flag_button_pressed = 0;
			b2->flag_long_button_pressed = 0;
			b2->flag_button_pressed = 0;
			hmi->state = 7;
			if(!(hmi->ref_hours == 0 && hmi->ref_min == 0))
			{
				set_RTC_time(hrtc, 0, 0);
				set_alarm(hrtc, hmi->ref_hours, hmi->ref_min);
			}
		}
		break;



	default:
		break;
	}

}

void CNTRL_stMachine_init(CNTRL *cntrl_SV, GPIO_TypeDef *port_wp, GPIO_TypeDef *port_hr, GPIO_TypeDef *port_bz, uint16_t pin_wp, uint16_t pin_hr, uint16_t pin_bz)
{
	cntrl_SV->gpio_bz = port_bz;
	cntrl_SV->gpio_hr = port_hr;
	cntrl_SV->gpio_wp = port_wp;
	cntrl_SV->gpio_pin_bz = pin_bz;
	cntrl_SV->gpio_pin_wp = pin_wp;
	cntrl_SV->gpio_pin_hr = pin_hr;
	cntrl_SV->state = UNKNOWN;
	cntrl_SV->prev_state = UNKNOWN;
}

void CNTRL_stMachine(CNTRL *cntrl_SV, HMI *hmi, DS18B20_TSens *Tsen, QueueHandle_t hmiToCntrlQueue, QueueHandle_t CntrlToHmiQueue){
	//hmi->state_cntrl_sv == 1 Iddle state, 2 Pre-heating state, 3 Cooking state, 4 Alarm state

	static uint8_t hysteresis_state = 1;
	static ControlEvent eventState;
	static uint8_t flag_event = 0;

	//Check if the other task has change the event saves the value of queue

	if (xQueueReceive(hmiToCntrlQueue, &eventState, 0) == pdPASS)
	{
		flag_event = 1;
	}
	else
	{
		flag_event = 0;
	}


	switch (cntrl_SV->state)
	{
	default: //Initial state wait for HMI state machine to continue
		if(flag_event == 1)
		{
			cntrl_SV->state = eventState;
			cntrl_SV->prev_state = UNKNOWN;
		}
		break;
	case EVENT_IDDLE: //Iddle state all gpio pins low
		if(cntrl_SV->state != cntrl_SV->prev_state)
		{
			HAL_GPIO_WritePin(cntrl_SV->gpio_hr, cntrl_SV->gpio_pin_hr, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(cntrl_SV->gpio_wp, cntrl_SV->gpio_pin_wp, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(cntrl_SV->gpio_bz, cntrl_SV->gpio_pin_bz, GPIO_PIN_RESET);
			cntrl_SV->prev_state = cntrl_SV->state;
		}

		if(flag_event == 1)
		{
			cntrl_SV->prev_state = cntrl_SV->state;
			cntrl_SV->state = eventState;

		}
		break;

	case EVENT_PREHEAT: //Pre-heating sous vide
		if( Tsen->Temperature < (float)hmi->ref_temperature) //Check if temperature is lower than set point
		{
			if(HAL_GPIO_ReadPin(cntrl_SV->gpio_hr, cntrl_SV->gpio_pin_hr) == 0)
			{
				HAL_GPIO_WritePin(cntrl_SV->gpio_hr, cntrl_SV->gpio_pin_hr, GPIO_PIN_SET); //Enable heating element
				HAL_GPIO_WritePin(cntrl_SV->gpio_wp, cntrl_SV->gpio_pin_wp, GPIO_PIN_SET); //Enable recirculation
			}
		}
		else{ //Temperature reached move to alarm state and inform HMI task
			HAL_GPIO_WritePin(cntrl_SV->gpio_hr, cntrl_SV->gpio_pin_hr, GPIO_PIN_RESET); //Disable heating element
			HAL_GPIO_WritePin(cntrl_SV->gpio_wp, cntrl_SV->gpio_pin_wp, GPIO_PIN_RESET); //Disable recirculation
			cntrl_SV->prev_state = cntrl_SV->state;
			cntrl_SV ->state = EVENT_ALARM;

		}

		break;

	case EVENT_ALARM: //Alarm mode for cooking or preheating ended
		if(cntrl_SV->state != cntrl_SV->prev_state)
		{
			HAL_GPIO_WritePin(cntrl_SV->gpio_bz, cntrl_SV->gpio_pin_bz, GPIO_PIN_SET);
			HAL_GPIO_WritePin(cntrl_SV->gpio_wp, cntrl_SV->gpio_pin_wp, GPIO_PIN_RESET); //Disable recirculation
			cntrl_SV->prev_state = 3;
		}
		else{
			//Toggle alarm
			HAL_GPIO_TogglePin(cntrl_SV->gpio_bz, cntrl_SV->gpio_pin_bz);
		}
		//En este estado hay que enviar mensaje a otro HMI, poner una temporizacion

		if(flag_event == 1)
		{
			cntrl_SV->prev_state = cntrl_SV->state;
			cntrl_SV->state = eventState;

		}
		break;
	case EVENT_COOK: //Cooking state
		if(cntrl_SV->state != cntrl_SV->prev_state)
		{
			HAL_GPIO_WritePin(cntrl_SV->gpio_wp, cntrl_SV->gpio_pin_wp, GPIO_PIN_SET); //Enable recirculation
			cntrl_SV->prev_state = cntrl_SV->state;
		}

		if(hysteresis_state == 1) //On-off control with hysteresis
		{
			if(Tsen->Temperature >= ((float)hmi->ref_temperature + 2.0)) //Check if temperature is lower than set point
			{
				HAL_GPIO_WritePin(cntrl_SV->gpio_hr, cntrl_SV->gpio_pin_hr, GPIO_PIN_RESET);
				hysteresis_state = 2;
			}
			else{
				HAL_GPIO_WritePin(cntrl_SV->gpio_hr, cntrl_SV->gpio_pin_hr, GPIO_PIN_SET);
			}
		}

		else if (hysteresis_state == 2)
		{
			if(Tsen->Temperature < ((float)hmi->ref_temperature - 2.0)) //Check if temperature is lower than set point
			{
				hysteresis_state = 1;

			}
		}

		//En este proceso tambien hay que poner la interrupccion de tiempo y hay que comunicarse con hmi
		//if(hmi->state_cntrl_sv == 1) //If process has been paused go to iddle
		if(flag_event == 1)
		{
			cntrl_SV->prev_state = cntrl_SV->state;
			cntrl_SV->state = eventState;

		}

		break;

	}
}

void set_RTC_time (RTC_HandleTypeDef *hrtc, uint8_t hr, uint8_t min)
{
	RTC_TimeTypeDef sTime = {0};
	sTime.Hours = hr;
	sTime.Minutes = min;
	sTime.Seconds = 0;
	if (HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		//Todo
	}
}

void set_alarm (RTC_HandleTypeDef *hrtc, uint8_t hr, uint8_t min)
{
	RTC_AlarmTypeDef sAlarm = {0};
	sAlarm.AlarmTime.Hours = hr;
	sAlarm.AlarmTime.Minutes = min;
	sAlarm.AlarmTime.Seconds = 0;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		//Todo
	}
}
