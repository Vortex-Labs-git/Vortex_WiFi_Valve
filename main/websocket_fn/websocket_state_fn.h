#ifndef WEBSOCKET_STATE_FN_H
#define WEBSOCKET_STATE_FN_H

#include "websocket_server_fn.h"


void process_message(const char *payload, bool *connection_authorized);

#endif