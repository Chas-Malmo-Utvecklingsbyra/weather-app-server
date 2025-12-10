#include "city_handler.h"
#include "core/locationiq/locationiq.h"
#include "core/json/json_locationiq.h"

int city_handler_handle(QueryParams_t *params, Http_Response_t *http_response)
{
    if (params == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error\"}");
        return http_response->code;
    }

    const char *city = query_params_get(params, "city");
    if (city == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Missing required parameter: city\"}");
        return http_response->code;
    }
    
    const int limit = query_params_get(params, "limit") ? atoi(query_params_get(params, "limit")) : CITY_HANDLER_LOCATIONIQ_DEFAULT_LIMIT;
    if (limit < CITY_HANDLER_LOCATIONIQ_MIN_LIMIT || limit > CITY_HANDLER_LOCATIONIQ_MAX_LIMIT)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Invalid count parameter, must be between 1 and 10\"}");
        return http_response->code;
    }

    /* Make request to get city information */
    char *locationiq_api_response = locationiq_api_call(city, limit);
    if (locationiq_api_response == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Failed to fetch city information\"}");
        return http_response->code;
    }

    char *parsed_location_response = locationiq_json_parse(locationiq_api_response, limit);
    free(locationiq_api_response);
    
    if (parsed_location_response == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Failed to parse city information\"}");
        return http_response->code;
    }
    
    http_response->response_data = strdup(parsed_location_response);
    http_response->code = HTTP_STATUS_CODE_OK;
    http_response->content_type = HTTP_CONTENT_TYPE_JSON;
    free(parsed_location_response);

    return http_response->code;
}