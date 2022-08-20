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

#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/transport/core/testutil/FakeChannel.h>
#include <thrift/lib/cpp2/transport/core/testutil/ServerConfigsMock.h>

using namespace apache::thrift;

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(
    void,
    handleFrameworkMetadata,
    std::unique_ptr<folly::IOBuf>&& frameworkMetadata) {
  const auto& buf = *frameworkMetadata;
  std::string content(reinterpret_cast<const char*>(buf.data()), buf.length());
  EXPECT_EQ(content, "abc");
}
} // namespace apache::thrift::detail

TEST(ThriftRequest, frameworkMetadata) {
  folly::EventBase eb;

  server::ServerConfigsMock serverConfigs;
  auto channel = std::make_shared<FakeChannel>(&eb);
  RequestRpcMetadata metadata;
  auto connContext = std::make_unique<Cpp2ConnContext>();

  metadata.frameworkMetadata_ref() =
      folly::IOBuf::copyBuffer(std::string("abc"));
  ThriftRequest req(
      serverConfigs, channel, std::move(metadata), std::move(connContext));
}
