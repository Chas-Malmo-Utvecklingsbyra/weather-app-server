#include "query_params.h"
#include <stdlib.h>
#include <string.h>

int query_params_create(QueryParams_t *params, size_t capacity)
{
    if (params == NULL || capacity == 0)
    {
        return -1; /* Invalid arguments */
    }

    params->keys = (char **)malloc(sizeof(char *) * capacity);
    params->values = (char **)malloc(sizeof(char *) * capacity);
    if (params->keys == NULL || params->values == NULL)
    {
        free(params->keys);
        free(params->values);
        return -1; /* Memory allocation failure */
    }

    /* Allocate memory for each key and value */
    for (size_t i = 0; i < capacity; i++)
    {
        params->keys[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        params->values[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        if (params->keys[i] == NULL || params->values[i] == NULL)
        {
            /* Free previously allocated memory on failure */
            for (size_t j = 0; j <= i; j++)
            {
                free(params->keys[j]);
                free(params->values[j]);
            }
            free(params->keys);
            free(params->values);
            return -1; /* Memory allocation failure */
        }
    }

    params->count = 0;
    params->capacity = capacity;

    return 0; /* Success */
}

int query_params_parse(QueryParams_t *params, const char *path)
{
    if (params == NULL || path == NULL)
    {
        return -1; /* Invalid arguments */
    }

    size_t number_of_params = 0;
    size_t key_index = 0;
    size_t value_index = 0;

    /* Find the '?' to locate query string start */
    const char *query_start = strchr(path, '?');
    if (query_start == NULL)
    {
        return -1; /* No query parameters */
    }

    size_t path_length = strlen(path);
    size_t i = query_start - path;
    for (; i < path_length; i++)
    {
        if (number_of_params >= params->capacity)
        {
            break; /* Reached maximum expected parameters, should be redone if optional parameters needed */
        }

        if (path[i] == '?')
        {
            key_index = i + 1;
            if (key_index >= path_length)
            {
                return -1; /* No query parameters */
            }
        }
        else if (path[i] == '=') /* End of parameter key, start of parameter value */
        {
            if (i == key_index)
            {
                return -1; /* Empty key, malformed URL */
            }

            size_t key_length = (i - key_index);
            if (key_length >= QUERY_PARAMETER_MAX_LENGTH)
            {
                return -1; /* Key too long */
            }

            strncpy(params->keys[number_of_params], &path[key_index], key_length);
            params->keys[number_of_params][key_length] = '\0';

            value_index = i + 1;
            if (value_index >= path_length || path[value_index] == '&')
            {
                return -1; /* Key but empty value, malformed URL, or should empty value be allowed? */
            }
        }
        else if ((path[i] == '&' || i == path_length - 1)) /* End of parameter value, start of next key */
        {
            size_t value_length = (path[i] == '&') ? (i - value_index) : (i - value_index + 1);
            if (value_length >= QUERY_PARAMETER_MAX_LENGTH)
            {
                return -1; /* Value too long */
            }

            strncpy(params->values[number_of_params], &path[value_index], value_length);
            params->values[number_of_params][value_length] = '\0';

            key_index = i + 1;
            number_of_params++;
        }
    }
    params->count = number_of_params;

    return 0;
}

const char *query_params_get(QueryParams_t *params, const char *key)
{
    if (params == NULL || key == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < params->count; i++)
    {
        if (strcmp(params->keys[i], key) == 0)
        {
            return params->values[i];
        }
    }
    return NULL; /* Key not found */
}

void query_params_dispose(QueryParams_t *params)
{
    if (params == NULL || params->capacity == 0)
    {
        return;
    }

    if (params->keys != NULL)
    {
        for (size_t i = 0; i < params->capacity; i++)
        {
            free(params->keys[i]);
        }
        free(params->keys);
        params->keys = NULL;
    }

    if (params->values != NULL)
    {
        for (size_t i = 0; i < params->capacity; i++)
        {
            free(params->values[i]);
        }
        free(params->values);
        params->values = NULL;
    }
}