#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string.h>
#include "core/http/parser.h"
#include "core/http/http.h"

#define QUERY_PARAMETER_MAX_LENGTH 256

/**
 * @brief Structure to hold the result of an API call.
 * @param code HTTP status code (200, 400, 404, 500, etc.)
 * @param response Response data (JSON string, HTML, etc.) - caller must free
 * @param content_type Content type of the response (JSON, HTML, etc.)
 * @note The response string has to be freed by the caller if not NULL.
 */
typedef struct Request_Handler_Result_t
{
  int code;
  char *response;
  Http_Content_Type content_type;
} Request_Handler_Result_t;

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

typedef enum Request_Handler_Result_E
{
  ROUTE_HANDLER_RESULT_UNKNOWN = -2,
  ROUTE_HANDLER_RESULT_ERROR = -1,
  ROUTE_HANDLER_RESULT_OK = 200,
  ROUTE_HANDLER_RESULT_BAD_REQUEST = 400,
  ROUTE_HANDLER_RESULT_NOT_FOUND = 404,
  ROUTE_HANDLER_RESULT_INTERNAL_SERVER_ERROR = 500
} Request_Handler_Result_E;

/**
 * @brief Handles routing of HTTP requests to appropriate handlers.
 * @param request Pointer to the HTTP request structure.
 * @param api_result Pointer to the API result structure to populate.
 * @return HTTP status code indicating the result of the routing.
 */
int handle_route(Http_Request *request, Request_Handler_Result_t *api_result);

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
void query_params_free(QueryParams_t *params);

#endif /* REQUEST_HANDLER_H */