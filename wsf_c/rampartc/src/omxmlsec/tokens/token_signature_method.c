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
 * Creates <ds:SignatureMethod> element
 */
AXIS2_EXTERN axiom_node_t* AXIS2_CALL
oxs_token_build_signature_method_element(
    const axutil_env_t *env,
    axiom_node_t *parent,
    axis2_char_t* algorithm)
{
    axiom_node_t *signature_method_node = NULL;
    axiom_element_t *signature_method_ele = NULL;
    axiom_attribute_t *algo_attr = NULL;
    axiom_namespace_t *ns_obj = NULL;

    ns_obj = axiom_namespace_create(env, OXS_DSIG_NS, OXS_DS);
    signature_method_ele = axiom_element_create(
        env, parent, OXS_NODE_SIGNATURE_METHOD, ns_obj, &signature_method_node);
    if(!signature_method_ele)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,"[rampart]Error creating signature method element.");
        axiom_namespace_free(ns_obj, env);
        return NULL;
    }

    if(!algorithm)
    {
        algorithm = (axis2_char_t*)OXS_HREF_RSA_SHA1;
    }

    algo_attr =  axiom_attribute_create(env, OXS_ATTR_ALGORITHM, algorithm, NULL);
    axiom_element_add_attribute(signature_method_ele, env, algo_attr, signature_method_node);
    return signature_method_node;
}



AXIS2_EXTERN axis2_char_t *AXIS2_CALL
oxs_token_get_signature_method(
    const axutil_env_t *env, 
    axiom_node_t *enc_mtd_node)
{
    axis2_char_t *enc_mtd = NULL;
    axiom_element_t *enc_mtd_ele = NULL;

    if(!enc_mtd_node)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "[rampart]Error retrieving signature method node.");
        return NULL;
    }

    enc_mtd_ele = axiom_node_get_data_element(enc_mtd_node, env);
    if (!enc_mtd_ele)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, 
            "[rampart]Error retrieving signature method element.");
        return NULL;
    }

    enc_mtd = axiom_element_get_attribute_value_by_name(enc_mtd_ele, env, OXS_ATTR_ALGORITHM);
    if((!enc_mtd) ||(0 == axutil_strcmp("", enc_mtd)))
    {
        return NULL;
    }
    return enc_mtd;
}

