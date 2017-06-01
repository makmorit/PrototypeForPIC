#include "common.h"

// for FatFs
#include "fatfs_diskio.h"
#include "fatfs_ff.h"

FATFS FatFs;
FIL Fil;

void fatfs_test()
{
	UINT bw;

    printf("fatfs_test: start \r\n");

	FRESULT res = f_mount(&FatFs, "", 0);
    if (res != FR_OK) {
        printf("fatfs_test: f_mount failed %d \r\n", res);
        return;
    }
    printf("fatfs_test: f_mount done %d \r\n", res);

	if (f_open(&Fil, "test0601.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {

		f_write(&Fil, "It works!\r\n", 11, &bw);
		f_close(&Fil);
        // If data written well
		if (bw == 11) {
            printf("fatfs_test: f_write success \r\n");
		} else {
            printf("fatfs_test: f_write failed \r\n");
        }
	} else {
        printf("fatfs_test: f_open failed \r\n");
    }

    printf("fatfs_test: end \r\n");
}
