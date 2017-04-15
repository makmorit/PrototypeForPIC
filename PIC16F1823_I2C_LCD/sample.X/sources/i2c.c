#include "common.h"
#include "i2c.h"

// ACK待ち
static int ack_check;
// バス衝突検出
static int collision_check;

static void i2c_idle_check()
{
    // ACKEN, RCEN, PEN, RSEN, SEN, R/W, BFが全て0ならOK
    while ((SSP1CON2 & 0x1F) | (SSP1STAT & 0x5));
}

void i2c_intr(void)
{
    if (SSP1IF == 1) {
        if (ack_check == 1) {
            ack_check = 0;
        }
        SSP1IF = 0;
    }
    if (BCL1IF == 1) {
        // MSSP(I2C)バス衝突割り込み発生
        collision_check = 1;
        BCL1IF = 0;
    }
}

void i2c_init()
{
    // SMP: 1 = Standard Speed mode(100kHz)
    SSP1STAT= 0b10000000;

    // SSPEN: 1 = Enables the serial port and configures the SDA and SCL pins as the source of the serial port pins
    // SSPM<3:0>: 1000 = I2C Master mode, clock = FOSC / (4 * (SSPxADD+1))
    SSP1CON1= 0b00101000;

    // clock = FOSC / (4 * (SSPxADD+1))
    //   8MHz/(4*(19+1))=100KHz
    SSP1ADD = 19;

    // SSP(I2C)割り込みを許可
    SSP1IE = 1;
    // MSSP(I2C)バス衝突割り込みを許可
    BCL1IE = 1;

    // SSP(I2C)割り込みフラグをクリア
    SSP1IF = 0;
    // MSSP(I2C)バス衝突割り込みフラグをクリア
    BCL1IF = 0;
}

int i2c_start_condition(int adrs, int rw)
{
    collision_check = 0;

    i2c_idle_check();
    SSP1CON2bits.SEN = 1;

    i2c_idle_check();
    if (collision_check == 1) {
        return -1;
    }

    ack_check = 1;
    SSP1BUF = (char)((adrs<<1)+rw);
    while (ack_check);
    if (collision_check == 1) {
        return -1;
    }

    return SSP1CON2bits.ACKSTAT;
}

int i2c_stop_condition()
{
    collision_check = 0;

    i2c_idle_check();
    SSP1CON2bits.PEN = 1;
    if (collision_check == 1) {
        return -1;
    }

    return  0;
}

int i2c_send_byte(char dt)
{
    collision_check = 0;

    i2c_idle_check();
    if (collision_check == 1) {
        return -1;
    }

    ack_check = 1;
    SSP1BUF = dt;
    while (ack_check);
    if (collision_check == 1) {
        return -1;
    }

    return SSP1CON2bits.ACKSTAT;
}
