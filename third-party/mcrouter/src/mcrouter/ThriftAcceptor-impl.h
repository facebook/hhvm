/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <utility>

#include <wangle/acceptor/Acceptor.h>

namespace folly {
class EventBase;
} // namespace folly

namespace apache {
namespace thrift {
class ThriftServer;
} // namespace thrift
} // namespace apache

namespace facebook {
namespace memcache {

class ThriftAcceptorFactory final : public wangle::AcceptorFactory {
  using ThriftAclCheckerFunc =
      std::function<bool(const folly::AsyncTransportWrapper*)>;

 public:
  explicit ThriftAcceptorFactory(
      apache::thrift::ThriftServer& server,
      ThriftAclCheckerFunc /* unused */,
      int trafficClass /* unused */)
      : server_(server) {}
  ~ThriftAcceptorFactory() override = default;

  std::shared_ptr<wangle::Acceptor> newAcceptor(folly::EventBase* evb) override;

 private:
  apache::thrift::ThriftServer& server_;
};

} // namespace memcache
} // namespace facebook
