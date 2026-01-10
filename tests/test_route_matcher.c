#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../src/httpserver/request_handler/route_matcher/route_matcher.h"

#define mu_assert(message, test) if (!(test)) { printf("FAIL: %s\n", message); return 1; }
#define mu_test(test_func) if (test_func()) { return 1; }

/**
 * Test: route_matcher_matches exact path match
 */
int test_route_matcher_matches_exact_path()
{
    bool result = route_matcher_matches("/v1/weather", "GET", "/v1/weather", "GET");
    mu_assert("Exact path match should return true", result == true);
    return 0;
}

/**
 * Test: route_matcher_matches path with query parameters
 */
int test_route_matcher_matches_path_with_query()
{
    bool result = route_matcher_matches("/v1/weather?lat=40&lon=-74", "GET", "/v1/weather", "GET");
    mu_assert("Path with query params should match route pattern", result == true);
    return 0;
}

/**
 * Test: route_matcher_matches different paths
 */
int test_route_matcher_matches_different_paths()
{
    bool result = route_matcher_matches("/v1/city", "GET", "/v1/weather", "GET");
    mu_assert("Different paths should not match", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches similar but distinct paths
 */
int test_route_matcher_matches_similar_paths()
{
    bool result = route_matcher_matches("/v1/weather-extended", "GET", "/v1/weather", "GET");
    mu_assert("Similar but distinct paths should not match", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches different HTTP methods
 */
int test_route_matcher_matches_different_methods()
{
    bool result = route_matcher_matches("/v1/weather", "POST", "/v1/weather", "GET");
    mu_assert("Different HTTP methods should not match", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches case sensitivity
 */
int test_route_matcher_matches_case_sensitive()
{
    bool result = route_matcher_matches("/V1/Weather", "GET", "/v1/weather", "GET");
    mu_assert("Different case paths should not match", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches root path
 */
int test_route_matcher_matches_root_path()
{
    bool result = route_matcher_matches("/", "GET", "/", "GET");
    mu_assert("Root path should match", result == true);
    return 0;
}

/**
 * Test: route_matcher_matches root path with query
 */
int test_route_matcher_matches_root_path_with_query()
{
    bool result = route_matcher_matches("/?param=value", "GET", "/", "GET");
    mu_assert("Root path with query should match root pattern", result == true);
    return 0;
}

/**
 * Test: route_matcher_matches NULL request_path
 */
int test_route_matcher_matches_null_request_path()
{
    bool result = route_matcher_matches(NULL, "GET", "/v1/weather", "GET");
    mu_assert("NULL request path should return false", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches NULL route_pattern
 */
int test_route_matcher_matches_null_route_pattern()
{
    bool result = route_matcher_matches("/v1/weather", "GET", NULL, "GET");
    mu_assert("NULL route pattern should return false", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches NULL request_method
 */
int test_route_matcher_matches_null_request_method()
{
    bool result = route_matcher_matches("/v1/weather", NULL, "/v1/weather", "GET");
    mu_assert("NULL request method should return false", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches NULL route_method
 */
int test_route_matcher_matches_null_route_method()
{
    bool result = route_matcher_matches("/v1/weather", "GET", "/v1/weather", NULL);
    mu_assert("NULL route method should return false", result == false);
    return 0;
}

/**
 * Test: route_matcher_matches POST method
 */
int test_route_matcher_matches_post_method()
{
    bool result = route_matcher_matches("/api/data", "POST", "/api/data", "POST");
    mu_assert("POST method should match", result == true);
    return 0;
}

/**
 * Test: route_matcher_matches multiple query params
 */
int test_route_matcher_matches_multiple_query_params()
{
    bool result = route_matcher_matches("/v1/weather?lat=40&lon=-74&units=metric", "GET", "/v1/weather", "GET");
    mu_assert("Multiple query params should match", result == true);
    return 0;
}

/**
 * Test: route_matcher_extract_path simple path
 */
int test_route_matcher_extract_path_simple()
{
    char buffer[256];
    int result = route_matcher_extract_path("/weather", buffer, sizeof(buffer));
    
    mu_assert("route_matcher_extract_path should return 0 on success", result == 0);
    mu_assert("Extracted path should match", strcmp(buffer, "/weather") == 0);
    return 0;
}

/**
 * Test: route_matcher_extract_path with query string
 */
int test_route_matcher_extract_path_with_query()
{
    char buffer[256];
    int result = route_matcher_extract_path("/weather?lat=40&lon=-74", buffer, sizeof(buffer));
    
    mu_assert("route_matcher_extract_path should return 0", result == 0);
    mu_assert("Extracted path should not include query", strcmp(buffer, "/weather") == 0);
    return 0;
}

/**
 * Test: route_matcher_extract_path NULL full_path
 */
int test_route_matcher_extract_path_null_path()
{
    char buffer[256];
    int result = route_matcher_extract_path(NULL, buffer, sizeof(buffer));
    
    mu_assert("route_matcher_extract_path should return -1 for NULL path", result == -1);
    return 0;
}

/**
 * Test: route_matcher_extract_path NULL buffer
 */
int test_route_matcher_extract_path_null_buffer()
{
    int result = route_matcher_extract_path("/weather", NULL, 256);
    
    mu_assert("route_matcher_extract_path should return -1 for NULL buffer", result == -1);
    return 0;
}

/**
 * Test: route_matcher_extract_path buffer too small
 */
int test_route_matcher_extract_path_buffer_too_small()
{
    char buffer[5];
    int result = route_matcher_extract_path("/very/long/path", buffer, sizeof(buffer));
    
    mu_assert("route_matcher_extract_path should return -1 for buffer too small", result == -1);
    return 0;
}

/**
 * Test: route_matcher_extract_path zero buffer size
 */
int test_route_matcher_extract_path_zero_buffer()
{
    char buffer[256];
    int result = route_matcher_extract_path("/weather", buffer, 0);
    
    mu_assert("route_matcher_extract_path should return -1 for zero buffer size", result == -1);
    return 0;
}

int main(void)
{
    printf("\nRunning route_matcher tests...\n");

    mu_test(test_route_matcher_matches_exact_path);
    mu_test(test_route_matcher_matches_path_with_query);
    mu_test(test_route_matcher_matches_different_paths);
    mu_test(test_route_matcher_matches_similar_paths);
    mu_test(test_route_matcher_matches_different_methods);
    mu_test(test_route_matcher_matches_case_sensitive);
    mu_test(test_route_matcher_matches_root_path);
    mu_test(test_route_matcher_matches_root_path_with_query);
    mu_test(test_route_matcher_matches_null_request_path);
    mu_test(test_route_matcher_matches_null_route_pattern);
    mu_test(test_route_matcher_matches_null_request_method);
    mu_test(test_route_matcher_matches_null_route_method);
    mu_test(test_route_matcher_matches_post_method);
    mu_test(test_route_matcher_matches_multiple_query_params);
    mu_test(test_route_matcher_extract_path_simple);
    mu_test(test_route_matcher_extract_path_with_query);
    mu_test(test_route_matcher_extract_path_null_path);
    mu_test(test_route_matcher_extract_path_null_buffer);
    mu_test(test_route_matcher_extract_path_buffer_too_small);
    mu_test(test_route_matcher_extract_path_zero_buffer);
    return 0;
}
