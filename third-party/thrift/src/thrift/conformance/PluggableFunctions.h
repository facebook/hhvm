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

#include <string_view>

#include <folly/Subprocess.h>
#include <thrift/conformance/if/gen-cpp2/ConformanceServiceAsyncClient.h>
#include <thrift/conformance/if/gen-cpp2/RPCConformanceSetupServiceAsyncClient.h>
#include <thrift/conformance/if/gen-cpp2/rpc_clients.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace apache::thrift::conformance {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<Client<ConformanceService>>,
    create_conformance_service_client_,
    std::string_view /*serviceName or smc tier*/);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<Client<RPCConformanceService>>,
    create_rpc_conformance_service_client_,
    std::string_view /*serviceName or smc tier*/);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<Client<BasicRPCConformanceService>>,
    create_basic_rpc_conformance_service_client_,
    std::string_view /*serviceName or smc tier*/);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<Client<RPCConformanceSetupService>>,
    create_rpc_conformance_setup_service_client_,
    std::string_view /*serviceName or smc tier*/);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    int, update_server_props_, apache::thrift::ThriftServer&);

THRIFT_PLUGGABLE_FUNC_DECLARE(std::string, get_server_ip_addr_);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    folly::Subprocess, launch_client_process_, const std::vector<std::string>&);
} // namespace apache::thrift::conformance
