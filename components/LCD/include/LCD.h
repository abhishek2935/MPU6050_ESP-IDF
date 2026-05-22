#include "i2c_bus.h"

#define LCD_ADDER 0x27 

void lcd_i2c_init(void);
void lcd_init(void) ; 
void lcd_cmd(uint8_t cmd);
void lcd_print(char *str);

void task_lcd(void *param);