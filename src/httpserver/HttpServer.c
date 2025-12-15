#include "HttpServer.h"
#include "request_handler/request_handler.h"
#include <string.h>
#include "core/weather/http.h"
#include "core/weather/api.h"
#include "core/string/strdup.h"
#include <stdio.h>
#include "core/http/parser.h"
#include "core/http/http.h"
#include "core/weather/weather.h"
#include "core/json/fileHelper/fileHelper.h"
#include <stdlib.h>

void on_received_bytes_from_client(TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size) {

    /* TODO: SS - Try to parse the contents of the request buffer as a HTTP-request. */
    printf("Received %u bytes from client:\n", buffer_size);

    printf("'");
    uint32_t i = 0;
    for(; i < buffer_size; i++) {
        printf("%c", *(buffer + i));
    }
    printf("'\n");
    
    Http_Request* httpblob =  Http_Parser_Parse((const char*)buffer);
    if(httpblob == NULL)
    {
        handle_request(server, client, "<h1>Invalid HTTP Request</h1>", HTTP_CONTENT_TYPE_HTML);
        return;
    }

    printf("METHOD: [%s]\n", Http_Request_Get_Method_String(httpblob));
    printf("PATH: [%s]\n", httpblob->start_line.path);

    /*
        char* response = route(httpblob->start_line.path, httpblob->start_line.method);
    */
    if(strcmp(Http_Request_Get_Method_String(httpblob), "OPTIONS") == 0)
    {
        printf("Hello from inside OPTIONS handling\n");
        handle_request(server, client, "<h1>OPTIONS</h1>", HTTP_CONTENT_TYPE_HTML);
    }
    else
    {
        Request_Handler_Result_t api_result = {0};
        handle_route(httpblob, &api_result);
        char *response_buffer = NULL;

        if (api_result.response != NULL)
        {
            response_buffer = strdup(api_result.response);
            free(api_result.response);
            api_result.response = NULL;
        }
        else
        {
            response_buffer = strdup("{\"error\":\"No response from handler\"}");
            api_result.code = ROUTE_HANDLER_RESULT_INTERNAL_SERVER_ERROR;
        }

        handle_request(server, client, response_buffer, api_result.content_type);

        free(response_buffer);
        response_buffer = NULL;
    }

    Http_Parser_Cleanup(httpblob);
}


bool HttpServer_Initialize(HttpServer* http_server, uint16_t port, size_t max_connections)
{
    /* TODO: HW - Use this */
    (void)max_connections;

    memset(http_server, 0, sizeof(HttpServer));
    printf("Initializing TCP server on port %d...\n", port);
    TCP_Server_Result server_init_result = tcp_server_init(
        &http_server->tcp_server,
        port,
        &on_received_bytes_from_client
    );

    if(server_init_result != TCP_Server_Result_OK) 
    {
        printf("Failed to initialize TCP server. Result: %i.\n", server_init_result); // TODO: SS - tcp_server_get_result_as_string(server_init_result)
        return false;
    }

    http_server->has_been_initialized = true;

    return true;
}


bool HttpServer_Start(HttpServer* http_server)
{
    if (!http_server->has_been_initialized)
    {
        printf("Server has not yet been initialized\n");
        return false;
    }

    TCP_Server_Result start_server_result = tcp_server_start(&http_server->tcp_server);
    if(start_server_result != TCP_Server_Result_OK) 
    {
        printf("Failed to start TCP server. Result: %i.\n", start_server_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
        return false;
    }
    printf("Server running.\n");

    return true;
}

void HttpServer_Work(HttpServer* http_server)
{
    
    // TEMP: SS - tcp_server_is_running(server)?

    TCP_Server_Result work_result = tcp_server_work(&http_server->tcp_server);

    switch(work_result){
        case TCP_Server_Result_OK:
        {
            break;
        }
        default: 
        {
            printf("Something went wrong\n");
            break;
        }
    }

    /* Dispose config on exit */
}

void HttpServer_Dispose(HttpServer* http_server)
{
    tcp_server_dispose(&http_server->tcp_server);
}