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
#include <stdlib.h>
#include <string.h>
#include <guththila_reader.h>

GUTHTHILA_EXPORT guththila_reader_t * GUTHTHILA_CALL
guththila_reader_create_for_file(
    guththila_char_t *file_name,
    const axutil_env_t * env)
{
    guththila_reader_t * reader;
    FILE * f = NULL;

    if(!file_name)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "[guththila] invalid file name");
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_COULD_NOT_OPEN_FILE, AXIS2_FAILURE);
        return NULL;
    }

    reader = (guththila_reader_t *)AXIS2_MALLOC(env->allocator, sizeof(guththila_reader_t));
    if(!reader)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "[guththila]insufficient memory to create guththila parser");
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        return NULL;
    }

    f = fopen(file_name, "r");
    if(!f)
    {
        AXIS2_FREE(env->allocator, reader);
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_COULD_NOT_OPEN_FILE, AXIS2_FAILURE);
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "[guththila] unable to open file %s", file_name);
        return NULL;
    }

    reader->fp = f;
    reader->type = GUTHTHILA_FILE_READER;
    reader->buff = NULL;
    reader->buff_size = 0;
    reader->input_read_callback = NULL;
    reader->context = NULL;
    return reader;
}

GUTHTHILA_EXPORT guththila_reader_t * GUTHTHILA_CALL
guththila_reader_create_for_memory(
    void *buffer,
    int size,
    const axutil_env_t * env)
{
    guththila_reader_t * reader;
    reader = (guththila_reader_t *)AXIS2_MALLOC(env->allocator, sizeof(guththila_reader_t));
    if(!reader)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "[guththila]insufficient memory to create guththila parser");
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        return NULL;
    }

    reader->type = GUTHTHILA_MEMORY_READER;
    reader->buff = buffer;
    reader->buff_size = size;
    reader->fp = NULL;
    reader->input_read_callback = NULL;
    reader->context = NULL;
    return reader;
}

GUTHTHILA_EXPORT guththila_reader_t * GUTHTHILA_CALL
guththila_reader_create_for_io(
    GUTHTHILA_READ_INPUT_CALLBACK input_read_callback,
    void *ctx,
    const axutil_env_t * env)
{
    guththila_reader_t * reader;
    reader = (guththila_reader_t *)AXIS2_MALLOC(env->allocator, sizeof(guththila_reader_t));
    if(!reader)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "[guththila]insufficient memory to create guththila parser");
        AXIS2_ERROR_SET(env->error, AXIS2_ERROR_NO_MEMORY, AXIS2_FAILURE);
        return NULL;
    }

    reader->input_read_callback = input_read_callback;
    reader->context = ctx;
    reader->type = GUTHTHILA_IO_READER;
    reader->buff = NULL;
    reader->buff_size = 0;
    reader->fp = NULL;
    return reader;
}

GUTHTHILA_EXPORT void GUTHTHILA_CALL
guththila_reader_free(
    guththila_reader_t * r,
    const axutil_env_t * env)
{
    if(r->type == GUTHTHILA_FILE_READER && r->fp)
    {
        fclose(r->fp);
    }
    if(r->type == GUTHTHILA_IO_READER && r->context)
    {
        AXIS2_FREE(env->allocator, r->context);
    }
    AXIS2_FREE(env->allocator, r);

}

GUTHTHILA_EXPORT int GUTHTHILA_CALL
guththila_reader_read(
    guththila_reader_t * r,
    guththila_char_t * buffer,
    int offset,
    int length,
    const axutil_env_t * env)
{
    int rt = r->type;
    (void)env;
    switch(rt)
    {
        case GUTHTHILA_FILE_READER:
            return (int)fread(buffer + offset, 1, (size_t)length, r->fp);
        case GUTHTHILA_IO_READER:
            return r->input_read_callback((buffer + offset), length, r->context);
        default:
            return 0;
    }
}

