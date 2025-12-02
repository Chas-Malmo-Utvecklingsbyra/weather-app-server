#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "core/http/server/http_server.h"
#include "core/utils/file.h"

#include "core/weather/weather.h" // NOTE: SS -Should not be in Core.

bool route_root(HTTP_Method method, const HTTP_Query_Map *query_map, char *out_response, uint32_t response_max_buffer_size) {
    (void)method; // NOTE: SS - We can check that 'method' is GET.
    (void)query_map;

    uint32_t data_length = 0; // NOTE: SS - Not used at the moment.
    char *data = file_open_read_only("index.html", &data_length);
    if(data != NULL) {
        snprintf(&out_response[0], response_max_buffer_size, "%s", data); // NOTE: SS - This will most likely not work for binary.
    }
    else {
        snprintf(&out_response[0], response_max_buffer_size, "<h1></h1>");
    }

    return true;
}

bool route_weather(HTTP_Method method, const HTTP_Query_Map *query_map, char *out_response, uint32_t response_max_buffer_size) {
    (void)method; // NOTE: SS - We can check that 'method' is GET.

    double latitude = 0; {
        const char *str = NULL;
        if(http_query_map_get(query_map, "lat", &str)) {
            char *v = NULL;
            latitude = strtod(str, &v);
        }
    }
    
    double longitude = 0; {
        const char *str = NULL;
        if(http_query_map_get(query_map, "lon", &str)) {
            char *v = NULL;
            longitude = strtod(str, &v);
        }
    }
    
    if(longitude <= LONGITUDE_MIN) {
        longitude = LONGITUDE_MIN + 1;
    }
    else if (longitude >= LONGITUDE_MAX) {
        longitude = LONGITUDE_MAX - 1;
    }

    if(latitude <= LATITUDE_MIN) {
        latitude = LATITUDE_MIN;
    }
    else if (latitude >= LATITUDE_MAX) {
        latitude = LATITUDE_MAX;
    }
    
    printf("Longitude: %f, latitude: %f.\n", longitude, latitude);

    Weather_Response weather_response;
    memset(&weather_response, 0, sizeof(Weather_Response));
    if(weather_get_data_double(longitude, latitude, &weather_response)) {
        snprintf(
            &out_response[0],
            response_max_buffer_size,
            "<html>"
                "<head>"
                    "<title>Weather</title>"
                "</head>"
                "<body>"
                    "<h2>Latitude: %.2f, Longitude: %.2f</h2>"
                    "<h3>Temperature (%c): %.2f (feels like %.2f)</h3>"
                    "<a href=\"https://open-meteo.com/\">Weather data by Open-Meteo.com</a>"
                "</body>"
            "</html>"
            ,
            latitude,
            longitude,
            weather_response.unit,
            weather_response.temperature,
            weather_response.feels_like
        );
    }
    else {
        snprintf(
            &out_response[0],
            response_max_buffer_size,
            "<html>"
                "<head>"
                    "<title>Weather</title>"
                "</head>"
                "<body>"
                    "<h2>Failed to get a weather-report.</h2>"
                "</body>"
            "</html>"
        );
    }
    
    return true;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Hello, world! I am the Server.\n");

    HTTP_Server http_server;
    HTTP_Server_Result init_result = http_server_init(
        &http_server,
        8080 // TODO: SS - Get from argc.
    );
    if(init_result != HTTP_Server_Result_OK) {
        printf("Failed to start HTTP-server. Result: %i.\n", init_result);
        return -1;
    } 

    http_server_add_route(&http_server, "/", route_root);
    http_server_add_route(&http_server, "/weather", route_weather);

    printf("Server running.\n");

    while(true) {
        HTTP_Server_Result work_result = http_server_work(&http_server);
        if(work_result != HTTP_Server_Result_OK) {
            printf("Failed to work the HTTP_Server. Got work-result %i.\n", work_result);
            break;
        }
        
        // NOTE: SS - Sleep?
    }
    
    printf("Server stopped.\n");
    
    assert(http_server_dispose(&http_server) == HTTP_Server_Result_OK);

    return 0;
}