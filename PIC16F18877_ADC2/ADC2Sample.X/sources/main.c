#include "common.h"
#include "device.h"
#include "process.h"
#include "uart.h"
#include "timer0.h"
#include "i2c.h"

// CONFIG1
#pragma config FCMEN = OFF
#pragma config CSWEN = OFF
#pragma config CLKOUTEN = OFF
#pragma config RSTOSC = HFINT32 // 32MHz
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
// 各種設定／初期化処理
//
static void setup()
{
    // 各種設定処理
    //   ピン設定、タイマー０、I2C、ADC2
    //   
    setup_port();
    setup_timer0();
    setup_i2c();
    setup_adc2();
    setup_uart();
    setup_spi();

    // 全割込み処理を許可する
    PEIE = 1;
    GIE  = 1;
}

//
// 割込み処理
//
// for disk_timerproc
#include "fatfs_diskio.h" 
static void interrupt intr(void)
{
    uart_intr();
    timer0_intr();
    i2c_intr();
    
    // fatfs用の割込み
    disk_timerproc();
}

//
// メインルーチン
//
void main() 
{
    // ピンや機能等の設定処理
    setup();

    // 初期化処理
    timer0_init();
    uart_init();

    // 各種初期化処理
    process_init();

    while (1) {
        // イベント処理
        process();
    }
}
