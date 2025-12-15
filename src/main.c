#include "httpserver/HttpServer.h"
#include <stdio.h>

int main(void) 
{
    if (HttpServer_Initialize() != 0)
    {
        printf("Server failed to Initialize\n");
        return -1;
    }

    if (HttpServer_Start() != 0)
    {
        printf("Server failed to start\n");
        return -1;
    }

    while (1)
    {
        HttpServer_Work();
    }

    HttpServer_Dispose();
    return 0;
}