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

#include <thrift/lib/cpp2/transport/rocket/client/RocketStreamServerCallback.h>

#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>

namespace apache::thrift::rocket {

bool RocketStreamServerCallback::onStreamRequestN(uint64_t tokens) {
  return client_.sendRequestN(streamId_, tokens);
}

void RocketStreamServerCallback::onStreamCancel() {
  client_.cancelStream(streamId_);
}

bool RocketStreamServerCallback::onSinkHeaders(HeadersPayload&& payload) {
  return client_.sendHeadersPush(streamId_, std::move(payload));
}

bool RocketStreamServerCallback::onInitialPayload(
    FirstResponsePayload&& payload, folly::EventBase* evb) {
  return clientCallback_->onFirstResponse(std::move(payload), evb, this);
}

void RocketStreamServerCallback::onInitialError(folly::exception_wrapper ew) {
  clientCallback_->onFirstResponseError(std::move(ew));
  client_.cancelStream(streamId_);
}

bool RocketStreamServerCallback::onStreamPayload(StreamPayload&& payload) {
  return clientCallback_->onStreamNext(std::move(payload));
}

void RocketStreamServerCallback::onStreamFinalPayload(StreamPayload&& payload) {
  if (onStreamPayload(std::move(payload))) {
    onStreamComplete();
  }
}

void RocketStreamServerCallback::onStreamComplete() {
  clientCallback_->onStreamComplete();
}

void RocketStreamServerCallback::onStreamError(folly::exception_wrapper ew) {
  ew.handle(
      [&](RocketException& ex) {
        if (ex.hasErrorData()) {
          clientCallback_->onStreamError(
              thrift::detail::EncodedStreamRpcError(ex.moveErrorData()));
        } else {
          clientCallback_->onStreamError(std::move(ew));
        }
      },
      [&](...) {
        clientCallback_->onStreamError(std::move(ew));
        return false;
      });
}

void RocketStreamServerCallback::onStreamHeaders(HeadersPayload&& payload) {
  std::ignore = clientCallback_->onStreamHeaders(std::move(payload));
}

} // namespace apache::thrift::rocket
