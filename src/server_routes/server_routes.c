#include "server_routes.h"
#include "core/config/config.h"
#include <string.h>
#include "core/weather/http.h"
#include "core/weather/api.h"
#include "core/string/strdup.h"
#include <stdio.h>
#include "core/http/parser.h"
#include "core/weather/weather.h"

/**
 * @brief Handles routing for incoming HTTP requests.
 * @param request Pointer to the Http_Request structure containing the request details.
 * @param response Pointer to a char pointer where the JSON response will be stored.
 * @return Route_Handler_Result Result of the route handling operation.
 * @note The caller is responsible for freeing the memory allocated for the response.
 */
/* TODO, handle empty request and query parameters*/
HTTP_STATUS_CODE handle_route(Http_Request *request, char **response)
{
    Config_t *cfg = config_get_instance(NULL);

    if (request == NULL)
    {
        if (cfg->config_debug)
            printf("Error: Null HTTP request received\n");
        return HTTP_STATUS_CODE_BAD_REQUEST;
    }
    
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
                if (cfg->config_debug)
                    printf("Matched allowed route: %s %s\n", cfg->allowed_routes[i].method, cfg->allowed_routes[i].route);

                char **keys = malloc(sizeof(char *) * cfg->allowed_routes[i].args_count);
                char **values = malloc(sizeof(char *) * cfg->allowed_routes[i].args_count);

                int param_count = get_query_params(request->start_line.path, keys, values);
                
                if (param_count < 0)
                {
                    if (cfg->config_debug)
                        printf("Error: Invalid or insufficient query parameters\n");

                    for (size_t j = 0; j < cfg->allowed_routes[i].args_count; j++)
                    {
                        if (keys[j])
                            free(keys[j]);

                        if (values[j])
                            free(values[j]);
                    }

                    if (keys)
                        free(keys);
                        
                    if (values)
                        free(values);

                    return HTTP_STATUS_CODE_BAD_REQUEST;
                }

                if (cfg->config_debug)
                    printf("Fetching weather data for lat: %s, lon: %s\n", values[0], values[1]);
                    
                Weather_Response weather_response = weather_get_data(values[0], values[1]);

                if (weather_response.error == true)
                {
                    if (cfg->config_debug)
                        printf("Error: Failed to get weather data\n");
                        
                    for (size_t j = 0; j < cfg->allowed_routes[i].args_count; j++)
                    {
                        if (keys[j])
                            free(keys[j]);
                        if (values[j])
                            free(values[j]);
                    }
                    
                    if (keys != NULL)
                        free(keys);
                        
                    if (values != NULL)
                        free(values);
                    
                    return HTTP_STATUS_CODE_BAD_REQUEST;
                }

                *response = weather_convert_response_to_json(&weather_response);
                
                if (cfg->config_debug)
                    printf("Response set to: %s\n", *response);

                for (size_t j = 0; j < cfg->allowed_routes[i].args_count; j++)
                {
                    if (keys[j])
                        free(keys[j]);
                    if (values[j])
                        free(values[j]);
                }
                
                if (keys != NULL)
                    free(keys);
                    
                if (values != NULL)
                    free(values);
                    
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
Route_Get_Params_Result get_query_params(const char *path, char **keys, char **values)
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

    /* TODO add check for parameters names if needed */
    for (; i < path_length; i++)
    {
        if (path[i] == '?')
        {
            number_of_params++;
            key_index = i + 1;
            
            if (key_index >= path_length)
            {
                if (cfg->config_debug)
                    printf("Error: No query parameters after '?'\n");
                return Route_Get_Params_Result_No_Params; /* No query parameters, not necessarily an error */
            }
        }
        else if (path[i] == '=' && number_of_params > 0)
        {
            if(i == key_index)
            {
                if (cfg->config_debug)
                    printf("Error: Empty key in query parameters\n");
                return Route_Get_Params_Result_Malformed_Request; /* Empty key, malformed URL */
            }
            
            keys[number_of_params - 1] = malloc((i - key_index + 1) * sizeof(char));
            if (keys[number_of_params - 1] == NULL)
            {
                if (cfg->config_debug)
                    printf("Error: Memory allocation failed for key\n");
                return Route_Get_Params_Result_Error; /* Memory allocation failure, Server error */
            }
            strncpy(keys[number_of_params - 1], &path[key_index], i - key_index);
            keys[number_of_params - 1][i - key_index] = '\0';

            value_index = i + 1;

            // Check if '=' is at the end or followed immediately by '&'
            if (value_index >= path_length || path[value_index] == '&')
            {
                if (cfg->config_debug)
                    printf("Warning: Empty value for key '%s'\n", keys[number_of_params - 1]);
                return Route_Get_Params_Result_Malformed_Request; /* Empty value, malformed URL */
            }

            if (cfg->config_debug)
                printf("Key: %s\n", keys[number_of_params - 1]);
        }
        else if ((path[i] == '&' || i == path_length - 1))
        {
            /* Check if we have a matching key for this value */
            if (number_of_params == 0 || !keys[number_of_params - 1])
            {
                if (cfg->config_debug)
                    printf("Error: Value without key at position %zu\n", i);
                return Route_Get_Params_Result_Malformed_Request; /* Value without key, malformed URL */
            }
            
            size_t value_length = (path[i] == '&') ? (i - value_index) : (i - value_index + 1);
            values[number_of_params - 1] = malloc((value_length + 1) * sizeof(char));

            if (!values[number_of_params - 1])
            {
                if (cfg->config_debug)
                    printf("Error: Memory allocation failed for value\n");
                /* Clean up the key we just allocated */
                free(keys[number_of_params - 1]);
                keys[number_of_params - 1] = NULL;
                return Route_Get_Params_Result_Error; /* Memory allocation failure, Server error */
            }

            strncpy(values[number_of_params - 1], &path[value_index], value_length);
            values[number_of_params - 1][value_length] = '\0';

            if (cfg->config_debug)
                printf("Value: %s\n", values[number_of_params - 1]);

            key_index = i + 1;
            number_of_params++;
        }
    }
    return number_of_params;
}