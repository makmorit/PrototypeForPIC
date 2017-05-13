#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "sdcard.h"

#define _XTAL_FREQ  8000000

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

static char dt[81];

//
// ピンなどの設定を行う
//
static void setup_port()
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
// UARTの設定
//
void setup_uart()
{
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

static void setup_sdcard_spi()
{
    // SPI Master 通信速度:Fosc/64
    SSPCON1bits.SSPM1 = 1;
    SSPCON1bits.SSPEN = 1;
    
    // クロック極性1(HIGH)
    // クロック位相0(立上りエッジでデータを送信)
    SSPCON1bits.CKP = 1;
    SSPSTATbits.CKE = 0; 

    SSPIF   = 0;
}

// 各種設定
static void setup()
{
    // 8MHz 内部クロック
    OSCCON = 0b01110010;

    setup_port();
    setup_uart();
    setup_sdcard_spi();

    // 全割込み処理を許可
    PEIE = 1;
    GIE  = 1;
}

// 割込み処理
static void interrupt intr(void)
{
}

//
// printf 内で使用する putch 関数
// (UART TX に１バイト出力します)
//
void putch(unsigned char byte)
{
    while (!TXIF) {
        continue;
    }
    TXREG = byte;
}

static void sdcard_test_read(const unsigned char *filename)
{
    FILE_ACCESS_INFO_T fp;
	int ans;

    ans = sdcard_open_file(&fp, filename, OP_READ_ONLY);
    printf("sdcard_test_read: sdcard_open_file(): ans=%d \r\n", ans);
    if (ans != 0) {
        return;
    }

    printf("sdcard_test_read: sdcard_read_line() start \r\n");
    while(1) {
        memset(dt, 0x00, sizeof(dt));
        ans = sdcard_read_line(&fp, dt, sizeof(dt)-1);
        if (ans != 0) {
            printf("%s\r\n", dt);
        } else {
            break;
        }
        __delay_ms(1);
    }
    printf("sdcard_test_read: sdcard_read_line() end \r\n");

    sdcard_close_file(&fp);
    printf("sdcard_test_read: sdcard_close_file() done \r\n");
}

static void sdcard_test_write(const unsigned char *filename)
{
    FILE_ACCESS_INFO_T fp;
	int ans;

    // 書き込むファイル／文字列
    const unsigned char *line1 = "PIC16F1938\r\n";
    const unsigned char *line2 = "microSD\r\n";
    const unsigned char *line3 = "Test write demo\r\n";
    const unsigned char *line4 = "[EOF]\r\n";

    ans = sdcard_open_file(&fp, filename, OP_READ_WRITE);
    printf("sdcard_test_write: sdcard_open_file(): ans=%d \r\n", ans);
    if (ans != 0) {
        printf("sdcard_test_write: file open error \r\n");
        return;
    }

    // ファイルへ１行ずつ書き込む
    ans = sdcard_write_line(&fp, line1, strlen(line1));
    ans = sdcard_write_line(&fp, line2, strlen(line2));
    ans = sdcard_write_line(&fp, line3, strlen(line3));
    ans = sdcard_write_line(&fp, line4, strlen(line4));

    sdcard_close_file(&fp);
    printf("sdcard_test_write: sdcard_close_file() done \r\n");
}

static void sdcard_test_append(const unsigned char *filename)
{
    FILE_ACCESS_INFO_T fp;
	int ans;

    // 書き込むファイル／文字列
    const unsigned char *line1 = "PIC16F1938\r\n";
    const unsigned char *line2 = "microSD\r\n";
    const unsigned char *line3 = "Test append demo\r\n";
    const unsigned char *line4 = "[EOF]\r\n";

    ans = sdcard_open_file(&fp, filename, OP_APPEND);
    printf("sdcard_test_append: sdcard_open_file(): ans=%d \r\n", ans);
    if (ans != 0) {
        // 新規書込みモードとしてオープンし直す
        ans = sdcard_open_file(&fp, filename, OP_READ_WRITE);
        if (ans != 0) {
            printf("sdcard_test_append: file open error \r\n");
            return;
        }
        printf("sdcard_test_append: file created \r\n");
    }

    // ファイルへ１行ずつ書き込む
    ans = sdcard_write_line(&fp, line1, strlen(line1));
    ans = sdcard_write_line(&fp, line2, strlen(line2));
    ans = sdcard_write_line(&fp, line3, strlen(line3));
    ans = sdcard_write_line(&fp, line4, strlen(line4));

    sdcard_close_file(&fp);
    printf("sdcard_test_append: sdcard_close_file() done \r\n");
}

void main()
{
	int ans;
    const unsigned char *writer = "WRITER.TXT";
    const unsigned char *appender = "APPENDER.TXT";

    setup();
    __delay_ms(3000);

    ans = sdcard_initialize();
    if (ans == 0) {
        // 常に同じ内容を書き込まないと、
        // 後ろに前回書き込んだ内容が残ってしまう可能性があります
        printf("Write to %s\r\n", writer);
        sdcard_test_write(writer);
        sdcard_test_read(writer);

        // すでに存在するファイルに追加書込み
        printf("Append to %s\r\n", appender);
        sdcard_test_append(appender);
        sdcard_test_read(appender);
    }

    while(1);
}
