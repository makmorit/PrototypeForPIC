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

    if (rw == RW_0) // for debug
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

#if 0
int i2c_repeated_start_condition(int adrs, int rw)
{
    collision_check = 0;

    i2c_idle_check();
    SSP1CON2bits.RSEN = 1;
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
#endif

int i2c_receive_byte(int ack)
{
    int dt;

    collision_check = 0;
    i2c_idle_check();
    SSP1CON2bits.RCEN = 1;
    while (RCEN == 1);

    dt = SSP1BUF;

    SSP1CON2bits.ACKDT = ack;
    SSP1CON2bits.ACKEN = 1;

    return dt;
}
