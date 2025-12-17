#ifndef FRONTEND_HANDLER_H
#define FRONTEND_HANDLER_H

#include "../request_handler.h"
#include "../query_parameters/query_parameters.h"

/**
 * @brief Handles the / endpoint request.
 *
 * Loads the frontend HTML file and populates the result structure.
 *
 * @param response Pointer to the result structure to populate
 * @return int HTTP status code (200, 500, etc.)
 */
int frontend_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *request_handler_response);

#endif /* FRONTEND_HANDLER_H */