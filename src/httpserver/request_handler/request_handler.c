#include "request_handler.h"
#include <stdio.h>
#include "core/config/config.h"
#include "core/string/strdup.h"
#include "core/weather/weather.h"
#include "core/json/fileHelper/fileHelper.h"
#include "core/locationiq/locationiq.h"

/**
 * @brief Sets API result for error responses with JSON content.
 * @param api_result Pointer to the API result structure.
 * @param code HTTP status code.
 * @param message Error message (will be duplicated).
 */
static void set_api_error(Request_Handler_Result_t *api_result, int code, const char *message)
{
    if (api_result == NULL || message == NULL)
        return;

    api_result->response = strdup(message);
    api_result->code = code;
    api_result->content_type = HTTP_CONTENT_TYPE_JSON;
}

int handle_route(Http_Request *request, Request_Handler_Result_t *api_result)
{
    Config_t *cfg = config_get_instance(NULL);

    if (request == NULL)
    {
        set_api_error(api_result, ROUTE_HANDLER_RESULT_INTERNAL_SERVER_ERROR,
                      "{\"error\":\"Internal server error: NULL request\"}");

        return api_result->code;
    }

    if (cfg == NULL)
    {
        set_api_error(api_result, ROUTE_HANDLER_RESULT_INTERNAL_SERVER_ERROR,
                      "{\"error\":\"Internal server error\"}");
        return api_result->code;
    }

    char *method = Http_Request_Get_Method_String(request);
    QueryParams_t params = {0};
    bool params_initialized = false;

    api_result->code = ROUTE_HANDLER_RESULT_UNKNOWN;
    api_result->response = NULL;
    api_result->content_type = HTTP_CONTENT_TYPE_JSON;

    size_t i = 0;
    for (; i < cfg->allowed_routes_count; i++)
    {
        if (cfg->config_debug)
            printf("Checking route: %s with method: %s\n", cfg->allowed_routes[i].route, cfg->allowed_routes[i].method);

        /* Match Route and Method for request */
        /* Use exact match for routes to avoid partial matches like /v1/weather matching /v1/weather-extended */
        size_t route_len = strlen(cfg->allowed_routes[i].route);
        if ((strcmp(request->start_line.path, cfg->allowed_routes[i].route) == 0 ||
             (strncmp(request->start_line.path, cfg->allowed_routes[i].route, route_len) == 0 &&
              (request->start_line.path[route_len] == '?' ||
               request->start_line.path[route_len] == '\0'))) &&
            strcmp(cfg->allowed_routes[i].method, method) == 0)
        {
            /* Parse query parameters if any are expected */
            if (cfg->allowed_routes[i].args_count > 0)
            {
                if (query_params_create(&params, cfg->allowed_routes[i].args_count) != 0)
                {
                    if (cfg->config_debug)
                        printf("Error: Failed to create QueryParams structure\n");

                    set_api_error(api_result, ROUTE_HANDLER_RESULT_INTERNAL_SERVER_ERROR,
                                  "{\"error\":\"Internal server error: memory allocation failed\"}");
                    break;
                }
                params_initialized = true;

                int result = query_params_parse(&params, request->start_line.path);
                if (result < 0)
                {
                    query_params_free(&params);
                    params_initialized = false;
                    set_api_error(api_result, ROUTE_HANDLER_RESULT_BAD_REQUEST,
                                  "{\"error\":\"Failed to parse query parameters\"}");
                    break;
                }
            }
            /* Handle specific routes */
            if (strcmp(cfg->allowed_routes[i].route, "/v1/weather") == 0)
            {
                const char *latitude = query_params_get(&params, "latitude");
                const char *longitude = query_params_get(&params, "longitude");
                if (latitude == NULL || longitude == NULL)
                {
                    if (cfg->config_debug)
                        printf("Error: Missing latitude or longitude parameters\n");

                    set_api_error(api_result, ROUTE_HANDLER_RESULT_BAD_REQUEST,
                                  "{\"error\":\"Missing required parameters: latitude and longitude\"}");
                    break;
                }

                Weather_Response weather_response = weather_get_data(latitude, longitude);
                if (weather_response.error == true)
                {
                    if (cfg->config_debug)
                        printf("Error: Failed to get weather data\n");

                    set_api_error(api_result, ROUTE_HANDLER_RESULT_BAD_REQUEST,
                                  "{\"error\":\"Failed to fetch weather data\"}");
                    break;
                }

                api_result->response = weather_convert_response_to_json(&weather_response);
                api_result->code = ROUTE_HANDLER_RESULT_OK;
                api_result->content_type = HTTP_CONTENT_TYPE_JSON;
                break;
            }
            else if (strcmp(cfg->allowed_routes[i].route, "/v1/city") == 0)
            {
                const char *city_name = query_params_get(&params, "city");
                if (city_name == NULL)
                {
                    if (cfg->config_debug)
                        printf("Error: Missing city parameter\n");

                    set_api_error(api_result, ROUTE_HANDLER_RESULT_BAD_REQUEST,
                                  "{\"error\":\"Missing required parameter: city\"}");
                    break;
                }

                api_result->response = locationiq_api_call(city_name);
                api_result->code = ROUTE_HANDLER_RESULT_OK;
                api_result->content_type = HTTP_CONTENT_TYPE_JSON;
                break;
            }
            else if (strcmp(cfg->allowed_routes[i].route, "/") == 0)
            {
                static const char *file_locations[] = {"src/frontend/index.html", "frontend/index.html"};
                static const size_t location_count = sizeof(file_locations) / sizeof(file_locations[0]);
                bool found_frontend = false;

                for (size_t j = 0; j < location_count; j++)
                {
                    char *file = file_read_to_string(file_locations[j]);
                    if (file != NULL)
                    {
                        api_result->response = file;
                        api_result->code = ROUTE_HANDLER_RESULT_OK;
                        api_result->content_type = HTTP_CONTENT_TYPE_HTML;
                        found_frontend = true;
                        break;
                    }
                }

                if (!found_frontend)
                {
                    if (cfg->config_debug)
                        printf("Error: Frontend file not found in expected locations\n");

                    api_result->response = strdup("<html><body><h1>500 Internal Server Error</h1><p>Frontend not found</p></body></html>");
                    api_result->code = ROUTE_HANDLER_RESULT_INTERNAL_SERVER_ERROR;
                    api_result->content_type = HTTP_CONTENT_TYPE_HTML;
                }
                break;
            }
        }
        if ((strstr(request->start_line.path, "/v1/") != NULL) && (i + 1) == cfg->allowed_routes_count && api_result->code == ROUTE_HANDLER_RESULT_UNKNOWN)
        {
            /* No matching API route found */
            set_api_error(api_result, ROUTE_HANDLER_RESULT_NOT_FOUND,
                          "{\"error\":\"Route not found\"}");
        }
    }

    if (params_initialized)
    {
        query_params_free(&params);
    }

    return api_result->code;
}

int query_params_create(QueryParams_t *params, size_t capacity)
{
    if (params == NULL || capacity == 0)
    {
        return -1; /* Invalid arguments */
    }

    params->keys = (char **)malloc(sizeof(char *) * capacity);
    params->values = (char **)malloc(sizeof(char *) * capacity);
    if (params->keys == NULL || params->values == NULL)
    {
        free(params->keys);
        free(params->values);
        return -1; /* Memory allocation failure */
    }

    for (size_t i = 0; i < capacity; i++)
    {
        params->keys[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        params->values[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        if (params->keys[i] == NULL || params->values[i] == NULL)
        {
            /* Free previously allocated memory on failure */
            for (size_t j = 0; j <= i; j++)
            {
                free(params->keys[j]);
                free(params->values[j]);
            }
            free(params->keys);
            free(params->values);
            return -1; /* Memory allocation failure */
        }
    }

    params->count = 0;
    params->capacity = capacity;

    return 0; /* Success */
}

int query_params_parse(QueryParams_t *params, const char *path)
{
    if (params == NULL || path == NULL)
    {
        return -1; /* Invalid arguments */
    }

    size_t number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;

    /* Find the '?' to locate query string start */
    const char *query_start = strchr(path, '?');
    if (query_start == NULL)
    {
        return ROUTE_HANDLER_RESULT_ERROR; /* No query parameters */
    }

    size_t path_length = strlen(path);
    size_t i = query_start - path;
    for (; i < path_length; i++)
    {
        if (number_of_params >= params->capacity)
        {
            break; /* Reached maximum expected parameters, should be redone if optional parameters needed */
        }

        if (path[i] == '?')
        {
            key_index = i + 1;
            if (key_index >= path_length)
            {
                return ROUTE_HANDLER_RESULT_ERROR; /* No query parameters */
            }
        }
        else if (path[i] == '=') /* End of parameter key, start of parameter value */
        {
            if (i == key_index)
            {
                return ROUTE_HANDLER_RESULT_ERROR; /* Empty key, malformed URL */
            }

            size_t key_length = (i - key_index);
            if (key_length >= QUERY_PARAMETER_MAX_LENGTH)
            {
                return ROUTE_HANDLER_RESULT_ERROR; /* Key too long */
            }

            strncpy(params->keys[number_of_params], &path[key_index], key_length);
            params->keys[number_of_params][key_length] = '\0';

            value_index = i + 1;
            if (value_index >= path_length || path[value_index] == '&')
            {
                return ROUTE_HANDLER_RESULT_ERROR; /* Key but empty value, malformed URL, or should empty value be allowed? */
            }
        }
        else if ((path[i] == '&' || i == path_length - 1)) /* End of parameter value, start of next key */
        {
            size_t value_length = (path[i] == '&') ? (i - value_index) : (i - value_index + 1);
            if (value_length >= QUERY_PARAMETER_MAX_LENGTH)
            {
                return ROUTE_HANDLER_RESULT_ERROR; /* Value too long */
            }

            strncpy(params->values[number_of_params], &path[value_index], value_length);
            params->values[number_of_params][value_length] = '\0';

            key_index = i + 1;
            number_of_params++;
        }
    }
    params->count = number_of_params;

    return 0;
}

const char *query_params_get(QueryParams_t *params, const char *key)
{
    if (params == NULL || key == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < params->count; i++)
    {
        if (strcmp(params->keys[i], key) == 0)
        {
            return params->values[i];
        }
    }
    return NULL; /* Key not found */
}

void query_params_free(QueryParams_t *params)
{
    if (params == NULL || params->capacity == 0)
    {
        return;
    }

    if (params->keys != NULL)
    {
        for (size_t i = 0; i < params->capacity; i++)
        {
            free(params->keys[i]);
        }
        free(params->keys);
        params->keys = NULL;
    }

    if (params->values != NULL)
    {
        for (size_t i = 0; i < params->capacity; i++)
        {
            free(params->values[i]);
        }
        free(params->values);
        params->values = NULL;
    }
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
