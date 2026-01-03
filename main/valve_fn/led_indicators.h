#ifndef LED_INDICATORS_H
#define LED_INDICATORS_H

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK,
    LED_MODE_BLINK2
} led_mode_t;


typedef struct {
    uint8_t pin;
} LedIndicator;


void led_init(LedIndicator *led);

void led_on(LedIndicator *led);
void led_off(LedIndicator *led);
void led_blink(LedIndicator *led, uint32_t period_ms);
void led_blink2(LedIndicator *led, uint32_t on_ms, uint32_t off_ms);


#endif // LED_INDICATORS_H