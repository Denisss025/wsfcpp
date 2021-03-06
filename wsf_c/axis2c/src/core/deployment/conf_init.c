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

#include <axis2_conf_init.h>
#include <axis2_transport_sender.h>
#include <axis2_transport_receiver.h>
#include <axis2_const.h>
#include <axutil_error.h>
#include <axutil_allocator.h>
#include <axutil_class_loader.h>
#include <axis2_dep_engine.h>
#include <axis2_module.h>

#define DEFAULT_REPO_PATH "."

axis2_status_t AXIS2_CALL
axis2_init_modules(
    const axutil_env_t * env,
    axis2_conf_ctx_t * conf_ctx);

static axis2_status_t AXIS2_CALL
axis2_load_services(
    const axutil_env_t * env,
    axis2_conf_ctx_t * conf_ctx);

axis2_status_t AXIS2_CALL
axis2_init_transports(
    const axutil_env_t * env,
    axis2_conf_ctx_t * conf_ctx);

static axis2_conf_ctx_t *AXIS2_CALL
axis2_build_conf_ctx_with_dep_engine(
    const axutil_env_t * env,
    axis2_dep_engine_t *dep_engine,
    axis2_char_t *is_server_side)
{
    axis2_conf_ctx_t *conf_ctx = NULL;
    axis2_conf_t *conf = NULL;
    axutil_property_t *property = NULL;
    axis2_ctx_t *conf_ctx_base = NULL;

    conf = axis2_dep_engine_load(dep_engine, env);
    if(!conf)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Loading deployment engine failed");
        axis2_dep_engine_free(dep_engine, env);
        return NULL;
    }
    axis2_conf_set_dep_engine(conf, env, dep_engine);

    conf_ctx = axis2_conf_ctx_create(env, conf);
    if(!conf_ctx)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Creating Axis2 configuration context failed.");
        return NULL;
    }
    axis2_conf_ctx_set_root_dir(conf_ctx, env, axis2_dep_engine_get_repos_path(dep_engine, env));

    conf_ctx_base = axis2_conf_ctx_get_base(conf_ctx, env);
    property = axutil_property_create_with_args(env, 2, 0, 0, is_server_side);
    axis2_ctx_set_property(conf_ctx_base, env, AXIS2_IS_SVR_SIDE, property);

    axis2_init_modules(env, conf_ctx);
    axis2_init_transports(env, conf_ctx);

    if(!axutil_strcmp(is_server_side, AXIS2_VALUE_TRUE))
    {
        axis2_load_services(env, conf_ctx);
    }

    return conf_ctx;
}

AXIS2_EXTERN axis2_conf_ctx_t *AXIS2_CALL
axis2_build_conf_ctx(
    const axutil_env_t * env,
    const axis2_char_t * repo_name)
{
    axis2_conf_ctx_t *conf_ctx = NULL;
    axis2_dep_engine_t *dep_engine = NULL;

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Entry:axis2_build_conf_ctx");
    dep_engine = axis2_dep_engine_create_with_repos_name(env, repo_name);
    if(!dep_engine)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Creating deployment engine failed for repository %s", repo_name);
        return NULL;
    }

    conf_ctx = axis2_build_conf_ctx_with_dep_engine(env, dep_engine, AXIS2_VALUE_TRUE);
    if(!conf_ctx)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Loading configuration context failed for repository %s.", repo_name);
        return NULL;
    }

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Exit:axis2_build_conf_ctx");
    return conf_ctx;
}

AXIS2_EXTERN axis2_conf_ctx_t *AXIS2_CALL
axis2_build_conf_ctx_with_file(
    const axutil_env_t * env,
    const axis2_char_t * file)
{
    axis2_conf_ctx_t *conf_ctx = NULL;
    axis2_dep_engine_t *dep_engine = NULL;
	axis2_char_t *repos_path = NULL;
	axis2_char_t *temp_path = NULL;
	axis2_char_t *r_idx = NULL;
	axis2_char_t *xmlfile = NULL;
	
	AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Entry:axis2_build_conf_ctx_with_file");
	temp_path = axutil_strdup(env, file);
	r_idx = axutil_rindex(temp_path, AXIS2_PATH_SEP_CHAR);
	if(!r_idx)
	{/* searching from windows specific path seperator */
		r_idx = axutil_rindex(temp_path, '\\');
	}

	if(!r_idx)
	{
		/** only the xml file name is provided. Assume the default repo path */
		repos_path = AXIS2_GETENV("AXIS2C_HOME");
		xmlfile = (axis2_char_t*)file;
	}else
	{
		xmlfile = r_idx+1;
		temp_path[r_idx-temp_path] = '\0';
		repos_path = temp_path;
	}
	
	dep_engine = axis2_dep_engine_create_with_repos_name_and_svr_xml_file(env, repos_path, xmlfile);
    if(!dep_engine)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Creating deployment engine for given Axis2 configuration file"
                "(axis2.xml) failed");
        return NULL;
    }

    conf_ctx = axis2_build_conf_ctx_with_dep_engine(env, dep_engine, AXIS2_VALUE_TRUE);
    if(!conf_ctx)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Loading configuration context failed for given Axis2 configuration %s.", file);
        return NULL;
    }
	AXIS2_FREE(env->allocator, temp_path);
    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Exit:axis2_build_conf_ctx_with_file");
    return conf_ctx;
}

axis2_conf_ctx_t *AXIS2_CALL
axis2_build_client_conf_ctx(
    const axutil_env_t * env,
    const axis2_char_t * axis2_home)
{
    axis2_conf_ctx_t *conf_ctx = NULL;
    axis2_dep_engine_t *dep_engine = NULL;
    axis2_conf_t *conf = NULL;
    axutil_property_t *property = NULL;
    axis2_ctx_t *conf_ctx_base = NULL;

    axis2_status_t status;
    size_t len = 0;

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Entry:axis2_build_client_conf_ctx");
    /* Building conf using axis2.xml, in that case we check whether
     * last 9 characters of the axis2_home equals to the "axis2.xml"
     * else treat it as a directory 
     */

    status = axutil_file_handler_access (axis2_home, AXIS2_R_OK);
    if (status == AXIS2_SUCCESS)
    {
        len = strlen(axis2_home);
        /* We are sure that the difference lies within the int range */
        if ((len >= 9) &&
            !strcmp ((axis2_home + (len - 9)), "axis2.xml"))
        {
            dep_engine = axis2_dep_engine_create_with_axis2_xml (env,
                axis2_home);
        }
        else
        {
            dep_engine = axis2_dep_engine_create(env);
        }
    }
    else
    {
        AXIS2_LOG_WARNING (env->log, AXIS2_LOG_SI,
            "Provided client repository path %s does not exsist or no "\
            "permission to read, therefore set axis2c home to DEFAULT_REPO_PATH.",
            axis2_home);
        axis2_home = DEFAULT_REPO_PATH;
        dep_engine = axis2_dep_engine_create(env);
    }
    if (!dep_engine)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Creating deployment engine for client repository %s failed." ,
            axis2_home);
        return NULL;
    }
    conf = axis2_dep_engine_load_client(dep_engine, env, axis2_home);
    if (!conf)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Loading deployment engine failed for client repository %s",
            axis2_home);
        axis2_dep_engine_free(dep_engine, env);
        return NULL;
    }
    axis2_conf_set_dep_engine(conf, env, dep_engine);

    conf_ctx = axis2_conf_ctx_create(env, conf);
    if (!conf_ctx)
    {
        AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
            "Creating Axis2 configuration context failed");
        return NULL;
    }
    axis2_conf_ctx_set_root_dir(conf_ctx, env, axis2_dep_engine_get_repos_path(dep_engine, env));

    conf_ctx_base = axis2_conf_ctx_get_base(conf_ctx, env);
    property = axutil_property_create_with_args(env, 2, 0, 0, AXIS2_VALUE_FALSE);
    axis2_ctx_set_property(conf_ctx_base, env, AXIS2_IS_SVR_SIDE, property);

    axis2_init_modules(env, conf_ctx);
    axis2_init_transports(env, conf_ctx);

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Exit:axis2_build_client_conf_ctx");
    return conf_ctx;
}

axis2_status_t AXIS2_CALL
axis2_init_modules(
    const axutil_env_t * env,
    axis2_conf_ctx_t * conf_ctx)
{
    axis2_conf_t *conf = NULL;
    axis2_status_t status = AXIS2_FAILURE;

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Entry:axis2_init_modules");
    AXIS2_PARAM_CHECK(env->error, conf_ctx, AXIS2_FAILURE);

    conf = axis2_conf_ctx_get_conf(conf_ctx, env);
    if(conf)
    {
        axutil_hash_t *module_map = axis2_conf_get_all_modules(conf, env);
        if(module_map)
        {
            axutil_hash_index_t *hi = NULL;
            void *module = NULL;
            for(hi = axutil_hash_first(module_map, env); hi; hi = axutil_hash_next(env, hi))
            {
                axutil_hash_this(hi, NULL, NULL, &module);
                if(module)
                {
                    axis2_module_desc_t *mod_desc = (axis2_module_desc_t *)module;
                    axis2_module_t *mod = axis2_module_desc_get_module(mod_desc, env);
                    if(mod)
                    {
                        axis2_module_init(mod, env, conf_ctx, mod_desc);
                    }
                }
            }
        }
        status = AXIS2_SUCCESS;
    }
    else
    {
        AXIS2_LOG_WARNING(env->log, AXIS2_LOG_SI,
            "Retrieving Axis2 configuration from Axis2 configuration context "
                "failed. Initializing modules failed");
    }
    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Exit:axis2_init_modules");
    return status;
}

static axis2_status_t AXIS2_CALL
axis2_load_services(
    const axutil_env_t * env,
    axis2_conf_ctx_t * conf_ctx)
{
    axis2_conf_t *conf = NULL;
    axis2_status_t status = AXIS2_FAILURE;
    axutil_hash_t *svc_map = NULL;

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Entry:axis2_load_services");
    AXIS2_PARAM_CHECK(env->error, conf_ctx, AXIS2_FAILURE);

    conf = axis2_conf_ctx_get_conf(conf_ctx, env);
    if(!conf)
    {
        AXIS2_LOG_WARNING(env->log, AXIS2_LOG_SI, "Retrieving Axis2 configuration from Axis2 "
            "configuration context failed, Loading services failed");
        return status;
    }

    svc_map = axis2_conf_get_all_svcs_to_load(conf, env);
    if(svc_map)
    {
        axutil_hash_index_t *hi = NULL;
        void *svc = NULL;
        for(hi = axutil_hash_first(svc_map, env); hi; hi = axutil_hash_next(env, hi))
        {
            void *impl_class = NULL;
			axutil_hash_t *ops_hash = NULL;
			axis2_msg_recv_t *msg_recv = NULL;
            axutil_hash_this(hi, NULL, NULL, &svc);
            if(!svc)
            {
                continue;
            }
			impl_class = axis2_svc_get_impl_class(svc, env);
			if(impl_class)
				continue;

			ops_hash = axis2_svc_get_all_ops(svc, env);
			if(ops_hash)
			{
				axutil_hash_index_t *op_hi = NULL;
				void *op = NULL;
				op_hi = axutil_hash_first(ops_hash, env);
				if(op_hi)
				{
					axutil_hash_this(op_hi, NULL, NULL, &op);
					if(op)
					{
						msg_recv = axis2_op_get_msg_recv(op, env);
						if(msg_recv)
						{
							axis2_msg_recv_set_conf_ctx(msg_recv, env, conf_ctx);
							axis2_msg_recv_load_and_init_svc(msg_recv, env, svc);
						}
					}
				}

			}
			/*
            svc_desc = (axis2_svc_t *)svc;
            if(!svc_desc)
            {
                continue;
            }

            svc_name = axis2_svc_get_name(svc_desc, env);
            impl_info_param = axis2_svc_get_param(svc_desc, env, AXIS2_SERVICE_CLASS);

            if(!impl_info_param)
            {
                AXIS2_ERROR_SET(env->error, AXIS2_ERROR_INVALID_STATE_SVC, AXIS2_FAILURE);
                AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Invalid state of the service %s", svc_name);
                return AXIS2_FAILURE;
            }
            axutil_class_loader_init(env);
            impl_class = axutil_class_loader_create_dll(env, impl_info_param);
            if(!impl_class)
            {
                AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Service %s could not be loaded", svc_name);
                return AXIS2_FAILURE;
            }

            axis2_svc_set_impl_class(svc_desc, env, impl_class);
            if(AXIS2_SVC_SKELETON_INIT_WITH_CONF((axis2_svc_skeleton_t *)impl_class, env, conf)
                != AXIS2_SUCCESS)
            {
                AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Initialization failed for the service %s. "
                    "Check the service's init_with_conf() function for errors and retry", svc_name);
            }
			*/

        }
    }
    status = AXIS2_SUCCESS;

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Exit:axis2_load_services");
    return status;
}

axis2_status_t AXIS2_CALL
axis2_init_transports(
    const axutil_env_t * env,
    axis2_conf_ctx_t * conf_ctx)
{
    axis2_conf_t *conf = NULL;
    axis2_status_t status = AXIS2_FAILURE;

    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Entry:axis2_init_transports");
    AXIS2_PARAM_CHECK(env->error, conf_ctx, AXIS2_FAILURE);

    conf = axis2_conf_ctx_get_conf(conf_ctx, env);
    if(conf)
    {
        axis2_transport_in_desc_t **transport_in_map = NULL;
        axis2_transport_out_desc_t **transport_out_map = NULL;
        int i = 0;

        transport_in_map = axis2_conf_get_all_in_transports(conf, env);
        for(i = 0; i < AXIS2_TRANSPORT_ENUM_MAX; i++)
        {
            if(transport_in_map[i])
            {
                axis2_transport_receiver_t *listener = axis2_transport_in_desc_get_recv(
                    transport_in_map[i], env);
                if(listener)
                {
                    status = axis2_transport_receiver_init(listener, env, conf_ctx,
                        transport_in_map[i]);
		    if (AXIS2_SUCCESS != status)
		    {
			    AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, 
					    "Transport receiver initialization failed.");
		    }
                }
            }
        }

        transport_out_map = axis2_conf_get_all_out_transports(conf, env);
        for(i = 0; i < AXIS2_TRANSPORT_ENUM_MAX; i++)
        {
            if(transport_out_map[i])
            {
                axis2_transport_sender_t *sender = axis2_transport_out_desc_get_sender(
                    transport_out_map[i], env);
                if(sender)
                {
                    status = AXIS2_TRANSPORT_SENDER_INIT(sender, env, conf_ctx,
                        transport_out_map[i]);
		    if (AXIS2_SUCCESS != status)
		    {
			    AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, 
					    "Axis2 transport sender initialization failed.");
		    }
                }
            }
        }
        status = AXIS2_SUCCESS;
    }
    else
    {
        AXIS2_LOG_WARNING(env->log, AXIS2_LOG_SI,
            "Retrieving Axis2 configuration from Axis2 configuration context "
                "failed. Initializing transports failed");
    }
    AXIS2_LOG_TRACE(env->log, AXIS2_LOG_SI, "Exit:axis2_init_transports");
    return status;
}

