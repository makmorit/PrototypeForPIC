#include "common.h"
#include "i2c.h"
#include "STTS751.h"

void STTS751_init()
{
    int  ans;

    // Configuration register
    ans = i2c_start_condition(STTS751_I2C_ADDR, RW_0);
    if (ans == 0) {
        i2c_send_byte(0x03);
        i2c_send_byte(0b10001100);
    }
    i2c_stop_condition();
    __delay_us(25);

    // conversation rate
    ans = i2c_start_condition(STTS751_I2C_ADDR, RW_0);
    if (ans == 0) {
        i2c_send_byte(0x04);
        i2c_send_byte(5);
    }
    i2c_stop_condition();
    __delay_us(25);
}

unsigned char STTS751_get_value()
{
    int  ans;
    unsigned char value = 30;

    RC3=1;
    // Temperature value high byte
    ans = i2c_start_condition(STTS751_I2C_ADDR, RW_0);
    if (ans == 0) {
        i2c_send_byte(0);
        i2c_start_condition(STTS751_I2C_ADDR, RW_1);
        value = i2c_receive_byte(NOACK);
    }
    i2c_stop_condition();
    __delay_us(25);

    return value;
}
