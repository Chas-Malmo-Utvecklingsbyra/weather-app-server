#include "request_handler.h"
#include "route_registry/route_registry.h"
#include "routes/weather_handler.h"
#include "routes/city_handler.h"
#include "routes/frontend_handler.h"
#include <stdio.h>

#define ROUTE_HANDLER_REGISTRY_COUNT 3

/* TODO: LS - improve "request_handler_set_response" with predefined error msgs etc */
/* TODO: LS - Change response types to enums */

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

void request_handler_set_response(Request_Handler_Response_t *http_response, const HTTP_Status_Code code, const Http_Content_Type content_type, const char *message)
{
    if (http_response == NULL || message == NULL)
        return;

    http_response->response_data = strdup(message);
    http_response->code = code;
    http_response->content_type = content_type;
}

int request_handler_handle_route(Http_Request *request, Request_Handler_Response_t *http_response)
{
    if (request == NULL || g_route_registry == NULL)
    {
        request_handler_set_response(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Bad Request\"}");
        return http_response->code;
    }
    
    /* Handle ALL OPTIONS requests, should it always respond with OK? */
    char* method = Http_Request_Get_Method_String(request);
    if (strcmp(method, "OPTIONS") == 0)
    {
        http_response->code = HTTP_STATUS_CODE_OK;
        http_response->content_type = HTTP_CONTENT_TYPE_HTML;
        http_response->response_data = strdup("<h1>OPTIONS</h1>");
        return http_response->code;
    }

    QueryParameters_t query_parameter = {0};
    bool params_initialized = false;
    http_response->code = HTTP_STATUS_CODE_UNDEFINED;
    http_response->response_data = NULL;
    http_response->content_type = HTTP_CONTENT_TYPE_JSON;

    /* Get expected args count for this route */
    int args_count = route_registry_get_args_count(g_route_registry, request->start_line.path, method);
    if (args_count >= 0)
    {
        /* Route exists, parse parameters if needed */
        if (args_count > 0)
        {
            if (query_parameter_create(&query_parameter, (size_t)args_count) != 0)
            {
                request_handler_set_response(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error: memory allocation failed\"}");
                return http_response->code;
            }
            params_initialized = true;

            if (query_parameter_parse(&query_parameter, request->start_line.path) != 0)
            {
                query_parameter_dispose(&query_parameter);
                params_initialized = false;
                request_handler_set_response(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Failed to parse query parameters\"}");
                return http_response->code;
            }
        }
        /* Dispatch to the handler for this route */
        route_registry_dispatch(g_route_registry, request->start_line.path, method, &query_parameter, http_response);
        if(params_initialized == true)
        {
            query_parameter_dispose(&query_parameter);
            params_initialized = false;
        }
        return http_response->code;
    }
    else /* Route not found */
    {
        request_handler_set_response(http_response, HTTP_STATUS_CODE_NOT_FOUND, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Route not found\"}");
    }

    if (params_initialized == true)
    {
        query_parameter_dispose(&query_parameter);
        params_initialized = false;
    }
    return http_response->code;
}

void send_response_to_client(TCP_Server* server, TCP_Server_Client* client, char* response_string, Http_Content_Type type, HTTP_Status_Code status_code)
{
    uint8_t buffer[TCP_MAX_CLIENT_BUFFER_SIZE];
    uint32_t size = 0;

    http_create_response(buffer, sizeof(buffer), response_string, status_code, strlen(response_string), &size, type);

    TCP_Server_Result send_result = tcp_server_send_to_client(server, client, buffer, size);

    if (send_result != TCP_Server_Result_OK)
    {
        printf("Error on client send\n");
    }
}

void dispose_request_handler_response(Request_Handler_Result_t* api_result)
{
    free(api_result->response);
    api_result->response = NULL;
}


void request_handler_dispose(void)
{
    if (g_route_registry != NULL)
    {
        route_registry_dispose(g_route_registry);
        g_route_registry = NULL;
    }
}