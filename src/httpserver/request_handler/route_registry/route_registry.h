#ifndef ROUTE_REGISTRY_H
#define ROUTE_REGISTRY_H

#include "../request_handler.h"
#include "../query_parameters/query_parameters.h"

/**
 * @brief Single route entry in the registry
 */
typedef struct
{
    const char *path;           /* Route path (e.g., "/v1/weather") */
    const char *method;         /* HTTP method (e.g., "GET") */
    size_t args_count;          /* Expected number of arguments */
    RouteHandler handler;       /* Handler function for this route */
} RouteRegistry_Entry;

/**
 * @brief Route registry - manages all registered routes
 */
typedef struct
{
    RouteRegistry_Entry *entries;
    size_t count;
    size_t capacity;
} RouteRegistry;

/**
 * @brief Create a new route registry with initial capacity
 * @param capacity Initial number of routes to allocate space for
 * @return Pointer to new RouteRegistry, or NULL on error
 */
RouteRegistry* route_registry_create(size_t capacity);

/**
 * @brief Register a new route
 * @param registry Pointer to the registry
 * @param path Route path (e.g., "/v1/weather")
 * @param method HTTP method (e.g., "GET")
 * @param args_count Expected number of query parameters
 * @param handler Function pointer to handle this route
 * @return 0 on success, -1 on error
 */
int route_registry_register(RouteRegistry *registry, const char *path, const char *method, size_t args_count, RouteHandler handler);

/**
 * @brief Find and execute a handler for a request
 * @param registry Pointer to the registry
 * @param path Request path
 * @param method HTTP method
 * @param params Query parameters (may be NULL)
 * @param response HTTP response to populate
 * @return 0 on success, -1 if no matching route found
 */
int route_registry_dispatch(RouteRegistry *registry, const char *path, const char *method, QueryParameters_t *params, Request_Handler_Response_t *response);

/**
 * @brief Get the argument count for a matched route
 * @param registry Pointer to the registry
 * @param path Request path
 * @param method HTTP method
 * @return Number of expected arguments, or -1 if route not found
 */
int route_registry_get_args_count(RouteRegistry *registry, const char *path, const char *method);

/**
 * @brief Free all resources associated with the registry
 * @param registry Pointer to the registry
 */
void route_registry_dispose(RouteRegistry *registry);

#endif /* ROUTE_REGISTRY_H */
