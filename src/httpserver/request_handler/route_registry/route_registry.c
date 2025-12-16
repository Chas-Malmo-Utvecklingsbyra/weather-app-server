#include "route_registry.h"
#include "../route_matcher/route_matcher.h"
#include <stdlib.h>
#include <string.h>

RouteRegistry* route_registry_create(size_t capacity)
{
    if (capacity == 0)
        return NULL;

    RouteRegistry *registry = malloc(sizeof(RouteRegistry));
    if (registry == NULL)
        return NULL;

    registry->entries = malloc(sizeof(RouteRegistry_Entry) * capacity);
    if (registry->entries == NULL)
    {
        free(registry);
        return NULL;
    }

    registry->count = 0;
    registry->capacity = capacity;
    return registry;
}

int route_registry_register(RouteRegistry *registry, const char *path, const char *method, size_t args_count, RouteHandler handler)
{
    if (registry == NULL || path == NULL || method == NULL || handler == NULL)
    {
        return -1;
    }

    /* Make it dynamically resizable? */
    /*
    if (registry->count >= registry->capacity)
    {
        printf("Expanding route registry from capacity %zu\n", registry->capacity);
        size_t new_capacity = registry->capacity * 2;
        RouteRegistry_Entry *new_entries = realloc(registry->entries, sizeof(RouteRegistry_Entry));
        if (new_entries == NULL)
            return -1;
    
        registry->entries = new_entries;
        registry->capacity = new_capacity;
    }
    */

    registry->entries[registry->count].path = path;
    registry->entries[registry->count].method = method;
    registry->entries[registry->count].args_count = args_count;
    registry->entries[registry->count].handler = handler;
    registry->count++;

    return 0;
}

HTTP_Status_Code route_registry_dispatch(RouteRegistry *registry, const char *path, const char *method, Request_Handler_Response_t *request_handler_response)
{
    bool params_initialized = false;
    
    if (registry == NULL || path == NULL || method == NULL || request_handler_response == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }
    
    /* Search for matching route */
    size_t i = 0;
    for (; i < registry->count; i++)
    {
        if (route_matcher_matches(path, method, registry->entries[i].path, registry->entries[i].method))
        {
            QueryParameters_t query_parameters = {0};
            memset(&query_parameters, 0, sizeof(QueryParameters_t));
            
            if (registry->entries[i].args_count > 0) /* Parse query parameters if expected */
            {
                if (query_parameter_create(&query_parameters, registry->entries[i].args_count) != 0)
                {
                    request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
                    return request_handler_response->status_code;
                }
                params_initialized = true;
                
                if (query_parameter_parse(&query_parameters, path) != 0)
                {
                    query_parameter_dispose(&query_parameters);
                    request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
                    return request_handler_response->status_code;
                }
            }
            /* Call the handler, expected to return HTTP status code */
            registry->entries[i].handler(&query_parameters, request_handler_response);
            if (params_initialized == true)
            {
                query_parameter_dispose(&query_parameters);
                params_initialized = false;
            }
            
            return request_handler_response->status_code;
        }
    }
    request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_NOT_FOUND, HTTP_CONTENT_TYPE_JSON, NULL);
    return request_handler_response->status_code; /* No matching route found */
}

void route_registry_dispose(RouteRegistry *registry)
{
    if (registry == NULL)
        return;

    if (registry->entries != NULL)
        free(registry->entries);

    free(registry);
}
