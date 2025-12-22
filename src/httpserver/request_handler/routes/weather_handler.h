#ifndef WEATHER_HANDLER_H
#define WEATHER_HANDLER_H

#include "../request_handler.h"
#include "../query_parameters/query_parameters.h"

/**
 * @brief Handles the /v1/weather endpoint request.
 *
 * Validates that latitude and longitude parameters are present,
 * fetches weather data, and populates the result structure.
 *
 * @param params Pointer to parsed query parameters
 * @param result Pointer to the result structure to populate
 * @param context Optional context pointer for user data
 * @return int HTTP status code (200, 400, 500, etc.)
 */
int weather_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *request_handler_response, void *context);

#endif /* WEATHER_HANDLER_H */