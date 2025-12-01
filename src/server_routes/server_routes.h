#ifndef SERVER_ROUTES_H
#define SERVER_ROUTES_H

#include <string.h>
#include "core/http/parser.h"

typedef struct Route_t Route_t;

typedef enum Route_Handler_Result
{
  Route_Handler_Result_Error = -1,
  Route_Handler_Result_OK = 0
} Route_Handler_Result;

struct Route_t
{
    const char* route;
    void (*handler)(const char* route, char* response);
};

Route_Handler_Result handle_route(Http_Request *request, char **json_response);

int get_query_params(const char *path, const int args_count, char *key[], char *value[]);

#endif /* SERVER_ROUTES_H */

    /*

      "allowed_routes" : [
        {
          "route" : "/weather",
          "method" : "POST"
        },
        {
          "route" : "/weather",
          "method" : "GET"
        }
      ]

    */