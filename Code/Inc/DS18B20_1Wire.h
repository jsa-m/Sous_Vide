/*
 * DS18B20_1Wire.h
 *
 *  Created on: Aug 16, 2024
 *      Author: joaso
 */

#include "stm32f1xx_hal.h"

typedef struct{
	UART_HandleTypeDef *uart_handle;
	int8_t presence, pos;
	uint8_t TempLSB, TempMSB;
	float Temperature;

}DS18B20_TSens;

uint8_t uart_change_baud_rate(UART_HandleTypeDef *uart_handle, uint32_t baud_rate);
void DS18B20_1Wire_Init(DS18B20_TSens *Tsensor);
void DS18B20_1Wire_Write(DS18B20_TSens *Tsensor, uint8_t command);
void DS18B20_1Wire_ReadTemp(DS18B20_TSens *Tsensor);
float DS18B20_1Wire_ExtractTemp(DS18B20_TSens *Tsensor);
