#include "common.h"
#include "device.h"
#include "process.h"
#include "i2c.h"

// CONFIG1
#pragma config FCMEN = OFF
#pragma config CLKOUTEN = OFF
#pragma config RSTOSC = HFINT32
// CONFIG2
#pragma config STVREN = ON
#pragma config BORV = 1
#pragma config BOREN = ON
#pragma config PWRTE = ON
#pragma config MCLRE = OFF
// CONFIG3
#pragma config WDTE = OFF
// CONFIG4
#pragma config LVP = OFF
#pragma config WRT = OFF
// CONFIG5
#pragma config CPD = OFF
#pragma config CP = OFF

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
    // ピンなどの初期設定を行う
    port_init();

    // タイマー０の設定を行う
    timer0_init();

    // ADC2設定
    adc2_init();

    // I2C設定
    i2c_init();

    // 全割込み処理を許可する
    PEIE = 1;
    GIE  = 1;

    // TIMER2 on prescale=1
    // TIMER2スタート（onビットを１）
    T2CON = 0b100;
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
        // 256カウント（1.024ms）で割込み発生させる
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
    // 割込みごとに処理（1.024ms）
    if (tmr0_toggle == 1) {
        tmr0_toggle = 0;
    }

    //
    // 約1秒ごとに処理（1.024ms × 977回）
    //
    if (total_tmr0_cnt_1s > 977) {
        // カウンターを初期化
        total_tmr0_cnt_1s = 0;
        // イベントごとの処理を行う
        process_on_one_second();
    }
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
