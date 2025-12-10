#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string.h>
#include <stdlib.h>
#include "core/string/strdup.h"
#include "core/http/parser.h"
#include "core/http/http.h"
#include "core/tcp/server/tcp_server.h"
#include "query_params/query_params.h"

/**
 * @brief Structure to hold the result of an API call.
 * @param code HTTP status code (200, 400, 404, 500, etc.)
 * @param response_data Response data (JSON string, HTML, etc.) - caller must free
 * @param content_type Content type of the response (JSON, HTML, etc.)
 * @note The response string has to be freed by the caller if not NULL.
 */
typedef struct Http_Response_t
{
  HTTP_Status_Code code;
  char *response_data;
  Http_Content_Type content_type;
} Http_Response_t;
/*
typedef int (*RouteHandler)(QueryParams_t *params, Http_Response_t *response);
*/

/**
 * @brief Sets API result for error responses with JSON content.
 * @param http_response Pointer to the HTTP response structure.
 * @param code HTTP status code.
 * @param content_type Content type of the response.
 * @param message Error message (will be duplicated).
 */
void set_request_error(Http_Response_t *http_response, const HTTP_Status_Code code, const Http_Content_Type content_type, const char *message);

/**
 * @brief Handles routing of HTTP requests to appropriate handlers.
 * @param request Pointer to the HTTP request structure.
 * @param http_response Pointer to the HTTP response structure to populate.
 * @return HTTP status code indicating the result of the routing.
 */
int handle_route(Http_Request *request, Http_Response_t *http_response);

/**
 * @brief Handles a HTTP request
 * @param server Pointer to the server structure.
 * @param client Pointer to the client structure.
 * @param response_string The response to send back.
 * @param type the content type (HTTP_CONTENT_TYPE_HTML or HTTP_CONTENT_TYPE_JSON).
 * @param status_code the HTTP status code to send back.
 * @return void.
 */
void handle_request(TCP_Server *server, TCP_Server_Client *client, char *response_string, Http_Content_Type type, HTTP_Status_Code status_code);

#endif /* REQUEST_HANDLER_H */