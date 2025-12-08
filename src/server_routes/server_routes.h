#ifndef SERVER_ROUTES_H
#define SERVER_ROUTES_H

#include <string.h>
#include "core/http/parser.h"
#include "core/weather/http.h"

#define QUERY_PARAMETER_MAX_LENGTH 256

typedef enum Server_Route_Result
{
  SERVER_ROUTE_RESULT_UNKNOWN = -2,
  SERVER_ROUTE_RESULT_ERROR = -1,
  SERVER_ROUTE_RESULT_OK = 0,
  SERVER_ROUTE_RESULT_BAD_REQUEST = 400,
  SERVER_ROUTE_RESULT_NOT_FOUND = 404,
  SERVER_ROUTE_RESULT_INTERNAL_SERVER_ERROR = 500

} Server_Route_Result;

int handle_route(Http_Request *request, char **json_response);

int get_query_params(const char *path, const int max_params, char **keys, char **values);

#endif /* SERVER_ROUTES_H */