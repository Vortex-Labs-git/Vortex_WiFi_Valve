#include <stdbool.h>
#include "driver/gpio.h"

#include "limit_switch.h"



void limit_switch_init(LimitSwitches *switches) {
    gpio_set_direction(switches->pinA, GPIO_MODE_INPUT);
    gpio_set_pull_mode(switches->pinA, GPIO_PULLUP_ONLY); 

    gpio_set_direction(switches->pinB, GPIO_MODE_INPUT);
    gpio_set_pull_mode(switches->pinB, GPIO_PULLUP_ONLY);
}

int limit_switch_click(LimitSwitches *switches) {
    int state;
    if (gpio_get_level(switches->pinA) == 1 && gpio_get_level(switches->pinB) == 0) {
        state = 10;  // clicked
    } else if (gpio_get_level(switches->pinA) == 0 && gpio_get_level(switches->pinB) == 1) {
        state = 1;   // not clicked
    } else {
        state = 0;   // error
    }
    return state;
}
