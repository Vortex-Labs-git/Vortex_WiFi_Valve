
#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"


#define MQTT_BROKER_URI "mqtt://192.168.1.100"
#define DEVICE_ID "dev0016"
#define BASE_TOPIC "vortex_device/wifi_valve/" DEVICE_ID


static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t mqtt_client;


// MQTT Event Handler Callback
static esp_err_t mqtt_event_handler_callback(esp_mqtt_event_handle_t event)
{
    char topic[128];
    char data[512];

    switch (event->event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");

            // publish_device_status("online");
            // snprintf(topic, sizeof(topic), "%s/cmd_data", BASE_TOPIC);
            // esp_mqtt_client_subscribe(mqtt_client, topic, 1);
            // snprintf(topic, sizeof(topic), "%s/control_data", BASE_TOPIC);
            // esp_mqtt_client_subscribe(mqtt_client, topic, 1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            memset(topic, 0, sizeof(topic));
            memcpy(topic, event->topic, event->topic_len);

            memset(data, 0, sizeof(data));
            memcpy(data, event->data, event->data_len);

            ESP_LOGI(TAG, "MQTT_EVENT_DATA Topic: %s, Data: %s", topic, data);

            cJSON *root = cJSON_Parse(data);
            if (!root) 
                break;
            // else if (strstr(topic, "/cmd_data"))
            //     handle_cmd_data(root);
            // else if (strstr(topic, "/control_data"))
            //     handle_control_data(root);

            cJSON_Delete(root);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}


static void mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_URI,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler_callback, NULL);
    esp_mqtt_client_start(mqtt_client);
}




// #include <stdio.h>
// #include <string.h>
// #include <time.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"

// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "nvs_flash.h"

// #include "mqtt_client.h"
// #include "cJSON.h"
// #include "esp_sntp.h"


// /* ================= CONFIG ================= */

// #define MQTT_BROKER_URI "mqtt://192.168.1.100"

// #define DEVICE_ID "dev0016"
// #define BASE_TOPIC "vortex_device/wifi_valve/" DEVICE_ID

// /* ========================================= */

// static const char *TAG = "MAIN";
// static esp_mqtt_client_handle_t mqtt_client;
// static uint32_t seq_num = 0;

// #define WIFI_CONNECTED_BIT BIT0


// /* ================= MQTT ================= */

// static void handle_cmd_data(cJSON *root)
// {
//     cJSON *valve = cJSON_GetObjectItem(root, "valve_data");
//     if (!valve) return;

//     cJSON *set_angle = cJSON_GetObjectItem(valve, "set_angle");
//     cJSON *angle = cJSON_GetObjectItem(valve, "angle");

//     if (cJSON_IsBool(set_angle) && cJSON_IsNumber(angle)) {
//         ESP_LOGI(TAG, "CMD: Set valve angle to %d", angle->valueint);
//         /* 👉 Call valve motor control here */
//     }
// }

// static void handle_control_data(cJSON *root)
// {
//     cJSON *schedule = cJSON_GetObjectItem(root, "set_sheduledata");
//     if (!schedule) return;

//     cJSON *enable = cJSON_GetObjectItem(schedule, "set_shedule");
//     if (cJSON_IsBool(enable)) {
//         ESP_LOGI(TAG, "CONTROL: Schedule %s",
//                  cJSON_IsTrue(enable) ? "ENABLED" : "DISABLED");
//     }
// }

// static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
// {
//     char topic[128];
//     char data[512];

//     switch (event->event_id) {

//     case MQTT_EVENT_CONNECTED:
//         ESP_LOGI(TAG, "MQTT connected");

//         publish_device_status("online");

//         snprintf(topic, sizeof(topic), "%s/cmd_data", BASE_TOPIC);
//         esp_mqtt_client_subscribe(mqtt_client, topic, 1);

//         snprintf(topic, sizeof(topic), "%s/control_data", BASE_TOPIC);
//         esp_mqtt_client_subscribe(mqtt_client, topic, 1);
//         break;

//     case MQTT_EVENT_DATA:
//         memset(topic, 0, sizeof(topic));
//         memcpy(topic, event->topic, event->topic_len);

//         memset(data, 0, sizeof(data));
//         memcpy(data, event->data, event->data_len);

//         ESP_LOGI(TAG, "RX Topic: %s", topic);
//         ESP_LOGI(TAG, "RX Data: %s", data);

//         cJSON *root = cJSON_Parse(data);
//         if (!root) break;

//         if (strstr(topic, "/cmd_data"))
//             handle_cmd_data(root);
//         else if (strstr(topic, "/control_data"))
//             handle_control_data(root);

//         cJSON_Delete(root);
//         break;

//     default:
//         break;
//     }
//     return ESP_OK;
// }

// static void mqtt_start(void)
// {
//     esp_mqtt_client_config_t mqtt_cfg = {
//         .uri = MQTT_BROKER_URI,
//     };

//     mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID,
//                                    mqtt_event_handler_cb, NULL);
//     esp_mqtt_client_start(mqtt_client);
// }

// static void publish_device_status(const char *status)
// {
//     char topic[128], ts[32];
//     snprintf(topic, sizeof(topic), "%s/status", BASE_TOPIC);
//     get_iso_timestamp(ts, sizeof(ts));

//     cJSON *root = cJSON_CreateObject();
//     cJSON_AddStringToObject(root, "event", "valve_status");
//     cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
//     cJSON_AddStringToObject(root, "status", status);
//     cJSON_AddStringToObject(root, "timestamp", ts);

//     char *payload = cJSON_PrintUnformatted(root);

//     esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 1);

//     cJSON_Delete(root);
//     free(payload);
// }

// static void publish_state_data(void)
// {
//     char topic[128], ts[32];
//     snprintf(topic, sizeof(topic), "%s/state_data", BASE_TOPIC);
//     get_iso_timestamp(ts, sizeof(ts));

//     cJSON *root = cJSON_CreateObject();
//     cJSON_AddStringToObject(root, "event", "valve_basic_data");
//     cJSON_AddStringToObject(root, "timestamp", ts);
//     cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
//     cJSON_AddNumberToObject(root, "seq", seq_num++);

//     cJSON *controller = cJSON_CreateObject();
//     cJSON_AddBoolToObject(controller, "schedule", true);
//     cJSON_AddBoolToObject(controller, "sensor", false);
//     cJSON_AddItemToObject(root, "get_controller", controller);

//     cJSON *valve = cJSON_CreateObject();
//     cJSON_AddNumberToObject(valve, "angle", 45);
//     cJSON_AddBoolToObject(valve, "is_open", true);
//     cJSON_AddBoolToObject(valve, "is_closed", false);
//     cJSON_AddItemToObject(root, "get_valvedata", valve);

//     cJSON *limit = cJSON_CreateObject();
//     cJSON_AddBoolToObject(limit, "open_limit_reached", true);
//     cJSON_AddBoolToObject(limit, "close_limit_reached", false);
//     cJSON_AddItemToObject(root, "get_limitdata", limit);

//     cJSON_AddStringToObject(root, "error", "");

//     char *payload = cJSON_PrintUnformatted(root);

//     esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 0, 0);

//     cJSON_Delete(root);
//     free(payload);
// }


// /* ================= TASK ================= */

// static void publish_task(void *arg)
// {
//     while (1) {
//         publish_state_data();
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }


// // obtain_time();
// // mqtt_start();

// // xTaskCreate(publish_task, "publish_task", 4096, NULL, 5, NULL);