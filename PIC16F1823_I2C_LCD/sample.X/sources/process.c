#include "common.h"
#include "device.h"
#include "process.h"
#include "timer0.h"
#include "i2c_lcd.h"

// １秒間当たりの割込み回数（2.048×488）
#define INT_PER_SEC 488

// ボタン押下連続検知抑止カウンター（2.048×244 = 約0.5秒）
#define BTN_PUSH_PREVENT_CNT 244

// カウントダウン秒数（180秒）
#define COUNT_DOWN_SEC 180

//
// 桁数表示用の変数
//
#define N_DIGIT 3
static unsigned char digit_cnt;
static unsigned char digit_ctrl_cnt;

// カウンターとして使用する変数
static unsigned long user_sec_count;
static unsigned long cnt_int_per_sec;

//
// ボタン押下検知処理
//
static unsigned long btn_push_prevent_cnt;

// ボタン押下時の処理
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

// 割込みごとに処理（2.048 ms）
static void switch_prevent()
{
    // カウンターが０の時は終了
    if (0 == btn_push_prevent_cnt) {
        return;
    }

    // ボタン押下連続検知抑止カウンターを更新
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
        // ボタン押下連続検知抑止カウンターを設定
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
    // I2C LCD DEMO 開始
    i2c_lcd_init(); 
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F1823 DEMO ");
    
    // カウンターの初期化
    user_sec_count = 0;
}


// 割込みごとに処理（2.048 ms）
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

// 約 1.0 秒ごとに処理
static void process_on_one_second()
{
    char c[17];
    
    // ヘルスチェックLEDを点滅
    HCHECK_LED = ~HCHECK_LED;
    
    // カウンター表示
    sprintf(c, "     counter=%3ld", user_sec_count);
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string(c);
}

//
// 主処理
//
void process()
{
    // 割込みごとに処理（2.048 ms）
    if (tmr0_toggle == 1) {
        tmr0_toggle = 0;
        process_on_interval();

        // ボタン押下連続検知抑止
        switch_prevent();
    }

    //
    // 約1秒ごとに処理（2.048ms × 488回）
    //
    if (tmr0_total_cnt_1s > 488) {
        // カウンターを初期化
        tmr0_total_cnt_1s = 0;
        // イベントごとの処理を行う
        process_on_one_second();
    }

    // ボタン押下検知処理
    switch_detection();
}
