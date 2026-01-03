#include "sdkconfig.h" 
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_indicators.h"
#include "valve_motor.h"
#include "limit_switch.h"
#include "valve_process.h"


// define Valve pins
#define MOTOR_EN_PIN        CONFIG_MOTOR_EN_PIN
#define MOTOR_IN1_PIN       CONFIG_MOTOR_IN1_PIN
#define MOTOR_IN2_PIN       CONFIG_MOTOR_IN2_PIN

#define CLOSE_LIMIT_PIN_A   CONFIG_CLOSE_LIMIT_PIN_A
#define CLOSE_LIMIT_PIN_B   CONFIG_CLOSE_LIMIT_PIN_B
#define OPEN_LIMIT_PIN_A    CONFIG_OPEN_LIMIT_PIN_A
#define OPEN_LIMIT_PIN_B    CONFIG_OPEN_LIMIT_PIN_B

#define RED_LED_PIN         CONFIG_RED_LED_PIN
#define GREEN_LED_PIN       CONFIG_GREEN_LED_PIN


Motor motor = { MOTOR_IN1_PIN, MOTOR_IN2_PIN, MOTOR_EN_PIN, 0 };
LimitSwitches closeLimit = { CLOSE_LIMIT_PIN_A, CLOSE_LIMIT_PIN_B };
LimitSwitches openLimit = { OPEN_LIMIT_PIN_A, OPEN_LIMIT_PIN_B };
LedIndicator redLED = { RED_LED_PIN };
LedIndicator greenLED = { GREEN_LED_PIN };



// motor_init(&motor);
// limit_switch_init(&closeLimit);
// limit_switch_init(&openLimit);

// led_init(&redLED);
// led_init(&greenLED);


static const char *TAG = "VALVE_PROCESS";


int valve_test(void)
{
    if (limit_switch_click(&closeLimit) == 0) {
        return 111;
    }
    if (limit_switch_click(&openLimit) == 0) {
        return 121;
    }
    return 0;
}


int motor_open(void) {
    unsigned long op_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
    const unsigned long op_timeout = 10000;
    int errorCode = 300;

    errorCode = valve_test();

    if (errorCode != 0) return errorCode;

    if (motor.state != 1) {
        bool turn_state = true;
        
        while (turn_state) {
            if (limit_switch_click(&openLimit) == 1) {
                turn_state = true;
            } else {
                ESP_LOGI(TAG, "Open limit switch clicked");
                motor_stop(&motor);
                errorCode = 0;
                turn_state = false;
            }

            if ( (xTaskGetTickCount() * portTICK_PERIOD_MS) - op_start > op_timeout) {
                motor_stop(&motor);
                errorCode = 331;
                turn_state = false;
            }

            motor_run_aclck(&motor, 200);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    motor_stop(&motor);
    
    if (errorCode == 0) {
        led_off(&redLED);
        ESP_LOGI(TAG, "motor is opened");
        motor.state = 1;
    } else {
        led_on(&redLED);
        ESP_LOGE(TAG, "motor open error: %d", errorCode);
    }
    
    return errorCode;
}


int motorClose(void) {
    unsigned long op_start = xTaskGetTickCount() * portTICK_PERIOD_MS;
    const unsigned long op_timeout = 10000;
    int errorCode = 200;

    errorCode = valve_test();

    if (errorCode != 0) return errorCode;

    if (motor.state != 10) {
        bool turn_state = true;

        while (turn_state) {
            if (limit_switch_click(&closeLimit) == 1) {
                turn_state = true;
            } else {
                ESP_LOGI(TAG, "Close limit switch clicked");
                motor_stop(&motor);
                errorCode = 0;
                turn_state = false;
            }

            if ( (xTaskGetTickCount() * portTICK_PERIOD_MS) - op_start > op_timeout) {
                motor_stop(&motor);
                errorCode = 231;
                turn_state = false;
            }

            motor_run_clk(&motor, 200);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    motor_stop(&motor);
    
    if (errorCode == 0) {
        led_off(&redLED);
        ESP_LOGI(TAG, "motor is closed");
        motor.state = 0;
    } else {
        led_on(&redLED);
        ESP_LOGE(TAG, "motor close error: %d", errorCode);
    }
    
    return errorCode;
}

// int valveSetPosition(int angle) {
//     if (angle == 90) return motorOpen();
//     if (angle == 0) return motorClose();
//     return 901;  // Invalid angle
// }



