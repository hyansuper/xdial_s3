#include "lv_port.h"
#include "lv_port_config.h"

#include "driver/gpio.h"

// value of left/right button and infrared
static volatile int lbtn;
static volatile int rbtn;
static volatile int lir;
static volatile int rir;
static volatile int enc_step =2;

static lv_indev_t* button_indev, *encoder_indev;
static lv_obj_t* lv_lbtn, *lv_rbtn;

static encoder_indev_cb_t encoder_indev_cb;

static void read_button(lv_indev_t * indev, lv_indev_data_t * data) {
    // 如果之前有按钮被按下，应该先检查该按钮是否弹起
    static int last_pr_btn = -1;
    data->state = LV_INDEV_STATE_RELEASED;
    if(last_pr_btn == 0) {
        data->btn_id = 0;
        if(BUTTON_PRESS_LEVEL== (lbtn = gpio_get_level(GPIO_LEFT_BUTTON))) {
            data->state = LV_INDEV_STATE_PRESSED;
        } else {
            last_pr_btn = -1;
        }
    } else if(last_pr_btn == 1) {
        data->btn_id = 1;
        if(BUTTON_PRESS_LEVEL== (rbtn = gpio_get_level(GPIO_RIGHT_BUTTON))) {
            data->state = LV_INDEV_STATE_PRESSED;
        } else {
            last_pr_btn = -1;
        }
    } else if(BUTTON_PRESS_LEVEL== (lbtn=gpio_get_level(GPIO_LEFT_BUTTON))) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->btn_id = 0;
        last_pr_btn = 0;
    } else if(BUTTON_PRESS_LEVEL== (rbtn=gpio_get_level(GPIO_RIGHT_BUTTON))) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->btn_id = 1;
        last_pr_btn = 1;
    }
}

static void read_encoder(lv_indev_t* indev, lv_indev_data_t* data) {
    static int last_changed_ir;
    static int enc_diff_to_read;
    static int real_enc_diff;
    if(gpio_get_level(GPIO_LEFT_IR) != lir) {
        lir = !lir;
        if(last_changed_ir == 1) {
            last_changed_ir = 0;
            real_enc_diff += rir==lir? 1: -1;

            enc_diff_to_read += real_enc_diff / enc_step;
            real_enc_diff %= enc_step;
        }
    } else if(gpio_get_level(GPIO_RIGHT_IR) != rir) {
        rir = !rir;
        if(last_changed_ir == 0) {
            last_changed_ir = 1;
            real_enc_diff += rir==lir? -1: 1;

            enc_diff_to_read += real_enc_diff / enc_step;
            real_enc_diff %= enc_step;
        }
    }
    if(enc_diff_to_read) {
        if(encoder_indev_cb)
            encoder_indev_cb(enc_diff_to_read);
        data->enc_diff = enc_diff_to_read;
        enc_diff_to_read = 0;
    }
}

void lv_port_init_indev() {
	lbtn = gpio_get_level(GPIO_LEFT_BUTTON);
    rbtn = gpio_get_level(GPIO_RIGHT_BUTTON);
    lir = gpio_get_level(GPIO_LEFT_IR);
    rir = gpio_get_level(GPIO_RIGHT_IR);

    button_indev = lv_indev_create();
    lv_indev_set_type(button_indev, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(button_indev, read_button);

    encoder_indev = lv_indev_create();
    lv_indev_set_type(encoder_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(encoder_indev, read_encoder);
}


#define DEFAULT_BUTTON_MARGIN 5
const lv_point_t* get_default_button_points() {
    const static lv_point_t points_array[] = {{DEFAULT_BUTTON_MARGIN, DEFAULT_BUTTON_MARGIN}, {DISPLAY_WIDTH-DEFAULT_BUTTON_MARGIN, DEFAULT_BUTTON_MARGIN}};
    return points_array;
}

lv_obj_t* get_default_left_button() {
    return lv_lbtn;
}

lv_obj_t* get_default_right_button() {
    return lv_rbtn;
}

static lv_obj_t* create_default_lv_btn(int x, int y) {
    lv_obj_t* btn = lv_button_create(lv_layer_top());
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, DEFAULT_BUTTON_MARGIN*2, DEFAULT_BUTTON_MARGIN*2);
    return btn;
}

void lv_port_init_default_button() {
    lv_indev_set_button_points(button_indev, get_default_button_points());
    lv_lbtn = create_default_lv_btn(0, 0);
    lv_rbtn = create_default_lv_btn(DISPLAY_WIDTH-DEFAULT_BUTTON_MARGIN*2, 0);
}

lv_indev_t* get_encoder_indev() {
    return encoder_indev;
}
lv_indev_t* get_button_indev() {
    return button_indev;
}

void encoder_indev_set_step(int i) {
    if(i>0) enc_step =i;
}

int encoder_indev_get_step() {
    return enc_step;
}

void encoder_indev_set_group(lv_group_t* g) {
    // encoder_indev_cb = NULL;
    lv_indev_set_group(encoder_indev, g);
}

void encoder_indev_set_cb(encoder_indev_cb_t cb) {
    // lv_indev_set_group(encoder_indev, NULL);
    encoder_indev_cb = cb;
}