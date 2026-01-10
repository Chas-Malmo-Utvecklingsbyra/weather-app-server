#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../src/httpserver/request_handler/route_registry/route_registry.h"

#define mu_assert(message, test) if (!(test)) { printf("FAIL: %s\n", message); return 1; }
#define mu_test(test_func) if (test_func()) { return 1; }

/**
 * Mock handler that returns a simple response
 */
HTTP_Status_Code mock_handler_ok(QueryParameters_t *params, Request_Handler_Response_t *response, void *context)
{
    (void)params;
    (void)context;
    response->status_code = HTTP_STATUS_CODE_OK;
    response->response_data = malloc(20);
    if (response->response_data != NULL)
    {
        strcpy(response->response_data, "{\"status\":\"ok\"}");
    }
    return HTTP_STATUS_CODE_OK;
}

/**
 * Mock handler that returns an error
 */
HTTP_Status_Code mock_handler_error(QueryParameters_t *params, Request_Handler_Response_t *response, void *context)
{
    (void)params;
    (void)response;
    (void)context;
    return HTTP_STATUS_CODE_BAD_REQUEST;
}

/**
 * Mock handler that uses context
 */
HTTP_Status_Code mock_handler_with_context(QueryParameters_t *params, Request_Handler_Response_t *response, void *context)
{
    (void)params;
    (void)response;
    if (context != NULL)
    {
        const char *context_str = (const char *)context;
        if (strcmp(context_str, "test_context") == 0)
        {
            return HTTP_STATUS_CODE_OK;
        }
    }
    return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
}

/**
 * Test: route_registry_create with valid capacity
 */
int test_route_registry_create_success()
{
    RouteRegistry registry;
    bool result = route_registry_create(&registry, 10);
    
    mu_assert("route_registry_create should return true on success", result == true);
    mu_assert("count should be 0", registry.count == 0);
    mu_assert("capacity should be 10", registry.capacity == 10);
    mu_assert("entries should be allocated", registry.entries != NULL);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_create with NULL registry
 */
int test_route_registry_create_null_registry()
{
    bool result = route_registry_create(NULL, 10);
    mu_assert("route_registry_create should return false for NULL registry", result == false);
    return 0;
}

/**
 * Test: route_registry_create with zero capacity
 */
int test_route_registry_create_zero_capacity()
{
    RouteRegistry registry;
    bool result = route_registry_create(&registry, 0);
    mu_assert("route_registry_create should return false for zero capacity", result == false);
    return 0;
}

/**
 * Test: route_registry_register single route
 */
int test_route_registry_register_single()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    int result = route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    
    mu_assert("route_registry_register should return 0 on success", result == 0);
    mu_assert("count should be incremented to 1", registry.count == 1);
    mu_assert("path should be stored", strcmp(registry.entries[0].path, "/weather") == 0);
    mu_assert("method should be stored", strcmp(registry.entries[0].method, "GET") == 0);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_register multiple routes
 */
int test_route_registry_register_multiple()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    route_registry_register(&registry, "/city", "GET", 0, mock_handler_ok, NULL);
    
    mu_assert("count should be 2", registry.count == 2);
    mu_assert("first route path correct", strcmp(registry.entries[0].path, "/weather") == 0);
    mu_assert("second route path correct", strcmp(registry.entries[1].path, "/city") == 0);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_register with NULL registry
 */
int test_route_registry_register_null_registry()
{
    int result = route_registry_register(NULL, "/weather", "GET", 0, mock_handler_ok, NULL);
    mu_assert("route_registry_register should return -1 for NULL registry", result == -1);
    return 0;
}

/**
 * Test: route_registry_register with NULL path
 */
int test_route_registry_register_null_path()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    int result = route_registry_register(&registry, NULL, "GET", 0, mock_handler_ok, NULL);
    
    mu_assert("route_registry_register should return -1 for NULL path", result == -1);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_register with NULL method
 */
int test_route_registry_register_null_method()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    int result = route_registry_register(&registry, "/weather", NULL, 0, mock_handler_ok, NULL);
    
    mu_assert("route_registry_register should return -1 for NULL method", result == -1);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_register with NULL handler
 */
int test_route_registry_register_null_handler()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    int result = route_registry_register(&registry, "/weather", "GET", 0, NULL, NULL);
    
    mu_assert("route_registry_register should return -1 for NULL handler", result == -1);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_register at capacity
 */
int test_route_registry_register_at_capacity()
{
    RouteRegistry registry;
    route_registry_create(&registry, 1);
    
    route_registry_register(&registry, "/route1", "GET", 0, mock_handler_ok, NULL);
    int result = route_registry_register(&registry, "/route2", "GET", 0, mock_handler_ok, NULL);
    
    mu_assert("route_registry_register should return -1 when registry at capacity", result == -1);
    mu_assert("count should still be 1", registry.count == 1);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_register with context
 */
int test_route_registry_register_with_context()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    char *context = malloc(20);
    strcpy(context, "test_context");
    
    int result = route_registry_register(&registry, "/api", "POST", 0, mock_handler_with_context, context);
    
    mu_assert("route_registry_register should succeed with context", result == 0);
    mu_assert("context should be stored", registry.entries[0].context != NULL);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_dispatch route found
 */
int test_route_registry_dispatch_found()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    
    Request_Handler_Response_t response = {0};
    HTTP_Status_Code status = route_registry_dispatch(&registry, "/weather", "GET", &response);
    
    mu_assert("dispatch should return HTTP_STATUS_CODE_OK", status == HTTP_STATUS_CODE_OK);
    mu_assert("response should be set", response.response_data != NULL);
    
    free(response.response_data);
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_dispatch route not found
 */
int test_route_registry_dispatch_not_found()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    
    Request_Handler_Response_t response = {0};
    HTTP_Status_Code status = route_registry_dispatch(&registry, "/notfound", "GET", &response);
    
    mu_assert("dispatch should return HTTP_STATUS_CODE_NOT_FOUND", status == HTTP_STATUS_CODE_NOT_FOUND);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_dispatch with query parameters
 */
int test_route_registry_dispatch_with_query()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    route_registry_register(&registry, "/weather", "GET", 2, mock_handler_ok, NULL);
    
    Request_Handler_Response_t response = {0};
    HTTP_Status_Code status = route_registry_dispatch(&registry, "/weather?lat=40&lon=-74", "GET", &response);
    
    mu_assert("dispatch should return HTTP_STATUS_CODE_OK", status == HTTP_STATUS_CODE_OK);
    
    free(response.response_data);
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_dispatch NULL registry
 */
int test_route_registry_dispatch_null_registry()
{
    Request_Handler_Response_t response = {0};
    HTTP_Status_Code status = route_registry_dispatch(NULL, "/weather", "GET", &response);
    
    mu_assert("dispatch should return error for NULL registry", status == HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR);
    return 0;
}

/**
 * Test: route_registry_dispatch NULL path
 */
int test_route_registry_dispatch_null_path()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    
    Request_Handler_Response_t response = {0};
    HTTP_Status_Code status = route_registry_dispatch(&registry, NULL, "GET", &response);
    
    mu_assert("dispatch should return error for NULL path", status == HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_dispatch NULL response
 */
int test_route_registry_dispatch_null_response()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    
    HTTP_Status_Code status = route_registry_dispatch(&registry, "/weather", "GET", NULL);
    
    mu_assert("dispatch should return error for NULL response", status == HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR);
    
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: route_registry_dispose
 */
int test_route_registry_dispose()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    route_registry_register(&registry, "/weather", "GET", 0, mock_handler_ok, NULL);
    
    route_registry_dispose(&registry);
    
    mu_assert("After dispose, entries should be NULL", registry.entries == NULL);
    mu_assert("After dispose, count should be 0", registry.count == 0);
    mu_assert("After dispose, capacity should be 0", registry.capacity == 0);
    
    return 0;
}

int main(void)
{
    printf("\nRunning route_registry tests...\n");

    mu_test(test_route_registry_create_success);
    mu_test(test_route_registry_create_null_registry);
    mu_test(test_route_registry_create_zero_capacity);
    mu_test(test_route_registry_register_single);
    mu_test(test_route_registry_register_multiple);
    mu_test(test_route_registry_register_null_registry);
    mu_test(test_route_registry_register_null_path);
    mu_test(test_route_registry_register_null_method);
    mu_test(test_route_registry_register_null_handler);
    mu_test(test_route_registry_register_at_capacity);
    mu_test(test_route_registry_register_with_context);
    mu_test(test_route_registry_dispatch_found);
    mu_test(test_route_registry_dispatch_not_found);
    mu_test(test_route_registry_dispatch_with_query);
    mu_test(test_route_registry_dispatch_null_registry);
    mu_test(test_route_registry_dispatch_null_path);
    mu_test(test_route_registry_dispatch_null_response);
    mu_test(test_route_registry_dispose);
    return 0;
}
