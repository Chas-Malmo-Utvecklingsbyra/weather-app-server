#include "request_handler.h"
#include "core/config/config.h"
#include "route_matcher/route_matcher.h"
#include "routes/weather_handler.h"
#include "routes/city_handler.h"
#include "routes/frontend_handler.h"

void set_request_error(Http_Response_t *http_response, const HTTP_Status_Code code, const Http_Content_Type content_type, const char *message)
{
    if (http_response == NULL || message == NULL)
        return;

    http_response->response_data = strdup(message);
    http_response->code = code;
    http_response->content_type = content_type;
}

int handle_route(Http_Request *request, Http_Response_t *http_response)
{
    if (request == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Bad Request\"}");
        return http_response->code;
    }
    
    /* Handle all OPTIONS requests */
    char* method = Http_Request_Get_Method_String(request);
    if (strcmp(method, "OPTIONS") == 0)
    {
        http_response->code = HTTP_STATUS_CODE_OK;
        http_response->content_type = HTTP_CONTENT_TYPE_HTML;
        http_response->response_data = strdup("<h1>OPTIONS</h1>");
        return http_response->code;
    }
    
    Config_t *cfg = config_get_instance(NULL);
    if (cfg == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error\"}");
        return http_response->code;
    }

    QueryParams_t params = {0};
    bool params_initialized = false;

    http_response->code = HTTP_STATUS_CODE_UNDEFINED;
    http_response->response_data = NULL;
    http_response->content_type = HTTP_CONTENT_TYPE_JSON;

    size_t i = 0;
    for (; i < cfg->allowed_routes_count; i++)
    {
        if (route_matcher_matches(request->start_line.path, method, cfg->allowed_routes[i].route, cfg->allowed_routes[i].method))
        {
            if (cfg->allowed_routes[i].args_count > 0)
            {
                if (query_params_create(&params, cfg->allowed_routes[i].args_count) != 0)
                {
                    set_request_error(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error: memory allocation failed\"}");
                    break;
                }

                params_initialized = true;;

                if (query_params_parse(&params, request->start_line.path) != 0)
                {
                    query_params_dispose(&params);
                    params_initialized = false;
                    set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Failed to parse query parameters\"}");
                    break;
                }
            }
            /* Handle specific routes */
            if (strcmp(cfg->allowed_routes[i].route, "/v1/weather") == 0)
            {
                weather_handler_handle(&params, http_response);
                break;
            }
            else if (strcmp(cfg->allowed_routes[i].route, "/v1/city") == 0)
            {
                city_handler_handle(&params, http_response);
                break;
            }
            else if (strcmp(cfg->allowed_routes[i].route, "/") == 0)
            {
                frontend_handler_handle(NULL, http_response);
                break;
            }
        }
        if ((strstr(request->start_line.path, "/v1/") != NULL) && (i + 1) == cfg->allowed_routes_count && http_response->code == -1)
        {
            set_request_error(http_response, HTTP_STATUS_CODE_NOT_FOUND, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Route not found\"}");
        }
    }

    if (params_initialized)
    {
        query_params_dispose(&params);
    }

    return http_response->code;
}

void handle_request(TCP_Server* server, TCP_Server_Client* client, char* response_string, Http_Content_Type type, HTTP_Status_Code status_code)
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