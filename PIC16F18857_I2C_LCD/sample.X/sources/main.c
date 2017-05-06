#include "common.h"
#include "device.h"
#include "process.h"
#include "uart.h"
#include "timer0.h"
#include "i2c.h"
#include "i2c_lcd.h"

#include "M25PX16.h"

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
// 各種設定
//   ピン設定, TIME 0, UART, I2C, SPI
//
static void setup()
{
    setup_port();
    setup_timer0();
    setup_i2c();
    setup_uart();
    setup_spi();

    // 全割込み処理を許可
    PEIE = 1;
    GIE  = 1;
}

//
// 割込み処理
//   UART, TIMER 0, I2C
//
static void interrupt intr(void)
{
    uart_intr();
    timer0_intr();
    i2c_intr();
    spi_intr();
}

//
// メインルーチン
//
void main() 
{
    // ピンや機能等の設定処理
    setup();

    // 初期化処理
    //   TIMER 0, UART
    timer0_init();
    uart_init();
    
    // 主処理ループで使用される変数の初期化
    process_init();

    // 主処理ループ
    while (1) {
        process();
    }
}
