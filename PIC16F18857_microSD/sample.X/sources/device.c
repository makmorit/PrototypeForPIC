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
    TRISA   = 0b00000000;
    PORTA   = 0b00000000;
    WPUA    = 0b00000000;

    // Port B
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELB  = 0b00000000;
    TRISB   = 0b00010100; // RB2(SDI2),RB4(SW)=入力に設定
    PORTB   = 0b00000000;
    WPUB    = 0b00000000;

    // Port C
    //   アナログは使用しない（すべてデジタルI/Oに割当てる）
    //          76543210
    ANSELC  = 0b00000000;
    TRISC   = 0b10011000; // RC3(SCL1),RC4(SDA1),RC7(RX)=入力に設定
    WPUC    = 0b00000000;
    PORTC   = 0b00000000;

    // Port E
    //          76543210
    PORTE   = 0b00000000; // RE3(SW)は入力(10k pull up)
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
// UARTの設定
//
void setup_uart()
{
    // Peripheral Pin Select (PPS) module settings
    //   RC6 = TX/CK(0x10) for output
    RC6PPS = 0x10;
    //   RX = RC7(0x17) for input
    RXPPS = 0x17;

    // 送信情報設定
    // 非同期モード(SYNC=0),8bit(TX9=0),ノンパリティ(TX9D=0)
    TX1STAbits.TXEN = 1;
    TX1STAbits.SYNC = 0;
    TX1STAbits.TX9 = 0;
    TX1STAbits.TX9D = 0;

    // 受信情報設定
    RC1STAbits.SPEN = 1;
    RC1STAbits.CREN = 1;
    RC1STAbits.RX9 = 0;
    RC1STAbits.RX9D = 0;

    // 19200 の場合（Fosc=32MHz）
    //   SYNC = 0, BRGH = 0, BRG16 = 1
    //   SPBRG = 103 (32,000,000/(16 * 19,200) - 1)
    TX1STAbits.BRGH = 0;
    BAUD1CONbits.BRG16 = 1;
    SP1BRG = 103;

    // ＵＳＡＲＴ割込み受信フラグの初期化
    RCIF  = 0;
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
// SPIの設定
//
void setup_spi()
{
    // Peripheral Pin Select (PPS) module settings
    //   RB1 = SCK2(0x16) for output
    RB1PPS = 0x16;
    //   RB3 = SDO2(0x17) for output
    RB3PPS = 0x17;

    // Clock Polarity Select bit
    //   In SPI mode: 0 = Idle state for clock is a low level
    SSP2CON1bits.CKP = 1;
    
    // SPI Clock Edge Select bit
    //   0 = Transmit occurs on transition from Idle to active clock state
    SSP2STATbits.CKE = 0;
    
    // SPI Data Input Sample bit
    //   SPI Master mode: 
    //   0 = Input data sampled at middle of data output time
    SSP2STATbits.SMP = 0;

    // Enables serial port and configures 
    // SCK, SDO, SDI and SS as the source of the serial port pins
    SSP2CON1bits.SSPEN = 1;
    
    // 0010 = SPI Master mode, clock = FOSC/64
    //   (32MHz / 64 = 500kHz)
    SSP2CON1bits.SSPM = 0b0010;
    
    // 割込みフラグクリア
    SSP2IF = 0;
}

unsigned char spi_transmit(unsigned char dt)
{
    // SPI送受信
    SSP2BUF = dt;
    while(SSP2IF == 0);
    SSP2IF = 0;

    return SSP2BUF;
}

void spi_ss_select(unsigned char flag)
{
    // SPI SS選択 (RB0)
    LATBbits.LATB0 = flag;
    spi_transmit(0xff);
}
