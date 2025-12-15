#include "httpserver/HttpServer.h"
#include "core/config/config.h"
#include <stdio.h>

int main(void) 
{
    printf("Hello, world! I am the Server.\n");

    uint16_t port = 8080;
    size_t max_connections = 1024;
    
    {   /* Tries to overwrite default HTTP server settings using config file */
        Config_t* cfg = config_get_instance("settings.json");

        if (cfg != NULL)
        {
            if (cfg->config_server_port > 0)
                port = cfg->config_server_port;
    
            if (cfg->config_max_connections > 0)
                max_connections = cfg->config_max_connections;
            
            config_instance_dispose();
        }
    }
    
    HttpServer http_server;

    if (HttpServer_Initialize(&http_server, max_connections) == false)
    {
        printf("Server failed to Initialize\n");
        return -1;
    }

    if (HttpServer_Start(&http_server, port) == false)
    {
        printf("Server failed to start\n");
        return -1;
    }

    while (1)
    {
        HttpServer_Work(&http_server);
    }

    HttpServer_Dispose(&http_server);
    return 0;
}