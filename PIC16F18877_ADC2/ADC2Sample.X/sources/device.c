#include "common.h"
#include "device.h"

//
// ピンなどの設定を行う
//
void setup_port()
{
    // Port A
    //   アナログを一部使用
    //          76543210
    ANSELA  = 0b00000001; // アナログ入力はANA0を使用
    TRISA   = 0b00000001; // RA0=入力に設定
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
// TIMER0の設定
//
void setup_timer0()
{
    // Enables Timer0
    // Timer0 is configured as an 8-bit timer/counter
    T0CON0bits.T0EN = 1;
    T0CON0bits.T016BIT = 0;

    // T0CS<2:0>: 010 = FOSC/4
    // T0ASYNC: 0 = The input to the TMR0 counter is synchronized to FOSC/4
    // T0CKPS<3:0>: 0101 = 1:32 (Prescaler Rate Select bit)
    //   1 count = 4us(=1/32MHz*4*32)
    T0CON1bits.T0CS = 0b010;
    T0CON1bits.T0ASYNC = 0;
    T0CON1bits.T0CKPS = 0b0101;

    // 256カウント（1.024 ms）で割込み発生
    TMR0 = 0;

    // TMR0割り込み許可
    TMR0IE = 1;
}

//
// I2Cの設定
//
void setup_i2c()
{
    // Peripheral Pin Select (PPS) module settings
    // Use SSPxCLKPPS, SSPxDATPPS, and RxyPPS to select the pins.
    //   RC3 = MSSP1:SCL1(0x14) for output
    //   MSSP1:SCL1 = RC3(0x13) for input
    RC3PPS = 0x14;
    SSP1CLKPPS = 0x13;
    //   RC4 = MSSP1:SDA1(0x15) for output
    //   MSSP1:SDA1 = RC4(0x14) for input
    RC4PPS = 0x15;
    SSP1DATPPS = 0x14;

    // SMP: 1 = Standard Speed mode(100kHz)
    SSP1STATbits.SMP = 1;

    // SSPEN: 1 = Enables the serial port and configures the SDA and SCL pins as the source of the serial port pins
    // SSPM<3:0>: 1000 = I2C Master mode, clock = FOSC / (4 * (SSPxADD+1))
    SSP1CON1bits.SSPEN = 1;
    SSP1CON1bits.SSPM = 0b1000;

    // clock = FOSC / (4 * (SSPxADD+1))
    //   32MHz/(4*(79+1))=100KHz
    SSP1ADD = 79;

    // SSP(I2C)割り込みを許可
    SSP1IE = 1;
    // MSSP(I2C)バス衝突割り込みを許可
    BCL1IE = 1;

    // SSP(I2C)割り込みフラグをクリア
    SSP1IF = 0;
    // MSSP(I2C)バス衝突割り込みフラグをクリア
    BCL1IF = 0;
}

//
// ADC2の設定
//
void setup_adc2()
{
    // ADON: 1 = ADC is enabled
    // ADCONT: 1 = ADGO is retriggered upon completion of each conversion trigger until ADTIF is set
    // ADFRM0: 0 = ADRES and ADPREV data are left-justified, zero-filled(左寄せ)
    ADCON0bits.ADON = 1;
    ADCON0bits.ADCONT = 1;
    ADCON0bits.ADFRM0 = 0;

    // チャンネル信号源の設定
    // ADPCH=000000 (ANA0)
    ADPCHbits.ADPCH = 0;
    
    // 参照電圧源の設定
    // ADNREF=0 (VREF- is connected to AVSS)
    // ADPREF=00 (VREF+ is connected to VDD)
    ADREFbits.ADNREF = 0;
    ADREFbits.ADPREF = 0b00;
    
    // 変換クロックの設定 500KHz(2us毎に変換)
    // ADCS: 0 = Clock supplied by FOSC, divided according to ADCLK register
    // ADCCS = FOSC/2*(ADCCS+1) = 32MHz/2*(31+1) = 0.5MHz
    ADCON0bits.ADCS = 0;
    ADCLKbits.ADCCS = 0b011111;
    
    // 自動変換トリガーの設定
    // 0x03 = TMR1 (Timer1 overflow condition)
    //ADACTbits.ADACT = 0x03;
    // 0x02 = TMR0 (Timer0 overflow condition)
    ADACTbits.ADACT = 0x02;

    // 計算モードの設定
    // ADMD<2:0>: ADC Operating Mode Selection bits
    // 000 = Basic (Legacy) mode
    // 011 = Burst Average mode
    ADCON2bits.ADMD = 0b011;

    // サンプル収集数の指定
    // (平均化モード選択時に有効)
    // The accumulated value is right-shifted by ADCRS
    // 0b100: divided by 2^4(=16)
    ADCON2bits.ADCRS = 0b100;
    
    // Precharge time is not included in the data conversion cycle
    // Acquisition time is not included in the data conversion cycle
    ADPRE = 0;
    ADACQ = 0;
}
