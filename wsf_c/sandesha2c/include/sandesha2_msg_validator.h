/*
 * Copyright 2004,2005 The Apache Software Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SANDESHA2_MSG_VALIDATOR_H
#define SANDESHA2_MSG_VALIDATOR_H

/**
 * @file sandesha2_msg_validator.h
 * @brief Sandesha Message Validator
 */

#include <axutil_allocator.h>
#include <axutil_env.h>
#include <axutil_error.h>
#include <axutil_string.h>
#include <axutil_utils.h>
#include <sandesha2_msg_ctx.h>
#include <axis2_msg_ctx.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct sandesha2_seq_property_mgr;

/** @defgroup sandesha2_msg_validator
  * @ingroup sandesha2
  * @{
  */

axis2_status_t AXIS2_CALL
sandesha2_msg_validator_validate_msg(
    const axutil_env_t *env,
    sandesha2_msg_ctx_t *rm_msg_ctx,
    struct sandesha2_seq_property_mgr *seq_prop_mgr);


/** @} */
#ifdef __cplusplus
}
#endif
#endif /* SANDESHA2_MSG_VALIDATOR_H */
