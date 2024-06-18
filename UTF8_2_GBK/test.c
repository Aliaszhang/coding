#include "utf8_gbk.h"

int utf8_unicode_gbk_test(void) {
    uint8_t utf8[] = {"一二三四五"};
    uint16_t gbk[100] = {0};
    int len;

    len = utf8_2_gbk(utf8, sizeof(utf8), gbk);

    MAIN_DEBUG("UTF-8 to GBK: len = %d: %s\n", len , gbk);

    for (int i = 0; i < len; i++) {
        rt_kprintf("%X ", gbk[i]);
    }
    rt_kprintf("\r\n");

    return 0;
}