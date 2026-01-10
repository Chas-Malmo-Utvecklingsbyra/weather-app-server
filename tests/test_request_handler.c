#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/httpserver/request_handler/request_handler.h"

#define mu_assert(message, test) if (!(test)) { printf("FAIL: %s\n", message); return 1; }
#define mu_test(test_func) if (test_func()) { return 1; }

/**
 * Mock handler for testing
 */
HTTP_Status_Code test_mock_handler(QueryParameters_t *params, Request_Handler_Response_t *response, void *context)
{
    (void)params;
    (void)context;
    response->status_code = HTTP_STATUS_CODE_OK;
    response->response_data = malloc(50);
    if (response->response_data != NULL)
    {
        strcpy(response->response_data, "{\"message\":\"test response\"}");
    }
    return HTTP_STATUS_CODE_OK;
}

/**
 * Test: request_handler_set_response with OK status
 */
int test_request_handler_set_response_ok()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, "{\"test\":\"data\"}");
    
    mu_assert("status_code should be OK", response.status_code == HTTP_STATUS_CODE_OK);
    mu_assert("content_type should be JSON", response.content_type == HTTP_CONTENT_TYPE_JSON);
    mu_assert("response_data should be set", response.response_data != NULL);
    mu_assert("response_data should match input", strcmp(response.response_data, "{\"test\":\"data\"}") == 0);
    
    free(response.response_data);
    return 0;
}

/**
 * Test: request_handler_set_response with error status
 */
int test_request_handler_set_response_error()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
    
    mu_assert("status_code should be BAD_REQUEST", response.status_code == HTTP_STATUS_CODE_BAD_REQUEST);
    mu_assert("response_data should be error message", response.response_data != NULL);
    mu_assert("response should contain error", strstr(response.response_data, "Bad Request") != NULL);
    
    free(response.response_data);
    return 0;
}

/**
 * Test: request_handler_set_response with NULL response pointer
 */
int test_request_handler_set_response_null_response()
{
    /* Should not crash - function checks for NULL */
    request_handler_set_response(NULL, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, "{\"test\":\"data\"}");
    return 0;
}

/**
 * Test: request_handler_set_response with NOT_FOUND status
 */
int test_request_handler_set_response_not_found()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_NOT_FOUND, HTTP_CONTENT_TYPE_JSON, NULL);
    
    mu_assert("status_code should be NOT_FOUND", response.status_code == HTTP_STATUS_CODE_NOT_FOUND);
    mu_assert("response_data should be error message", response.response_data != NULL);
    mu_assert("response should contain error", strstr(response.response_data, "Not Found") != NULL);
    
    free(response.response_data);
    return 0;
}

/**
 * Test: request_handler_set_response with INTERNAL_SERVER_ERROR status
 */
int test_request_handler_set_response_server_error()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
    
    mu_assert("status_code should be INTERNAL_SERVER_ERROR", response.status_code == HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR);
    mu_assert("response_data should be error message", response.response_data != NULL);
    mu_assert("response should contain error", strstr(response.response_data, "Internal Server Error") != NULL);
    
    free(response.response_data);
    return 0;
}

/**
 * Test: request_handler_set_response with HTML content type
 */
int test_request_handler_set_response_html()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, "<h1>Test</h1>");
    
    mu_assert("status_code should be OK", response.status_code == HTTP_STATUS_CODE_OK);
    mu_assert("content_type should be HTML", response.content_type == HTTP_CONTENT_TYPE_HTML);
    mu_assert("response_data should match HTML", strcmp(response.response_data, "<h1>Test</h1>") == 0);
    
    free(response.response_data);
    return 0;
}

/**
 * Test: dispose_request_handler_response with data
 */
int test_dispose_request_handler_response_with_data()
{
    Request_Handler_Response_t response = {0};
    response.response_data = malloc(50);
    strcpy(response.response_data, "test data");
    
    dispose_request_handler_response(&response);
    
    mu_assert("response_data should be NULL after dispose", response.response_data == NULL);
    return 0;
}

/**
 * Test: dispose_request_handler_response without data
 */
int test_dispose_request_handler_response_null_data()
{
    Request_Handler_Response_t response = {0};
    response.response_data = NULL;
    
    dispose_request_handler_response(&response);
    
    mu_assert("response_data should remain NULL", response.response_data == NULL);
    return 0;
}

/**
 * Test: request_handler_handle_request with NULL request
 */
int test_request_handler_handle_request_null_request()
{
    RouteRegistry registry;
    route_registry_create(&registry, 10);
    
    Request_Handler_Response_t response = request_handler_handle_request(&registry, NULL);
    
    mu_assert("status_code should be BAD_REQUEST", response.status_code == HTTP_STATUS_CODE_BAD_REQUEST);
    mu_assert("response_data should be error", response.response_data != NULL);
    
    dispose_request_handler_response(&response);
    route_registry_dispose(&registry);
    return 0;
}

/**
 * Test: request_handler_handle_request with NULL registry
 */
int test_request_handler_handle_request_null_registry()
{
    /* When request is NULL, it returns BAD_REQUEST regardless of registry.
       This test just verifies it doesn't crash with NULL registry */
    Request_Handler_Response_t response = request_handler_handle_request(NULL, NULL);
    
    mu_assert("status_code should be BAD_REQUEST (checked before registry)", response.status_code == HTTP_STATUS_CODE_BAD_REQUEST);
    
    dispose_request_handler_response(&response);
    return 0;
}

/**
 * Test: request_handler_set_response with SERVICE_UNAVAILABLE
 */
int test_request_handler_set_response_service_unavailable()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_SERVICE_UNAVAILABLE, HTTP_CONTENT_TYPE_JSON, NULL);
    
    mu_assert("status_code should be SERVICE_UNAVAILABLE", response.status_code == HTTP_STATUS_CODE_SERVICE_UNAVAILABLE);
    mu_assert("response_data should be error message", response.response_data != NULL);
    mu_assert("response should contain error", strstr(response.response_data, "Service Unavailable") != NULL);
    
    free(response.response_data);
    return 0;
}

/**
 * Test: request_handler_set_response with BAD_GATEWAY
 */
int test_request_handler_set_response_bad_gateway()
{
    Request_Handler_Response_t response = {0};
    request_handler_set_response(&response, HTTP_STATUS_CODE_BAD_GATEWAY, HTTP_CONTENT_TYPE_JSON, NULL);
    
    mu_assert("status_code should be BAD_GATEWAY", response.status_code == HTTP_STATUS_CODE_BAD_GATEWAY);
    mu_assert("response_data should be error message", response.response_data != NULL);
    mu_assert("response should contain error", strstr(response.response_data, "Bad Gateway") != NULL);
    
    free(response.response_data);
    return 0;
}

int main(void)
{
    printf("\nRunning request_handler tests...\n");

    mu_test(test_request_handler_set_response_ok);
    mu_test(test_request_handler_set_response_error);
    mu_test(test_request_handler_set_response_null_response);
    mu_test(test_request_handler_set_response_not_found);
    mu_test(test_request_handler_set_response_server_error);
    mu_test(test_request_handler_set_response_html);
    mu_test(test_dispose_request_handler_response_with_data);
    mu_test(test_dispose_request_handler_response_null_data);
    mu_test(test_request_handler_handle_request_null_request);
    mu_test(test_request_handler_handle_request_null_registry);
    mu_test(test_request_handler_set_response_service_unavailable);
    mu_test(test_request_handler_set_response_bad_gateway);
    return 0;
}
