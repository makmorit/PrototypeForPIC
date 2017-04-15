#include "common.h"
#include "device.h"
#include "i2c_lcd.h"
#include "process.h"

// ディスプレイに表示させるカウンター
// １秒ごとに更新される
static unsigned char cnt;

//
// ボタン押下検知処理
//
static unsigned long btn_push_prevent_cnt;

// 割込みごとに処理（2.048ms）
void switch_prevent()
{
	// カウンターが０の時は終了
	if (0 == btn_push_prevent_cnt) {
		return;
	}

	// ボタン連続押下抑止カウンターを更新
	btn_push_prevent_cnt-- ;
}

static int process_on_button_press()
{
	int ret = 1;

	// スイッチが押された時
	if (RA3 == 0) {
        // カウンターの初期化
        cnt = 180;

	} else {
		ret = 0;
	}
	return ret;
}

// イベントループ内の最後のステップで処理
void switch_detection()
{
	// カウンターが０でない時は終了
	if (0 < btn_push_prevent_cnt) {
		return;
	}

	// スイッチ押下時の処理を実行
	if (process_on_button_press() != 0) {
		// 押下抑止カウンターを設定(約１秒に設定)
		btn_push_prevent_cnt = 500;
	} else {
		btn_push_prevent_cnt = 0;
	}
}

// 約 1.0 秒ごとに処理
void process_on_one_second()
{
    char c[17];
    
    // ヘルスチェックLEDを点滅
    RA1 = ~RA1;
    
    // カウンター表示
    sprintf(c, "     counter=%3d", cnt);
    i2c_lcd_set_cursor(1, 0);
    i2c_lcd_put_string(c);

    // カウントダウン
    if (cnt > 0) {
        cnt--;
    }
}

// 初期処理
void process_init()
{
    // I2C LCD DEMO 開始
    i2c_lcd_init(); 
    i2c_lcd_set_cursor(0, 0);
    i2c_lcd_put_string("PIC16F1823 DEMO ");
    
    // カウンターの初期化
    cnt = 0;
}
