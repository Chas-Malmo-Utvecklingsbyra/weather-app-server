#include "server_routes.h"
#include "core/config/config.h"
#include <string.h>
#include "core/weather/http.h"
#include "core/weather/api/openmeteo.h"
#include "core/string/strdup.h"
#include <stdio.h>

int handle_route(Http_Request *request, char *json_response)
{
    Config_t* cfg = config_get_instance(NULL);

    char* path = request->start_line.path;
    char* method = request->start_line.method;
    int error_code = 0;
    
    size_t i;
    for (i = 0; i < cfg->allowed_routes_count; i++)
    {
        if (strstr(path, cfg->allowed_routes[i].route) != NULL)
        {
            if (strcmp(cfg->allowed_routes[i].method, method) == 0)
            {
                Http h = {0};
                http_initialize(&h);
                
                struct curl_slist* headers = NULL;

                http_get(&h, OPENMETEO_API, json_response, headers);

                if(json_response == NULL)
                    error_code = - 1;
                
                http_dispose(&h);
                
                return error_code;
            }
        }
        else
        {
            // return error
            continue;
        }
    }

    return 0;
}

int get_query_params(const char* path, char* key, char* value)
{
    size_t number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;
    
    for (int i = 0; i < strlen(path); i++)
    {
        if(path[i] == '?' || path[i] == '&')
        {
            number_of_params++;
            key_index = i;
        }
        else if(path[i] == '=' && number_of_params == 1)
        {
            strncpy(key, &path[key_index + 1], i - key_index - 1);
            value_index = i + 1;
        }
        else if((path[i] == '&' || i == strlen(path) -1))
        {
            strncpy(value, &path[value_index], i - value_index + 1);
        }
        
    }
    return 0;
}