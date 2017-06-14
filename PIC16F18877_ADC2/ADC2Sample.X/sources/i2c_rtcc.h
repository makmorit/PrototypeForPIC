#ifndef _I2C_RTCC_H_
#define _I2C_RTCC_H_

// RTC-8564NB RTCCモジュールのアドレス
#define RTC_8564NB_I2C_ADDR 0b1010001

// 使用可能フラグ 1:使用可能 0:使用不可
unsigned char rtcc_available;

// 現在時刻
unsigned char rtcc_years;
unsigned char rtcc_months;
unsigned char rtcc_days;
unsigned char rtcc_weekdays;
unsigned char rtcc_hours;
unsigned char rtcc_minutes;
unsigned char rtcc_seconds;

void i2c_rtcc_init();
void i2c_rtcc_read_time();
void i2c_rtcc_set_time();
char *get_timestamp_str();

#endif // _I2C_RTCC_H_
