#include "common.h"
#include "i2c.h"
#include "i2c_rtcc.h"

unsigned int bcd2bin(unsigned char dt)
{
    // dt の上位４ビット／下位４ビットを
    // それぞれに１０進数の桁に格納する
    unsigned char number = ((dt >> 4) * 10) + (dt & 0xf);
    return (unsigned int)number;
}

unsigned char bin2bcd(unsigned int num) 
{
    // num の下２桁を
    // それぞれ上位４ビット／下位４ビットに格納する
    unsigned int number = (((num % 100) / 10) << 4) | (num % 10);
    return (unsigned char)number;
}

void i2c_rtcc_init()
{
    unsigned char reg = 0;
    int ack;

    // RTCC使用可能フラグを初期化
    rtcc_available = 0;
    __delay_ms(1000);

    // VLビットを読み出すために、
    // アドレス0x02のSecondsレジスターを読込む
    ack = i2c_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
    if (ack == 0) {
        i2c_send_byte(0x02);
        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_1);
        reg = i2c_receive_byte(NOACK);

    } else {
        i2c_stop_condition();
        return;
    }

    // VLビットが1の場合は全データの初期化を行う
    if (reg & 0x80) {
        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
        // Control 1, STOP=1（計時停止）
        i2c_send_byte(0x00);
        i2c_send_byte(0x20);
        i2c_send_byte(0x11);

        // 初めての起動時は、2017/1/1 0:00:00 で
        // 時刻を初期化します
        rtcc_years = 17;
        rtcc_months = 1;
        rtcc_days = 1;
        rtcc_weekdays = 0;
        rtcc_hours = 0;
        rtcc_minutes = 0;
        rtcc_seconds = 0;

        // Address=02-08: 時計・カレンダー
        //  weekdays: 0=日, 1=月.., 6=土
        i2c_send_byte(bin2bcd(rtcc_seconds));
        i2c_send_byte(bin2bcd(rtcc_minutes));
        i2c_send_byte(bin2bcd(rtcc_hours));
        i2c_send_byte(bin2bcd(rtcc_days));
        i2c_send_byte(bin2bcd(rtcc_weekdays));
        i2c_send_byte(bin2bcd(rtcc_months));
        i2c_send_byte(bin2bcd(rtcc_years));

        // Address=09-0C: アラーム
        //  アラームは発生しない
        i2c_send_byte(0x80);
        i2c_send_byte(0x80);
        i2c_send_byte(0x80);
        i2c_send_byte(0x80);

        // Address=0D-0F: CLKOUT周波数・タイマー
        //  CLKOUT=1Hz出力、タイマー＝使用しない
        i2c_send_byte(0x83);
        i2c_send_byte(0x00);
        i2c_send_byte(0x00);

        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
        // Control 1, STOP=0（計時開始）
        i2c_send_byte(0x00);
        i2c_send_byte(0x00);
        i2c_stop_condition();

        __delay_ms(1000);

    } else {
        i2c_stop_condition();
    }

    // RTCC使用可能フラグを設定
    rtcc_available = 1;
}

void i2c_rtcc_read_time()
{
    int ack;

    ack = i2c_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
    if (ack == 0) {
        i2c_send_byte(0x02);
        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_1);

        rtcc_seconds = bcd2bin(i2c_receive_byte(ACK) & 0x7f);
        rtcc_minutes = bcd2bin(i2c_receive_byte(ACK) & 0x7f);
        rtcc_hours = bcd2bin(i2c_receive_byte(ACK) & 0x3f);
        rtcc_days = bcd2bin(i2c_receive_byte(ACK) & 0x3f);
        rtcc_weekdays = i2c_receive_byte(ACK) & 0x07;
        rtcc_months = bcd2bin(i2c_receive_byte(ACK) & 0x1f);
        rtcc_years = bcd2bin(i2c_receive_byte(NOACK));
    }
    i2c_stop_condition();
}

void i2c_rtcc_set_time()
{
    int ack ;

    ack = i2c_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
    if (ack == 0) {
        // Control 1, STOP=1（計時停止）
        i2c_send_byte(0x00);
        i2c_send_byte(0x20);

        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
        // Address=02-05: 時計・カレンダー
        i2c_send_byte(0x02);
        i2c_send_byte((char)bin2bcd(rtcc_seconds));
        i2c_send_byte((char)bin2bcd(rtcc_minutes));
        i2c_send_byte((char)bin2bcd(rtcc_hours));
        i2c_send_byte((char)bin2bcd(rtcc_days));

        // Address=07-08: 時計・カレンダー
        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
        i2c_send_byte(0x07);
        i2c_send_byte((char)bin2bcd(rtcc_months));
        i2c_send_byte((char)bin2bcd(rtcc_years));

        i2c_repeated_start_condition(RTC_8564NB_I2C_ADDR, RW_0);
        // Control 1, STOP=0（計時開始）
        i2c_send_byte(0x00);
        i2c_send_byte(0x00);
        i2c_stop_condition();

        __delay_ms(1000);

    } else {
        i2c_stop_condition();
    }
}

char timestamp_str[20];
char *get_timestamp_str()
{
    // 現在時刻を取得し文字列を編集 2017/05/28 11:45:30 形式
    sprintf(timestamp_str, "20%02d/%02d/%02d %02d:%02d:%02d", 
            rtcc_years, rtcc_months, rtcc_days,
            rtcc_hours, rtcc_minutes, rtcc_seconds);

    return timestamp_str;
}
