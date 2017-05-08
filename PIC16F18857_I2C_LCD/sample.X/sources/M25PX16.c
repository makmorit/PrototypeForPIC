#include "common.h"
#include "M25PX16.h"

#define SPI_SS_ON 0
#define SPI_SS_OFF 1

#define CMD_WRITE_ENABLE (0x06)
#define CMD_WRITE_DISABLE (0x04)
#define CMD_READ_STATUS_REGISTER (0x05)
#define CMD_READ_IDENTIFICATION (0x9F)
#define CMD_PAGE_PROGRAM (0x02)
#define CMD_READ_DATA_BYTES (0x03)
#define CMD_SUBSECTOR_ERASE (0x20)
#define CMD_SECTOR_ERASE (0xD8)

#define SPI_HCHECK_LED LATBbits.LATB5

static void spi_set_ss(unsigned char s) 
{
    LATBbits.LATB0 = s;
}

static unsigned char spi_transmit(unsigned char c)
{
    unsigned char d;

    SSP2BUF = c;
    while (SSP2STATbits.BF == 0);
    d = SSP2BUF;

    return d;
}

static unsigned char M25PX16_read_status_register()
{
    unsigned char sreg;

    spi_set_ss(SPI_SS_ON);
    spi_transmit(CMD_READ_STATUS_REGISTER);
    sreg = spi_transmit(0);
    spi_set_ss(SPI_SS_OFF);

    return sreg;
}

static void M25PX16_wait_in_progress()
{
	unsigned char status;

	do {
		status = M25PX16_read_status_register();
		__delay_ms(10);
	} while (status & 0x01);
}

static void M25PX16_write_enable()
{
    SPI_HCHECK_LED = 1;

    M25PX16_wait_in_progress();

    spi_set_ss(SPI_SS_ON);
    spi_transmit(CMD_WRITE_ENABLE);
    spi_set_ss(SPI_SS_OFF);

    M25PX16_wait_in_progress();
}

static void M25PX16_write_disable()
{
    M25PX16_wait_in_progress();

    spi_set_ss(SPI_SS_ON);
    spi_transmit(CMD_WRITE_DISABLE);
    spi_set_ss(SPI_SS_OFF);

    M25PX16_wait_in_progress();

    SPI_HCHECK_LED = 0;
}

void M25PX16_page_program(unsigned long addr, unsigned char *buf, size_t size)
{
    unsigned long i;

    M25PX16_write_enable();

    spi_set_ss(SPI_SS_ON);

    spi_transmit(CMD_PAGE_PROGRAM);
    spi_transmit(addr >> 16);
    spi_transmit(addr >>  8);
    spi_transmit(addr >>  0);
    for (i = 0; i < size; i++) {
        spi_transmit(buf[i]);
    }

    spi_set_ss(SPI_SS_OFF);

    M25PX16_write_disable();
}

void M25PX16_read_data_bytes(unsigned long addr, unsigned char *buf, size_t size)
{
    unsigned long i;

    M25PX16_wait_in_progress();

    spi_set_ss(SPI_SS_ON);

    spi_transmit(CMD_READ_DATA_BYTES);
    spi_transmit(addr >> 16);
    spi_transmit(addr >>  8);
    spi_transmit(addr >>  0);
    for (i = 0; i < size; i++) {
        buf[i] = spi_transmit(0);
    }

    spi_set_ss(SPI_SS_OFF);
}

void M25PX16_sector_erase(unsigned long addr)
{
    M25PX16_write_enable();
 
    spi_set_ss(SPI_SS_ON);

    spi_transmit(CMD_SECTOR_ERASE);
    spi_transmit(addr >> 16);
    spi_transmit(addr >>  8);
    spi_transmit(addr >>  0);

    spi_set_ss(SPI_SS_OFF);

    M25PX16_write_disable();
}
