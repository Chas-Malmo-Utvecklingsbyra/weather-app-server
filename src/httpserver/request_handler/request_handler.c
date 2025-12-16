#include "request_handler.h"
#include "routes/weather_handler.h"
#include "routes/city_handler.h"
#include "routes/frontend_handler.h"
#include <stdio.h>
#include <assert.h>

/* TODO: LS - Change response types to enums */
/* TODO: LS - Add logging for request handling */
/* TODO: LS - request_handler_register_routes fix parameter count magic number */

int request_handler_register_routes(RouteRegistry *registry, int capacity)
{
    (void)capacity;  /* Unused parameter */
    
    if (registry == NULL)
        return -1;  /* not initialized */

    /* Register all routes */
    if (route_registry_register(registry, "/v1/weather", "GET", 2, weather_handler_handle) != 0)
        return -1;

    if (route_registry_register(registry, "/v1/city", "GET", 2, city_handler_handle) != 0)
        return -1;

    if (route_registry_register(registry, "/", "GET", 0, frontend_handler_handle) != 0)
        return -1;

    return 0;
}

static int request_handler_response_init(Request_Handler_Response_t *request_handler_response)
{
    if (request_handler_response == NULL)
        return -1;

    request_handler_response->status_code = HTTP_STATUS_CODE_UNDEFINED;
    request_handler_response->response_data = NULL;
    request_handler_response->content_type = HTTP_CONTENT_TYPE_JSON;

    return 0;
}

static void request_handler_set_error_response(Request_Handler_Response_t *request_handler_response)
{
    if (request_handler_response == NULL)
        return;

    const char *error_msg = NULL;
    
    switch (request_handler_response->status_code)
    {
        case HTTP_STATUS_CODE_BAD_REQUEST:
            error_msg = "{\"error\":\"Bad Request\"}";
            break;
        case HTTP_STATUS_CODE_NOT_FOUND:
            error_msg = "{\"error\":\"Not Found\"}";
            break;
        case HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR:
            error_msg = "{\"error\":\"Internal Server Error\"}";
            break;
        case HTTP_STATUS_CODE_SERVICE_UNAVAILABLE:
            error_msg = "{\"error\":\"Service Unavailable\"}";
            break;
        case HTTP_STATUS_CODE_BAD_GATEWAY:
            error_msg = "{\"error\":\"Bad Gateway\"}";
            break;
        default:
            request_handler_response->status_code = HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
            error_msg = "{\"error\":\"Internal Server Error\"}";
            break;
    }
    
    request_handler_response->response_data = strdup(error_msg);
    if (request_handler_response->response_data == NULL)
    {
        fprintf(stderr, "Warning: Failed to allocate memory for error response\n");
    }
}

void request_handler_set_response(Request_Handler_Response_t *request_handler_response, const HTTP_Status_Code status_code, const Http_Content_Type content_type, const char *response_data)
{
    if (request_handler_response == NULL)
        return;
    
    if (status_code != HTTP_STATUS_CODE_OK)
    {
        request_handler_response->status_code = status_code;
        request_handler_response->content_type = content_type;
        request_handler_set_error_response(request_handler_response);
        return;
    }

    request_handler_response->status_code = status_code;
    request_handler_response->content_type = content_type;
    request_handler_response->response_data = strdup(response_data);
    if (request_handler_response->response_data == NULL)
    {
        fprintf(stderr, "Warning: Failed to allocate memory for response data\n");
    }
}

int request_handler_handle_request(RouteRegistry *registry, Http_Request * request, Request_Handler_Response_t *request_handler_response)
{
    if (request == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }
    if (registry == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    request_handler_response_init(request_handler_response);

    /* Handle ALL OPTIONS requests, should it always respond with OK? */
    char *method = Http_Request_Get_Method_String(request);
    if (strcmp(method, "OPTIONS") == 0)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, "<h1>OPTIONS</h1>");
        return request_handler_response->status_code;
    }
    

    HTTP_Status_Code result_code = route_registry_dispatch(registry, request->start_line.path, method, request_handler_response);
    if (result_code != HTTP_STATUS_CODE_OK) /* Ensure error response is set */
    {
        request_handler_set_response(request_handler_response, result_code, request_handler_response->content_type, NULL);
    }
    return request_handler_response->status_code;
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