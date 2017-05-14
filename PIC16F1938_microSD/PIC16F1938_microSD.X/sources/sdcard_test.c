#include "common.h"
#include "sdcard.h"

// 文字列バッファ
static char dt[81];

static void sdcard_test_read(const unsigned char *filename)
{
    FILE_ACCESS_INFO_T fp;
	int ans;

    ans = sdcard_open_file(&fp, filename, OP_READ_ONLY);
    printf("sdcard_test_read: sdcard_open_file(): ans=%d \r\n", ans);
    if (ans != 0) {
        return;
    }

    printf("sdcard_test_read: sdcard_read_line() start \r\n");
    while(1) {
        memset(dt, 0x00, sizeof(dt));
        ans = sdcard_read_line(&fp, dt, sizeof(dt)-1);
        if (ans != 0) {
            printf("%s\r\n", dt);
        } else {
            break;
        }
        __delay_ms(1);
    }
    printf("sdcard_test_read: sdcard_read_line() end \r\n");

    sdcard_close_file(&fp);
    printf("sdcard_test_read: sdcard_close_file() done \r\n");
}

static void sdcard_test_write(const unsigned char *filename)
{
    FILE_ACCESS_INFO_T fp;
	int ans;

    // 書き込むファイル／文字列
    const unsigned char *line1 = "PIC16F1938\r\n";
    const unsigned char *line2 = "microSD\r\n";
    const unsigned char *line3 = "Test write demo\r\n";
    const unsigned char *line4 = "[EOF]\r\n";

    ans = sdcard_open_file(&fp, filename, OP_READ_WRITE);
    printf("sdcard_test_write: sdcard_open_file(): ans=%d \r\n", ans);
    if (ans != 0) {
        printf("sdcard_test_write: file open error \r\n");
        return;
    }

    // ファイルへ１行ずつ書き込む
    ans = sdcard_write_line(&fp, line1, strlen(line1));
    ans = sdcard_write_line(&fp, line2, strlen(line2));
    ans = sdcard_write_line(&fp, line3, strlen(line3));
    ans = sdcard_write_line(&fp, line4, strlen(line4));

    sdcard_close_file(&fp);
    printf("sdcard_test_write: sdcard_close_file() done \r\n");
}

static void sdcard_test_append(const unsigned char *filename)
{
    FILE_ACCESS_INFO_T fp;
	int ans;

    // 書き込むファイル／文字列
    const unsigned char *line1 = "PIC16F1938\r\n";
    const unsigned char *line2 = "microSD\r\n";
    const unsigned char *line3 = "Test append demo\r\n";
    const unsigned char *line4 = "[EOF]\r\n";

    ans = sdcard_open_file(&fp, filename, OP_APPEND);
    printf("sdcard_test_append: sdcard_open_file(): ans=%d \r\n", ans);
    if (ans != 0) {
        // 新規書込みモードとしてオープンし直す
        ans = sdcard_open_file(&fp, filename, OP_READ_WRITE);
        if (ans != 0) {
            printf("sdcard_test_append: file open error \r\n");
            return;
        }
        printf("sdcard_test_append: file created \r\n");
    }

    // ファイルへ１行ずつ書き込む
    ans = sdcard_write_line(&fp, line1, strlen(line1));
    ans = sdcard_write_line(&fp, line2, strlen(line2));
    ans = sdcard_write_line(&fp, line3, strlen(line3));
    ans = sdcard_write_line(&fp, line4, strlen(line4));

    sdcard_close_file(&fp);
    printf("sdcard_test_append: sdcard_close_file() done \r\n");
}

void sdcard_test()
{
    // テストに使用するファイル名
    const unsigned char *writer = "WRITER.TXT";
    const unsigned char *appender = "APPENDER.TXT";

    // 常に同じ内容を書き込まないと、
    // 後ろに前回書き込んだ内容が残ってしまう可能性があります
    printf("Write to %s\r\n", writer);
    sdcard_test_write(writer);
    sdcard_test_read(writer);

    // すでに存在するファイルに追加書込み
    printf("Append to %s\r\n", appender);
    sdcard_test_append(appender);
    sdcard_test_read(appender);
}