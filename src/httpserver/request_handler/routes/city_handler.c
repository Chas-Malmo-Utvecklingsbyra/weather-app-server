#include "city_handler.h"
#include "core/locationiq/locationiq.h"
#include "core/json/json_locationiq.h"

int city_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *request_handler_response)
{
    if (params == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    const char *city = query_parameter_get(params, "city");
    const int limit = query_parameter_get(params, "limit") ? atoi(query_parameter_get(params, "limit")) : CITY_HANDLER_LOCATIONIQ_DEFAULT_LIMIT;
    
    if (city == NULL || limit < CITY_HANDLER_LOCATIONIQ_MIN_LIMIT || limit > CITY_HANDLER_LOCATIONIQ_MAX_LIMIT)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    /* Make request to get city information */
    char *locationiq_api_response = locationiq_api_call(city, limit);
    if (locationiq_api_response == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    char *parsed_location_response = locationiq_json_parse(locationiq_api_response, limit);
    if (parsed_location_response == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, parsed_location_response);
    free(locationiq_api_response);
    free(parsed_location_response);

    return request_handler_response->status_code;
}