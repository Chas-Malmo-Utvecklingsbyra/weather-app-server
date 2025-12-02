#ifndef SERVER_ROUTES_H
#define SERVER_ROUTES_H

#include <string.h>
#include "core/http/parser.h"
#include "core/weather/http.h"

typedef struct Route_t Route_t;

typedef enum Route_Handler_Result
{
  Route_Handler_Result_Error = -1,
  Route_Handler_Result_OK = 0,
  Route_Handler_Result_Route_Not_Found = 1,
  Route_Handler_Result_No_Params = 2,
} Route_Handler_Result;

typedef enum Route_Get_Params_Result
{
  Route_Get_Params_Result_Error = -3,
  Route_Get_Params_Result_Malformed_Request = -2,
  Route_Get_Params_Result_No_Params = -1
} Route_Get_Params_Result;

typedef enum HTTP_STATUS_CODE
{
  HTTP_STATUS_CODE_OK = 200,
  HTTP_STATUS_CODE_BAD_REQUEST = 400,
  HTTP_STATUS_CODE_NOT_FOUND = 404,
  HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,
} HTTP_STATUS_CODE;

typedef enum Route_HTTP_Status_Code
{
  HTTP_STATUS_OK = 200,
  HTTP_STATUS_BAD_REQUEST = 400,
  HTTP_STATUS_NOT_FOUND = 404,
  HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
} HTTP_Status_Code;

struct Route_t
{
    const char* route;
    void (*handler)(const char* route, char* response);
};

HTTP_STATUS_CODE handle_route(Http_Request *request, char **json_response);

Route_Get_Params_Result get_query_params(const char *path, const int max_params, char **keys, char **values);

#endif /* SERVER_ROUTES_H */