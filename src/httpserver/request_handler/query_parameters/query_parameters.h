#ifndef QUERY_PARAMETERS_H
#define QUERY_PARAMETERS_H

#include <string.h>

#define QUERY_PARAMETER_MAX_LENGTH 256

/**
 * @brief Structure to hold query parameters parsed from a URL.
 * @param keys Array of parameter key strings
 * @param values Array of parameter value strings
 * @param count Number of parameters currently stored
 * @param capacity Maximum number of parameters that can be stored
 */
typedef struct QueryParameters_t
{
    char **keys;
    char **values;
    size_t count;
    size_t capacity;
} QueryParameters_t;

/**
 * @brief Creates a QueryParameters_t structure with a specified capacity.
 * @param param Pointer to the QueryParameters_t structure to initialize.
 * @param capacity The maximum number of parameters that can be stored.
 * @return int 0 on success, negative value on error.
 */
int query_parameter_create(QueryParameters_t *param, size_t capacity);

/**
 * @brief Parses query parameters from a URL path into a QueryParameters_t structure.
 * @param param Pointer to the QueryParameters_t structure to populate.
 * @param path The URL path containing query parameters.
 * @return int 0 on success, negative value on error.
 */
int query_parameter_parse(QueryParameters_t *param, const char *path);

/**
 * @brief Retrieves the value of a query parameter by key.
 * @param param Pointer to the QueryParameters_t structure.
 * @param key The key of the query parameter to retrieve.
 * @return const char* The value of the query parameter, or NULL if not found.
 */
const char *query_parameter_get(QueryParameters_t *param, const char *key);

/**
 * @brief Frees the memory allocated for a QueryParameters_t structure.
 * @param param Pointer to the QueryParameters_t structure to free.
 */
void query_parameter_dispose(QueryParameters_t *param);

#endif // QUERY_PARAMETERS_H