#ifndef STATE_OFFLINE_H
#define STATE_OFFLINE_H

#include "websocket_server.h"


void process_message(const char *payload, bool *connection_authorized);

#endif