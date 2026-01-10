#include "query_parameters.h"
#include <stdlib.h>
#include <string.h>

int query_parameter_create(QueryParameters_t *param, size_t capacity)
{
    if (param == NULL || capacity == 0)
    {
        return -1; /* Invalid arguments */
    }

    param->keys = (char **)malloc(sizeof(char *) * capacity);
    param->values = (char **)malloc(sizeof(char *) * capacity);
    if (param->keys == NULL || param->values == NULL)
    {
        free(param->keys);
        free(param->values);
        return -1; /* Memory allocation failure */
    }

    /* Allocate memory for each key and value */
    for (size_t i = 0; i < capacity; i++)
    {
        param->keys[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        param->values[i] = (char *)malloc(sizeof(char) * QUERY_PARAMETER_MAX_LENGTH);
        if (param->keys[i] == NULL || param->values[i] == NULL)
        {
            /* Free previously allocated memory on failure */
            for (size_t j = 0; j <= i; j++)
            {
                free(param->keys[j]);
                free(param->values[j]);
            }
            free(param->keys);
            free(param->values);
            return -1; /* Memory allocation failure */
        }
    }

    param->count = 0;
    param->capacity = capacity;

    return 0; /* Success */
}

int query_parameter_parse(QueryParameters_t *param, const char *path)
{
    if (param == NULL || path == NULL)
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
        if (number_of_params >= param->capacity)
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

            strncpy(param->keys[number_of_params], &path[key_index], key_length);
            param->keys[number_of_params][key_length] = '\0';

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

            strncpy(param->values[number_of_params], &path[value_index], value_length);
            param->values[number_of_params][value_length] = '\0';

            key_index = i + 1;
            number_of_params++;
        }
    }
    param->count = number_of_params;

    return 0;
}

const char *query_parameter_get(QueryParameters_t *param, const char *key)
{
    if (param == NULL || key == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < param->count; i++)
    {
        if (strcmp(param->keys[i], key) == 0)
        {
            return param->values[i];
        }
    }
    return NULL; /* Key not found */
}

void query_parameter_dispose(QueryParameters_t *param)
{
    if (param == NULL || param->capacity == 0)
    {
        return;
    }

    if (param->keys != NULL)
    {
        for (size_t i = 0; i < param->capacity; i++)
        {
            if (param->keys[i] == NULL) continue;
            free(param->keys[i]);
            param->keys[i] = NULL;
        }
        free(param->keys);
        param->keys = NULL;
    }

    if (param->values != NULL)
    {
        for (size_t i = 0; i < param->capacity; i++)
        {
            if (param->values[i] == NULL) continue;
            free(param->values[i]);
            param->values[i] = NULL;
        }
        free(param->values);
        param->values = NULL;
    }
}