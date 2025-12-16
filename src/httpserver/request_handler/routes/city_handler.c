#include "city_handler.h"
#include "core/locationiq/locationiq.h"
#include "core/json/json_locationiq.h"

int city_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *request_handler_response)
{
    if (params == NULL)
    {
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }

    const char *city = query_parameter_get(params, "city");
    const int limit = query_parameter_get(params, "limit") ? atoi(query_parameter_get(params, "limit")) : CITY_HANDLER_LOCATIONIQ_DEFAULT_LIMIT;
    
    if (city == NULL || limit < CITY_HANDLER_LOCATIONIQ_MIN_LIMIT || limit > CITY_HANDLER_LOCATIONIQ_MAX_LIMIT)
    {
        return HTTP_STATUS_CODE_BAD_REQUEST;
    }

    /* Make request to get city information */
    char *locationiq_api_response = locationiq_api_call(city, limit);
    if (locationiq_api_response == NULL)
    {
        return HTTP_STATUS_CODE_BAD_REQUEST;
    }

    char *parsed_location_response = locationiq_json_parse(locationiq_api_response, limit);
    if (parsed_location_response == NULL)
    {
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }

    request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, parsed_location_response);
    free(locationiq_api_response);
    free(parsed_location_response);

    return request_handler_response->status_code;
}