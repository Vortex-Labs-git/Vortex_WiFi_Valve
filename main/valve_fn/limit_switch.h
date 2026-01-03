#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include <stdint.h>


typedef struct {
    const uint8_t pinA;
    const uint8_t pinB;
} LimitSwitches;

void limit_switch_init(LimitSwitches *switches);
int limit_switch_click(LimitSwitches *switches);


#endif // LIMIT_SWITCH_H
