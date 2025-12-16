#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string.h>
#include <stdlib.h>
#include "query_parameters/query_parameters.h"
#include "core/string/strdup.h"
#include "core/http/parser.h"
#include "core/http/http.h"
#include "core/tcp/server/tcp_server.h"
#include "route_registry/route_registry.h"

/**
 * @brief Initialize the request handler and register all routes.
 * Must be called once at application startup.
 * @return 0 on success, -1 on error
 * @note The route registry is created inside this function.
 */
int request_handler_register_routes(RouteRegistry *registry, int capacity);

/**
 * @brief Sets API result for responses with JSON content.
 * @param request_handler_response Pointer to the HTTP response structure.
 * @param status_code HTTP status status_code.
 * @param content_type Content type of the response.
 * @param response_data Response data (will be duplicated).
 * @return void
 * @note response_data should be NULL for error responses.
 * 
 */
void request_handler_set_response(Request_Handler_Response_t *request_handler_response, const HTTP_Status_Code status_code, const Http_Content_Type content_type, const char *response_data);

/**
 * @brief Handles a HTTP request and populates the response structure.
 * @param request Pointer to the HTTP request structure.
 * @param request_handler_response Pointer to the HTTP response structure to populate.
 * @return HTTP status status_code of the response.
 */
int request_handler_handle_request(RouteRegistry *registry,Http_Request * request, Request_Handler_Response_t *request_handler_response);

/**
 * @brief Handles a HTTP request
 * @param server Pointer to the server structure.
 * @param client Pointer to the client structure.
 * @param response_string The response to send back.
 * @param type the content type (HTTP_CONTENT_TYPE_HTML or HTTP_CONTENT_TYPE_JSON).
 * @param status_code the HTTP status status_code to send back.
 * @return void.
 */
void send_response_to_client(TCP_Server *server, TCP_Server_Client *client, char *response_string, Http_Content_Type type, HTTP_Status_Code status_code);

/**
 * @brief Frees the memory allocated for a Request_Handler_Response_t structure.
 * @param request_handler_response Pointer to the Request_Handler_Response_t structure to free.
 */
void dispose_request_handler_response(Request_Handler_Response_t *request_handler_response);

#endif /* REQUEST_HANDLER_H */