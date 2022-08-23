/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxyRequestContext.h"

#include <memory>

#include "mcrouter/CarbonRouterClientBase.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/config.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

ProxyRequestContext::ProxyRequestContext(
    ProxyBase& pr,
    ProxyRequestPriority priority__,
    const void* ptr)
    : ptr_(ptr), proxyBase_(pr), priority_(priority__) {
  proxyBase_.stats().incrementSafe(proxy_request_num_outstanding_stat);
}

ProxyRequestContext::~ProxyRequestContext() {
  if (recording_) {
    recordingState_.~unique_ptr<RecordingState>();
    return;
  }

  assert(replied_);

  if (processing_) {
    --proxyBase_.numRequestsProcessing_;
    proxyBase_.stats().decrement(proxy_reqs_processing_stat);
    proxyBase_.pump();
  }

  if (requester_) {
    if (requester_->maxOutstanding() != 0) {
      counting_sem_post(requester_->outstandingReqsSem(), 1);
    }
  }

  proxyBase_.stats().decrementSafe(proxy_request_num_outstanding_stat);
}

uint64_t ProxyRequestContext::senderId() const {
  uint64_t id = 0;
  if (requester_) {
    id = requester_->clientId();
  } else {
    id = senderIdForTest_;
  }

  return id;
}

void ProxyRequestContext::setSenderIdForTest(uint64_t id) {
  senderIdForTest_ = id;
}

ProxyRequestContext::ProxyRequestContext(
    RecordingT,
    ProxyBase& pr,
    ClientCallback clientCallback,
    ShardSplitCallback shardSplitCallback,
    BucketIdCallback bucketIdCallback)
    /* pr.nextRequestId() is not threadsafe */
    : proxyBase_(pr), recording_(true) {
  new (&recordingState_)
      std::unique_ptr<RecordingState>(std::make_unique<RecordingState>());
  recordingState_->clientCallback = std::move(clientCallback);
  recordingState_->shardSplitCallback = std::move(shardSplitCallback);
  recordingState_->bucketIdCallback = std::move(bucketIdCallback);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
