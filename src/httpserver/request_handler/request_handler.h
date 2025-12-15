#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string.h>
#include <stdlib.h>
#include "query_parameters/query_parameters.h"
#include "core/string/strdup.h"
#include "core/http/parser.h"
#include "core/http/http.h"
#include "core/tcp/server/tcp_server.h"

/**
 * @brief Structure to hold the result of an API call.
 * @param code HTTP status code (200, 400, 404, 500, etc.)
 * @param response_data Response data (JSON string, HTML, etc.) - caller must free
 * @param content_type Content type of the response (JSON, HTML, etc.)
 * @note The response string has to be freed by the caller if not NULL.
 * @note Maybe change name to request_handler_response_t
 */
typedef struct Request_Handler_Response_t
{
  HTTP_Status_Code code;
  char *response_data;
  Http_Content_Type content_type;
} Request_Handler_Response_t;

/**
  * @brief Function pointer type for route handlers
  * @param params Query parameters extracted from the request
  * @param response HTTP response structure to populate
  */

typedef int (*RouteHandler)(QueryParameters_t *params, Request_Handler_Response_t *response);

/**
 * @brief Initialize the request handler and register all routes.
 * Must be called once at application startup.
 * @return 0 on success, -1 on error
 */
int request_handler_init(int capacity);

/**
 * @brief Sets API result for error responses with JSON content.
 * @param http_response Pointer to the HTTP response structure.
 * @param code HTTP status code.
 * @param content_type Content type of the response.
 * @param message Error message (will be duplicated).
 */
void request_handler_set_response(Request_Handler_Response_t *http_response, const HTTP_Status_Code code, const Http_Content_Type content_type, const char *message);

/**
 * @brief Handles routing of HTTP requests to appropriate handlers.
 * @param request Pointer to the HTTP request structure.
 * @param http_response Pointer to the HTTP response structure to populate.
 * @return HTTP status code indicating the result of the routing.
 */
int request_handler_handle_route(Http_Request *request, Request_Handler_Response_t *http_response);

/**
 * @brief Handles a HTTP request
 * @param server Pointer to the server structure.
 * @param client Pointer to the client structure.
 * @param response_string The response to send back.
 * @param type the content type (HTTP_CONTENT_TYPE_HTML or HTTP_CONTENT_TYPE_JSON).
 * @param status_code the HTTP status code to send back.
 * @return void.
 */
void send_response_to_client(TCP_Server* server, TCP_Server_Client* client, char* response_string, Http_Content_Type type, HTTP_Status_Code status_code);

/**
 * @brief Creates a QueryParams_t structure with a specified capacity.
 * @param params Pointer to the QueryParams_t structure to initialize.
 * @param capacity The maximum number of parameters that can be stored.
 * @return int 0 on success, negative value on error.
 */
int query_params_create(QueryParams_t *params, size_t capacity);

/**
 * @brief Dispose of the request handler resources.
 * Must be called once at application shutdown.
 */
void request_handler_dispose(void);

void dispose_request_handler_response(Request_Handler_Result_t* api_result);

#endif /* REQUEST_HANDLER_H */