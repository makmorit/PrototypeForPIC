#ifndef _I2C_H_
#define _I2C_H_

#define ACK   0
#define NOACK 1
#define RW_0  0
#define RW_1  1

void i2c_intr();
int i2c_start_condition(int adrs, int rw);
int i2c_stop_condition();
int i2c_send_byte(char dt);

#endif
