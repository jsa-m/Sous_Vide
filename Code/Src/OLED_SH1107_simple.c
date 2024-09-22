/*
 * OLED_SH1107_simple.c
 *
 *  Created on: Aug 12, 2024
 *      Author: joaso
 */

#include "OLED_SH1107_simple.h"

void Oled_Simple_Init(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle){

	oled->OLED_STATE = 0b0;
	Oled_simple_send_Command(i2c_handle, DISPLAY_OFF);       //Set Display OFF
	Oled_simple_send_Command(i2c_handle, MEM_ADD_MODE);      //Set Address mode
	Oled_simple_send_Command(i2c_handle, HOR_ADD_MODE);      //Horizontal Address mode
	Oled_simple_send_Command(i2c_handle, PAGE_STRT_ADD);     //Set Page Start Address
	Oled_simple_send_Command(i2c_handle, COM_STRT_DIR);      //Set COM Output Scan Direction
	Oled_simple_send_Command(i2c_handle, LOW_COL_ADD);       //Set Low Column Address
	Oled_simple_send_Command(i2c_handle, HIGH_COL_ADD);      //Set High Column Address
	Oled_simple_send_Command(i2c_handle, STRT_LINE_ADD); 	 //Set Start Line Address
	Oled_simple_send_Command(i2c_handle, CONTRAST_CNTR_REG); //Access Contrast Control Register
	Oled_simple_send_Command(i2c_handle, CONTRAST_VAL);      //Set Contrast Value
	Oled_simple_send_Command(i2c_handle, SEG_REMAP);         //Segment re-map 0 to 127
	Oled_simple_send_Command(i2c_handle, NORM_COLOR);        //Set normal color
	Oled_simple_send_Command(i2c_handle, MUL_RATIO_128);	 //Set multiplex ratio
	Oled_simple_send_Command(i2c_handle, FOLLOW_RAM_CONTENT);//Follow RAM content
	Oled_simple_send_Command(i2c_handle, DISPLAY_OFFSET);    //Set display offset
	Oled_simple_send_Command(i2c_handle, NO_OFFSET);         //Set no offset
	Oled_simple_send_Command(i2c_handle, OSC_FREQ);          //Set display clock divide ratio/oscillator frequency
	Oled_simple_send_Command(i2c_handle, OSC_DIV_RATIO);     //Set divide ratio
	Oled_simple_send_Command(i2c_handle, PRE_CHARGE_PERIOD1);//Set pre-charge period
	Oled_simple_send_Command(i2c_handle, PRE_CHARGE_PERIOD2);
	Oled_simple_send_Command(i2c_handle, COM_PINS);          //Set com pins hardware configuration
	Oled_simple_send_Command(i2c_handle, VOLT_COM);          //Set Vcomh
	Oled_simple_send_Command(i2c_handle, VOLT_VAL);          //0.77xVcc
	Oled_simple_send_Command(i2c_handle, DC_EN);             //Set DC-DC enable
	Oled_simple_send_Command(i2c_handle, DC_EN_VAL);
	Oled_simple_send_Command(i2c_handle, DISPLAY_ON);        //Set Display ON
	oled->OLED_STATE = 0b1;
	oled->OLED_HALF = 1; 									 //First half of screen
}

//Fill buffer with 0xFF (screen white) or with 0x00 (screen black)
void Oled_Simple_Fill(OLED_SIMPLE *oled, uint8_t color)
{
	memset(oled->OLED_BUFFER, (color == 0xFF) ? 0xFF : 0x00, sizeof(oled->OLED_BUFFER));
}

//Update display writing the contents of the buffer to RAM
void Oled_Simple_Update_Screen(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle)
{
	// Write data to each page of RAM. Number of pages
	// depends on the screen height:
	//  * 32px   ==  4 pages
	//  * 64px   ==  8 pages
	//  * 128px  ==  16 pages

	for(uint8_t i = 0; i < (OLED_HEIGHT*2)/8; i++) {	//Workaround to use half of RAM
		Oled_simple_send_Command(i2c_handle, BASE_RAM_ADD+i);                 //Set Display ON
		Oled_simple_send_Command(i2c_handle, OLED_OFFSETX_LOW + 0x00);        //Set Display ON
		Oled_simple_send_Command(i2c_handle, OLED_OFFSETX_UPP + 0x10);        //Set Display ON
		Oled_simple_send_Data(oled, i2c_handle,i);
	}
}

//Update display writing the contents of the buffer to RAM
void Oled_Simple_Update_Half_screen(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle)
{
	// Write data to each page of RAM. Number of pages
	// depends on the screen height:
	//  * 32px   ==  4 pages
	//  * 64px   ==  8 pages
	//  * 128px  ==  16 pages

	uint8_t j,condition;
	if(oled->OLED_HALF == 1)
	{
		j=0;
		condition = (OLED_HEIGHT)/8;
	}
	else{
		j = 8;
		condition = (OLED_HEIGHT*2)/8;
	}

	for(uint8_t i = j; i < condition; i++) {	  //Workaround to use half of RAM
		Oled_simple_send_Command(i2c_handle, BASE_RAM_ADD+i);                 //Set Display ON
		Oled_simple_send_Command(i2c_handle, OLED_OFFSETX_LOW + 0x00);        //Set Display ON
		Oled_simple_send_Command(i2c_handle, OLED_OFFSETX_UPP + 0x10);        //Set Display ON
		Oled_simple_send_Data(oled, i2c_handle,i);
	}
}

//Base function to write characters, color == 0 write white, else black
void Oled_Simple_Draw_Pixel(OLED_SIMPLE *oled, uint8_t color, uint16_t offset_x, uint16_t offset_y)
{
	uint16_t pos_x = oled->OLED_CurrentX + offset_x;
	uint16_t pos_y = oled->OLED_CurrentY + offset_y;
    if(pos_x >= OLED_WIDTH || pos_y >= OLED_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    // Draw in the right color
    if(color == 0) {
        oled->OLED_BUFFER[pos_x + (pos_y / 8) * OLED_WIDTH] |= 1 << (pos_y % 8);
    } else {
    	oled->OLED_BUFFER[pos_x + (pos_y / 8) * OLED_WIDTH] &= ~(1 << (pos_y % 8));
    }
}

//Write character with specified font size from library
char Oled_Simple_Write_CHAR(OLED_SIMPLE *oled, uint8_t color, char ch, FontDef Font)
{
	uint32_t i, b, j;

	// Check if character is valid
	if (ch < 32 || ch > 126)
		return 0;

	// Check remaining space on current line
	if (OLED_WIDTH < (oled->OLED_CurrentX + Font.FontWidth) ||
			OLED_HEIGHT < (oled->OLED_CurrentY + Font.FontHeight))
	{
		// Not enough space on current line
		return 0;
	}
	// Use the font to write
	for(i = 0; i < Font.FontHeight; i++) {
		b = Font.data[(ch - 32) * Font.FontHeight + i];
		for(j = 0; j < Font.FontWidth; j++) {
			if((b << j) & 0x8000)  {
				Oled_Simple_Draw_Pixel(oled, color,j,i);//Check como se llama a la funcion!!!!
			} else {
				Oled_Simple_Draw_Pixel(oled, !color,j,i);
			}
		}
	}
	// The current space is now taken
	oled->OLED_CurrentX += Font.FontWidth;

	// Return written char for validation
	return ch;
}

char Oled_Simple_Write_STRING(OLED_SIMPLE *oled, char *str, FontDef Font, uint8_t color)
{
	while (*str) {
		if (Oled_Simple_Write_CHAR(oled, color, *str, Font) != *str)
		{
			// Char could not be written
			return *str;
		}
		str++;
	}

	// Everything ok
	return *str;
}

void Oled_Simple_Set_cursor(OLED_SIMPLE *oled, uint16_t cursor_x, uint16_t cursor_y)
{
	oled->OLED_CurrentX = cursor_x;
	oled->OLED_CurrentY = cursor_y;
}

// Send a byte to the command register
//todo Implement errors
uint8_t Oled_simple_send_Command(I2C_HandleTypeDef *i2c_handle, uint8_t command)
{
	if(HAL_I2C_Mem_Write(i2c_handle, OLED_I2C_ADDR, 0x00, 1, &command, 1, COMMS_TIMEOUT) == HAL_OK) //Wait max 1ms
	{
		return 0;
	}
	else{
		return 1;
	}
}

// Send data to Oled with dma
//todo Implement errors
uint8_t Oled_simple_send_Data(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle, uint8_t RAM_PAGE)
{
	//Check if we are in first half or second half of screen
	if(RAM_PAGE > 7)
	{
		oled->OLED_HALF = 2;
		if(HAL_I2C_Mem_Write(i2c_handle, OLED_I2C_ADDR, 0x40, 1, &oled->OLED_BUFFER[OLED_WIDTH*(RAM_PAGE-7)], OLED_WIDTH, COMMS_TIMEOUT*20) == HAL_OK) //Wait max 1ms
		{
			return 0;
		}
		else{
			return 1;
		}
	}
	else{
		oled->OLED_HALF = 1;
		if(HAL_I2C_Mem_Write(i2c_handle, OLED_I2C_ADDR, 0x40, 1, &oled->OLED_BUFFER[OLED_WIDTH*RAM_PAGE], OLED_WIDTH, COMMS_TIMEOUT*20) == HAL_OK) //todo ojo con estos tiempos
		{
			return 0;
		}
		else{
			return 1;
		}

	}

}


