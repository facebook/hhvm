/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McServerRequestContext.h"

#include "mcrouter/lib/network/McServerSession.h"
#include "mcrouter/lib/network/MultiOpParent.h"

namespace facebook {
namespace memcache {

McServerSession& McServerRequestContext::session() {
  assert(session_ != nullptr);
  return *session_;
}

McServerRequestContext::McServerRequestContext(
    McServerSession& s,
    uint64_t r,
    bool nr,
    std::shared_ptr<MultiOpParent> parent,
    bool isEndContext)
    : session_(&s), isEndContext_(isEndContext), noReply_(nr), reqid_(r) {
  if (parent) {
    asciiState_ = std::make_unique<AsciiState>();
    asciiState_->parent_ = std::move(parent);
    asciiState_->parent_->recordRequest();
  }

  session_->onTransactionStarted(hasParent() || isEndContext_);
}

McServerRequestContext::McServerRequestContext(
    McServerRequestContext&& other) noexcept
    : session_(other.session_),
      isEndContext_(other.isEndContext_),
      noReply_(other.noReply_),
      replied_(other.replied_),
      reqid_(other.reqid_),
      asciiState_(std::move(other.asciiState_)) {
  other.session_ = nullptr;
}

McServerRequestContext& McServerRequestContext::operator=(
    McServerRequestContext&& other) {
  session_ = other.session_;
  isEndContext_ = other.isEndContext_;
  reqid_ = other.reqid_;
  noReply_ = other.noReply_;
  replied_ = other.replied_;
  asciiState_ = std::move(other.asciiState_);

  other.session_ = nullptr;

  return *this;
}

McServerRequestContext::~McServerRequestContext() {
  if (session_) {
    /* Check that a reply was returned */
    assert(replied_);
    session_->onTransactionCompleted(hasParent() || isEndContext_);
  }
}

// Note: defined in .cpp in order to avoid circular dependency between
// McServerRequestContext.h and MultiOpParent.h.
bool McServerRequestContext::moveReplyToParent(
    carbon::Result result,
    uint32_t errorCode,
    std::string&& errorMessage) const {
  return hasParent() &&
      parent().reply(result, errorCode, std::move(errorMessage));
}

// Also defined in .cpp to avoid the same circular dependency
bool McServerRequestContext::isParentError() const {
  return parent().error();
}

ServerLoad McServerRequestContext::getServerLoad() const noexcept {
  if (session_) {
    if (const auto& cpuController = session_->getCpuController()) {
      return cpuController->getServerLoad();
    }
  }
  return ServerLoad::zero();
}

folly::Optional<std::string> McServerRequestContext::getPeerSocketAddressStr() {
  folly::Optional<std::string> peerAddressStr;
  if (session_) {
    peerAddressStr = session_->getSocketAddress().getAddressStr();
  }
  return peerAddressStr;
}

folly::Optional<struct sockaddr_storage>
McServerRequestContext::getPeerSocketAddress() {
  folly::Optional<struct sockaddr_storage> peerAddress;
  if (session_) {
    peerAddress.emplace();
    session_->getSocketAddress().getAddress(peerAddress.get_pointer());
  }
  return peerAddress;
}

folly::EventBase& McServerRequestContext::getSessionEventBase() const noexcept {
  return session_->getEventBase();
}

const apache::thrift::Cpp2RequestContext*
McServerRequestContext::getThriftRequestContext() const noexcept {
  return session_->getConnectionLevelThriftRequestContext();
}

const folly::AsyncTransportWrapper* McServerRequestContext::getTransport()
    const noexcept {
  return getThriftRequestContext()->getConnectionContext()->getTransport();
}

void McServerRequestContext::markAsTraced() {
  isTraced_ = true;
}

void markContextAsTraced(McServerRequestContext& ctx) {
  ctx.markAsTraced();
}

void* McServerRequestContext::getConnectionUserData() {
  return session_->userContext();
}

} // namespace memcache
} // namespace facebook
