#include "SYN8086.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t syn_special_cmd_prefix[] = { // 命令格式为 [m3]
/* 0  */    'b', // 0 设置标点符号朗读: 0 – 不读标点符号; 1 – 读标点符号
/* 1  */    'd', // 1 恢复默认的合成参数: 所有设置（除发音人设置外）恢复为默认值
/* 2  */    'f', // 2 设置发音风格: 0 - 一字一顿;  1 - 平铺直叙
/* 3  */    'm', // 3 选择发音人: (*= 3, 51, 52, 53, 54, 55)， 默认3
/* 4  */    'i', // 4 设置识别汉语拼音: 0 - 不识别汉语拼音;  1 - 识别汉语拼音
/* 5  */    'n', // 5 设置数字处理策略: 0 - 自动判断;  1 - 数字作号码处理;  2 - 数字作数值处理
/* 6  */    'p', // 6 静音一段时间： 插入静音的时间长度，单位：毫秒(ms)
/* 7  */    'r', // 7 设置姓名读音策略: 0 - 自动判断姓名读音;  1 - 强制以后的每句开头字均按姓氏读音;  2 - 强制紧跟句的开头字按姓氏读音
/* 8  */    's', // 8 设置语速: 语速值（0至30）语速值越小，语速越慢
/* 9  */    't', // 9 设置语调: 语调值（0至10）语调值越小，基频值越低
/* 10 */    'v', // 10 设置音量: 音量值（0至10）音量的调节范围为静音到音频设备支持的最大值
/* 11 */    'x', // 11 设置提示音处理策略: 0 - 不使用提示音;  1 - 自动使用提示音
/* 12 */    'y', // 12 设置号码中"1"的读法: 0 - 合成号码时"1"读成"幺";  1 - 合成号码时"1"读成"一"
/* 13 */    'z', // 13 设置韵律标注处理策略: 0 - 不处理韵律标注;  1 - 处理韵律标注(使用“#”标出韵律短语划分位置,使用“*”标出韵律词划分位置)
/* 14 */    '=', // 14 为单个汉字强制拼音: 为前一个汉字强制设定的拼音(拼音模式为：1至6位字母 + 1位数字(1:阴平 2:阳平 3:上声 4:去声 5:轻声))
};

extern UART_HandleTypeDef UART2_Handler;
extern uint8_t syn_rx_buffer;

static int syn_send_message(syn_cmd_t cmd, uint8_t cmd_parma, char *text, uint16_t text_len)
{
    uint8_t *syn_frame = NULL;
    uint16_t frame_len = 0;
    int ret = -1;
    uint16_t time_count = 0;

    frame_len = 4;
    if (cmd == SYN_PLAY)
    {
        if (text_len > 0 && text != NULL)
        {
            frame_len += (text_len + 1);
        }
        else
        {
            return -1;
        }
    }

    syn_frame = (uint8_t *)malloc(frame_len);
    memset(syn_frame, 0, frame_len);
    if (syn_frame == NULL)
    {
        printf("malloc syn buffer failed\r\n");
        return -1;
    }

    syn_frame[0] = 0xFD;
    syn_frame[1] = ((frame_len - 3) >> 8) & 0xFF;
    syn_frame[2] = (frame_len - 3) & 0xFF;
    syn_frame[3] = cmd;
    if (cmd == SYN_PLAY)
    {
        syn_frame[4] = cmd_parma;
        memcpy(syn_frame + 5, text, text_len);
    }
    syn_rx_buffer = 0;
    ret = HAL_UART_Transmit(&UART2_Handler, syn_frame, frame_len, 0x1000);

    free(syn_frame);
    while (syn_rx_buffer != SYN_ACK_OK && syn_rx_buffer != SYN_ACK_INIT_SUCCESS)
    {
        if (time_count++ > 100)
        {
            ret = -1;
            break;
        }
        HAL_Delay(10);
    }
    return ret;
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  text a pointer to the text whick need to play
  * @param  text_len the len of text
  * @param  type indicate eccode type,such as utf-8
  * @param  audio_len the time len of audio, unit is 100ms
  * @retval 0: success 
  */
int syn_play_audio(char *text, uint16_t text_len, char *type, uint16_t audio_len)
{
    syn_cmd_param_t param = UTF8;
    uint16_t time_count = 0;
    int ret = 0;

    if (text == NULL || text_len == 0)
    {
        printf("input param is wrong\r\n");
        return -1;
    }

    if (type == NULL || strncmp(type, "UTF-8", 5) == 0)
    {
        param = UTF8;
    }
    else if (strncmp(type, "GB2312", 6) == 0)
    {
        param = GB2312;
    }
    else if (strncmp(type, "GBK", 3) == 0)
    {
        param = GBK;
    }
    else if (strncmp(type, "UNICODE1", 8) == 0)
    {
        param = UNICODE_MOTOROLA;
    }
    else if (strncmp(type, "UNICODE2", 8) == 0)
    {
        param = UNICODE_INTEL;
    }

    ret = syn_send_message(SYN_PLAY, param, text, text_len);
    if (ret != 0)
    {
        return ret;
    }

    syn_rx_buffer = 0;

    while (syn_rx_buffer != SYN_ACK_FREE)
    {
        if (time_count++ > audio_len)
        {
            ret = -1;
            break;
        }
        HAL_Delay(100);
    }

    return ret;
}

int syn_text_ctrl(uint8_t mark_ctl, uint16_t value)
{
    char cmd[64] = {0};
    uint16_t cmd_len = 0;

    if (mark_ctl >= 14)
    {
        return -1;
    }

    if (mark_ctl == 1)
    {
        cmd_len = snprintf(cmd, 64, "[%c]", syn_special_cmd_prefix[mark_ctl]);
    }
    else
    {
        cmd_len = snprintf(cmd, 64, "[%c%d]", syn_special_cmd_prefix[mark_ctl], value);
    }
    return syn_send_message(SYN_PLAY, GBK, cmd, cmd_len);
}

int syn_ctl(syn_cmd_t cmd)
{
    return syn_send_message(cmd, 0xFF, NULL, 0);
}


