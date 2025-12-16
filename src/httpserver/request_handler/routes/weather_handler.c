#include "weather_handler.h"
#include "core/weather/weather.h"

int weather_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *request_handler_response)
{    
    if (params == NULL) /* Should not happen */
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    /* get query parameters */
    const char *latitude = query_parameter_get(params, "latitude");
    const char *longitude = query_parameter_get(params, "longitude");
    
    if (latitude == NULL || longitude == NULL)
    {
        query_parameter_dispose(params);
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }

    /* try to get weather data */
    Weather_Response weather_response = weather_get_data(latitude, longitude);
    if (weather_response.error == true)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }
    /* convert to json */
    char *weather_response_json = weather_convert_response_to_json(&weather_response);
    if (weather_response_json == NULL)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
        return request_handler_response->status_code;
    }
    
    /* OK */
    request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, weather_response_json);
    free(weather_response_json);
    
    return request_handler_response->status_code;
}