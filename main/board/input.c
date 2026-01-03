#include "board_config.h"
#include "board.h"

#include "driver/gpio.h"

void input_init_gpio() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1<<GPIO_LEFT_IR)|(1<<GPIO_RIGHT_IR),
        .pull_down_en = 0,
        .pull_up_en = IR_PULLUP,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1<<GPIO_LEFT_BUTTON)|(1<<GPIO_RIGHT_BUTTON);// |(1<<GPIO_BOOT)
    io_conf.pull_up_en = BUTTON_PULLUP;
    gpio_config(&io_conf);
    
}

