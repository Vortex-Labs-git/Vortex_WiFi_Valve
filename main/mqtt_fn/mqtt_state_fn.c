#include <string.h>
#include <stdbool.h>
#include "cJSON.h"
#include "esp_log.h"

#include "time_func.h"
#include "global_var.h"
#include "mqtt_state_fn.h"

#define DEVICE_ID CONFIG_WIFI_VALVE_ID

static const char *TAG = "MQTT_STATE";



// Handle incoming cmd_data MQTT message
void mqtt_handle_cmd_data(const char *data) {
    cJSON *json_cmd_data = cJSON_Parse(data);
    if (json_cmd_data == NULL) {
        ESP_LOGE(TAG, "Invalid JSON received");
        return;
    }

    cJSON *event = cJSON_GetObjectItem(json_cmd_data, "event");
    cJSON *device_id = cJSON_GetObjectItem(json_cmd_data, "device_id");
    cJSON *ota_update = cJSON_GetObjectItem(json_cmd_data, "ota_update");

    cJSON *set_controller = cJSON_GetObjectItem(json_cmd_data, "set_controller");
    if (cJSON_IsObject(set_controller)) {
        cJSON *schedule = cJSON_GetObjectItem(set_controller, "schedule");
        cJSON *sensor = cJSON_GetObjectItem(set_controller, "sensor");

        if (cJSON_IsBool(schedule) && cJSON_IsBool(sensor)) {
            serverData.schedule_control = cJSON_IsTrue(schedule);
            serverData.sensor_control = cJSON_IsTrue(sensor);
        }
    }

    cJSON *valve_data = cJSON_GetObjectItem(json_cmd_data, "valve_data");
    if (cJSON_IsObject(valve_data)) {
        cJSON *name = cJSON_GetObjectItem(valve_data, "name");
        cJSON *set_angle = cJSON_GetObjectItem(valve_data, "set_angle");
        cJSON *angle = cJSON_GetObjectItem(valve_data, "angle");

        if (cJSON_IsBool(set_angle) && cJSON_IsNumber(angle)) {
            serverData.set_angle = cJSON_IsTrue(set_angle);
            serverData.angle = angle->valueint;
        }

    }

    cJSON_Delete(json_cmd_data);
}


// Handle incoming control_data MQTT message
void mqtt_handle_control_data(const char *data) {
    cJSON *json_control_data = cJSON_Parse(data);
    if (json_control_data == NULL) {
        ESP_LOGE(TAG, "Invalid JSON received");
        return;
    }

    cJSON *event = cJSON_GetObjectItem(json_control_data, "event");
    cJSON *device_id = cJSON_GetObjectItem(json_control_data, "device_id");
    
    cJSON *set_controllerdata = cJSON_GetObjectItem(json_control_data, "set_controllerdata");
    if (cJSON_IsObject(set_controllerdata)) {
        
        cJSON *schedule = cJSON_GetObjectItem(set_controllerdata, "schedule");
        cJSON *sensor = cJSON_GetObjectItem(set_controllerdata, "sensor");

        if (cJSON_IsBool(schedule) && cJSON_IsBool(sensor)) {
            serverControl.schedule_control = cJSON_IsTrue(schedule);
            serverControl.sensor_control = cJSON_IsTrue(sensor);
        }
    }

    cJSON *set_scheduledata = cJSON_GetObjectItem(json_control_data, "set_scheduledata");
    if (cJSON_IsObject(set_scheduledata)) {
        
        cJSON *set_schedule = cJSON_GetObjectItem(set_scheduledata, "set_schedule");
        if (cJSON_IsBool(set_schedule)) {
            serverControl.set_schedule = cJSON_IsTrue(set_schedule);
        }

        cJSON *schedule_info = cJSON_GetObjectItem(set_scheduledata, "schedule_info");
        if (cJSON_IsArray(schedule_info)) {
            int schedule_count = cJSON_GetArraySize(schedule_info);
            for (int i = 0; i < schedule_count && i < 10; i++) {

                cJSON *schedule_item = cJSON_GetArrayItem(schedule_info, i);
                if (cJSON_IsObject(schedule_item)) {

                    cJSON *day = cJSON_GetObjectItem(schedule_item, "day");
                    cJSON *time = cJSON_GetObjectItem(schedule_item, "time");
                    cJSON *process = cJSON_GetObjectItem(schedule_item, "process");
                    if (cJSON_IsString(day) && cJSON_IsString(time) && cJSON_IsString(process)) {
                        strncpy(serverControl.schedule_info[i].day, day->valuestring, sizeof(serverControl.schedule_info[i].day) - 1);
                        strncpy(serverControl.schedule_info[i].time, time->valuestring, sizeof(serverControl.schedule_info[i].time) - 1);
                        strncpy(serverControl.schedule_info[i].process, process->valuestring, sizeof(serverControl.schedule_info[i].process) - 1);
                    }
                }
            }
        }
    }

    cJSON *set_sensordata = cJSON_GetObjectItem(json_control_data, "set_sensordata");
    if (cJSON_IsObject(set_sensordata)) {

        cJSON *upper_limit = cJSON_GetObjectItem(set_sensordata, "upper_limit");
        cJSON *lower_limit = cJSON_GetObjectItem(set_sensordata, "lower_limit");
        if (cJSON_IsNumber(upper_limit) && cJSON_IsNumber(lower_limit)) {
            serverControl.sensor_upper_limit = upper_limit->valueint;
            serverControl.sensor_lower_limit = lower_limit->valueint;
        }
    }

    cJSON_Delete(json_control_data);
    
}



// Create JSON object for valve status
cJSON* create_valve_status() {
    cJSON *json = cJSON_CreateObject();

    char timestamp[20];
    get_current_timestamp(timestamp, sizeof(timestamp));

    cJSON_AddStringToObject(json, "event", "valve_status");
    cJSON_AddStringToObject(json, "timestamp", timestamp);
    cJSON_AddStringToObject(json, "device_id", DEVICE_ID);
    cJSON_AddStringToObject(json, "status", "online");

    return json;
}

// Create JSON object for valve state data
cJSON* create_valve_state_data() {
    cJSON *json = cJSON_CreateObject();

    char timestamp[20];
    get_current_timestamp(timestamp, sizeof(timestamp));

    cJSON_AddStringToObject(json, "event", "valve_basic_data");
    cJSON_AddStringToObject(json, "timestamp", timestamp);
    cJSON_AddStringToObject(json, "device_id", DEVICE_ID);

    cJSON *controller_data = cJSON_CreateObject();
    cJSON_AddBoolToObject(controller_data, "schedule", valveData.schedule_control);
    cJSON_AddBoolToObject(controller_data, "sensor", valveData.sensor_control);
    cJSON_AddItemToObject(json, "get_controller", controller_data);

    cJSON *valve_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(valve_data, "angle", valveData.angle);
    cJSON_AddBoolToObject(valve_data, "is_open", valveData.is_open);
    cJSON_AddBoolToObject(valve_data, "is_close", valveData.is_close);
    cJSON_AddItemToObject(json, "get_valvedata", valve_data);

    cJSON *limit_data = cJSON_CreateObject();
    cJSON_AddBoolToObject(limit_data, "is_open_limit", valveData.open_limit_available);
    cJSON_AddBoolToObject(limit_data, "open_limit", valveData.open_limit_click);
    cJSON_AddBoolToObject(limit_data, "is_close_limit", valveData.close_limit_available);
    cJSON_AddBoolToObject(limit_data, "close_limit", valveData.close_limit_click);
    cJSON_AddItemToObject(json, "get_limitdata", limit_data);

    return json;
}

// Create JSON object for valve error
cJSON* create_valve_error() {
    cJSON *json = cJSON_CreateObject();

    char timestamp[20];
    get_current_timestamp(timestamp, sizeof(timestamp));

    cJSON_AddStringToObject(json, "event", "valve_status");
    cJSON_AddStringToObject(json, "timestamp", timestamp);
    cJSON_AddStringToObject(json, "device_id", DEVICE_ID);
    cJSON_AddStringToObject(json, "error", valveData.error_msg);

    return json;
}

