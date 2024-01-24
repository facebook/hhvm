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

#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>

using namespace apache::thrift;
using namespace apache::thrift::detail;

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(
    std::unique_ptr<folly::IOBuf>,
    makeFrameworkMetadata,
    const RpcOptions& rpcOptions,
    folly::dynamic&,
    folly::F14NodeMap<std::string, std::string>&) {
  EXPECT_EQ(rpcOptions.getShardId(), "123");
  return folly::IOBuf::copyBuffer(std::string("linked"));
}
} // namespace apache::thrift::detail

TEST(RpcMetadataUtil, frameworkMetadata) {
  RpcOptions rpcOptions;
  auto kind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  auto protocolId = ProtocolId::COMPACT;
  std::string methodName = "foo";
  std::chrono::milliseconds timeout(100);
  transport::THeader header;

  rpcOptions.setShardId("123");
  auto requestRpcMetadata = makeRequestRpcMetadata(
      rpcOptions, kind, protocolId, methodName, timeout, header);
  const auto& buf = **requestRpcMetadata.frameworkMetadata_ref();
  std::string content(reinterpret_cast<const char*>(buf.data()), buf.length());
  EXPECT_EQ(content, "linked");
}
