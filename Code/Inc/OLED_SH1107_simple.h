#include "stm32f1xx_hal.h"
#include "string.h"
#include "ssd1306_fonts.h" //de momento para enviar datos por I2C default sin DMA

#define COMMS_TIMEOUT 1 //1 ms I2C timeout for command
#define COMMS_TIMEOUT_DATA 10 //10 ms I2C timeout for data (128bytes)

#define OLED_WIDTH 128
#define OLED_HEIGHT 128/2
#define OLED_BUFFER_SIZE OLED_WIDTH*OLED_HEIGHT //Set oled height and width
#define OLED_OFFSETX_LOW 0
#define OLED_OFFSETX_UPP 0
#define OLED_I2C_ADDR    (0x3C << 1)

//Commands
#define DISPLAY_ON 			0xAF
#define DISPLAY_OFF 		0xAE
#define MEM_ADD_MODE 		0x20
#define HOR_ADD_MODE 		0x00
#define VER_ADD_MODE 		0x02
#define PAG_ADD_MODE 		0x03
#define PAGE_STRT_ADD 		0xB0
#define COM_STRT_DIR 		0xC8
#define LOW_COL_ADD 		0x00
#define HIGH_COL_ADD 		0x10
#define STRT_LINE_ADD 		0x40
#define CONTRAST_CNTR_REG 	0x81
#define CONTRAST_VAL		0xFF //100%
#define SEG_REMAP			0xA1
#define NORM_COLOR			0xA6
#define INV_COLOR			0xA7
#define MUL_RATIO_128	    0xFF
#define MUL_RATIO_64	    0x3F
#define MUL_RATIO_32	    0x1F
#define FOLLOW_RAM_CONTENT  0xA4
#define IGNORE_RAM_CONTENT  0xA5
#define DISPLAY_OFFSET  	0xD3
#define NO_OFFSET  		    0x00
#define OSC_FREQ  		    0xD5
#define OSC_DIV_RATIO		0xF0
#define PRE_CHARGE_PERIOD1  0xD9
#define PRE_CHARGE_PERIOD2  0x22
#define COM_PINS		    0xDA
#define COM_PINS_128		0x12
#define COM_PINS_64  		0x12
#define COM_PINS_32 		0x02
#define VOLT_COM	 		0xDB
#define VOLT_VAL	 		0x20 //0.77*VCC = 3.85V
#define DC_EN  		 		0x8D
#define DC_EN_VAL  	 		0x14
#define BASE_RAM_ADD		0xB0


typedef struct{
	uint8_t OLED_BUFFER[OLED_BUFFER_SIZE];
	uint16_t OLED_CurrentX;
	uint16_t OLED_CurrentY;
	uint8_t OLED_INIT;
	uint8_t OLED_STATE;
	uint8_t OLED_HALF;
	I2C_HandleTypeDef *i2cHandle;

}OLED_SIMPLE;


void Oled_Simple_Init(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle);
void Oled_Simple_Fill(OLED_SIMPLE *oled, uint8_t color);
void Oled_Simple_Update_Screen(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle);
void Oled_Simple_Update_Half_screen(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle);
void Oled_Simple_Draw_Pixel(OLED_SIMPLE *oled, uint8_t color, uint16_t offset_x, uint16_t offset_y);
char Oled_Simple_Write_CHAR(OLED_SIMPLE *oled, uint8_t color, char ch, FontDef Font);
char Oled_Simple_Write_STRING(OLED_SIMPLE *oled, char *str, FontDef Font, uint8_t color);
void Oled_Simple_Set_cursor(OLED_SIMPLE *oled, uint16_t cursor_x, uint16_t cursor_y);
uint8_t Oled_simple_send_Command(I2C_HandleTypeDef *i2c_handle, uint8_t command);
uint8_t Oled_simple_send_Data(OLED_SIMPLE *oled, I2C_HandleTypeDef *i2c_handle, uint8_t RAM_PAGE);


//DMA para actualizar string
