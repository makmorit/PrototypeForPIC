#include "common.h"
#include "adc2.h"

//
// A/D変換ポートから値（0-255の値）を取得
//
unsigned char adc2_conv()
{
    // 変換値を戻す
    //   変換値10bitのうち、上位8bitだけを取得
    return ADRESH;
}
