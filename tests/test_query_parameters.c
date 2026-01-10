#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/httpserver/request_handler/query_parameters/query_parameters.h"

#define mu_assert(message, test) if (!(test)) { printf("FAIL: %s\n", message); return 1; }
#define mu_test(test_func) if (test_func()) { return 1; }

/**
 * Test: query_parameter_create with valid capacity
 */
int test_query_parameter_create_success()
{
    QueryParameters_t params;
    int result = query_parameter_create(&params, 5);
    
    mu_assert("query_parameter_create should return 0 on success", result == 0);
    mu_assert("count should be initialized to 0", params.count == 0);
    mu_assert("capacity should be set to requested size", params.capacity == 5);
    mu_assert("keys should be allocated", params.keys != NULL);
    mu_assert("values should be allocated", params.values != NULL);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_create with NULL pointer
 */
int test_query_parameter_create_null_pointer()
{
    int result = query_parameter_create(NULL, 5);
    mu_assert("query_parameter_create should return -1 for NULL pointer", result == -1);
    return 0;
}

/**
 * Test: query_parameter_create with zero capacity
 */
int test_query_parameter_create_zero_capacity()
{
    QueryParameters_t params;
    int result = query_parameter_create(&params, 0);
    mu_assert("query_parameter_create should return -1 for zero capacity", result == -1);
    return 0;
}

/**
 * Test: query_parameter_parse simple query string
 */
int test_query_parameter_parse_simple()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    
    int result = query_parameter_parse(&params, "/path?key=value");
    
    mu_assert("query_parameter_parse should return 0 on success", result == 0);
    mu_assert("count should be 1 after parsing one parameter", params.count == 1);
    mu_assert("key should match", strcmp(params.keys[0], "key") == 0);
    mu_assert("value should match", strcmp(params.values[0], "value") == 0);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_parse multiple parameters
 */
int test_query_parameter_parse_multiple()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    
    int result = query_parameter_parse(&params, "/weather?lat=40.7&lon=-74.0");
    
    mu_assert("query_parameter_parse should return 0 on success", result == 0);
    mu_assert("count should be 2", params.count == 2);
    mu_assert("first key should be 'lat'", strcmp(params.keys[0], "lat") == 0);
    mu_assert("first value should be '40.7'", strcmp(params.values[0], "40.7") == 0);
    mu_assert("second key should be 'lon'", strcmp(params.keys[1], "lon") == 0);
    mu_assert("second value should be '-74.0'", strcmp(params.values[1], "-74.0") == 0);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_parse with no query parameters
 */
int test_query_parameter_parse_no_params()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    
    int result = query_parameter_parse(&params, "/weather");
    
    mu_assert("query_parameter_parse should return -1 when no params present", result == -1);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_parse with NULL pointer
 */
int test_query_parameter_parse_null_param()
{
    int result = query_parameter_parse(NULL, "/weather?lat=40");
    mu_assert("query_parameter_parse should return -1 for NULL param", result == -1);
    return 0;
}

/**
 * Test: query_parameter_parse with NULL path
 */
int test_query_parameter_parse_null_path()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    
    int result = query_parameter_parse(&params, NULL);
    
    mu_assert("query_parameter_parse should return -1 for NULL path", result == -1);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_parse with empty value
 */
int test_query_parameter_parse_empty_value()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    
    int result = query_parameter_parse(&params, "/weather?key=");
    
    mu_assert("query_parameter_parse should return -1 for empty value", result == -1);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_parse with empty key
 */
int test_query_parameter_parse_empty_key()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    
    int result = query_parameter_parse(&params, "/weather?=value");
    
    mu_assert("query_parameter_parse should return -1 for empty key", result == -1);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_get existing key
 */
int test_query_parameter_get_existing()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    query_parameter_parse(&params, "/weather?lat=40.7&lon=-74.0");
    
    const char *value = query_parameter_get(&params, "lat");
    
    mu_assert("query_parameter_get should return the correct value", value != NULL && strcmp(value, "40.7") == 0);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_get non-existing key
 */
int test_query_parameter_get_nonexisting()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    query_parameter_parse(&params, "/weather?lat=40.7&lon=-74.0");
    
    const char *value = query_parameter_get(&params, "elevation");
    
    mu_assert("query_parameter_get should return NULL for non-existing key", value == NULL);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_get with NULL key
 */
int test_query_parameter_get_null_key()
{
    QueryParameters_t params;
    query_parameter_create(&params, 10);
    query_parameter_parse(&params, "/weather?lat=40.7");
    
    const char *value = query_parameter_get(&params, NULL);
    
    mu_assert("query_parameter_get should return NULL for NULL key", value == NULL);
    
    query_parameter_dispose(&params);
    return 0;
}

/**
 * Test: query_parameter_dispose
 */
int test_query_parameter_dispose()
{
    QueryParameters_t params;
    query_parameter_create(&params, 5);
    query_parameter_parse(&params, "/path?key=value");
    
    query_parameter_dispose(&params);
    
    mu_assert("After dispose, keys should be NULL", params.keys == NULL);
    mu_assert("After dispose, values should be NULL", params.values == NULL);
    
    return 0;
}

int main(void)
{
    printf("\nRunning query_parameters tests...\n");

    mu_test(test_query_parameter_create_success);
    mu_test(test_query_parameter_create_null_pointer);
    mu_test(test_query_parameter_create_zero_capacity);
    mu_test(test_query_parameter_parse_simple);
    mu_test(test_query_parameter_parse_multiple);
    mu_test(test_query_parameter_parse_no_params);
    mu_test(test_query_parameter_parse_null_param);
    mu_test(test_query_parameter_parse_null_path);
    mu_test(test_query_parameter_parse_empty_value);
    mu_test(test_query_parameter_parse_empty_key);
    mu_test(test_query_parameter_get_existing);
    mu_test(test_query_parameter_get_nonexisting);
    mu_test(test_query_parameter_get_null_key);
    mu_test(test_query_parameter_dispose);
    return 0;
}
