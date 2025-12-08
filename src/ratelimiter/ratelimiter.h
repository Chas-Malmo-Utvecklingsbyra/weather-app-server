#ifndef RATELIMITER_H
#define RATELIMITER_H

#include "core/tcp/server/tcp_server.h"
#include "core/utils/clock_monotonic.h"

bool check_rate_limit(TCP_Server_Client* client);

#endif