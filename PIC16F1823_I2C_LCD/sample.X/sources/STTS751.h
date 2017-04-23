#ifndef __STTS751_H_
#define __STTS751_H_

// STTS751 のアドレス
#define STTS751_I2C_ADDR 0b0111001

void STTS751_init();
unsigned char STTS751_get_value();

#endif
