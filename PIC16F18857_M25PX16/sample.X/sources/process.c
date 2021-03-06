#include "common.h"
#include "device.h"
#include "i2c_lcd.h"
#include "process.h"
#include "uart.h"
#include "timer0.h"
#include "STTS751.h"
#include "M25PX16.h"

// １秒間当たりの割込み回数（1.024ms × 977回）
#define INT_PER_SEC 977

// ボタン押下連続検知抑止カウンター（1.024ms × 488 = 約0.5秒）
#define BTN_PUSH_PREVENT_CNT 488

// カウントダウン秒数（180秒）
#define COUNT_DOWN_SEC 180

// 温度計の計測値
static unsigned char stts751_value;

// カウンターとして使用する変数
static unsigned long user_sec_count;
static unsigned long cnt_int_per_sec;

//
// フラッシュメモリー用処理群
//
#define TEMP_DATA_RECLEN 16
static unsigned long mem_write_cnt;

static void initialize_flash_mem()
{
    // フラッシュメモリー書込み位置の初期化
    mem_write_cnt = 0;

    // フラッシュメモリー初期化
    M25PX16_sector_erase(0);
}

static int read_temperature_record(unsigned long addr)
{
    unsigned char c[TEMP_DATA_RECLEN+1];

    memset(c, 0, sizeof(c));
    M25PX16_read_data_bytes(addr * TEMP_DATA_RECLEN, c, TEMP_DATA_RECLEN);

    if (c[0] > 223) {
        // 表示不能文字の場合はブランクデータと判断して処理終了
        return -1;
    }
    
    // uartに出力
    printf("%s\r\n", c);
    return 0;
}

static void print_temperature_data()
{
    // フラッシュメモリーに格納された
    // データのプリント開始
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("Reading tempdata");
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string("  Please wait...");

    for (int i = 0; i < 255; i++) {
        if (read_temperature_record(i) < 0) {
            break;
        }
    }

    // ディスプレイをクリア
    i2c_lcd_clear_display();

    // セクターを初期化
    initialize_flash_mem();

    // 最初のプロンプトを表示
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F18857 DEMO");
}

// フラッシュメモリーに温度情報を出力
static void save_temperature_data(unsigned char value, unsigned char decimals)
{
    char c[TEMP_DATA_RECLEN+1];

    // 秒数カウンターと温度をカンマ区切りで出力
    memset(c, 0, sizeof(c));
    sprintf(c, "%ld,%2d.%1d", mem_write_cnt, value, decimals);

    // フラッシュメモリーに書込み
    M25PX16_page_program(mem_write_cnt * TEMP_DATA_RECLEN, c, TEMP_DATA_RECLEN);

    // 次の書込み位置に移動
    mem_write_cnt++;
    if (mem_write_cnt == 3600) {
        // 1時間経過ごとにセクターを初期化
        initialize_flash_mem();
    }
}

//
// UARTに入力された内容を解析する
//
static void parse_uart_input()
{
    unsigned char c;

    unsigned char *rc_buff = get_uart_recv_buff();
    if (rc_buff == NULL) {
        return;
    }
    
    // TODO: 以下に入力された文字に対する処理を記述
}

//
// ボタン押下検知処理
//
static unsigned long btn_push_prevent_cnt;

// 基板上のボタン押下時の処理
static int process_on_button_press()
{
    int ret = 1;

    // スイッチOnに対する処理を実行
    if (BUTTON_0 == 0) { // プルアップされているので Low 判定
        //
        // 表示用秒数のカウントダウンをスタートさせる
        //
        user_sec_count = COUNT_DOWN_SEC;
        cnt_int_per_sec = INT_PER_SEC;

    } else if (BUTTON_1 == 0) { // プルアップされているので Low 判定
        //
        // 気温計測データをUARTへ出力
        //
        print_temperature_data();

    } else {
        ret = 0;
    }
    return ret;
}

// 割込みごとに処理（1.024 ms）
static void switch_prevent()
{
    // カウンターが０の時は終了
    if (0 == btn_push_prevent_cnt) {
        return;
    }

    // ボタン連続押下抑止カウンターを更新
    btn_push_prevent_cnt-- ;
}

// イベントループ内の最後のステップで処理
static void switch_detection()
{
    // カウンターが０でない時は終了
    if (0 < btn_push_prevent_cnt) {
        return;
    }

    // スイッチ押下時の処理を実行
    if (process_on_button_press() != 0) {
        // 押下抑止カウンターを設定
        btn_push_prevent_cnt = BTN_PUSH_PREVENT_CNT;
    } else {
        btn_push_prevent_cnt = 0;
    }
}

//
// 初期化処理
//
void process_init()
{
    // ローカル変数の初期化
    btn_push_prevent_cnt = 0;

    // STTS751 初期化
    STTS751_init();

    // I2C LCD DEMO 開始
    i2c_lcd_init(); 
    
    // カウンターの初期化
    user_sec_count = 0;

    // フラッシュメモリー初期化
    initialize_flash_mem();

    // 最初のプロンプトを表示
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F18857 DEMO");
}

// 割込みごとに処理（1.024 ms）
static void process_on_interval()
{
    // 秒あたり割込み回数をカウントダウン
    if (0 < cnt_int_per_sec) {
        cnt_int_per_sec--;
    } else {
        // 表示用秒数のカウントダウンを続ける
        if (0 < user_sec_count) {
            user_sec_count--;
            cnt_int_per_sec = INT_PER_SEC;
        }
    }
}

// 約 1.0 秒ごとに処理（1.024 ms × 305回）
static void process_on_one_second()
{
    char c[17];
    unsigned char v, d, dd;
    
    // STTS751の計測値を取得
    v = STTS751_get_value();
    dd = STTS751_get_decimals();

    // 小数点部を、表示できる値に変換
    // (取得した値を四捨五入します)
    d = (dd * 10 + 8) / 16;

    // 温度とカウンター表示
    // (0x02は、度を表すカスタム文字)
    sprintf(c, " %2d.%1d%c%c cnt=%3ld", v, d, 0x02, 'C', user_sec_count);
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string(c);
    
    // フラッシュメモリーに温度出力
    save_temperature_data(v, d);
}

//
// 主処理
//
void process()
{
    // UART入力を優先させる
    parse_uart_input();

    // 割込みごとに処理（1.024 ms）
    if (tmr0_toggle == 1) {
        process_on_interval();

        // スイッチ連続検知抑止
        switch_prevent();
        tmr0_toggle = 0;
    }

    // 約 1.0 秒ごとに処理
    if (tmr0_total_cnt_1s > INT_PER_SEC) {
        // カウンターを初期化
        tmr0_total_cnt_1s = 0;
        // イベントごとの処理を行う
        process_on_one_second();
    }

    // スイッチ検知処理
    switch_detection();
}
