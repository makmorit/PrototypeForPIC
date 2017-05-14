#ifndef __DEVICE_H
#define __DEVICE_H

void setup_port();
void setup_uart();
void setup_spi();

unsigned char spi_transmit(unsigned char dt);
void spi_ss_select(unsigned char flag);

#endif // __DEVICE_H
