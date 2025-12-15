#ifndef CITY_HANDLER_H
#define CITY_HANDLER_H

#include "../request_handler.h"
#include "../query_parameters/query_parameters.h"

#define CITY_HANDLER_LOCATIONIQ_DEFAULT_LIMIT 5
#define CITY_HANDLER_LOCATIONIQ_MIN_LIMIT 1
#define CITY_HANDLER_LOCATIONIQ_MAX_LIMIT 10

/**
 * @brief Handles the /v1/city endpoint request.
 *
 * Validates that city parameter is present,
 * fetches city data, and populates the result structure.
 *
 * @param params Pointer to parsed query parameters
 * @param http_response Pointer to the result structure to populate
 * @return int HTTP status code (200, 400, 500, etc.)
 */
int city_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *http_response);

#endif /* CITY_HANDLER_H */