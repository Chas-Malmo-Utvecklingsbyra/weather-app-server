#include "weather_handler.h"
#include <stdio.h>
#include "core/config/config.h"
#include "core/weather/weather.h"

int weather_handler_handle(QueryParams_t *params, Http_Response_t *http_response)
{
    Config_t *cfg = config_get_instance(NULL);

    if (cfg == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error\"}");
        return http_response->code;
    }
    
    if (params == NULL)
    {
            set_request_error(http_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, "{\"error\":\"Internal server error\"}");
        
        return http_response->code;
    }

    // Validate required parameters
    const char *latitude = query_params_get(params, "latitude");
    const char *longitude = query_params_get(params, "longitude");

    if (latitude == NULL || longitude == NULL)
    {
        set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON,
                          "{\"error\":\"Missing required parameters: latitude and longitude\"}");
        return http_response->code;
    }

    // Fetch weather data
    Weather_Response weather_response = weather_get_data(latitude, longitude);

    if (weather_response.error == true)
    {
        if (cfg->config_debug)
            printf("Error: Failed to get weather data\n");

        set_request_error(http_response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON,
                          "{\"error\":\"Failed to fetch weather data\"}");
        return http_response->code;
    }

    /* OK */
    http_response->response_data = weather_convert_response_to_json(&weather_response);
    http_response->code = HTTP_STATUS_CODE_OK;
    http_response->content_type = HTTP_CONTENT_TYPE_JSON;

    return http_response->code;
}