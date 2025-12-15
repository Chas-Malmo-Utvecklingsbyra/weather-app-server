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

int route_registry_get_args_count(RouteRegistry *registry, const char *path, const char *method)
{
    if (registry == NULL || path == NULL || method == NULL)
    {
        return -1;
    }
    /* Search for matching route */
    for (size_t i = 0; i < registry->count; i++)
    {
        if (route_matcher_matches(path, method, registry->entries[i].path, registry->entries[i].method))
        {
            return (int)registry->entries[i].args_count;
        }
    }
    return -1;
}

int route_registry_dispatch(RouteRegistry *registry, const char *path, const char *method, QueryParameters_t *params, Request_Handler_Response_t *response)
{
    if (registry == NULL || path == NULL || method == NULL || response == NULL)
    {
        return -1;
    }

    /* Search for matching route */
    for (size_t i = 0; i < registry->count; i++)
    {
        if (route_matcher_matches(path, method, registry->entries[i].path, registry->entries[i].method))
        {
            return registry->entries[i].handler(params, response); /* Call the handler, expected to return HTTP status code */
        }
    }
    return -1; /* No matching route found */
}

void route_registry_dispose(RouteRegistry *registry)
{
    if (registry == NULL)
        return;

    if (registry->entries != NULL)
        free(registry->entries);

    free(registry);
}
