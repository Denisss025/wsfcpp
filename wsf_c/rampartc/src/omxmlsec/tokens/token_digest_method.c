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

#include <oxs_tokens.h>

/**
 * Creates <ds:DigestMethod> element
 */
AXIS2_EXTERN axiom_node_t* AXIS2_CALL
oxs_token_build_digest_method_element(
    const axutil_env_t *env,
    axiom_node_t *parent,
    axis2_char_t* algorithm)
{
    axiom_node_t *digest_method_node = NULL;
    axiom_element_t *digest_method_ele = NULL;
    axiom_attribute_t *algo_attr = NULL;
    axiom_namespace_t *ns_obj = NULL;

    ns_obj = axiom_namespace_create(env, OXS_DSIG_NS, OXS_DS);
    digest_method_ele = axiom_element_create(
        env, parent, OXS_NODE_DIGEST_METHOD, ns_obj, &digest_method_node);

    if (!digest_method_ele)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "[rampart]Error creating digest method element.");
        axiom_namespace_free(ns_obj, env);
        return NULL;
    }

    /* If digest algorithm is NULL then use the default */
    if(!algorithm)
    {
        algorithm = (axis2_char_t*)OXS_HREF_SHA1;
    }

    algo_attr =  axiom_attribute_create(env, OXS_ATTR_ALGORITHM, algorithm, NULL);
    axiom_element_add_attribute(digest_method_ele, env, algo_attr, digest_method_node);
    return digest_method_node;
}



AXIS2_EXTERN axis2_char_t *AXIS2_CALL
oxs_token_get_digest_method(
    const axutil_env_t *env, 
    axiom_node_t *digest_mtd_node)
{
    axis2_char_t *digest_mtd = NULL;
    axiom_element_t *digest_mtd_ele = NULL;

    if(!digest_mtd_node)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "[rampart]Error retrieving digest method node.");
        return NULL;
    }

    digest_mtd_ele = axiom_node_get_data_element(digest_mtd_node, env);
    if(!digest_mtd_ele)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "[rampart]Error retrieving digest method element.");
        return NULL;
    }

    digest_mtd = axiom_element_get_attribute_value_by_name(digest_mtd_ele, env, OXS_ATTR_ALGORITHM);
    if((!digest_mtd) ||(!axutil_strcmp("", digest_mtd)))
    {
        return NULL;
    }

    return digest_mtd;
}

