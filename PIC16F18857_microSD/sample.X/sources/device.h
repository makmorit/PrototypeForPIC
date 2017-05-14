#ifndef __DEVICE_H
#define __DEVICE_H

// スイッチ入力
//   ボタン０
#define BUTTON_0    RE3
#define BUTTON_1    RB4

// 関数
void setup_port();
void setup_timer0();
void setup_timer2();
void setup_uart();
void setup_i2c();
void setup_spi();

unsigned char spi_transmit(unsigned char dt);
void spi_ss_select(unsigned char flag);

#endif // __DEVICE_H
