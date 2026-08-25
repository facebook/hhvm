/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "artillery/if/gen-cpp2/artillery_server_block_constants.h"
#include "mcrouter/lib/network/ThriftTransport.h"

namespace facebook::memcache {
namespace {

TEST(ThriftTransportTest, TraceResponseCopiesServerBlockReport) {
  McGetRequest request{"key"};
  request.setTraceContext("trace-context");

  using CompleteResponse = apache::thrift::RpcResponseComplete<McGetReply>;
  folly::Try<CompleteResponse> response{CompleteResponse{
      .response = folly::Try<McGetReply>{McGetReply{carbon::Result::FOUND}},
      .responseContext = {},
  }};
  const std::string serializedReport{"serialized-server-block-report"};
  response->responseContext.headers.emplace(
      facebook::artillery2::artillery_server_block_constants::
          artillery_server_block_header_,
      serializedReport);

  ThriftTransportUtil{}.traceResponse(request, response);

  EXPECT_EQ(serializedReport, response->response->artilleryServerBlockReport());
}

} // namespace
} // namespace facebook::memcache
