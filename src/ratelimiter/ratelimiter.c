#include "ratelimiter.h"

static int is_limited = 0;

bool check_rate_limit(TCP_Server_Client* client)
{
    (void)client;
    is_limited++;


    if (is_limited > 4)
    {
        return true;
    }
    return false;
}