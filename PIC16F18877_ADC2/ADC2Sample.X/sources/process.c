#include "common.h"
#include "device.h"
#include "i2c_lcd.h"
#include "process.h"

// 約 1.0 秒ごとに処理
void process_on_one_second()
{
    // for test: ヘルスチェックLED
    RA1 = ~RA1;
}

// 初期処理
void process_init()
{
    i2c_lcd_init(); 

    // for testing I2C LCD
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F18877     ");

    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string("      2017/04/05");
}
