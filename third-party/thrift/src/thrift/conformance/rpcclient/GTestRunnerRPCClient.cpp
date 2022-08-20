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

#include <thrift/conformance/Utils.h>
#include <thrift/conformance/rpcclient/GTestHarnessRPCClient.h>

namespace apache::thrift::conformance {
namespace {

std::map<std::string_view, std::string_view> getClientCmds() {
  return parseCmds(getEnvOrThrow("THRIFT_CONFORMANCE_CLIENT_BINARIES"));
}

// Register the tests with gtest.
THRIFT_RPC_CLIENT_CONFORMANCE_TEST(
    getSuites(), getClientCmds(), getNonconforming());
} // namespace
} // namespace apache::thrift::conformance
