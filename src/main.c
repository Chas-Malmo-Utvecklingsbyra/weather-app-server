#include <stdio.h>

#include "core/tcp/server/tcp_server.h"
#include <string.h>
#include <stdbool.h>

void on_received_bytes_from_client(const TCP_Server *server, const TCP_Server_Client *client, const uint8_t *buffer, const uint32_t buffer_size) {
    (void)server; // TODO: SS - Remove these later.
    (void)client; // TODO: SS - Remove these later.

    printf("Received %u bytes from client:\n", buffer_size);

    printf("'");
    uint32_t i = 0;
    for(; i < buffer_size; i++) {
        printf("%c", *(buffer + i));
    }
    printf("'\n");

    // TODO: SS - Try to parse the contents of the buffer as a HTTP-request.

    

    
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
        TCP_Server_Result server_accept_result = tcp_server_accept(&server);
        if (server_accept_result != TCP_Server_Result_OK)
        {
            printf("Failed to accept TCP server. Result: %i.\n", server_accept_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
            return -1;
        }

        TCP_Server_Result server_read_result = tcp_server_read(&server);
        if (server_read_result != TCP_Server_Result_OK)
        {
            printf("Failed to read TCP server. Result: %i.\n", server_read_result); // TODO: SS - tcp_server_get_result_as_string(start_server_result)
            return -1;
        }

        // TODO: SS - Tick the server.

        // NOTE: SS - Sleep? Or get console-input here to be able to stop the server from the program.
    }

    printf("Server stopped.\n");
    tcp_server_dispose(&server);

    return 0;
}