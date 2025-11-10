#include <stdio.h>

#include "core/tcp/server/tcp_server.h"
#include <string.h>
#include <stdbool.h>

#include "core/http/http.h"

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
    char *response_string = "<h1>Hello world!</h1>";

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

int main() {
    printf("Hello, world! I am the Server.\n");

    TCP_Server server;
    memset(&server, 0, sizeof(TCP_Server));

    TCP_Server_Result server_init_result = tcp_server_init(
        &server,
        8080,
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
    tcp_server_dispose(&server);

    return 0;
}