#include "frontend_handler.h"
#include "core/json/fileHelper/fileHelper.h"

int frontend_handler_handle(QueryParameters_t *params, Request_Handler_Response_t *request_handler_response)
{   
    (void)params;  /* Unused parameter */
    /* Load frontend HTML file */
    static const char *file_locations[] = {"src/frontend/index.html", "frontend/index.html"};
    static const size_t location_count = sizeof(file_locations) / sizeof(file_locations[0]);
    bool found_frontend = false;

    for (size_t j = 0; j < location_count; j++)
    {
        char *file = file_read_to_string(file_locations[j]);
        if (file != NULL)
        {
            request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, file);
            free(file);
            found_frontend = true;
            break;
        }
    }
    if (!found_frontend)
    {
        request_handler_set_response(request_handler_response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_HTML, NULL);
    }
    return request_handler_response->status_code;
}