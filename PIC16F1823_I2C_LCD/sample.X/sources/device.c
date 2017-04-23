#include "common.h"
#include "device.h"

//
// ピンなどの設定を行う
//
void setup_port()
{
    // Port A
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELA  = 0b00000000;
    TRISA   = 0b00001000; // RA3=入力に設定（10k でプルアップ）
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
void setup_timer0()
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

//
// I2Cの設定
//
void setup_i2c()
{
    // SMP: 1 = Standard Speed mode(100kHz)
    SSP1STAT= 0b10000000;

    // SSPEN: 1 = Enables the serial port and configures the SDA and SCL pins as the source of the serial port pins
    // SSPM<3:0>: 1000 = I2C Master mode, clock = FOSC / (4 * (SSPxADD+1))
    SSP1CON1= 0b00101000;

    // clock = FOSC / (4 * (SSPxADD+1))
    //   8MHz/(4*(19+1))=100KHz
    SSP1ADD = 19;

    // SSP(I2C)割り込みを許可
    SSP1IE = 1;
    // MSSP(I2C)バス衝突割り込みを許可
    BCL1IE = 1;

    // SSP(I2C)割り込みフラグをクリア
    SSP1IF = 0;
    // MSSP(I2C)バス衝突割り込みフラグをクリア
    BCL1IF = 0;
}
