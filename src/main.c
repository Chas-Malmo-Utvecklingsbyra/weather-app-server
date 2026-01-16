#include "httpserver/HttpServer.h"
#include "core/config/config.h"
#include <stdio.h>

int main(void) 
{
    printf("Hello, world! I am the Server.\n");

    uint16_t port = 8080;
    size_t max_connections = 1024;
    
    {   /* Tries to overwrite default HTTP server settings using config file */
        Config_t* cfg = Config_Get_Instance("settings.json");

        if (cfg != NULL)
        {
            bool found = false;
            port = Config_Get_Field_Value_Integer(cfg, "server_port", &found);
            if (found && port > 0)
                port = port;
            else
                port = 8080; /* reset to default if not found */

            max_connections = Config_Get_Field_Value_Integer(cfg, "max_connections", &found);
            if (found || max_connections >= 1)
                max_connections = max_connections;
            else
                max_connections = 1024; /* reset to default if not found */
            
            Config_Instance_Dispose();
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