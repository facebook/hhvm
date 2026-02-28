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

#include <thrift/lib/cpp2/transport/http2/common/testutil/FakeProcessors.h>

#include <glog/logging.h>

namespace apache::thrift {

void EchoProcessor::onThriftRequest(
    RequestRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> payload,
    std::shared_ptr<ThriftChannelIf> channel,
    std::unique_ptr<Cpp2ConnContext> /* connContext */) noexcept {
  evb_->runInEventBaseThread([this,
                              evbMetadata = std::move(metadata),
                              evbPayload = std::move(payload),
                              evbChannel = std::move(channel)]() mutable {
    onThriftRequestHelper(
        std::move(evbMetadata), std::move(evbPayload), std::move(evbChannel));
  });
}

void EchoProcessor::onThriftRequestHelper(
    RequestRpcMetadata&& requestMetadata,
    std::unique_ptr<folly::IOBuf> payload,
    std::shared_ptr<ThriftChannelIf> channel) noexcept {
  CHECK(payload);
  CHECK(channel);
  ResponseRpcMetadata responseMetadata;
  if (auto otherMetadata = requestMetadata.otherMetadata()) {
    responseMetadata.otherMetadata() = std::move(*otherMetadata);
  }
  (*responseMetadata.otherMetadata())[key_] = value_;
  auto iobuf = IOBuf::copyBuffer(trailer_);
  payload->prependChain(std::move(iobuf));
  channel->sendThriftResponse(std::move(responseMetadata), std::move(payload));
}

} // namespace apache::thrift
