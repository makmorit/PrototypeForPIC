#ifndef _I2C_RTCC_H_
#define _I2C_RTCC_H_

// RTC-8564NB RTCCモジュールのアドレス
#define RTC_8564NB_I2C_ADDR 0b1010001

// 現在時刻
unsigned char rtcc_years;
unsigned char rtcc_months;
unsigned char rtcc_days;
unsigned char rtcc_weekdays;
unsigned char rtcc_hours;
unsigned char rtcc_minutes;
unsigned char rtcc_seconds;

int i2c_rtcc_init();
void i2c_rtcc_read_time();
void i2c_rtcc_set_time();

#endif // _I2C_RTCC_H_
