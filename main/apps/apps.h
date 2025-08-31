#pragma once

#include "lv_app.h"
#include "lv_port.h"
#include "sntp_srv.h"
#include "wifi_manager.h"
#include "lv_observers.h"

#include "assets/imgs/imgs.h"
#include "assets/fonts/fonts.h"

#include "string.h"

// declare all apps
extern lv_app_t home_app, clock_app, settings_app, crypto_app, player_app, example_app;

void back_to_home_app(lv_event_t * e);


