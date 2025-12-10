#ifndef HTTPSERVER_QUERY_PARAMS_H
#define HTTPSERVER_QUERY_PARAMS_H

#include <string.h>

#define QUERY_PARAMETER_MAX_LENGTH 256

/**
 * @brief Structure to hold query parameters parsed from a URL.
 * @param keys Array of parameter key strings
 * @param values Array of parameter value strings
 * @param count Number of parameters currently stored
 * @param capacity Maximum number of parameters that can be stored
 */
typedef struct QueryParams_t
{
    char **keys;
    char **values;
    size_t count;
    size_t capacity;
} QueryParams_t;

/**
 * @brief Creates a QueryParams_t structure with a specified capacity.
 * @param params Pointer to the QueryParams_t structure to initialize.
 * @param capacity The maximum number of parameters that can be stored.
 * @return int 0 on success, negative value on error.
 */
int query_params_create(QueryParams_t *params, size_t capacity);

/**
 * @brief Parses query parameters from a URL path into a QueryParams_t structure.
 * @param params Pointer to the QueryParams_t structure to populate.
 * @param path The URL path containing query parameters.
 * @return int 0 on success, negative value on error.
 */
int query_params_parse(QueryParams_t *params, const char *path);

/**
 * @brief Retrieves the value of a query parameter by key.
 * @param params Pointer to the QueryParams_t structure.
 * @param key The key of the query parameter to retrieve.
 * @return const char* The value of the query parameter, or NULL if not found.
 */
const char *query_params_get(QueryParams_t *params, const char *key);

/**
 * @brief Frees the memory allocated for a QueryParams_t structure.
 * @param params Pointer to the QueryParams_t structure to free.
 */
void query_params_dispose(QueryParams_t *params);

#endif // HTTPSERVER_QUERY_PARAMS_H