#include <stdio.h>

#include "core/tcp/server/tcp_server.h"
#include <string.h>
#include <stdbool.h>

#include "core/http/parser.h"
#include "core/weather/weather.h"
#include "core/http/http.h"
#include "core/config/config.h"

void on_received_bytes_from_client(TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size) {
    (void)server; // TODO: SS - Remove these later.
    (void)client; // TODO: SS - Remove these later.

    /* TODO: SS - Try to parse the contents of the request buffer as a HTTP-request. */
    printf("Received %u bytes from client:\n", buffer_size);

    printf("'");
    uint32_t i = 0;
    for(; i < buffer_size; i++) {
        printf("%c", *(buffer + i));
    }
    printf("'\n");
    char *response_string = NULL;
    
    Http_Request* httpblob =  Http_Parser_Parse((const char*)buffer);
    if(httpblob == NULL){
        response_string = "<h1>Invalid HTTP Request</h1>";
        uint8_t outgoing_buffer[1024];
        uint32_t outgoing_size = 0;
        http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);

        TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);

        if(send_result != TCP_Server_Result_OK)
        {
            printf("Error on client send\n");

        }
        else{
            printf("Send buffer content[0]: %u\n", client->outgoing_buffer[0]);
        }
        return;
    }
    printf("METHOD: [%s]\n", httpblob->start_line.method);
    printf("PATH: [%s]\n", httpblob->start_line.path);

    if(strcmp(httpblob->start_line.path, "/") == 0){
        response_string = "<h1>Hello there</h1>";
        uint8_t outgoing_buffer[1024];
        uint32_t outgoing_size = 0;
        http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);

        TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);
        
        if(send_result != TCP_Server_Result_OK)
        {
            printf("Error on client send\n");

        }
        else{
            printf("Send buffer content[0]: %u\n", client->outgoing_buffer[0]);
        }
    }else if(strcmp(httpblob->start_line.path, "/weather") == 0){
        
        Weather_Response weather_data = weather_get_data("https://api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&current_weather=true&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,rain,showers,snowfall,weather_code,cloud_cover,wind_speed_10m,wind_direction_10m,wind_gusts_10m,surface_pressure,pressure_msl&wind_speed_unit=ms");
        response_string = weather_data.data;
        uint8_t outgoing_buffer[1024];
        uint32_t outgoing_size = 0;
        http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);

        TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);
        
        if(send_result != TCP_Server_Result_OK)
        {
            printf("Error on client send\n");

        }
        else{
            printf("Send buffer content[0]: %u\n", client->outgoing_buffer[0]);
        }
        weather_dispose(&weather_data);
    }
    else if (strcmp(httpblob->start_line.path, "/echo") == 0)
    {
        if (httpblob->data == NULL)
        {
            response_string = "<h1>No data</h1>";
        }
        else
        {
            response_string = httpblob->data;
        }
        uint8_t outgoing_buffer[1024];
        uint32_t outgoing_size = 0;
        http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);

        TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);

        if(send_result != TCP_Server_Result_OK)
        {
            printf("Error on client send\n");

        }
        else{
            printf("Send buffer content[0]: %u\n", client->outgoing_buffer[0]);
        }
    }
    
    
    /* 
    uint8_t outgoing_buffer[1024];
    uint32_t outgoing_size = 0;
    http_create_response(outgoing_buffer, sizeof(outgoing_buffer), response_string, strlen(response_string), &outgoing_size);

    TCP_Server_Result send_result = tcp_server_send_to_client(server, client, outgoing_buffer, outgoing_size);
 */
    /* if(send_result != TCP_Server_Result_OK)
    {
        printf("Error on client send\n");

    }
    else{
        printf("Send buffer content[0]: %u\n", client->outgoing_buffer[0]);
    } */



    Http_Parser_Cleanup(httpblob);
    // Here's an example for how we could send a response to the client.
    // char response_buf[4096];
    // memset(&response_buf[0], 0, sizeof(response_buf));
    // http_create_response(&response_buf[0], sizeof(response_buf), // TEMP: SS - Send JSON instead.
    //     "<html>"
    //         "<head>"
    //         "</head>"
    //         "<body>"
    //             "<p>Welcome to our site!</p>"
    //         "</body>"
    //     "</html>"
    // );
    
    // // TODO: SS - Add support for sending a string-buffer to the 'client'.
    // TCP_Server_Result send_result = tcp_server_send(client, &response_buf[0], sizeof(response_buf)); 
    // if(send_result != TCP_Server_Result_OK) {
    //     printf("Error! Failed to send bytes to client.\n");
    // }
}

int main(int argc, char** argv) {
    printf("Hello, world! I am the Server.\n");
    
    Config_t* cfg = config_get_instance("settings.json");

    if (cfg == NULL)
    {
        printf("Failed to load config. Error code: %d\n", config_instance_get_last_error());
        return -1;
    }
    if(cfg->config_debug) 
    {
        printf("Debug mode is enabled.\n");
        printf("config: \nserver_host: %s\nserver_port: %d\ndebug: %d\nmax_connections: %zu\n",
        cfg->config_server_host ? cfg->config_server_host : "NULL",
        cfg->config_server_port,
        cfg->config_debug,
        cfg->config_max_connections);
    }

    printf("%i\n", argc);
    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);
    int arg_port =  atoi(argv[1]);
    TCP_Server server;
    memset(&server, 0, sizeof(TCP_Server));

    TCP_Server_Result server_init_result = tcp_server_init(
        &server,
        arg_port,
        &on_received_bytes_from_client
    );
    if(server_init_result != TCP_Server_Result_OK) 
    {
        printf("Failed to initialize TCP server. Result: %i.\n", server_init_result); // TODO: SS - tcp_server_get_result_as_string(server_init_result)
        return -1;
    }

    TCP_Server_Result start_server_result = tcp_server_start(&server);
    if(start_server_result != TCP_Server_Result_OK) 
    {
        printf("Failed to start TCP server. Result: %i.\n", start_server_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
        return -1;
    }

    printf("Server running.\n");

    

    while(true) { // TEMP: SS - tcp_server_is_running(server)?

        TCP_Server_Result work_result = tcp_server_work(&server);

        switch(work_result){
            case TCP_Server_Result_OK:
            {
                break;
            }
            default: 
            {
                printf("Something went wrong\n");
                break;
            }


        }

        // TODO: SS - Tick the server.

        // NOTE: SS - Sleep? Or get console-input here to be able to stop the server from the program.
    }
    
    printf("Server stopped.\n");
    
    /* Dispose config on exit */
    config_instance_dispose();
    tcp_server_dispose(&server);

    return 0;
}