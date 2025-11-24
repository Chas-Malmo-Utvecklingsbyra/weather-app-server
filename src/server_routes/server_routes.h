#ifndef SERVER_ROUTES_H
#define SERVER_ROUTES_H

#include <string.h>
#include "core/http/parser.h"

typedef struct Route_t Route_t;

typedef enum HTTP_Method_Enum
{
    METHOD_GET,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_UNKNOWN
} HTTP_Method_Enum;

struct Route_t
{
    const char* route;
    HTTP_Method_Enum method; // Get
    void (*handler)(const char* route, char* response);
};

int handle_route(Http_Request *request, char *json_response);


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