#include "server_routes.h"
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

/**
 * @brief Frees memory allocated for query parameters.
 * @param params Array of char pointers containing the parameters.
 * @param count Number of parameters in the array.
 */
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
 * @brief Handles routing of HTTP requests to appropriate handlers.
 * @param request Pointer to the Http_Request structure.
 * @param response Pointer to a char pointer to store the JSON response.
 * @return Server_Route_Result The result of the routing operation.
 */
int handle_route(Http_Request *request, char **response)
{
    if (request == NULL)
    {
        printf("Error: handle_route called with NULL request\n");
        return SERVER_ROUTE_RESULT_INTERNAL_SERVER_ERROR;
    }
        
    Config_t *cfg = config_get_instance(NULL);
    size_t arg_count = 0;
    Server_Route_Result return_code = SERVER_ROUTE_RESULT_UNKNOWN;
    char *method = Http_Request_Get_Method_String(request);
    char **keys = NULL;
    char **values = NULL;

    size_t i = 0;
    for (; i < cfg->allowed_routes_count; i++)
    {        
        if (strstr(request->start_line.path, cfg->allowed_routes[i].route) != NULL)
        {
            if (strcmp(cfg->allowed_routes[i].method, method) == 0)
            {
                arg_count = cfg->allowed_routes[i].args_count;
                int param_count = 0;

                if (cfg->config_debug)
                    printf("Matched allowed route: %s %s\n", cfg->allowed_routes[i].method, cfg->allowed_routes[i].route);

                /* Get query parameters if any are expected */
                if (arg_count > 0)
                {
                    keys = malloc(sizeof(char *) * arg_count);
                    values = malloc(sizeof(char *) * arg_count);

                    if (cfg->config_debug)
                        printf("allocated memory for %zu query parameters\n", arg_count);
                        
                    size_t j = 0;
                    for (; j < arg_count; j++)
                    {
                        keys[j] = malloc(QUERY_PARAMETER_MAX_LENGTH * sizeof(char));
                        if (keys[j] == NULL)
                        {
                            return_code = SERVER_ROUTE_RESULT_INTERNAL_SERVER_ERROR;
                            break;
                        }

                        values[j] = malloc(QUERY_PARAMETER_MAX_LENGTH * sizeof(char));
                        if (values[j] == NULL)
                        {
                            return_code = SERVER_ROUTE_RESULT_INTERNAL_SERVER_ERROR;
                            break;
                        }
                    }
                    if (return_code == SERVER_ROUTE_RESULT_INTERNAL_SERVER_ERROR) /* Memory allocation failed */
                    {
                        break;
                    } 

                    param_count = get_query_params(request->start_line.path, arg_count, keys, values);
                    if (param_count < 0)
                    {
                        return_code = SERVER_ROUTE_RESULT_BAD_REQUEST;
                        break;
                    }
                }

                /* Handle specific routes */
                if(strcmp(cfg->allowed_routes[i].route, "/weather") == 0)
                {
                    char *latitude = NULL;
                    char *longitude = NULL;

                    size_t k = 0;
                    for (; k < arg_count; k++)
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

                        return_code = SERVER_ROUTE_RESULT_BAD_REQUEST;
                        break;
                    }
                    *response = weather_convert_response_to_json(&weather_response);
                }
                else
                {
                    static char* file_locations[2] = { "src/frontend/index.html", "frontend/index.html" };
                    bool found_frontend = false;

                    for (int i = 0; i < 2; i++)
                    {
                        char* file = file_read_to_string(file_locations[i]);

                        if (file != NULL)
                        {
                            *response = file;
                            found_frontend = true;
                            break;
                        }
                    }

                    if (!found_frontend)
                    {
                        *response = strdup("Something went wrong with the Frontend files, check server_routes.c");
                    }
                }
                if ((i + 1) == cfg->allowed_routes_count && return_code == SERVER_ROUTE_RESULT_UNKNOWN) /* No route matched */
                {
                    /* No matching route found */
                    return_code = SERVER_ROUTE_RESULT_NOT_FOUND;
                }
            }
        }
    }

    param_dispose(keys, arg_count);
    param_dispose(values, arg_count);
    keys = NULL;
    values = NULL;

    return return_code;
}

/**
 * @brief Extracts query parameters from a URL path.
 * @param path The URL path containing query parameters.
 * @param max_params The maximum number of parameters to extract.
 * @param keys An array of char pointers to store the parameter keys.
 * @param values An array of char pointers to store the parameter values.
 * @return int The number of parameters extracted, or a negative error code.
 */

int get_query_params(const char *path, const int max_params, char **keys, char **values)
{
    Config_t *cfg = config_get_instance(NULL);
    int number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;

    /* Find the '?' to locate query string start */
    const char *query_start = strchr(path, '?');
    if (query_start == NULL)
    {
        if (cfg->config_debug)
            printf("Error: No query parameters found in path\n");
        return SERVER_ROUTE_RESULT_ERROR; /* No query parameters */
    }

    size_t i = query_start - path;
    size_t path_length = strlen(path);

    for (; i < path_length; i++)
    {
        if (number_of_params == max_params || number_of_params < 0)
        {
            break; /* Reached maximum expected parameters, or encountered an error */
        }

        if (path[i] == '?')
        {
            key_index = i + 1;
            if (key_index >= path_length)
            {
                number_of_params = SERVER_ROUTE_RESULT_ERROR;
                break; /* No query parameters, not necessarily an error */
            }
        }
        else if (path[i] == '=') /* End of param key, start of param value */
        {
            if (i == key_index)
            {
                number_of_params = SERVER_ROUTE_RESULT_ERROR;
                break; /* Empty key, malformed URL */
            }

            size_t key_length = (i - key_index);
            strncpy(keys[number_of_params], &path[key_index], key_length);
            keys[number_of_params][key_length] = '\0';

            value_index = i + 1;
            if (value_index >= path_length || path[value_index] == '&')
            {
                number_of_params = SERVER_ROUTE_RESULT_ERROR;
                break; /* Key but empty value, malformed URL */
            }
        }
        else if ((path[i] == '&' || i == path_length - 1)) /* End of param value, could be start of next param */
        {
            size_t value_length = (path[i] == '&') ? (i - value_index) : (i - value_index + 1);
            strncpy(values[number_of_params], &path[value_index], value_length);
            values[number_of_params][value_length] = '\0';

            key_index = i + 1;
            number_of_params++;
        }
    }
    return number_of_params;
}