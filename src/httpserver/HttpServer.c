#include "HttpServer.h"
#include "request_handler/request_handler.h"
#include "core/config/config.h"
#include <string.h>
#include "core/weather/http.h"
#include "core/weather/api.h"
#include "core/string/strdup.h"
#include <stdio.h>
#include "core/http/parser.h"
#include "core/http/http.h"
#include "core/weather/weather.h"
#include "core/json/fileHelper/fileHelper.h"

static HttpServer http_server;

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
        handle_request(server, client, "<h1>Invalid HTTP Request</h1>", HTTP_CONTENT_TYPE_HTML, HTTP_STATUS_CODE_BAD_REQUEST);
        return;
    }

    printf("METHOD: [%s]\n", Http_Request_Get_Method_String(httpblob));
    printf("PATH: [%s]\n", httpblob->start_line.path);

    Http_Response_t http_response = {0};
    handle_route(httpblob, &http_response);
    
    char *response_buffer = NULL;
    if (http_response.response_data != NULL)
    {
        response_buffer = strdup(http_response.response_data);
        free(http_response.response_data);
        http_response.response_data = NULL;
    }
    else
    {
        response_buffer = strdup("{\"error\":\"Internal Server Error\"}");
        http_response.code = HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }
    handle_request(server, client, response_buffer, http_response.content_type, (HTTP_Status_Code)http_response.code);
    free(response_buffer);
    response_buffer = NULL;
    
    Http_Parser_Cleanup(httpblob);
}

int HttpServer_Initialize()
{
    printf("Hello, world! I am the Server.\n");
    
    Config_t* cfg = config_get_instance("settings.json");

    if (cfg == NULL)
    {
        printf("Failed to load config. Error code: %d\n", config_instance_get_last_error());
        return -1;
    }

    memset(&http_server, 0, sizeof(HttpServer));
    printf("Initializing TCP server on port %d...\n", cfg->config_server_port);
    TCP_Server_Result server_init_result = tcp_server_init(
        &http_server.tcp_server,
        cfg->config_server_port,
        &on_received_bytes_from_client
    );

    if(server_init_result != TCP_Server_Result_OK) 
    {
        printf("Failed to initialize TCP server. Result: %i.\n", server_init_result); // TODO: SS - tcp_server_get_result_as_string(server_init_result)
        return -1;
    }

    http_server.has_been_initialized = true;

    return 0;
}


int HttpServer_Start()
{
    if (!http_server.has_been_initialized)
    {
        printf("Server has not yet been initialized\n");
        return -1;
    }

    TCP_Server_Result start_server_result = tcp_server_start(&http_server.tcp_server);
    if(start_server_result != TCP_Server_Result_OK) 
    {
        printf("Failed to start TCP server. Result: %i.\n", start_server_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
        return -1;
    }
    printf("Server running.\n");

    return 0;
}

void HttpServer_Work()
{
    while(true) 
    { // TEMP: SS - tcp_server_is_running(server)?

        TCP_Server_Result work_result = tcp_server_work(&http_server.tcp_server);

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

        // TODO: SS - Tick the server.
        // NOTE: SS - Sleep? Or get console-input here to be able to stop the server from the program.
    }

    printf("Server stopped.\n");
    /* Dispose config on exit */
}

void HttpServer_Dispose()
{
    config_instance_dispose();
    tcp_server_dispose(&http_server.tcp_server);
}