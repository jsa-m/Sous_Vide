/*
 * DS18B20_1Wire.c
 *
 *  Created on: Aug 16, 2024
 *      Author: joaso
 */

#include "DS18B20_1Wire.h"

//Function to change baud rate, needed to send reset state line needs to be low for more time than normal operation
uint8_t uart_change_baud_rate(UART_HandleTypeDef *uart_handle, uint32_t baud_rate)
{
	uart_handle->Init.BaudRate = baud_rate;
	if(HAL_HalfDuplex_Init(uart_handle) != HAL_OK)
	{
		return 1;
	}
	return 0;
}

void DS18B20_1Wire_Init(DS18B20_TSens *Tsensor)
{
	uint8_t data = 0xF0;
	uart_change_baud_rate(Tsensor->uart_handle, 9600);
	HAL_UART_Transmit(Tsensor->uart_handle, &data, 1, 10);
	if(HAL_UART_Receive(Tsensor->uart_handle, &data, 1, 10)!= HAL_OK) //Failed check connection
	{
		Tsensor->presence = -1;
	}
	uart_change_baud_rate(Tsensor->uart_handle, 115200);
	if(data == 0xf0)
		{
		Tsensor->presence = -2;
		}
	Tsensor->presence = 1;	//Connection Ok
}

void DS18B20_1Wire_Write(DS18B20_TSens *Tsensor, uint8_t command)
{
	uint8_t buffer[8];
	for (int i = 0; i < 8; ++i) {
		if ((command & (1<<i)) != 0)
		{
			buffer[i] = 0xFF;

		} else {
			buffer[i] = 0x00;

		}

	}

	HAL_UART_Transmit(Tsensor->uart_handle, buffer, 8, 10);
}

void DS18B20_1Wire_ReadTemp(DS18B20_TSens *Tsensor)
{
	uint8_t ReadBitCMD = 0xFF;
	uint8_t RxBit;
	uint8_t RxByte = 0;
	uint16_t Temperature = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		RxByte >>= 1; //Shifts RxByte 1 position to let room to the next byte
		// Send Read Bit CMD
		HAL_UART_Transmit(Tsensor->uart_handle, &ReadBitCMD, 1, 1);
		HAL_UART_Receive(Tsensor->uart_handle, &RxBit, 1, 1);
		RxBit = RxBit & 0x01; //Obtain only the less significant
		if (RxBit == 1)
		{
			RxByte |= 0x80; //sets the highest bit to 1 if the read bit was 1
		}
	}

	if(Tsensor->pos == 1) //LSB
	{
		Tsensor->TempLSB = RxByte;
	}
	else if (Tsensor->pos == 2) {
		Tsensor->TempMSB = RxByte;

	}

	if(Tsensor->pos == 1)
	{

		Tsensor->pos = 2;
	}
	else if (Tsensor->pos == 2) {
		Tsensor->pos = 1;
		//Extract temperature
		Temperature = (Tsensor->TempMSB << 8) | Tsensor->TempLSB;
		Tsensor->Temperature = (float)Temperature/16.0; //resolution is 0.0625

	}
}

float DS18B20_1Wire_ExtractTemp(DS18B20_TSens *Tsensor)
{
	return Tsensor->Temperature;

}






