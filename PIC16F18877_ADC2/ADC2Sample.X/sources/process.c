#include "common.h"
#include "device.h"
#include "i2c_lcd.h"
#include "adc2.h"
#include "process.h"
#include "timer0.h"

#include "sdcard.h"
#include "sdcard_test.h"

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

// 初期処理
void process_init()
{
    // ローカル変数の初期化
    btn_push_prevent_cnt = 0;

    // I2C LCD DEMO 開始
    i2c_lcd_init(); 
    
    // カウンターの初期化
    user_sec_count = 0;

    // 最初のプロンプトを表示
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F18877 DEMO");
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string(" init microSD...");
    
    // microSD 初期化
    __delay_ms(1000);
    int ans = sdcard_initialize();
    if (ans == 0) {
        sdcard_test();
    }

    // 次のプロンプトを表示
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F18877     ");
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string("  ADC2 Evaluator");

    // 3秒まつ
    __delay_ms(3000);

    // 最初のプロンプトを表示
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string("                ");
}

// 割込みごとに処理（1.024 ms）
static void process_on_interval()
{
    // 秒あたり割込み回数をカウントダウン
    if (0 < cnt_int_per_sec) {
        cnt_int_per_sec--;
    } else {
        if (0 < user_sec_count) {
            // 表示用秒数のカウントダウンを続ける
            user_sec_count--;
            cnt_int_per_sec = INT_PER_SEC;
            // ヘルスチェックLEDを点灯
            HCHECK_LED = 1;
        } else {
            // ヘルスチェックLEDを消灯
            HCHECK_LED = 0;
        }
    }
}

// 約 1.0 秒ごとに処理
void process_on_one_second()
{
    char c[17];
    unsigned char adc;
    
    // ADC2変換値を取得
    adc = adc2_conv();

    // ADC2変換値とカウンター表示
    sprintf(c, " ADC=%3d cnt=%3ld", adc, user_sec_count);
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string(c);

    // uart
    printf("%s\r\n", c);
}

//
// 主処理
//
void process()
{
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
