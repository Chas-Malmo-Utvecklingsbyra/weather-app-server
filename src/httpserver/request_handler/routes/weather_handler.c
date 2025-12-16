#include "weather_handler.h"
#include "core/weather/weather.h"

int weather_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *http_response)
{    
    if (params == NULL)
    {
        request_handler_set_response(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error\"}");
        return http_response->code;
    }

    /* get query parameters */
    const char *latitude = query_parameter_get(params, "latitude");
    const char *longitude = query_parameter_get(params, "longitude");

    if (latitude == NULL || longitude == NULL)
    {
        query_parameter_dispose(params);
        request_handler_set_response(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON,
                          "{\"error\":\"Missing required parameters: latitude and longitude\"}");
        return http_response->code;
    }

    /* try to get weather data */
    Weather_Response weather_response = weather_get_data(latitude, longitude);

    if (weather_response.error == true)
    {
        request_handler_set_response(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Failed to fetch weather data\"}");
        return http_response->code;
    }

    /* OK */
    http_response->response_data = weather_convert_response_to_json(&weather_response);
    http_response->code = HTTP_STATUS_CODE_OK;
    http_response->content_type = HTTP_CONTENT_TYPE_JSON;
    return http_response->code;
}