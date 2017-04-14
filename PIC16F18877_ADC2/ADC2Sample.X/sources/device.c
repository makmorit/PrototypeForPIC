#include "main.h"
#include "device.h"

//
// ピンなどの設定を行う
//
void port_init()
{
	// Port A
	//   アナログ入力はANA0を使用
	//          76543210
	ANSELA  = 0b00000001;
	TRISA   = 0b00000001;
	PORTA   = 0b00000000;

	// Port B
	//   アナログは使用しない（すべてデジタルI/Oに割当てる）
	//          76543210
	ANSELB  = 0b00000000;
	TRISB   = 0b00000000;
	PORTB   = 0b00000000;

	// Port C
	//   アナログは使用しない（すべてデジタルI/Oに割当てる）
	//          76543210
	ANSELC  = 0b00000000;
	TRISC   = 0b00000000;
	PORTC   = 0b00000000;

	// Port D
	//   アナログは使用しない（すべてデジタルI/Oに割当てる）
	//          76543210
	ANSELD  = 0b00000000;
	TRISD   = 0b00000000;
	PORTD   = 0b00000000;

	// Port E
	//   アナログは使用しない（すべてデジタルI/Oに割当てる）
	//   RE3(SW)は入力(10k pull up)
	//          76543210
	ANSELE  = 0b00000000;
	TRISE   = 0b00001000;
	PORTE   = 0b00000000;
}

//
// ADC2の設定
//
void adc2_init()
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

//
// A/D変換ポートから値（０～２５５の値）を取得
//
unsigned char adc2_conv() 
{
	unsigned char temp;

	// ANポート選択
    // ADPCH=000000 (ANA0)
    ADPCH = 0;

	// アクィジション時間
	// （PIC内蔵のADC用コンデンサー充電完了待ち）
	// 入力抵抗が 10kΩ 時、約 20us
	__delay_us(20);

	// 変換スタート
	GO_nDONE = 1;

	// AD変換中（GO_DONE 1->0）
	while (GO_nDONE);

	// 変換値を戻す.
	temp = ADRESH;
	return temp;
}
