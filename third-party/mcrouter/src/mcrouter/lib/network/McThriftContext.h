/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/io/async/EventBase.h>

#include <mcrouter/lib/network/gen/gen-cpp2/Memcache.h>

namespace facebook {
namespace memcache {

template <class Reply>
class McThriftContext {
 public:
  McThriftContext(std::unique_ptr<apache::thrift::HandlerCallback<Reply>> ctx)
      : underlying_(std::move(ctx)) {}

  static void
  reply(McThriftContext<Reply>&& ctx, Reply&& reply, bool /* flush */ = false) {
    ctx.underlying_->result(std::move(reply));
    ctx.underlying_.reset();
  }

  template <class E>
  static void exception(McThriftContext<Reply>&& ctx, E&& ex) {
    ctx.underlying_->exception(std::move(ex));
  }

  folly::Optional<std::string> getPeerSocketAddressStr() {
    folly::Optional<std::string> peerAddressStr;
    auto connectionCxt = underlying_->getConnectionContext();
    if (connectionCxt) {
      peerAddressStr = connectionCxt->getPeerAddress()->getAddressStr();
    }
    return peerAddressStr;
  }

  folly::Optional<struct sockaddr_storage> getPeerSocketAddress() {
    folly::Optional<struct sockaddr_storage> peerAddress;
    auto connectionCxt = underlying_->getConnectionContext();
    if (connectionCxt && connectionCxt->getPeerAddress()) {
      peerAddress.emplace();
      connectionCxt->getPeerAddress()->getAddress(peerAddress.get_pointer());
    }
    return peerAddress;
  }

  folly::EventBase& getSessionEventBase() const noexcept {
    return *(underlying_->getEventBase());
  }

  const folly::AsyncTransportWrapper* getTransport() const noexcept {
    return underlying_->getConnectionContext()
        ->getConnectionContext()
        ->getTransport();
  }

  const apache::thrift::Cpp2RequestContext* getThriftRequestContext()
      const noexcept {
    return underlying_->getRequestContext();
  }

  void* getConnectionUserData() const {
    return underlying_->getConnectionContext()->getUserData();
  }

  apache::thrift::transport::THeader* getHeader() const {
    return underlying_->getConnectionContext()->getHeader();
  }

  apache::thrift::HandlerCallback<Reply>& getThriftHandlerCallback() {
    return *underlying_;
  }

 private:
  std::unique_ptr<apache::thrift::HandlerCallback<Reply>> underlying_;
};

} // namespace memcache
} // namespace facebook
