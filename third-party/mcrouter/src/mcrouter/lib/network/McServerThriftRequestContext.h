/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace facebook {
namespace memcache {

/*
 * Cpp2RequestContext is a Thrift interface to fetch identities etc.
 * For users of Mcrouter who are using Carbon, McServerThriftRequestContext is a
 * thin wrapper that allows us to expose a barebones Cpp2RequestContext to pass
 * to downstream callers. Right now, this context will support identity
 * extraction but nothing else.
 */
class McServerThriftRequestContext {
 public:
  explicit McServerThriftRequestContext(
      const folly::AsyncTransportWrapper* transport);

  const apache::thrift::Cpp2RequestContext* getRequestContext() const noexcept {
    return &reqCtx_;
  }

 private:
  const folly::SocketAddress peerAddress_;
  apache::thrift::Cpp2ConnContext connCtx_;
  const apache::thrift::Cpp2RequestContext reqCtx_;
};

} // namespace memcache
} // namespace facebook
