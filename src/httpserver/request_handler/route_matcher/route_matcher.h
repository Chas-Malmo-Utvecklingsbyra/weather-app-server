#ifndef ROUTE_MATCHER_H
#define ROUTE_MATCHER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Checks if both path and HTTP method match.
 *
 * @param request_path The full path from the HTTP request
 * @param request_method The HTTP method from the request (GET, POST, etc.)
 * @param route_pattern The route pattern to match against
 * @param route_method The HTTP method required by the route
 * @return true if both path and method match, false otherwise
 */
bool route_matcher_matches(const char *request_path, const char *request_method, const char *route_pattern, const char *route_method);

/**
 * @brief Extracts the path component without query parameters.
 *
 * @param full_path The full path including potential query parameters
 * @param buffer Buffer to store the extracted path (must be pre-allocated)
 * @param buffer_size Size of the buffer
 * @return int 0 on success, -1 on error
 * @note Not in use yet
 */
int route_matcher_extract_path(const char *full_path, char *buffer, size_t buffer_size);

#endif /* ROUTE_MATCHER_H */