#include "common.h"
#include "device.h"
#include "i2c_lcd.h"
#include "process.h"
#include "uart.h"
#include "timer0.h"
#include "STTS751.h"

#include "i2c_rtcc.h"

// １秒間当たりの割込み回数（1.024ms × 977回）
#define INT_PER_SEC 977

// ボタン押下連続検知抑止カウンター（1.024ms × 488 = 約0.5秒）
#define BTN_PUSH_PREVENT_CNT 488

// カウントダウン秒数（180秒）
#define COUNT_DOWN_SEC 180

// カウンターとして使用する変数
static unsigned long user_sec_count;
static unsigned long cnt_int_per_sec;

//
// STTS751から気温計測値を取得
//
static unsigned char stts751_value;
static unsigned char stts751_decimals;
static void get_stts751_temperature()
{
    unsigned char dd;

    // STTS751の計測値を取得
    stts751_value = STTS751_get_value();
    dd = STTS751_get_decimals();

    // 小数点部を、表示できる値に変換
    // (取得した値を四捨五入します)
    stts751_decimals = (dd * 10 + 8) / 16;
}

// UARTに現在時刻と気温を出力
static void print_time_and_temperature()
{
    printf("Current time: 20%02d/%02d/%02d %02d:%02d:%02d, Temperature: %1d.%1d\r\n", 
            rtcc_years, rtcc_months, rtcc_days,
            rtcc_hours, rtcc_minutes, rtcc_seconds,
            stts751_value, stts751_decimals);
}

// 時刻アジャスト
static unsigned char uart_input_buff[32];
static void rtcc_adjust_from_uart()
{
    // セットしたい時刻をUARTから入力させる
    printf("Please input current time:\r\n");
    printf(">");
    memset(uart_input_buff, 0, sizeof(uart_input_buff));
    gets(uart_input_buff);
    printf("\r\n");
    
    // 入力文字列が不正な場合は終了
    if (strlen(uart_input_buff) != 19) {
        printf("Cannot adjust: Invalid format [%s]\r\n", uart_input_buff);
        return;
    }
    
    // uart_input_buffにおける、時刻各要素の添え字
    //   2017/05/30 10:00:00 形式で入力した場合、
    //   年=2-3 月=5-6 日=8-9 時=11-12 分=14-15 秒=17-18
    //   それぞれの要素の境界に NULL 文字を入れて
    //   トークン化しておきます
    uart_input_buff[4] = 0;
    uart_input_buff[7] = 0;
    uart_input_buff[10] = 0;
    uart_input_buff[13] = 0;
    uart_input_buff[16] = 0;
    rtcc_years = atoi(uart_input_buff + 2);
    rtcc_months = atoi(uart_input_buff + 5);
    rtcc_days = atoi(uart_input_buff + 8);
    rtcc_hours = atoi(uart_input_buff + 11);
    rtcc_minutes = atoi(uart_input_buff + 14);
    rtcc_seconds = atoi(uart_input_buff + 17);
    
    // 時刻を合わせる
    i2c_rtcc_set_time();
    printf("Current time adjusted: 20%02d-%02d-%02d %02d:%02d:%02d\r\n", 
            rtcc_years, rtcc_months, rtcc_days,
            rtcc_hours, rtcc_minutes, rtcc_seconds);
}

//
// UARTに入力された内容を解析する
//
static void parse_uart_input()
{
    unsigned char *rc_buff = get_uart_recv_buff();
    if (rc_buff == NULL) {
        return;
    }
    if (rc_buff[0] == 'A') {
        // 'A' が入力された場合は
        // 時刻アジャスト処理に入る
        rtcc_adjust_from_uart();
    }
}


//
// ボタン押下検知処理
//
static unsigned long btn_push_prevent_cnt;

// 基板上のボタン押下時の処理
static int process_on_button_press()
{
    int ret = 1;

    // スイッチOnに対する処理を実行
    if (BUTTON_0 == 0) { // プルアップされているので Low 判定
        //
        // 表示用秒数のカウントダウンをスタートさせる
        //
        user_sec_count = COUNT_DOWN_SEC;
        cnt_int_per_sec = INT_PER_SEC;

        // UARTに現在時刻と気温を出力
        print_time_and_temperature();

    } else {
        ret = 0;
    }
    return ret;
}

// 割込みごとに処理（1.024 ms）
static void switch_prevent()
{
    // カウンターが０の時は終了
    if (0 == btn_push_prevent_cnt) {
        return;
    }

    // ボタン連続押下抑止カウンターを更新
    btn_push_prevent_cnt-- ;
}

// イベントループ内の最後のステップで処理
static void switch_detection()
{
    // カウンターが０でない時は終了
    if (0 < btn_push_prevent_cnt) {
        return;
    }

    // スイッチ押下時の処理を実行
    if (process_on_button_press() != 0) {
        // 押下抑止カウンターを設定
        btn_push_prevent_cnt = BTN_PUSH_PREVENT_CNT;
    } else {
        btn_push_prevent_cnt = 0;
    }
}

//
// 初期化処理
//
void process_init()
{
    // ローカル変数の初期化
    btn_push_prevent_cnt = 0;

    // STTS751 初期化
    STTS751_init();
	
	// I2C RTCC 初期化
    //   初めての起動時は、2017/1/1 0:00:00 で
    //   時刻を初期化します    
    i2c_rtcc_init();
	
    // I2C LCD DEMO 開始
    i2c_lcd_init(); 
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F18857 DEMO");
    
    // カウンターの初期化
    user_sec_count = 0;
}

// 割込みごとに処理（1.024 ms）
static void process_on_interval()
{
    // 秒あたり割込み回数をカウントダウン
    if (0 < cnt_int_per_sec) {
        cnt_int_per_sec--;
    } else {
        // 表示用秒数のカウントダウンを続ける
        if (0 < user_sec_count) {
            user_sec_count--;
            cnt_int_per_sec = INT_PER_SEC;
        }
    }
}

// 約 1.0 秒ごとに処理（1.024 ms × 305回）
static void process_on_one_second()
{
    char c[17];

    // 現在時刻を取得し表示 2017/05/28 11:45 形式
    i2c_rtcc_read_time();
    sprintf(c, "20%02d/%02d/%02d %02d:%02d", 
            rtcc_years, rtcc_months, rtcc_days,
            rtcc_hours, rtcc_minutes);
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string(c);

    // STTS751の計測値を取得し、
    // 温度とカウンター表示
    // (0x02は、度を表すカスタム文字)
    get_stts751_temperature();
    sprintf(c, " %2d.%1d%c%c cnt=%3ld", 
            stts751_value, stts751_decimals, 0x02, 'C', user_sec_count);
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string(c);
}

//
// 主処理
//
void process()
{
    // UART入力を優先させる
    parse_uart_input();

    // 割込みごとに処理（1.024 ms）
    if (tmr0_toggle == 1) {
        process_on_interval();

        // スイッチ連続検知抑止
        switch_prevent();
        tmr0_toggle = 0;
    }

    // 約 1.0 秒ごとに処理
    if (tmr0_total_cnt_1s > INT_PER_SEC) {
        // カウンターを初期化
        tmr0_total_cnt_1s = 0;
        // イベントごとの処理を行う
        process_on_one_second();
    }

    // スイッチ検知処理
    switch_detection();
}
