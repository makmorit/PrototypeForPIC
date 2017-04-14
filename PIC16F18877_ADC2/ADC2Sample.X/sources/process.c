#include "main.h"
#include "device.h"
#include "process.h"

// 約 1.0 秒ごとに処理（3.2768ms × 305回）
void process_on_one_second()
{
    // for test: ヘルスチェックLED
    RA1 = ~RA1;
}

// 初期処理
void process_init()
{
}
