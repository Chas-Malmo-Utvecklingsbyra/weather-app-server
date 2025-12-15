#include "route_matcher.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Local function - Checks if a request path matches a route pattern.
 *
 * Performs exact matching, accounting for query parameters.
 * Examples:
 *  - "/v1/weather" matches "/v1/weather"
 *  - "/v1/weather" matches "/v1/weather?lat=1&lon=2"
 *  - "/v1/weather" does NOT match "/v1/weather-extended"
 *
 * @param request_path The full path from the HTTP request (may include query params)
 * @param route_pattern The route pattern to match against
 * @return true if the path matches the pattern, false otherwise
 */
static bool route_matcher_matches_path(const char *request_path, const char *route_pattern)
{
    if (request_path == NULL || route_pattern == NULL)
    {
        return false;
    }
    
    if (strcmp(request_path, route_pattern) == 0)
    {
        return true; /* Exact match */
    }

    /* Pattern: route_pattern must match the start of request_path, */
    /* character after must be '?' or '\0' */
    size_t route_len = strlen(route_pattern);
    if (strncmp(request_path, route_pattern, route_len) == 0)
    {
        char next_char = request_path[route_len];
        if (next_char == '?' || next_char == '\0')
        {
            return true;
        }
    }
    return false;
}

bool route_matcher_matches(const char *request_path, const char *request_method, const char *route_pattern, const char *route_method)
{
    if (request_method == NULL || route_method == NULL)
    {
        return false;
    }

    if (strcmp(request_method, route_method) != 0)
    {
        return false;
    }

    return route_matcher_matches_path(request_path, route_pattern);
}

/* Not used atm TODO: LS remove? */
int route_matcher_extract_path(const char *full_path, char *buffer, size_t buffer_size)
{
    if (full_path == NULL || buffer == NULL || buffer_size == 0)
        return -1;

    const char *query_start = strchr(full_path, '?');
    size_t path_len;

    if (query_start != NULL)
    {
        path_len = query_start - full_path;
    }
    else
    {
        path_len = strlen(full_path);
    }

    if (path_len >= buffer_size)
        return -1; /* Buffer too small */

    strncpy(buffer, full_path, path_len);
    buffer[path_len] = '\0';

    return 0;
}