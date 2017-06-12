#ifndef _I2C_LCD_H_
#define _I2C_LCD_H_

// ST7032i LCDモジュールのアドレス
#define ST7032_I2C_ADDR 0x3E

void i2c_lcd_register_char(int p, char *dt);
void i2c_lcd_clear_display(void);
void i2c_lcd_set_cursor(int row, int col);
void i2c_lcd_put_string(const char * s);
void i2c_lcd_init();

#endif
