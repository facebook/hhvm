/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <thrift/lib/cpp2/server/Cpp2Worker.h>

#include <mcrouter/lib/network/McSSLUtil.h>
#include "mcrouter/lib/network/McServerThriftRequestContext.h"

namespace facebook {
namespace memcache {

namespace {

/* Connection context requires a worker (https://fburl.com/code/39bsp7uk)
 * We can initialize a dummy one that will stay alive throughout the lifetime
 * of the process.
 */
class DummyCpp2Worker : public apache::thrift::Cpp2Worker {
 public:
  explicit DummyCpp2Worker() : Cpp2Worker(nullptr, {}) {}
};
const DummyCpp2Worker gDummyCpp2Worker;

} // namespace
McServerThriftRequestContext::McServerThriftRequestContext(
    const folly::AsyncTransportWrapper* transport)
    : peerAddress_(transport->getPeerAddress()),
      connCtx_(
          &peerAddress_,
          transport,
          nullptr /* manager */,
          nullptr /* peerCert */,
          McSSLUtil::getClientIdentityHook(),
          &gDummyCpp2Worker),
      reqCtx_(&connCtx_) {}

} // namespace memcache
} // namespace facebook
