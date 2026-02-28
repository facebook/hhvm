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

#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>

#include <glog/logging.h>

#include <folly/MapUtil.h>
#include <folly/io/async/Request.h>

#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

namespace apache::thrift {
namespace {
void processExceptionHeaders(ResponseRpcMetadata& metadata) {
  auto otherMetadataRef = metadata.otherMetadata();
  if (!otherMetadataRef) {
    return;
  }
  auto& otherMetadata = *otherMetadataRef;

  if (auto anyExceptionPtr =
          folly::get_ptr(otherMetadata, "servicerouter:sr_internal_error")) {
    otherMetadata.insert(
        {std::string(detail::kHeaderAnyex), std::move(*anyExceptionPtr)});
    otherMetadata.insert(
        {std::string(detail::kHeaderAnyexType),
         "facebook.com/servicerouter/ServiceRouterError"});
    otherMetadata.erase("servicerouter:sr_internal_error");
  }

  if (auto proxiedAnyExceptionPtr =
          folly::get_ptr(otherMetadata, "servicerouter:sr_error")) {
    otherMetadata.insert(
        {std::string(detail::kHeaderProxiedAnyex),
         std::move(*proxiedAnyExceptionPtr)});
    otherMetadata.insert(
        {std::string(detail::kHeaderProxiedAnyexType),
         "facebook.com/servicerouter/ServiceRouterError"});
    otherMetadata.erase("servicerouter:sr_error");
  }
}
} // namespace

using namespace apache::thrift::transport;
using folly::EventBase;
using folly::exception_wrapper;
using folly::IOBuf;

const std::chrono::milliseconds ThriftClientCallback::kDefaultTimeout =
    std::chrono::milliseconds(500);

ThriftClientCallback::ThriftClientCallback(
    EventBase* evb,
    bool oneWay,
    RequestClientCallback::Ptr cb,
    std::chrono::milliseconds timeout)
    : evb_(evb),
      oneWay_(oneWay),
      cb_(std::move(cb)),
      active_(cb_),
      timeout_(timeout),
      timeBeginSend_(clock::now()) {}

ThriftClientCallback::~ThriftClientCallback() {
  active_ = false;
  cancelTimeout();
}

void ThriftClientCallback::onThriftRequestSent() {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  if (active_) {
    timeEndSend_ = clock::now();

    if (oneWay_) {
      cb_.release()->onResponse({});
    }

    if (timeout_.count() > 0) {
      evb_->timer().scheduleTimeout(this, timeout_);
    }
  }
}

void ThriftClientCallback::onThriftResponse(
    ResponseRpcMetadata&& metadata, std::unique_ptr<IOBuf> payload) noexcept {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  cancelTimeout();
  if (active_) {
    RpcTransportStats stats;
    stats.requestWriteLatency = timeEndSend_ - timeBeginSend_;
    stats.responseRoundTripLatency = clock::now() - timeEndSend_;

    active_ = false;
    auto tHeader = std::make_unique<transport::THeader>();
    tHeader->setClientType(THRIFT_HTTP2_CLIENT_TYPE);
    processExceptionHeaders(metadata);
    apache::thrift::detail::fillTHeaderFromResponseRpcMetadata(
        metadata, *tHeader);
    cb_.release()->onResponse(ClientReceiveState(
        -1, std::move(payload), std::move(tHeader), nullptr, stats));
  }
}

void ThriftClientCallback::onError(exception_wrapper ex) noexcept {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  cancelTimeout();
  if (active_) {
    active_ = false;
    cb_.release()->onResponseError(std::move(ex));
  }
}

EventBase* ThriftClientCallback::getEventBase() const {
  return evb_;
}

void ThriftClientCallback::timeoutExpired() noexcept {
  if (active_) {
    active_ = false;
    cb_.release()->onResponseError(
        folly::make_exception_wrapper<TTransportException>(
            apache::thrift::transport::TTransportException::TIMED_OUT));
    if (auto onTimedout = std::move(onTimedout_)) {
      onTimedout();
    }
  }
}

void ThriftClientCallback::callbackCanceled() noexcept {
  // nothing!
}

void ThriftClientCallback::setTimedOut(folly::Function<void()> onTimedout) {
  onTimedout_ = std::move(onTimedout);
}

} // namespace apache::thrift
