#include "server_routes.h"
#include "core/config/config.h"
#include <string.h>
#include "core/weather/http.h"
#include "core/weather/api.h"
#include "core/string/strdup.h"
#include <stdio.h>
#include "core/http/parser.h"
#include "core/weather/weather.h"


void param_dispose(char **params, size_t count)
{
    if (params == NULL)
        return;

    size_t i = 0;
    for (; i < count; i++)
    {
        if (params[i] != NULL)
        {
            free(params[i]);
            params[i] = NULL;
        }
    }

    free(params);
}

/**
 * @brief Handles routing for incoming HTTP requests.
 * @param request Pointer to the Http_Request structure containing the request details.
 * @param response Pointer to a char pointer where the JSON response will be stored.
 * @return Route_Handler_Result Result of the route handling operation.
 * @note The caller is responsible for freeing the memory allocated for the response.
 */
HTTP_STATUS_CODE handle_route(Http_Request *request, char **response)
{
    Config_t *cfg = config_get_instance(NULL);

    if (request == NULL)       
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    
    if (cfg->config_debug)
        printf("Requested path: %s\n", request->start_line.path);

    char *method = Http_Request_Get_Method_String(request);

    size_t i;
    for (i = 0; i < cfg->allowed_routes_count; i++)
    {
        /* Check if the request path matches any allowed route */
        if (strstr(request->start_line.path, cfg->allowed_routes[i].route) != NULL)
        {
            if (strcmp(cfg->allowed_routes[i].method, method) == 0)
            {
                size_t arg_count = cfg->allowed_routes[i].args_count;
                int param_count = 0;
                char **keys = NULL;
                char **values = NULL;

                if (cfg->config_debug)
                    printf("Matched allowed route: %s %s\n", cfg->allowed_routes[i].method, cfg->allowed_routes[i].route);

                /* Get query parameters if any are expected */
                if (arg_count > 0)
                {
                    keys = (char **)malloc(sizeof(char *) * arg_count);
                    values = (char **)malloc(sizeof(char *) * arg_count);
                    
                    size_t j = 0;
                    for (; j < arg_count; j++)
                    {
                        keys[j] = malloc(MAX_PARAM_LENGTH * sizeof(char));
                        values[j] = malloc(MAX_PARAM_LENGTH * sizeof(char));
                    }
                    param_count = get_query_params(request->start_line.path, arg_count, keys, values);

                    if (param_count < 0)
                    {
                        param_dispose(keys, arg_count);
                        keys = NULL;
                        param_dispose(values, arg_count);
                        values = NULL;

                        return HTTP_STATUS_CODE_BAD_REQUEST;
                    }
                }

                /* Handle specific routes */
                if(strcmp(cfg->allowed_routes[i].route, "/weather") == 0)
                {
                    if (cfg->config_debug)
                        printf("Handling /weather route\n");
                    
                    char* latitude = NULL;
                    char* longitude = NULL;
                    int k = 0;
                    
                    for (; k < param_count; k++)
                    {
                        if (strcmp(keys[k], "latitude") == 0)
                        {
                            latitude = values[k];
                        }
                        else if (strcmp(keys[k], "longitude") == 0)
                        {
                            longitude = values[k];
                        }
                    }
                    
                    Weather_Response weather_response = weather_get_data(latitude, longitude);

                    if (weather_response.error == true)
                    {
                        if (cfg->config_debug)
                            printf("Error: Failed to get weather data\n");

                        param_dispose(keys, arg_count);
                        keys = NULL;
                        param_dispose(values, arg_count);
                        values = NULL;

                        return HTTP_STATUS_CODE_BAD_REQUEST;
                    }
                    *response = weather_convert_response_to_json(&weather_response);
                }
                /* else if(strcmp(cfg->allowed_routes[i].route, "/otherroute") == 0) */

                param_dispose(keys, arg_count);
                keys = NULL;
                param_dispose(values, arg_count);
                values = NULL;
                
                return HTTP_STATUS_CODE_OK;
            }
        }
    }
    return HTTP_STATUS_CODE_NOT_FOUND;
}

/**
 * @brief Extracts query parameters from a URL path.
 * @param path The URL path containing query parameters.
 * @param key Array of char pointers to store extracted keys.
 * @param value Array of char pointers to store extracted values.
 * @return int Number of query parameters extracted.
 * @note TODO: error handling for malformed URLs.
 */
int get_query_params(const char *path, const int max_params, char **keys, char **values)
{
    Config_t *cfg = config_get_instance(NULL);
    size_t number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;
    
    /* Find the '?' to locate query string start */
    const char *query_start = strchr(path, '?');
    if (query_start == NULL)
    {
        if (cfg->config_debug)
            printf("Error: No query parameters found in path\n");
        return Route_Get_Params_Result_No_Params; /* No query parameters */
    }

    size_t i = query_start - path;
    size_t path_length = strlen(path);

    for (; i < path_length; i++)
    {
        if (number_of_params == (size_t)max_params)    
            break; /* Reached maximum expected parameters */
        
        if (path[i] == '?')
        {
            key_index = i + 1;
            
            if (key_index >= path_length)
            {
                if (cfg->config_debug)
                    printf("Error: No query parameters after '?'\n");
                    
                return Route_Get_Params_Result_No_Params; /* No query parameters, not necessarily an error */
            }
        }
        else if (path[i] == '=')
        {
            if(i == key_index)        
                return Route_Get_Params_Result_Malformed_Request; /* Empty key, malformed URL */
            
            // keys[number_of_params] = malloc((i - key_index + 1) * sizeof(char));
            
            if (keys[number_of_params] == NULL)
                return Route_Get_Params_Result_Error; /* Memory allocation failure, Server error */
            
            if (key_index > MAX_PARAM_LENGTH)
            {
                return Route_Get_Params_Result_Malformed_Request;
            }

            strncpy(keys[number_of_params], &path[key_index], i - key_index);
            keys[number_of_params][i - key_index] = '\0';

            value_index = i + 1;

            // Check if '=' is at the end or followed immediately by '&'
            if (value_index >= path_length || path[value_index] == '&')
                return Route_Get_Params_Result_Malformed_Request; /* Empty value, malformed URL */
        }
        else if ((path[i] == '&' || i == path_length - 1))
        {
            size_t value_length = (path[i] == '&') ? (i - value_index) : (i - value_index + 1);
            // values[number_of_params] = malloc((value_length + 1) * sizeof(char));

            if (!values[number_of_params])
            {
                return Route_Get_Params_Result_Error; /* Memory allocation failure, Server error */
            }

            if (value_length > MAX_PARAM_LENGTH)
            {
                return Route_Get_Params_Result_Malformed_Request;
            }

            strncpy(values[number_of_params], &path[value_index], value_length);
            values[number_of_params][value_length] = '\0';

            key_index = i + 1;
            number_of_params++;
        }
    }
    return number_of_params;
}