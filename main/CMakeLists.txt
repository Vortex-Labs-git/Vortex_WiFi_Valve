idf_component_register(SRCS 
                            "softap_sta.c"
                            "websocket_server.c"
                            "state_offline.c"
                            "time_func.c"
                            "mqtt_client_fn.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi mqtt nvs_flash json esp_http_server esp_event)
