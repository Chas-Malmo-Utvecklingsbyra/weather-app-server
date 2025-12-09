#include "core/tcp/server/tcp_server.h"
#include <stdbool.h>


typedef struct
{
    TCP_Server tcp_server;
    bool has_been_initialized;
}HttpServer;


int HttpServer_Initialize();
int HttpServer_Start();
void HttpServer_Work();
void HttpServer_Dispose();