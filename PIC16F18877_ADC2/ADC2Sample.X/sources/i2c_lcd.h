#ifndef _I2C_LCD_H_
#define _I2C_LCD_H_

// ST7032i LCDモジュールのアドレス
#define ST7032_I2C_ADDR 0x3E

// ST7032i アイコン用アドレス
#define LCD_ICON_ANTENNA           0x4010
#define LCD_ICON_PHONE             0x4210
#define LCD_ICON_COMMUNICATION     0x4410
#define LCD_ICON_INCOMING          0x4610
#define LCD_ICON_UP                0x4710
#define LCD_ICON_DOWN              0x4708
#define LCD_ICON_LOCK              0x4910
#define LCD_ICON_KINSHI            0x4B10
#define LCD_ICON_BATTERY_LOW       0x4D10
#define LCD_ICON_BATTERY_HALF      0x4D08
#define LCD_ICON_BATTERY_FULL      0x4D04
#define LCD_ICON_BATTERY_EMPTY     0x4D02
#define LCD_ICON_HANAMARU          0x4F10

#define LCD_ICON_ON  1
#define LCD_ICON_OFF 0

void i2c_lcd_clear_display(void) ;
void i2c_lcd_set_cursor(int row, int col) ;
void i2c_lcd_put_string(const char * s) ;
void i2c_lcd_init() ;
void i2c_lcd_icon_disp_area_clear(void) ;
void i2c_lcd_display_icon(int flag, unsigned int dt) ;

#endif
