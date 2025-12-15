#include "HttpServer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "request_handler/request_handler.h"
#include "core/weather/http.h"
#include "core/weather/api.h"
#include "core/string/strdup.h"
#include "core/http/parser.h"
#include "core/http/http.h"
#include "core/weather/weather.h"
#include "core/json/fileHelper/fileHelper.h"

void on_received_bytes_from_client(TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size) {

    /* TODO: SS - Try to parse the contents of the request buffer as a HTTP-request. */
    printf("Received %u bytes from client:\n", buffer_size);
    
    Http_Request* httpblob =  Http_Parser_Parse((const char*)buffer);
    if(httpblob == NULL)
    {
        send_response_to_client(server, client, "<h1>Invalid HTTP Request</h1>", HTTP_CONTENT_TYPE_HTML, HTTP_STATUS_CODE_BAD_REQUEST);
        return;
    }

    if(strcmp(Http_Request_Get_Method_String(httpblob), "OPTIONS") == 0)
    {
        send_response_to_client(server, client, "<h1>OPTIONS</h1>", HTTP_CONTENT_TYPE_HTML, HTTP_STATUS_CODE_BAD_REQUEST);
    }
    else
    {
        Request_Handler_Result_t api_result = {0};
        handle_route(httpblob, &api_result);
        
        /* Note: api_result.code is int right now, not enum */
        send_response_to_client(server, client, api_result.response, api_result.content_type, api_result.code);

        dispose_request_handler_response(&api_result);
    }

    Http_Parser_Cleanup(httpblob);
}

bool HttpServer_Initialize(HttpServer* http_server, size_t max_connections)
{
    /* TODO: HW - Use this */
    (void)max_connections;

    memset(http_server, 0, sizeof(HttpServer));
    TCP_Server_Result server_init_result = tcp_server_init(
        &http_server->tcp_server,
        &on_received_bytes_from_client
    );

    if(server_init_result != TCP_Server_Result_OK) 
    {
        printf("Failed to initialize TCP server. Result: %i.\n", server_init_result); // TODO: SS - tcp_server_get_result_as_string(server_init_result)
        return false;
    }

    return true;
}


bool HttpServer_Start(HttpServer* http_server, uint16_t port)
{
    
    if (http_server->port != 0)
    {
        printf("Server has not yet been initialized\n");
        return false;
    }
    
    http_server->port = port;

    TCP_Server_Result start_server_result = tcp_server_start(&http_server->tcp_server, http_server->port);
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
}

void HttpServer_Dispose(HttpServer* http_server)
{
    request_handler_dispose();
    assert(http_server != NULL);
    tcp_server_dispose(&http_server->tcp_server);
    http_server->port = 0;
}