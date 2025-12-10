#include "route_matcher.h"
#include <string.h>

bool route_matcher_matches_path(const char *request_path, const char *route_pattern)
{
    if (request_path == NULL || route_pattern == NULL)
        return false;

    size_t route_len = strlen(route_pattern);

    // Exact match
    if (strcmp(request_path, route_pattern) == 0)
        return true;

    // Match with query parameters
    // Pattern: route_pattern must match the start of request_path,
    // and the character after must be '?' or '\0'
    if (strncmp(request_path, route_pattern, route_len) == 0)
    {
        char next_char = request_path[route_len];
        if (next_char == '?' || next_char == '\0')
            return true;
    }

    return false;
}

bool route_matcher_matches(const char *request_path, const char *request_method, const char *route_pattern, const char *route_method)
{
    if (request_method == NULL || route_method == NULL)
        return false;

    // Check method first (cheaper operation)
    if (strcmp(request_method, route_method) != 0)
        return false;

    // Then check path
    return route_matcher_matches_path(request_path, route_pattern);
}

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
        return -1; // Buffer too small

    strncpy(buffer, full_path, path_len);
    buffer[path_len] = '\0';

    return 0;
}