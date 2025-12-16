#include "request_handler.h"
#include "route_registry/route_registry.h"
#include "routes/weather_handler.h"
#include "routes/city_handler.h"
#include "routes/frontend_handler.h"
#include <stdio.h>
#include <assert.h>

#define ROUTE_HANDLER_REGISTRY_COUNT 3

/* TODO: LS - improve "request_handler_set_response" with predefined error msgs etc */
/* TODO: LS - Change response types to enums */
/* TODO: LS - Add logging for request handling */
/* TODO: LS - "request_handler_init" create a more modular approach to config routes */

/* Global route registry, needs to be initialized and disposed */
static RouteRegistry *g_route_registry = NULL;

int request_handler_init(int capacity)
{
    (void)capacity;  /* Unused parameter */
    
    if (g_route_registry != NULL)
        return -1;  /* Already initialized */

    g_route_registry = route_registry_create(ROUTE_HANDLER_REGISTRY_COUNT);
    if (g_route_registry == NULL)
        return -1;

    /* Register all routes */ 
    /* Should be separated later */
    if (route_registry_register(g_route_registry, "/v1/weather", "GET", 2, weather_handler_handle) != 0)
        return -1;

    if (route_registry_register(g_route_registry, "/v1/city", "GET", 2, city_handler_handle) != 0)
        return -1;

    if (route_registry_register(g_route_registry, "/", "GET", 0, frontend_handler_handle) != 0)
        return -1;

    return 0;
}

void request_handler_set_response(Request_Handler_Response_t *request_handler_response, const HTTP_Status_Code code, const Http_Content_Type content_type, const char *message)
{
    if (request_handler_response == NULL || message == NULL)
        return;

    request_handler_response->response_data = strdup(message);
    request_handler_response->code = code;
    request_handler_response->content_type = content_type;
}

int request_handler_handle_request(Http_Request *request, Request_Handler_Response_t *request_handler_response)
{
    if (request == NULL || g_route_registry == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Bad Request\"}");
        return request_handler_response->code;
    }

    /* Handle ALL OPTIONS requests, should it always respond with OK? */
    char* method = Http_Request_Get_Method_String(request);
    
    if (strcmp(method, "OPTIONS") == 0)
    {
        request_handler_response->code = HTTP_STATUS_CODE_OK;
        request_handler_response->content_type = HTTP_CONTENT_TYPE_HTML;
        request_handler_response->response_data = strdup("<h1>OPTIONS</h1>");
        return request_handler_response->code;
    }
    
    request_handler_response->code = HTTP_STATUS_CODE_UNDEFINED;
    request_handler_response->response_data = NULL;
    request_handler_response->content_type = HTTP_CONTENT_TYPE_JSON;

    if (route_registry_dispatch(g_route_registry, request->start_line.path, method, request_handler_response) == HTTP_STATUS_CODE_NOT_FOUND)
    {
        printf("Route not found for path: %s method: %s\n", request->start_line.path, method);
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_NOT_FOUND, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Not Found\"}");
    }
    /* TODO: LS - Set response here if error */
    
    return request_handler_response->code;
}

void send_response_to_client(TCP_Server* server, TCP_Server_Client* client, char* response_string, Http_Content_Type type, HTTP_Status_Code status_code)
{
    uint8_t buffer[TCP_MAX_CLIENT_BUFFER_SIZE];
    uint32_t size = 0;

    assert(response_string != NULL); /* Should never be NULL here */
    
    http_create_response(buffer, sizeof(buffer), response_string, status_code, strlen(response_string), &size, type);

    TCP_Server_Result send_result = tcp_server_send_to_client(server, client, buffer, size);

    if (send_result != TCP_Server_Result_OK)
    {
        printf("Error on client send\n");
    }
}

void dispose_request_handler_response(Request_Handler_Response_t *request_handler_response)
{
    if (request_handler_response->response_data != NULL)
    {
        free(request_handler_response->response_data);
        request_handler_response->response_data = NULL;
    }
}


void request_handler_dispose(void)
{
    if (g_route_registry != NULL)
    {
        route_registry_dispose(g_route_registry);
        g_route_registry = NULL;
    }
}