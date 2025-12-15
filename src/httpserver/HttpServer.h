#include "core/tcp/server/tcp_server.h"
#include <stdbool.h>


typedef struct
{
    TCP_Server tcp_server;
    uint16_t port;
} HttpServer;


bool HttpServer_Initialize(HttpServer* http_server, size_t max_connections);
bool HttpServer_Start(HttpServer* http_server, uint16_t port);
void HttpServer_Work(HttpServer* http_server);
void HttpServer_Dispose(HttpServer* http_server);