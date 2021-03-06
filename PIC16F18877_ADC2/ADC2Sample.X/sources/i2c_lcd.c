#include "common.h"
#include "i2c.h"
#include "i2c_lcd.h"

// コントラスト調整
static char contrast = 40;

// LCD電源
//   5.0Vで使用の場合=0
//   3.3Vで使用の場合=1
static char bon = 1;

static unsigned char function_set_data;
static unsigned char contrast_set_data;

// カスタム文字 (5x7 Dot Character)
static char tiny_circle[7] = {
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
};

static void command(unsigned char c)
{
    int  ans;

    ans = i2c_start_condition(ST7032_I2C_ADDR, RW_0);
    if (ans == 0) {
        i2c_send_byte(0b10000000);
        i2c_send_byte(c);
    }
    i2c_stop_condition();
    __delay_us(26);
}

void i2c_lcd_register_char(int p, char *dt)
{
    int ans;
    char cnt;

    ans = i2c_start_condition(ST7032_I2C_ADDR, RW_0);
    if (ans == 0) {
        // 保存先のアドレスを指示
        i2c_send_byte(0b10000000);
        i2c_send_byte(0x40 | (p << 3));
        __delay_us(26);

        // キャラクターデータを送信
        i2c_send_byte(0b01000000);
        for (cnt = 0; cnt < 7; cnt++) {
            i2c_send_byte(*dt++);
            __delay_us(26);
        }
    }
    i2c_stop_condition();
}

void i2c_lcd_clear_display(void)
{
    // Clear Display: 画面全体に20Hのｽﾍﾟｰｽで表示、ｶｰｿﾙはcol=0,row=0に移動
    command(0x01);
    __delay_us(1100);

    // Return Home: 画面を初期ポジションに戻す
    command(0x02);
    __delay_us(1100);
}

void i2c_lcd_set_cursor(int row, int col)
{
    // Set DDRAM Adddress: 00H-27H,40H-67H
    char row_offsets[] = {0x00, 0x40};
    command(0x80 | (col + row_offsets[row]));
}

void i2c_lcd_put_string(const char * s)
{
    int  ans;

    ans = i2c_start_condition(ST7032_I2C_ADDR, RW_0);
    if (ans == 0) {
        i2c_send_byte(0b01000000);
        while (*s) {
            i2c_send_byte(*s++);
            __delay_us(26);
        }
    }
    i2c_stop_condition();
}

void i2c_lcd_init()
{
    unsigned char d;

    __delay_ms(40);

    // データ線は8本・表示は2行・フォントは5x8ドット
    function_set_data = 0x38;
    command(function_set_data);

    // 拡張コマンド開始
    command(0x39);

    // Internal OSC frequency
    command(0x14);

    // コントラスト調整(下位4ビット)
    d = 0x70 | (contrast & 0x0F);
    command(d);
    // コントラスト調整(上位2ビット)
    d = 0x50 | ((contrast & 0x30) >> 4);
    // 昇圧回路を利用する場合
    if (bon  == 1) {
        d = d | 0x04;
    }
    contrast_set_data = d;
    command(d);

    // Follower control
    command(0x6C);

    __delay_ms(200);

    // 拡張コマンド終了
    command(function_set_data);

    // display control
    // 画面表示ON・カーソル表示OFF・カーソル点滅OFF
    command(0x0C);
    
    // entry mode set
    // 表示文字の右にカーソル移動
    command(0x06);

    i2c_lcd_clear_display();
    
    // カスタム文字を登録
    i2c_lcd_register_char(2, tiny_circle);
}
