#include "common.h"
#include "device.h"

//
// ピンなどの設定を行う
//
void port_init()
{
    // Port A
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELA  = 0b00000000;
    TRISA   = 0b00001000; // RA3=入力に設定
    PORTA   = 0b00000000;
    WPUA    = 0b00000000;

    // Port C
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELC  = 0b00000000;
    TRISC   = 0b00000011; // RC0(SCL),RC1(SDA)=入力に設定
    WPUC    = 0b00000000;
    PORTC   = 0b00000000;
}

//
// TIMER0の設定
//
void timer0_init()
{
	// TMR0ON: Enables Timer0
	// T08BIT: Timer0 is configured as an 8-bit timer/counter
	// OPTION_REG の先頭ビット:!WPUEN (1:内部プルアップ無)
    // プリスケーラー:16  − １カウント8μ秒(=1/8MHz*4*16)
    OPTION_REG = 0b00000011;

    // 256カウント（2.048 ms）で割込み発生
    TMR0 = 0;

    // TMR0割り込み許可
    TMR0IE = 1;
}
