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

#include <string_view>

#include <thrift/conformance/PluggableFunctions.h>

namespace apache::thrift::conformance {

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<Client<ConformanceService>>,
    create_conformance_service_client_,
    std::string_view /*serviceName or smc tier*/) {
  throw std::invalid_argument(
      "Unimplemented Method create_conformance_service_client_");
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<Client<RPCConformanceService>>,
    create_rpc_conformance_service_client_,
    std::string_view /*serviceName or smc tier*/) {
  throw std::invalid_argument(
      "Unimplemented Method create_rpc_conformance_service_client_");
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<Client<BasicRPCConformanceService>>,
    create_basic_rpc_conformance_service_client_,
    std::string_view /*serviceName or smc tier*/) {
  throw std::invalid_argument(
      "Unimplemented Method create_basic_rpc_conformance_service_client_");
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<Client<RPCConformanceSetupService>>,
    create_rpc_conformance_setup_service_client_,
    std::string_view /*serviceName or smc tier*/) {
  throw std::invalid_argument(
      "Unimplemented Method create_rpc_conformance_setup_service_client_");
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    int, update_server_props_, apache::thrift::ThriftServer&) {
  throw std::invalid_argument("Unimplemented Method update_server_props_");
}

THRIFT_PLUGGABLE_FUNC_REGISTER(std::string, get_server_ip_addr_) {
  throw std::invalid_argument("Unimplemented Method get_server_ip_addr_");
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    folly::Subprocess,
    launch_client_process_,
    const std::vector<std::string>& argv) {
  return folly::Subprocess(argv);
}
} // namespace apache::thrift::conformance
