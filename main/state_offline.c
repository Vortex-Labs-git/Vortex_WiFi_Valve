#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "cJSON.h"

#include "state_offline.h"
#include "time_func.h"

static const char *TAG = "STATE UPDATE OFFLINE";


/* Example runtime variables */
static const char *user_id    = "useer001";
static const char *valve_id  = "dev0016";
static const char *device_name = "home valve";
/* Controller data */
bool schedule_enabled = true;
bool sensor_enabled   = false;
/* Valve data */
int  valve_angle = 45;
bool valve_open  = true;
bool valve_close = false;
/* Limit switch data */
bool is_open_limit  = true;
bool open_limit     = false;
bool is_close_limit = false;
bool close_limit    = true;
/* Error */
char *error_msg = "";


// Send device information
void send_device_info(void) {
    if (esp_server == NULL) return;

    char timestamp[20];
    get_current_timestamp(timestamp, sizeof(timestamp));

    // Create the JSON object
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "event", "device_info");
    cJSON_AddStringToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "device_id", valve_id);

    // Convert the JSON object to string (allocate memory)
    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // Send via WebSocket
    if (httpd_queue_work(esp_server, websocket_async_send, json_string) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to queue work");
        free(json_string);
    }
}


// Send valve basic data
void send_device_data(void) {
    if (esp_server == NULL) return;

    char timestamp[25];
    get_current_timestamp(timestamp, sizeof(timestamp));

    // Root object
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "event", "valve_data");
    cJSON_AddStringToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "device_id", valve_id);

    // get_controller object
    cJSON *controller = cJSON_CreateObject();
    cJSON_AddBoolToObject(controller, "schedule", schedule_enabled);
    cJSON_AddBoolToObject(controller, "sensor", sensor_enabled);
    cJSON_AddItemToObject(root, "get_controller", controller);

    // get_valvedata object
    cJSON *valvedata = cJSON_CreateObject();
    cJSON_AddNumberToObject(valvedata, "angle", valve_angle);
    cJSON_AddBoolToObject(valvedata, "is_open", valve_open);
    cJSON_AddBoolToObject(valvedata, "is_close", valve_close);
    cJSON_AddItemToObject(root, "get_valvedata", valvedata);

    // get_limitdata object
    cJSON *limitdata = cJSON_CreateObject();
    cJSON_AddBoolToObject(limitdata, "is_open_limit", is_open_limit);
    cJSON_AddBoolToObject(limitdata, "open_limit", open_limit);
    cJSON_AddBoolToObject(limitdata, "is_close_limit", is_close_limit);
    cJSON_AddBoolToObject(limitdata, "close_limit", close_limit);
    cJSON_AddItemToObject(root, "get_limitdata", limitdata);

    // Error
    cJSON_AddStringToObject(root, "Error", error_msg);

    /* Convert to string */
    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (json_string == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON string");
        return;
    }

    /* Send asynchronously via WebSocket */
    if (httpd_queue_work(esp_server, websocket_async_send, json_string) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to queue websocket work");
        free(json_string);
    }
}


// offline data communication
void offline_data(cJSON *event, cJSON *root) {
    if ( strcmp(event->valuestring, "device_basic_info") == 0) {
        ESP_LOGI(TAG, "Event matched: device_basic_info");

        cJSON *data = cJSON_GetObjectItem(root, "data");
        if (data != NULL && cJSON_IsObject(data)) {

            cJSON *device_id = cJSON_GetObjectItem(data, "device_id");
            if (device_id != NULL && cJSON_IsString(device_id)) {
                if (strcmp(device_id->valuestring, valve_id) == 0) {
                    ESP_LOGW(TAG, "User send the correct device ID");
                    send_device_data();
                    ESP_LOGW(TAG, "Send valve data");
                } else {
                    ESP_LOGW(TAG, "User dont send the correct device ID");
                }
            } else {
                ESP_LOGW(TAG, "\"device_id\" is false or missing");
            }
        } else {
            ESP_LOGW(TAG, "\"data\" is false or missing");
        }
    } else if ( strcmp(event->valuestring, "set_valve_basic") == 0) {
        ESP_LOGI(TAG, "Event matched: set_valve_basic");

        cJSON *valve_data = cJSON_GetObjectItem(root, "valve_data");
        if (valve_data != NULL && cJSON_IsObject(valve_data)) {

            cJSON *set_angle = cJSON_GetObjectItem(valve_data, "set_angle");
            if (set_angle != NULL && cJSON_IsBool(set_angle) && set_angle->valueint == 1) {
                
                cJSON *angle = cJSON_GetObjectItem(valve_data, "angle");
                if (angle != NULL && cJSON_IsNumber(angle)) {
                    ESP_LOGI(TAG, "Angle: %d", angle->valueint);

                } else {
                    ESP_LOGW(TAG, "Angle field is missing or not a number");
                }
            } else {
                ESP_LOGW(TAG, "\"set_angle\" is false or missing");
            }
        } else {
            ESP_LOGW(TAG, "\"valve_data\" field is missing or not an object");
        }

    }
    else if ( strcmp(event->valuestring, "set_valve_wifi") == 0) {
        ESP_LOGI(TAG, "Event matched: set_valve_wifi");

        cJSON *wifi_data = cJSON_GetObjectItem(root, "wifi_data");
        if (wifi_data != NULL && cJSON_IsObject(wifi_data)) {
            cJSON *ssid = cJSON_GetObjectItem(wifi_data, "ssid");
            cJSON *password = cJSON_GetObjectItem(wifi_data, "password");
            ESP_LOGI(TAG, "ssid: %s, password: %s", ssid->valuestring, password->valuestring);

        } else {
            ESP_LOGW(TAG, "\"wifi_data\" field is missing or not an object");
        }
    }
    else {
        ESP_LOGW(TAG, "Event type does not match: %s", event->valuestring);
    }
}


// Extract the json msg
void process_message(const char *payload, bool *connection_authorized) {

    // Parse the JSON string into a cJSON object
    cJSON *root = cJSON_Parse(payload);
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return;
    }

    // Extract the "event" field
    cJSON *event = cJSON_GetObjectItem(root, "event");
    if (event == NULL || !cJSON_IsString(event)) {
        ESP_LOGW(TAG, "\"event\" field is missing in the JSON message");
        cJSON_Delete(root);
        return;
    }


    if ( *connection_authorized) {
        offline_data( event, root);
    } else {
        if (strcmp(event->valuestring, "request_device_info") == 0) {
            ESP_LOGI(TAG, "Event matched: request_device_info");

            cJSON *passkey   = cJSON_GetObjectItem(root, "passkey");
            if ( cJSON_IsString(passkey) && (strcmp(passkey->valuestring, "12345") == 0)) {
                *connection_authorized = true;
                ESP_LOGI(TAG, "Passkey accept");

                send_device_info();
                ESP_LOGI(TAG, "Send Device info");
            } else {
                *connection_authorized = false;
                ESP_LOGI(TAG, "Passkey not accept");
            }
        } else {
            ESP_LOGW(TAG, "Connection not authorized");
        }
    }

    // Free memory to prevent leaks
    cJSON_Delete(root);
}


// void websocket_publish_