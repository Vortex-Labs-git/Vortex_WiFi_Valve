#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_net_stack.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif
#include "lwip/err.h"
#include "lwip/sys.h"

#include "websocket_server.h"


/* STA Configuration */
#define ESP_WIFI_STA_SSID                   CONFIG_ESP_WIFI_STA_SSID 
#define ESP_WIFI_STA_PASSWD                 CONFIG_ESP_WIFI_STA_PASSWD  
#define ESP_WIFI_STA_MAXIMUM_RETRY          CONFIG_ESP_WIFI_STA_MAXIMUM_RETRY


/* AP Configuration */
#define ESP_WIFI_AP_SSID                    CONFIG_ESP_WIFI_AP_SSID
#define ESP_WIFI_AP_PASSWD                  CONFIG_ESP_WIFI_AP_PASSWORD
#define ESP_WIFI_CHANNEL                    CONFIG_ESP_WIFI_AP_CHANNEL
#define MAX_STA_CONN                        CONFIG_ESP_MAX_STA_CONN_AP


static const char *TAG_AP = "WiFi SoftAP";
static const char *TAG_STA = "WiFi Sta";

static int s_ap_client_count = 0;



static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{

    // -----------------------------------------------------------
    // 1. ROUTER CONNECTION EVENTS
    // -----------------------------------------------------------
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG_STA, "STA Started. Connecting to Router...");
        esp_wifi_connect();
    }
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG_STA, "Router Disconnected/Not Found.");

        // If Router is lost, ensure AP is ON
        wifi_mode_t current_mode;
        esp_wifi_get_mode(&current_mode);
        
        if (current_mode == WIFI_MODE_STA) {
            ESP_LOGI(TAG_STA, "Switching to AP+STA mode (Turning AP ON)...");
            esp_wifi_set_mode(WIFI_MODE_APSTA);
        }

        if (s_ap_client_count == 0) {
            ESP_LOGI(TAG_STA, "Retrying Router connection...");
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_wifi_connect();
        } else {
            ESP_LOGI(TAG_STA, "AP is busy. NOT searching for router.");
        }

    }
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_STA, "Connected to Router! IP: " IPSTR, IP2STR(&event->ip_info.ip));

        // Router found. Turn OFF the AP.
        ESP_LOGI(TAG_STA, "Router connected. Switching to STA Mode (Turning AP OFF)...");
        esp_wifi_set_mode(WIFI_MODE_STA);
    }

    // -----------------------------------------------------------
    // 2. SOFTAP CLIENT EVENTS
    // -----------------------------------------------------------
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG_AP, "Client joined AP: "MACSTR, MAC2STR(event->mac));
        start_webserver();
        s_ap_client_count++;

        // A client is using the AP. Stop distracting the radio with STA scans.
        ESP_LOGI(TAG_AP, "Client connected. Stopping Router search (STA Idle).");
        esp_wifi_disconnect(); 
    }
    
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG_AP, "Client left AP: "MACSTR, MAC2STR(event->mac));
        stop_webserver();
        if (s_ap_client_count > 0) s_ap_client_count--;

        // AP is free. Go back to looking for the Router.
        if (s_ap_client_count == 0) {
            ESP_LOGI(TAG_AP, "No clients on AP. Resuming Router search...");
            esp_wifi_connect();
        }
    }

}


// Config SoftAP
esp_netif_t *wifi_init_softap(void)
{
    // Create netifs
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = ESP_WIFI_AP_SSID,
            .ssid_len = strlen(ESP_WIFI_AP_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_AP_PASSWD,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    if (strlen(ESP_WIFI_AP_PASSWD) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d", ESP_WIFI_AP_SSID, ESP_WIFI_AP_PASSWD, ESP_WIFI_CHANNEL);

    return esp_netif_ap;
}

// Config Router (STA)
esp_netif_t *wifi_init_sta(void)
{
    // Create netifs
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = ESP_WIFI_STA_SSID,
            .password = ESP_WIFI_STA_PASSWD,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config) );

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");

    return esp_netif_sta;
}


void wifi_init_smart_mode(void)
{
    // Register Event handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &wifi_event_handler,
                    NULL,
                    NULL));

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // START IN AP+STA MODE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // Initialize AP
    ESP_LOGI(TAG_AP, "ESP_WIFI_MODE_AP");
    esp_netif_t *esp_netif_ap = wifi_init_softap();

    // Initialize STA
    ESP_LOGI(TAG_STA, "ESP_WIFI_MODE_STA");
    esp_netif_t *esp_netif_sta = wifi_init_sta();

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start() );


}


void app_main(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // NVS Init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_smart_mode();
}