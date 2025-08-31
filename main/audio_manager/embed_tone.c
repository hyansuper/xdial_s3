#include "embed_tone.h"

extern const uint8_t ff_16b_1c_44100hz_mp3[] asm("_binary_ff_16b_1c_44100hz_mp3_start");
extern const uint8_t ff_16b_1c_44100hz_mp3_end[] asm("_binary_ff_16b_1c_44100hz_mp3_end");

embed_item_info_t embed_tone_info[] = {
    [0] = {
        .address = ff_16b_1c_44100hz_mp3, 
        .size    = 231725,//ff_16b_1c_44100hz_mp3_end - ff_16b_1c_44100hz_mp3,  
    },
};

const char *embed_tone_url[] = {
    "embed://tone/0_ff_16b_1c_44100hz.mp3",
};

// 这里将音乐文件嵌入程序，仅作为测试，大文件放入单独的分区更合适