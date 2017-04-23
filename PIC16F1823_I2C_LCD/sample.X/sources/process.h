#ifndef __PROCESS_H
#define __PROCESS_H

//
// タクトスイッチ
//
#define BUTTON_0 RA3
//
// ヘルスチェックLED
//
#define HCHECK_LED RC3

//
// 関数
//
void process_init();
void process();

#endif // __PROCESS_H
