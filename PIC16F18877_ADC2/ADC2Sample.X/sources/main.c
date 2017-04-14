#include "main.h"
#include "device.h"
#include "process.h"

// CONFIG1
#pragma config FCMEN = OFF
// CONFIG2
#pragma config STVREN = ON
#pragma config BORV = 1
#pragma config BOREN = ON
#pragma config PWRTE = OFF
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
static unsigned long total_tmr0_cnt_100m;
static unsigned long total_tmr0_cnt_1s;
static unsigned char tmr0_toggle;

//
// 初期化処理
//
static void initialize()
{
	// ピンなどの初期設定を行う
	port_init();

	//
	// タイマー０の設定を行う
	//   Enables Timer0
    T0EN = 1;
    //   Timer0 is configured as an 8-bit timer/counter
    T016BIT = 0;
    //   T0CS<2:0>: 010 = FOSC/4
    //   T0ASYNC: 0 = The input to the TMR0 counter is synchronized to FOSC/4
    //   T0CKPS<3:0>: 0110 = 1:64
	//     プリスケーラー=64の場合、
    //     １カウント=12.8μ秒(=1/20MHz*4*64)
    T0CON1 = 0b01000110;
    
    // 内部プルアップはすべて無効
    WPUA = 0;
    WPUB = 0;
    WPUC = 0;
    WPUD = 0;
    WPUE = 0;

    // 256カウント（3.2768 ms）で割込み発生させる
	TMR0 = 0;
	// TMR0割り込み許可
	TMR0IE = 1;

	// ADC2設定
    adc2_init();

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
	// タイマー０割込み（1ミリ秒ごと）の場合
	if (TMR0IF == 1) {
		// 割込みカウンター
		total_tmr0_cnt_100m++;
		total_tmr0_cnt_1s++;
		tmr0_toggle = 1;
		// 256カウント（3.2768 ms）で割込み発生させる
		TMR0 = 0;
		// TMR0割り込みクリア
		TMR0IF = 0;
	}
}

//
// イベント処理
//
static void do_events()
{
	//
	// 割込みごとに処理（3.2768 ms）
	if (tmr0_toggle == 1) {
		tmr0_toggle = 0;
	}

	//
	// 約 1.0 秒ごとに処理（3.2768ms × 305回）
	//
	if (total_tmr0_cnt_1s > 305) {
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
	total_tmr0_cnt_100m = 0;
	total_tmr0_cnt_1s = 0;

	// 手動モードで初期化
	process_init();

	while (1) {
		// イベント処理
		do_events();
	}
}
