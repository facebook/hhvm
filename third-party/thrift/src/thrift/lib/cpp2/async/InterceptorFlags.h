/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <thrift/lib/cpp2/Flags.h>

namespace apache::thrift {

/**
 * Boolean flag that controls whether or not the frameworkMetadata buffer of
 * RpcMetadata is populated from client interceptors.
 */
THRIFT_FLAG_DECLARE_bool(enable_client_interceptor_framework_metadata);

/**
 * Boolean flag that controls whether or not ServiceInterceptors have access
 * to the frameworkMetadata passed along as part of the incoming request.
 */
THRIFT_FLAG_DECLARE_bool(enable_service_interceptor_framework_metadata);

/**
 * This flag will prevent all client interceptors from running if set to true.
 */
THRIFT_FLAG_DECLARE_bool(disable_all_client_interceptors);

/**
 * This flag will prevent all client interceptors from running if set to true.
 */
THRIFT_FLAG_DECLARE_bool(disable_all_service_interceptors);

/**
 * This is a string flag that contains a comma separated list of fully qualified
 * names of service interceptors that should be disabled. An empty string
 * indicates that no service interceptors are disabled by name.
 */
THRIFT_FLAG_DECLARE_string(disabled_service_interceptors);

/**
 * This flag controls whether Python thrift clients use global client
 * interceptors. Defaults to false for gradual rollout. When false, Python
 * clients will not invoke any global client interceptors.
 */
THRIFT_FLAG_DECLARE_bool(enable_python_client_interceptors);

} // namespace apache::thrift
