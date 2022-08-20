/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McClientRequestContext.h"

namespace facebook {
namespace memcache {

constexpr size_t kSerializedRequestContextLength = 1024;

void McClientRequestContextBase::replyError(
    carbon::Result result,
    folly::StringPiece errorMessage) {
  assert(state() == ReqState::NONE);
  replyErrorImpl(result, errorMessage);
  setState(ReqState::COMPLETE);
  baton_.post();
}

void McClientRequestContextBase::fireStateChangeCallbacks(
    ReqState old,
    ReqState current) const {
  if (!onStateChange_) {
    return;
  }

  int pending = 0;
  int inflight = 0;

  switch (old) {
    case ReqState::PENDING_QUEUE:
      pending--;
      break;
    case ReqState::WRITE_QUEUE:
    case ReqState::PENDING_REPLY_QUEUE:
    case ReqState::REPLIED_QUEUE:
      inflight--;
      break;
    default:
      break;
  }
  switch (current) {
    case ReqState::PENDING_QUEUE:
      pending++;
      break;
    case ReqState::WRITE_QUEUE:
    case ReqState::PENDING_REPLY_QUEUE:
    case ReqState::REPLIED_QUEUE:
      inflight++;
      break;
    default:
      break;
  }

  if (pending != 0 || inflight != 0) {
    onStateChange_(pending, inflight);
  }
}

void McClientRequestContextBase::scheduleTimeout() {
  if (state() != ReqState::COMPLETE) {
    batonTimeoutHandler_.scheduleTimeout(batonWaitTimeout_);
  }
}

McClientRequestContextBase::~McClientRequestContextBase() {
  assert(state() == ReqState::NONE || state() == ReqState::COMPLETE);
}

McClientRequestContextQueue::McClientRequestContextQueue(
    bool outOfOrder) noexcept
    : outOfOrder_(outOfOrder),
      buckets_(kDefaultNumBuckets),
      set_(McClientRequestContextBase::UnorderedSet::bucket_traits(
          buckets_.data(),
          buckets_.size())) {}

void McClientRequestContextQueue::growBucketsArray() {
  // Allocate buckets array that is twice bigger than the current one.
  std::vector<McClientRequestContextBase::UnorderedSet::bucket_type> tmp(
      buckets_.size() * 2);
  set_.rehash(McClientRequestContextBase::UnorderedSet::bucket_traits(
      tmp.data(), tmp.size()));
  // Use swap here, since it has better defined behaviour regarding iterators.
  buckets_.swap(tmp);
}

size_t McClientRequestContextQueue::getPendingRequestCount() const noexcept {
  return pendingQueue_.size();
}

size_t McClientRequestContextQueue::getInflightRequestCount() const noexcept {
  return repliedQueue_.size() + writeQueue_.size() + pendingReplyQueue_.size();
}

void McClientRequestContextQueue::failAllSent(
    carbon::Result error,
    folly::StringPiece errorMessage) {
  clearStoredInitializers();
  failQueue(pendingReplyQueue_, error, errorMessage);
}

void McClientRequestContextQueue::failAllPending(
    carbon::Result error,
    folly::StringPiece errorMessage) {
  assert(pendingReplyQueue_.empty());
  assert(writeQueue_.empty());
  assert(repliedQueue_.empty());
  failQueue(pendingQueue_, error, errorMessage);
}

void McClientRequestContextQueue::clearStoredInitializers() {
  while (!timedOutInitializers_.empty()) {
    timedOutInitializers_.pop();
  }
}

size_t McClientRequestContextQueue::getFirstId() const {
  assert(getPendingRequestCount());
  return pendingQueue_.front().id;
}

void McClientRequestContextQueue::markAsPending(
    McClientRequestContextBase& req) {
  assert(req.state() == State::NONE);
  req.setState(State::PENDING_QUEUE);
  pendingQueue_.push_back(req);

  if (outOfOrder_) {
    // We hit the number of allocated buckets, grow the array.
    if (set_.size() >= buckets_.size()) {
      growBucketsArray();
    }
    set_.insert(req);
  }
}

McClientRequestContextBase& McClientRequestContextQueue::peekNextPending() {
  return pendingQueue_.front();
}

McClientRequestContextBase& McClientRequestContextQueue::markNextAsSending() {
  auto& req = pendingQueue_.front();
  pendingQueue_.pop_front();
  assert(req.state() == State::PENDING_QUEUE);
  req.setState(State::WRITE_QUEUE);
  writeQueue_.push_back(req);
  return req;
}

McClientRequestContextBase& McClientRequestContextQueue::markNextAsSent() {
  if (!repliedQueue_.empty()) {
    auto& req = repliedQueue_.front();
    repliedQueue_.pop_front();
    req.setState(State::COMPLETE);
    req.baton_.post();
    return req;
  }

  auto& req = writeQueue_.front();
  writeQueue_.pop_front();
  if (req.state() == State::COMPLETE) {
    req.baton_.post();
  } else {
    assert(req.state() == State::WRITE_QUEUE);
    req.setState(State::PENDING_REPLY_QUEUE);
    pendingReplyQueue_.push_back(req);
  }
  return req;
}

void McClientRequestContextQueue::failQueue(
    McClientRequestContextBase::Queue& queue,
    carbon::Result error,
    folly::StringPiece errorMessage) {
  while (!queue.empty()) {
    auto& req = queue.front();
    queue.pop_front();
    removeFromSet(req);
    req.setState(State::NONE);
    req.replyError(error, errorMessage);
  }
}

McClientRequestContextBase::UnorderedSet::iterator
McClientRequestContextQueue::getContextById(uint64_t id) {
  return set_.find(
      id,
      std::hash<uint64_t>(),
      [](uint64_t i, const McClientRequestContextBase& ctx) {
        return i == ctx.id;
      });
}

void McClientRequestContextQueue::removeFromSet(
    McClientRequestContextBase& req) {
  if (outOfOrder_) {
    set_.erase(req);
  }
}

void McClientRequestContextQueue::removePending(
    McClientRequestContextBase& req) {
  assert(req.state() == State::PENDING_QUEUE);
  removeFromSet(req);
  pendingQueue_.erase(pendingQueue_.iterator_to(req));
  req.setState(State::NONE);
}

void McClientRequestContextQueue::removePendingReply(
    McClientRequestContextBase& req) {
  assert(req.state() == State::PENDING_REPLY_QUEUE);
  assert(&req == &pendingReplyQueue_.front() || outOfOrder_);
  removeFromSet(req);
  pendingReplyQueue_.erase(pendingReplyQueue_.iterator_to(req));
  req.setState(State::NONE);
  // We need timedOutInitializers_ only for in order protocol.
  if (!outOfOrder_) {
    timedOutInitializers_.push(req.initializer_);
  }
}

McClientRequestContextBase::InitializerFuncPtr
McClientRequestContextQueue::getParserInitializer(uint64_t reqId) {
  if (outOfOrder_) {
    auto it = getContextById(reqId);
    if (it != set_.end()) {
      return it->initializer_;
    }
  } else {
    // In inorder protocol we expect to receive timedout requests first.
    if (!timedOutInitializers_.empty()) {
      return timedOutInitializers_.front();
    } else if (!pendingReplyQueue_.empty()) {
      return pendingReplyQueue_.front().initializer_;
    } else if (!writeQueue_.empty()) {
      return writeQueue_.front().initializer_;
    }
  }
  return nullptr;
}

std::string McClientRequestContextQueue::debugInfo() const {
  return folly::sformat(
      "Currently have {} timedout initializers, {} requests in "
      "replied queue, {} requests in pending reply queue, "
      "{} requests in write queue and {} requests in pending queue, "
      "the first alive request is: {}",
      timedOutInitializers_.size(),
      repliedQueue_.size(),
      pendingReplyQueue_.size(),
      writeQueue_.size(),
      pendingQueue_.size(),
      getFirstAliveRequestInfo());
}

std::string McClientRequestContextQueue::getFirstAliveRequestInfo() const {
  const McClientRequestContextBase* ctx{nullptr};

  if (!pendingReplyQueue_.empty()) {
    ctx = &pendingReplyQueue_.front();
  } else if (!writeQueue_.empty()) {
    ctx = &writeQueue_.front();
  } else {
    return "no alive requests that were sent or are being sent";
  }

  size_t dataLen = 0;
  for (size_t i = 0; i < ctx->reqContext.getIovsCount(); ++i) {
    dataLen += ctx->reqContext.getIovs()[i].iov_len;
  }
  dataLen = std::min(dataLen, kSerializedRequestContextLength);
  std::vector<char> data(dataLen);

  for (size_t i = 0, dataOffset = 0;
       i < ctx->reqContext.getIovsCount() && dataOffset < dataLen;
       ++i) {
    auto toCopy =
        std::min(dataLen - dataOffset, ctx->reqContext.getIovs()[i].iov_len);
    memcpy(
        data.data() + dataOffset,
        ctx->reqContext.getIovs()[i].iov_base,
        toCopy);
    dataOffset += toCopy;
  }

  return folly::sformat(
      "{}, serialized data was: \"{}\"",
      ctx->getContextTypeStr(),
      folly::cEscape<std::string>(
          folly::StringPiece(data.data(), data.size())));
}
} // namespace memcache
} // namespace facebook
