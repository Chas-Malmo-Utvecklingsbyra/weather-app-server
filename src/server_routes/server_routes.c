#include "server_routes.h"
#include "core/config/config.h"
#include <string.h>
#include "core/weather/http.h"
#include "core/weather/api/openmeteo.h"
#include "core/string/strdup.h"
#include <stdio.h>
#include "core/http/parser.h"


/**
 * @brief Handles routing for incoming HTTP requests.
 * @param request Pointer to the Http_Request structure containing the request details.
 * @param response Pointer to a char pointer where the JSON response will be stored.
 * @return Route_Handler_Result Result of the route handling operation.
 * @note The caller is responsible for freeing the memory allocated for the response.
 */
Route_Handler_Result handle_route(Http_Request *request, char **response)
{
    Config_t* cfg = config_get_instance(NULL);

    if (cfg->config_debug)
        printf("Requested path: %s\n", request->start_line.path);

    char* method = Http_Request_Get_Method_String(request);
    
    size_t i;
    for (i = 0; i < cfg->allowed_routes_count; i++)
    {
        if (cfg->config_debug)
            printf("Checking allowed route: %s %s\n", cfg->allowed_routes[i].method, cfg->allowed_routes[i].route);
        
        /* Check if the request path matches any allowed route */
        if (strstr(request->start_line.path, cfg->allowed_routes[i].route) != NULL)
        {
            if (strcmp(cfg->allowed_routes[i].method, method) == 0)
            {
                if(cfg->config_debug) 
                    printf("Matched allowed route: %s %s\n", cfg->allowed_routes[i].method, cfg->allowed_routes[i].route);
                    
                /* Populate needed headers or NULL*/
                const char **headers = NULL;

                // char *key[] = {NULL};
                // char *value[] = {NULL};
                // get_query_params(request->start_line.path, cfg->allowed_routes[i].args_count, key, value);

                /* build the URL with query parameters */
                const char *OPENMETEO_API_URL = "https://api.open-meteo.com/v1/forecast?latitude=55.707832&longitude=13.1866455";
                
                char* json_response;
                http_get(OPENMETEO_API_URL, &json_response, headers);

                if (cfg->config_debug)
                    printf("Received JSON response: %s\n", json_response);

                *response = strdup(json_response);
                    
                if (cfg->config_debug)
                    printf("Response set to: %s\n", *response);
                
                if(*response == NULL)
                    return Route_Handler_Result_Error;

                return Route_Handler_Result_OK;
            }
        }
    }
    return Route_Handler_Result_Error;
}

/**
 * @brief Extracts query parameters from a URL path.
 * @param path The URL path containing query parameters.
 * @param key Array of char pointers to store extracted keys.
 * @param value Array of char pointers to store extracted values.
 * @return int Number of query parameters extracted.
 * @note work in progress
 */
int get_query_params(const char *path, const int args_count, char *key[], char *value[])
{
    Config_t *cfg = config_get_instance(NULL);
    size_t number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;
    size_t i = 0;

    for (i = 0; i < strlen(path); i++)
    {
        if(path[i] == '?' || path[i] == '&')
        {
            number_of_params++;
            key_index = i+1;
        }
        else if(path[i] == '=' && number_of_params > 0)
        {
            key[number_of_params - 1] = (char*)malloc((i - key_index) * sizeof(char));
            strncpy(key[number_of_params - 1], &path[key_index], i - key_index - 1);
            value_index = i + 1;
            if (cfg->config_debug) printf("Key: %s\n", key[number_of_params - 1]);
        }
        else if((path[i] == '&' || i == strlen(path) -1))
        {
            value[number_of_params - 1] = (char*)malloc((i - value_index + 2) * sizeof(char));
            strncpy(value[number_of_params - 1], &path[value_index], i - value_index + 1);
            
            if(cfg->config_debug) 
                printf("Value: %s\n", value[number_of_params - 1]);
                
            key_index = i+1;
            value_index = 0;
        }
    }
    return 0;
}