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

#include <thrift/lib/cpp2/transport/rocket/client/RequestContextQueue.h>

#include <utility>

#include <folly/Likely.h>

namespace apache::thrift::rocket {

void RequestContextQueue::enqueueScheduledWrite(RequestContext& req) noexcept {
  if (UNLIKELY(writeBufferQueue_ != nullptr)) {
    writeBufferQueue_->push_back(req);
    return;
  }
  DCHECK(req.state_ == State::WRITE_NOT_SCHEDULED);

  req.state_ = State::WRITE_SCHEDULED;
  writeScheduledQueue_.push_back(req);
  trackIfRequestResponse(req);
}

void RequestContextQueue::timeOutSendingRequest(RequestContext& req) noexcept {
  DCHECK(req.state_ == State::WRITE_SENDING);
  DCHECK(!req.responsePayload_.hasException());

  req.responsePayload_.emplaceException(
      transport::TTransportException(
          transport::TTransportException::TIMED_OUT));
  untrackIfRequestResponse(req);
  removeFromWriteSendingQueue(req);
  req.state_ = State::COMPLETE;
}

void RequestContextQueue::abortSentRequest(
    RequestContext& req, transport::TTransportException ex) noexcept {
  if (req.state_ == State::COMPLETE) {
    return;
  }
  DCHECK(req.state() == State::WRITE_SENT);
  DCHECK(!req.responsePayload_.hasException());
  req.responsePayload_ = folly::Try<Payload>(std::move(ex));
  markAsResponded(req);
}

// For REQUEST_RESPONSE, this is called on the read path once the entire
// response payload has arrived.
// For REQUEST_STREAM and REQUEST_FNF, this is called once the write to the
// socket has completed.
void RequestContextQueue::markAsResponded(RequestContext& req) noexcept {
  untrackIfRequestResponse(req);

  if (LIKELY(req.state() == State::WRITE_SENT)) {
    writeSentQueue_.erase(writeSentQueue_.iterator_to(req));
  } else {
    // Response arrived after the socket write was initiated but before the
    // socket WriteCallback fired
    DCHECK(req.isRequestResponse());
    DCHECK(req.state() == State::WRITE_SENDING);
    removeFromWriteSendingQueue(req);
    req.onWriteSuccess();
  }

  req.state_ = State::COMPLETE;
  req.baton_.post();
}

void RequestContextQueue::failAllScheduledWrites(folly::exception_wrapper ew) {
  auto what = ew.what().toStdString();
  failQueue(
      writeScheduledQueue_,
      transport::TTransportException(
          transport::TTransportException::NOT_OPEN,
          fmt::format(
              "Dropping unsent request. Connection closed after: {}", what)));
  if (writeBufferQueue_) {
    failQueue(
        *writeBufferQueue_,
        transport::TTransportException(
            transport::TTransportException::NOT_OPEN,
            fmt::format(
                "Dropping unsent request. Connection closed after: {}", what)));
  }
}

void RequestContextQueue::failAllSentWrites(folly::exception_wrapper ew) {
  failQueue(writeSentQueue_, std::move(ew));
}

void RequestContextQueue::failQueue(
    RequestContext::Queue& queue, folly::exception_wrapper ew) {
  while (!queue.empty()) {
    auto& req = queue.front();
    queue.pop_front();
    DCHECK(!req.responsePayload_.hasException());
    req.responsePayload_ = folly::Try<Payload>(ew);
    untrackIfRequestResponse(req);
    req.state_ = State::COMPLETE;
    req.baton_.post();
  }
}

void RequestContextQueue::removeFromWriteSendingQueue(
    RequestContext& req) noexcept {
  DCHECK(req.state_ == State::WRITE_SENDING);
  auto it = writeSendingQueue_.erase(writeSendingQueue_.iterator_to(req));
  // If req marks the end of a write batch, swap req with a dummy RequestContext
  // that preserves the end-of-batch marking. This ensures that subsequent calls
  // to writeSuccess()/writeErr() operate on the correct requests.
  if (req.lastInWriteBatch_) {
    writeSendingQueue_.insert(
        it, RequestContext::createDummyEndOfBatchMarker(*this));
  }
}

RequestContext* RequestContextQueue::getRequestResponseContext(
    StreamId streamId) {
  const auto it = requestResponseContexts_.find(
      streamId,
      std::hash<StreamId>(),
      [](StreamId sid, const RequestContext& ctx) {
        return sid == ctx.streamId();
      });
  return it != requestResponseContexts_.end() ? &*it : nullptr;
}

void RequestContextQueue::growBuckets() {
  std::vector<RequestResponseSet::bucket_type> newBuckets(
      rrContextBuckets_.size() * 2);
  requestResponseContexts_.rehash(
      RequestResponseSet::bucket_traits(newBuckets.data(), newBuckets.size()));
  rrContextBuckets_.swap(newBuckets);
}

} // namespace apache::thrift::rocket
