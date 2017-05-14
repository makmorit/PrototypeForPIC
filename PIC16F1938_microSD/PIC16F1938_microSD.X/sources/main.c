#include "common.h"
#include "device.h"
#include "sdcard.h"
#include "sdcard_test.h"

// CONFIG1
#pragma config FOSC = INTOSC
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
#pragma config VCAPEN = OFF
#pragma config PLLEN = OFF
#pragma config STVREN = ON
#pragma config BORV = HI
#pragma config LVP = OFF

// 各種設定
static void setup()
{
    // 8MHz 内部クロック
    OSCCON = 0b01110010;

    setup_port();
    setup_uart();
    setup_spi();

    // 全割込み処理を許可
    PEIE = 1;
    GIE  = 1;
}

// 割込み処理
static void interrupt intr(void)
{
}

void main()
{
	int ans;

    setup();
    __delay_ms(3000);

    ans = sdcard_initialize();
    if (ans == 0) {
        sdcard_test();
    }

    while(1);
}
