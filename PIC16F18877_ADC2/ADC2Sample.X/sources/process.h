#ifndef __PROCESS_H
#define __PROCESS_H

// タクトスイッチ
#define BUTTON_0 RE3

// ヘルスチェックLED
#define HCHECK_LED RA1

//
// 関数
//
void process_on_one_second();
void process_init();
void switch_detection();
void switch_prevent();

#endif // __PROCESS_H
