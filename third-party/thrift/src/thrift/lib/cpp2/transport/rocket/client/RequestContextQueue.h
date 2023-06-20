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

#pragma once

#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/Likely.h>

#include <thrift/lib/cpp2/transport/rocket/client/RequestContext.h>

namespace apache {
namespace thrift {
namespace rocket {

class RequestContextQueue {
 public:
  RequestContextQueue() = default;

  ~RequestContextQueue() {
    DCHECK(writeScheduledQueue_.empty());
    DCHECK(writeSendingQueue_.empty());
    DCHECK(writeSentQueue_.empty());
  }

  void enqueueScheduledWrite(RequestContext& req) noexcept;

  template <typename F>
  void prepareNextScheduledWritesBatch(F&& foreachRequest) noexcept {
    while (!writeScheduledQueue_.empty()) {
      auto& req = writeScheduledQueue_.front();
      writeScheduledQueue_.pop_front();

      DCHECK(req.state_ == State::WRITE_SCHEDULED);
      req.state_ = State::WRITE_SENDING;
      if (req.isRequestResponse()) {
        req.scheduleTimeoutForResponse();
      }
      writeSendingQueue_.push_back(req);

      if (writeScheduledQueue_.empty()) {
        req.lastInWriteBatch_ = true;
      }

      foreachRequest(req);
    }
  }

  template <typename F>
  void markNextSendingBatchAsSent(F&& foreachRequest) noexcept {
    CHECK(!writeSendingQueue_.empty()) << "empty queue";
    for (bool lastInBatch = false; !lastInBatch;) {
      CHECK(!writeSendingQueue_.empty()) << "missing end of batch marker";
      auto& req = writeSendingQueue_.front();
      writeSendingQueue_.pop_front();
      DCHECK(req.state() == State::WRITE_SENDING);

      if (req.isDummyEndOfBatchMarker_) {
        DCHECK(req.lastInWriteBatch_);
        delete &req;
        return;
      }

      lastInBatch = req.lastInWriteBatch_;
      req.state_ = State::WRITE_SENT;
      // Move req to the WRITE_SENT queue even if req is not a
      // REQUEST_RESPONSE request.
      writeSentQueue_.push_back(req);

      foreachRequest(req);
    }
  }

  void timeOutSendingRequest(RequestContext& req) noexcept;
  void abortSentRequest(
      RequestContext& req, transport::TTransportException ex) noexcept;

  void markAsResponded(RequestContext& req) noexcept;

  void failAllScheduledWrites(folly::exception_wrapper ew);
  void failAllSentWrites(folly::exception_wrapper ew);

  RequestContext* getRequestResponseContext(StreamId streamId);

  bool startBufferingRequests() {
    if (!writeBufferQueue_) {
      writeBufferQueue_.reset(new RequestContext::Queue());
      return true;
    }
    return false;
  }

  bool resolveWriteBuffer(uint32_t serverVersion) {
    if (!writeBufferQueue_) {
      return false;
    }
    auto queue = std::move(writeBufferQueue_);
    if (queue->empty()) {
      return false;
    }
    while (!queue->empty()) {
      auto& req = queue->front();
      queue->pop_front();
      req.initWithVersion(serverVersion);
      enqueueScheduledWrite(req);
    }
    return true;
  }

 private:
  using RequestResponseSet = RequestContext::UnorderedSet;

  static constexpr size_t kDefaultNumBuckets = 100;
  std::vector<RequestResponseSet::bucket_type> rrContextBuckets_{
      kDefaultNumBuckets};

  // Only REQUEST_RESPONSE contexts are ever inserted/looked up in this set.
  // Allows response payloads to be matched with requests. (Streams have a
  // different mechanism for doing this, since there are potentially many
  // response payloads per initiating REQUEST_STREAM context.)
  RequestResponseSet requestResponseContexts_{RequestResponseSet::bucket_traits{
      rrContextBuckets_.data(), rrContextBuckets_.size()}};

  using State = RequestContext::State;

  std::unique_ptr<RequestContext::Queue> writeBufferQueue_;
  // Requests for which AsyncSocket::writev() has not been called yet
  RequestContext::Queue writeScheduledQueue_;
  // Requests for which AsyncSocket::writev() has been called but completion
  // of the write to the underlying transport (successful or otherwise) is
  // still pending.
  RequestContext::Queue writeSendingQueue_;
  // Once the attempt to write a request to the socket is complete, the
  // request moves to this queue. Note that the request flows into this queue
  // even if the write to the socket failed, i.e., regardless of whether
  // writeSuccess() or writeErr() was called.
  RequestContext::Queue writeSentQueue_;

  void failQueue(RequestContext::Queue& queue, folly::exception_wrapper ew);

  void removeFromWriteSendingQueue(RequestContext& req) noexcept;

  void trackIfRequestResponse(RequestContext& req) {
    if (req.isRequestResponse()) {
      if (UNLIKELY(
              requestResponseContexts_.size() > rrContextBuckets_.size())) {
        growBuckets();
      }
      requestResponseContexts_.insert(req);
    }
  }
  void untrackIfRequestResponse(RequestContext& req) {
    if (req.isRequestResponse()) {
      requestResponseContexts_.erase(req);
    }
  }

  void growBuckets();
};

} // namespace rocket
} // namespace thrift
} // namespace apache
