/*
 * Sous_Vide.h
 *
 *  Created on: Aug 17, 2024
 *      Author: joaso
 */

#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "OLED_SH1107_simple.h"
#include "Switches_debounced.h"
#include "DS18B20_1Wire.h"

typedef struct{
	//State machine variables
	uint8_t state;
	uint8_t prev_state;
	//Variables to operate the control state machine
	uint8_t state_cntrl_sv;
	//HMI stored variables
	uint8_t ref_temperature;
	uint8_t ref_temperature_prev;
	uint8_t ref_hours;
	uint8_t ref_hours_prev;
	uint8_t ref_min;
	uint8_t ref_min_prev;
	uint8_t internal_menu;
	float last_temp;
	uint8_t timer_hours;
	uint8_t timer_hours_prev;
	uint8_t timer_min;
	uint8_t timer_min_prev;
	uint8_t flag_alarm_rtc;
}HMI;

//Event queue to manage transitions between task
typedef enum{
	EVENT_IDDLE,
	EVENT_PREHEAT,
	EVENT_COOK,
	EVENT_ALARM,
	UNKNOWN,
}ControlEvent;

typedef enum{
	EVENT_FAULT,
	EVENT_PREHEAT_FINISHED,
}Control2HMIEvent;

typedef struct{
	//State machine variables
	ControlEvent state;
	ControlEvent prev_state;
	//Physical button
	GPIO_TypeDef *gpio_wp; //Water pump port
	uint16_t gpio_pin_wp;  //Water pump pin
	GPIO_TypeDef *gpio_hr; //Heat relay port
	uint16_t gpio_pin_hr;  //Heat relay pin
	GPIO_TypeDef *gpio_bz; //Buzzer port
	uint16_t gpio_pin_bz;  //Buzzer pin
}CNTRL;

void HMI_stMachine(HMI *hmi, OLED_SIMPLE *oled, Button *b1, Button *b2, Rotary_enc *enc1, Rotary_enc *enc2, DS18B20_TSens *Tsen, QueueHandle_t hmiToCntrlQueue, QueueHandle_t CntrlToHmiQueue, RTC_HandleTypeDef *hrtc);
void CNTRL_stMachine_init(CNTRL *cntrl_SV, GPIO_TypeDef *port_wp, GPIO_TypeDef *port_hr, GPIO_TypeDef *port_bz, uint16_t pin_wp, uint16_t pin_hr, uint16_t pin_bz);
void CNTRL_stMachine(CNTRL *cntrl_SV, HMI *hmi, DS18B20_TSens *Tsen, QueueHandle_t hmiToCntrlQueue, QueueHandle_t CntrlToHmiQueue);
void set_RTC_time (RTC_HandleTypeDef *hrtc, uint8_t hr, uint8_t min);
void set_alarm (RTC_HandleTypeDef *hrtc, uint8_t hr, uint8_t min);
