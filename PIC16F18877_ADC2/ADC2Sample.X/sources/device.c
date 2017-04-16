#include "common.h"
#include "device.h"
#include "i2c.h"

//
// ピンなどの設定を行う
//
void setup_port()
{
    // Port A
    //   アナログ入力はANA0を使用
    //          76543210
    ANSELA  = 0b00000001;
    TRISA   = 0b00000001;
    PORTA   = 0b00000000;
    WPUA    = 0b00000000;

    // Port B
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELB  = 0b00000000;
    TRISB   = 0b00000000;
    PORTB   = 0b00000000;
    WPUB    = 0b00000000;

    // Port C
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELC  = 0b00000000;
    TRISC   = 0b00011000; // RC3(SCL1),RC4(SDA1)=入力に設定
    WPUC    = 0b00000000;
    PORTC   = 0b00000000;

    // Port D
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELD  = 0b00000000;
    TRISD   = 0b00000000;
    PORTD   = 0b00000000;
    WPUD    = 0b00000000;

    // Port E
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELE  = 0b00000000;
    TRISE   = 0b00001000; // RE3(SW)は入力(10k pull up)
    PORTE   = 0b00000000;
    WPUE    = 0b00000000;
}

//
// I2Cの設定
//
void setup_i2c()
{
    // Peripheral Pin Select (PPS) module settings
    // Use SSPxCLKPPS, SSPxDATPPS, and RxyPPS to select the pins.
    //   RC3 = MSSP1:SCL1(0x14)
    //   MSSP1:SCL1 = RC3(0x13)
    RC3PPS = 0x14;
    SSP1CLKPPS = 0x13;
    //   RC4 = MSSP1:SCL1(0x15)
    //   MSSP1:SDA1 = RC4(0x14)
    RC4PPS = 0x15;
    SSP1DATPPS = 0x14;

    // SMP: 1 = Standard Speed mode(100kHz)
    SSP1STAT= 0b10000000;

    // SSPEN: 1 = Enables the serial port and configures the SDA and SCL pins as the source of the serial port pins
    // SSPM<3:0>: 1000 = I2C Master mode, clock = FOSC / (4 * (SSPxADD+1))
    SSP1CON1= 0b00101000;

    // clock = FOSC / (4 * (SSPxADD+1))
    //   32MHz/(4*(79+1))=100KHz
    SSP1ADD = 79;
}

//
// TIMER0の設定
//
void setup_timer0()
{
    // Enables Timer0
    T0EN = 1;
    // Timer0 is configured as an 8-bit timer/counter
    T016BIT = 0;

    // T0CS<2:0>: 010 = FOSC/4
    // T0ASYNC: 0 = The input to the TMR0 counter is synchronized to FOSC/4
    // T0CKPS<3:0>: 0101 = 1:32 (Prescaler Rate Select bit)
    //   1 count = 4us(=1/32MHz*4*32)
    T0CON1 = 0b01000101;

    // 256カウント（1.024 ms）で割込み発生
    TMR0 = 0;

    // TMR0割り込み許可
    TMR0IE = 1;
}

//
// ADC2の設定
//
void setup_adc2()
{
    // ADON: 1 = ADC is enabled
    // ADCS: 0 = Clock supplied by FOSC, divided according to ADCLK register
    // ADFRM0: 0 = ADRES and ADPREV data are left-justified, zero-filled(左寄せ)
    ADCON0 = 0b10000000;

    // ADCS=FOSC/32
    ADCLK = 0b001111;

    // ADNREF=0 (VREF- is connected to AVSS)
    // ADPREF=00 (VREF+ is connected to VDD)
    ADREF = 0;

    // ADPCH=000000 (ANA0)
    ADPCH = 0;

    // Precharge time is not included in the data conversion cycle
    // Acquisition time is not included in the data conversion cycle
    ADPRE = 0;
    ADACQ = 0;
}
