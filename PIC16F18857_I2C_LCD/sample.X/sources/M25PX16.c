#include "common.h"
//#include "spi.h"
#include "M25PX16.h"

#define CMD_READ_IDENTIFICATION (0x9F)

void spi_intr()
{
    if (SSP2IF == 1) {
        SSP2IF = 0;
    }
}

static void spi_ss_on() 
{
    LATBbits.LATB0 = 0;
}

static void spi_write_byte(unsigned char c)
{
    unsigned char d;

    //d = SSP2BUF;
    SSP2BUF = c;
    while (SSP2STATbits.BF == 0);

    d = SSP2BUF;
}

static unsigned char spi_instant_read_byte()
{
    unsigned char d;

    //d = SSP2BUF;

    SSP2BUF = 0;
    while (SSP2STATbits.BF == 0);
    d = SSP2BUF;

    return d;
}
static void spi_ss_off() 
{
    LATBbits.LATB0 = 1;
}

void M25PX16_init()
{
    RC2 = 1;
}

void M25PX16_get_id(m25px16_identification_t *p)
{
    int i;
    spi_ss_on();

    spi_write_byte(CMD_READ_IDENTIFICATION);
    RC2 = 0;

    memset(p, 0, sizeof(m25px16_identification_t));
    p->manufacturer = spi_instant_read_byte();
    p->memory_type = spi_instant_read_byte();
    p->memory_capacity = spi_instant_read_byte();
    p->cfd_length = spi_instant_read_byte();
    for (i = 0; i < p->cfd_length; i++) {
        p->cfd_content[i] = spi_instant_read_byte();
    }

    spi_ss_off();
}
