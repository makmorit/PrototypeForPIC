#include "common.h"
#include "device.h"

//
// ピンなどの設定を行う
//
void setup_port()
{
    // Port A
    //          76543210
    ANSELA  = 0b00000000;
    TRISA   = 0b00000000;
    PORTA   = 0b00000000;

    // Port B
    //          76543210
    ANSELB  = 0b00000000;
    TRISB   = 0b00000000;
    PORTB   = 0b00000000;

    // Port C
    //          76543210
    TRISC   = 0b10010000; // RC7(RX), RC4(SDI)は入力
    PORTC   = 0b00000000;

    // Port E
    //          76543210
    TRISE   = 0b00001000; // RE3(SW)は入力(10k pull down)
    PORTE   = 0b00000000;
}

//
// UART関連処理
//
void setup_uart()
{
    // UARTの設定
    // 送信情報設定：非同期モード(SYNC=0),8bit(TX9=0),ノンパリティ(TX9D=0)
    TXSTA = 0b00100100;
    // 受信情報設定
    RCSTA = 0b10010000;

    // 9600 の場合（Fosc=8MHz）
    //   SYNC = 0, BRGH = 1, BRG16 = 0
    //   SPBRG = 51 (8,000,000/(16 * 9,600) - 1)
    // 19200 の場合（Fosc=20MHz）
    //   SYNC = 0, BRGH = 0, BRG16 = 1
    //   SPBRG = 64 (20,000,000/(16 * 19,200) - 1)
    BRGH  = 1;
    BRG16 = 0;
    SPBRG = 51;

    // ＵＳＡＲＴ割込み受信フラグの初期化
    RCIF  = 0;
}

void putch(unsigned char byte)
{
    // printf 内で使用する putch 関数の再定義
    // (UART TX に１バイト出力します)
    while (!TXIF) {
        continue;
    }
    TXREG = byte;
}

//
// SPI関連処理
//
void setup_spi()
{
    // SPIの設定
    // SPI Master 通信速度:Fosc/64
    SSPCON1bits.SSPM1 = 1;
    SSPCON1bits.SSPEN = 1;
    
    // クロック極性1(HIGH)
    // クロック位相0(立上りエッジでデータを送信)
    SSPCON1bits.CKP = 1;
    SSPSTATbits.CKE = 0; 

    SSPIF   = 0;
}

unsigned char spi_transmit(unsigned char dt)
{
    // SPI送受信
    SSPBUF = dt;
    while(SSPIF == 0);
    SSPIF = 0;

    return SSPBUF;
}

void spi_ss_select(unsigned char flag)
{
    // SPI SS選択
    LATCbits.LATC2 = flag;
    spi_transmit(0xff);
}
