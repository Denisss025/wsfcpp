/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <axutil_array_list.h>
#include <axutil_utils.h>
#include <string.h>

struct axutil_array_list
{

    /**The number of elements in this list. */
    int size;

    /**Current capacity of this list. */
    int capacity;

    /** Where the data is stored. */
    void **data;
};

AXIS2_EXTERN struct axutil_array_list *AXIS2_CALL
axutil_array_list_create(
    const axutil_env_t * env,
    int capacity)
{
    axutil_array_list_t *array_list = NULL;

    array_list = AXIS2_MALLOC(env->allocator, sizeof(axutil_array_list_t));
    if(!array_list)
    {
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Out of memory");
        return NULL;
    }

    array_list->size = 0;
    array_list->capacity = 0;
    array_list->data = NULL;

    /* Check capacity, and set the default if error */
    if(capacity <= 0)
        capacity = AXIS2_ARRAY_LIST_DEFAULT_CAPACITY;
    array_list->data = AXIS2_MALLOC(env->allocator, sizeof(void *) * (size_t)capacity);
    if(!array_list->data)
    {
        axutil_array_list_free(array_list, env);
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Out of memory");
        return NULL;
    }
    array_list->capacity = capacity;

    return array_list;
}

AXIS2_EXTERN axis2_status_t AXIS2_CALL
axutil_array_list_ensure_capacity(
    struct axutil_array_list * array_list,
    const axutil_env_t * env,
    int min_capacity)
{
    AXIS2_PARAM_CHECK(env->error, array_list, AXIS2_FAILURE);
    if(min_capacity > array_list->capacity)
    {
        int new_capacity = (array_list->capacity * 2 > min_capacity) ? (array_list->capacity * 2)
            : min_capacity;
        void **data = (void **)AXIS2_MALLOC(env->allocator, sizeof(void *) * (size_t)new_capacity);
        if(!data)
        {
            AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
            AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Out of memory");
            return AXIS2_FAILURE;
        }
        memcpy(data, array_list->data, sizeof(void *) * (size_t)array_list->capacity);

        AXIS2_FREE(env->allocator, array_list->data);

        array_list->data = data;
        array_list->capacity = new_capacity;
    }
    return AXIS2_SUCCESS;
}

AXIS2_EXTERN int AXIS2_CALL
axutil_array_list_size(
    struct axutil_array_list *array_list,
    const axutil_env_t * env)
{
    /* Don't use AXIS2_PARAM_CHECK to verify array_list, as it clobbers 
     env->error->status_code on no error destroying the information
     therein that an error has already occurred. */
    (void)env;
    if(!array_list)
        return 0;
    return array_list->size;
}

AXIS2_EXTERN axis2_bool_t AXIS2_CALL
axutil_array_list_is_empty(
    struct axutil_array_list * array_list,
    const axutil_env_t * env)
{
    AXIS2_PARAM_CHECK(env->error, array_list, AXIS2_FAILURE);
    return array_list->size == 0;
}

AXIS2_EXTERN axis2_bool_t AXIS2_CALL
axutil_array_list_contains(
    struct axutil_array_list * array_list,
    const axutil_env_t * env,
    void *e)
{
    AXIS2_PARAM_CHECK(env->error, array_list, AXIS2_FAILURE);
    return axutil_array_list_index_of(array_list, env, e) != -1;
}

AXIS2_EXTERN int AXIS2_CALL
axutil_array_list_index_of(
    struct axutil_array_list *array_list,
    const axutil_env_t * env,
    void *e)
{
    int i = 0;

    AXIS2_PARAM_CHECK(env->error, array_list, AXIS2_FAILURE);
    for(i = 0; i < array_list->size; i++)
        if(e == array_list->data[i])
            return i;
    return -1;
}

AXIS2_EXTERN void *AXIS2_CALL
axutil_array_list_get(
    struct axutil_array_list *array_list,
    const axutil_env_t * env,
    int idx)
{
    /* Don't use AXIS2_PARAM_CHECK to verify array_list, as it clobbers 
     env->error->status_code on no error destroying the information
     therein that an error has already occurred. */
    if(axutil_array_list_check_bound_exclusive(array_list, env, idx))
        return array_list->data[idx];
    else
        return NULL;
}

AXIS2_EXTERN void *AXIS2_CALL
axutil_array_list_set(
    struct axutil_array_list *array_list,
    const axutil_env_t * env,
    int idx,
    void *e)
{
    void *result = NULL;

    AXIS2_PARAM_CHECK(env->error, array_list, NULL);
    if(axutil_array_list_check_bound_exclusive(array_list, env, idx))
    {
        result = array_list->data[idx];
        array_list->data[idx] = e;
    }
    return result;
}

AXIS2_EXTERN axis2_status_t AXIS2_CALL
axutil_array_list_add(
    struct axutil_array_list * array_list,
    const axutil_env_t * env,
    const void *e)
{
    AXIS2_PARAM_CHECK(env->error, array_list, AXIS2_FAILURE);
    if(array_list->size == array_list->capacity)
        if(axutil_array_list_ensure_capacity(array_list, env, array_list->size + 1)
            != AXIS2_SUCCESS)
            return AXIS2_FAILURE;
    array_list->data[array_list->size++] = (void *)e;
    return AXIS2_SUCCESS;
}

AXIS2_EXTERN axis2_status_t AXIS2_CALL
axutil_array_list_add_at(
    struct axutil_array_list * array_list,
    const axutil_env_t * env,
    const int idx,
    const void *e)
{
    int i = 0;
    AXIS2_PARAM_CHECK(env->error, array_list, AXIS2_FAILURE);

    if(!axutil_array_list_check_bound_inclusive(array_list, env, idx))
        return AXIS2_FAILURE;
    if(array_list->size == array_list->capacity)
    {
        if(axutil_array_list_ensure_capacity(array_list, env, array_list->size + 1)
            != AXIS2_SUCCESS)
            return AXIS2_FAILURE;
    }
    if(idx != array_list->size)
    {
        for(i = array_list->size; i > idx; i--)
            array_list->data[i] = array_list->data[i - 1];
    }
    array_list->data[idx] = (void *)e;
    array_list->size++;
    return AXIS2_SUCCESS;
}

AXIS2_EXTERN void *AXIS2_CALL
axutil_array_list_remove(
    struct axutil_array_list *array_list,
    const axutil_env_t * env,
    int idx)
{
    void *result = NULL;
    int i = 0;
    AXIS2_PARAM_CHECK(env->error, array_list, NULL);

    if(axutil_array_list_check_bound_exclusive(array_list, env, idx))
    {
        result = array_list->data[idx];
        for(i = idx; i < array_list->size - 1; i++)
            array_list->data[i] = array_list->data[i + 1];
        array_list->size--;
    }

    return result;
}

AXIS2_EXTERN axis2_bool_t AXIS2_CALL
axutil_array_list_check_bound_inclusive(
    struct axutil_array_list * array_list,
    const axutil_env_t * env,
    int idx)
{
    /* Don't use AXIS2_PARAM_CHECK to verify array_list, as it clobbers 
     env->error->status_code on no error destroying the information
     therein that an error has already occurred. */
    if(!array_list || idx < 0 || idx > array_list->size)
    {
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_INDEX_OUT_OF_BOUNDS, AXIS2_FAILURE);
        return AXIS2_FALSE;
    }
    return AXIS2_TRUE;
}

AXIS2_EXTERN axis2_bool_t AXIS2_CALL
axutil_array_list_check_bound_exclusive(
    struct axutil_array_list * array_list,
    const axutil_env_t * env,
    int idx)
{
    /* Don't use AXIS2_PARAM_CHECK to verify array_list, as it clobbers 
     env->error->status_code on no error destroying the information
     therein that an error has already occurred. */
    if(!array_list || idx < 0 || idx >= array_list->size)
    {
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_INDEX_OUT_OF_BOUNDS, AXIS2_FAILURE);
        return AXIS2_FALSE;
    }
    return AXIS2_TRUE;
}

AXIS2_EXTERN void AXIS2_CALL
axutil_array_list_free(
    struct axutil_array_list *array_list,
    const axutil_env_t * env)
{
    AXIS2_PARAM_CHECK_VOID(env->error, array_list);

    if(array_list->data)
    {
        AXIS2_FREE(env->allocator, array_list->data);
    }
    AXIS2_FREE(env->allocator, array_list);
    return;
}

AXIS2_EXTERN void AXIS2_CALL
axutil_array_list_free_void_arg(
    void *array_list,
    const axutil_env_t * env)
{
    axutil_array_list_t *array_list_l = NULL;

    AXIS2_PARAM_CHECK_VOID(env->error, array_list);

    array_list_l = (axutil_array_list_t *)array_list;
    axutil_array_list_free(array_list_l, env);
    return;
}

