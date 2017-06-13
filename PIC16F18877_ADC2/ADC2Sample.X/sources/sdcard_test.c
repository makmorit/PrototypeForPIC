#include "common.h"
#include "i2c_rtcc.h"

// for FatFs
#include "fatfs_diskio.h"
#include "fatfs_ff.h"

FATFS FatFs;
FIL Fil;

void fatfs_test()
{
	UINT bw;
    const char *words = "It works!\r\n";
    const char *file_name = "test0601.txt";
    int length;

    printf("fatfs_test: start \r\n");

	FRESULT res = f_mount(&FatFs, "", 0);
    if (res != FR_OK) {
        printf("fatfs_test: f_mount failed : result=%d\r\n", res);
        return;
    }
    printf("fatfs_test: f_mount done: result=%d\r\n", res);

	if (f_open(&Fil, file_name, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        length = strlen(words);
		f_write(&Fil, words, length, &bw);
		f_close(&Fil);

		if (bw == length) {
            printf("fatfs_test: f_write success: length=%d\r\n", length);
		} else {
            printf("fatfs_test: f_write failed\r\n");
        }

	} else {
        printf("fatfs_test: f_open failed \r\n");
    }

    printf("fatfs_test: end at %s\r\n", get_timestamp_str());
}
