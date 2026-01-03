#ifndef WEBSOCKET_SERVER_FN_H
#define WEBSOCKET_SERVER_FN_H

#include "esp_http_server.h"


// Global Web Socket server handle
extern httpd_handle_t esp_server;
extern bool connection_authorized;

httpd_handle_t start_webserver(void);
void stop_webserver(void);
void websocket_async_send(void *arg);

#endif