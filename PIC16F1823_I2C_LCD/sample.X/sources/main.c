#include "common.h"
#include "device.h"
#include "process.h"
#include "i2c.h"

// CONFIG1
#pragma config FOSC = INTOSC // 8MHz
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = OFF
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = ON
#pragma config CLKOUTEN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF

// CONFIG2
#pragma config WRT = OFF
#pragma config PLLEN = OFF
#pragma config STVREN = ON
#pragma config BORV = HI
#pragma config LVP = OFF

//
// タイマーで使用する変数
//
static unsigned long total_tmr0_cnt_1s;
static unsigned char tmr0_toggle;

//
// 初期化処理
//
static void initialize()
{
    // 内部クロック = 8MHz
    OSCCON  = 0b01110010;

    // ピンなどの初期設定を行う
    port_init();

    // タイマー０の設定を行う
    timer0_init();

    // I2C設定
    i2c_init();

    // 全割込み処理を許可する
    PEIE = 1;
    GIE  = 1;
}

//
// 割込み処理
//
static void interrupt intr(void)
{
    // タイマー０割込みの場合
    if (TMR0IF == 1) {
        // 割込みカウンター
        total_tmr0_cnt_1s++;
        tmr0_toggle = 1;
        // 256カウント（2.048ms）で割込み発生させる
        TMR0 = 0;
        // TMR0割り込みクリア
        TMR0IF = 0;
    }
    // I2C割込処理
    i2c_intr();
}

//
// イベント処理
//
static void do_events()
{
    //
    // 割込みごとに処理（2.048ms）
    if (tmr0_toggle == 1) {
        tmr0_toggle = 0;
        // ボタン連続押下抑止
		switch_prevent();
    }

    //
    // 約1秒ごとに処理（2.048ms × 488回）
    //
    if (total_tmr0_cnt_1s > 488) {
        // カウンターを初期化
        total_tmr0_cnt_1s = 0;
        // イベントごとの処理を行う
        process_on_one_second();
    }

	// ボタン検知処理
	switch_detection();
}

//
// メインルーチン
//
void main() 
{
    // ピンや機能等の初期化処理
    initialize();

    // do_events 処理回数カウンター
    //   処理時点での割込みカウンター
    total_tmr0_cnt_1s = 0;

    // 各種初期化処理
    process_init();

    while (1) {
        // イベント処理
        do_events();
    }
}