#pragma once
#include "lvgl.h"


#define __OBSERVER_LIST() \
					datetime_sub,\
                    wifi_sub,\
                    crypto_sub,\
                    weather_sub


extern lv_subject_t __OBSERVER_LIST();
