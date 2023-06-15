#ifndef __SYN8086_H__
#define __SYN8086_H__
#include "stm32h7xx_hal.h"

typedef enum
{
    SYN_SET_DEFAULT = 1,
    SYN_SET_VOICE = 3,
    SYN_SET_VOLUME = 10,
}syn_text_ctl_cmd_t;

typedef enum
{
    XIAO_LING = 3,
    YI_XIAO_JIAN = 51,
    YI_XIAO_QIANG = 52,
    TIAN_BEIBEI = 53,
    DUCK_TANG = 54,
    XIAO_YAN_ZI = 55,
}syn_voice_t;

typedef enum
{
    GB2312 = 0,             /* GB2312 */
    GBK = 1,                /* GBK */
    UNICODE_MOTOROLA = 3,   /* UNICODE1 */
    UNICODE_INTEL = 4,      /* UNICODE2 */
    UTF8 = 5                /* UTF-8 */
}syn_cmd_param_t;

typedef enum
{
    SYN_ACK_INIT_SUCCESS = 0x4A,
    SYN_ACK_OK = 0x41,
    SYN_ACK_FAIL = 0x45,
    SYN_ACK_BUSY = 0x4E,
    SYN_ACK_FREE = 0x4F
}syn_ack_code_t;

typedef enum
{
    SYN_PLAY = 1,
    SYN_STOP = 2,
    SYN_PAUSE = 3,
    SYN_RESTORE = 4,
    SYN_STATUS = 0x21,
    SYN_DEEPSLEEP = 0x22,
    SYN_WKUP = 0xFF
}syn_cmd_t;

int syn_play_audio(char *text, uint16_t text_len, char *type, uint16_t audio_len);
int syn_text_ctrl(uint8_t mark_ctl, uint16_t value);
int syn_ctl(syn_cmd_t cmd);

#endif

