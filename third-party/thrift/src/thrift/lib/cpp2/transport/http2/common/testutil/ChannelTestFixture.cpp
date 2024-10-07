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

#include <thrift/lib/cpp2/transport/http2/common/testutil/ChannelTestFixture.h>

#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>

namespace apache::thrift {

using folly::EventBaseManager;
using proxygen::HTTPMessage;
using std::string;
using std::unordered_map;

ChannelTestFixture::ChannelTestFixture() {
  EventBaseManager::get()->setEventBase(eventBase_.get(), true);
  responseHandler_ = std::make_unique<FakeResponseHandler>(eventBase_.get());
}

void ChannelTestFixture::sendAndReceiveStream(
    ThriftProcessor* processor,
    const unordered_map<string, string>& inputHeaders,
    const string& inputPayload,
    string::size_type chunkSize,
    unordered_map<string, string>*& outputHeaders,
    IOBuf*& outputPayload,
    bool omitEnvelope) {
  auto channel = std::make_shared<SingleRpcChannel>(
      responseHandler_->getTransaction(), processor, worker_);
  string payload;
  if (omitEnvelope) {
    payload = inputPayload;
  } else {
    auto envelopeBuf = std::make_unique<IOBufQueue>();
    CompactProtocolWriter writer;
    writer.setOutput(envelopeBuf.get());
    writer.writeMessageBegin("dummy", MessageType::T_CALL, 0);
    string envelope = envelopeBuf->move()->to<std::string>();
    payload = envelope + inputPayload;
  }
  eventBase_->runInEventBaseThread([&]() {
    auto msg = std::make_unique<HTTPMessage>();
    auto& headers = msg->getHeaders();
    for (auto it = inputHeaders.begin(); it != inputHeaders.end(); ++it) {
      headers.rawSet(it->first, it->second);
    }
    channel->onH2StreamBegin(std::move(msg));
    const char* data = payload.data();
    string::size_type len = payload.length();
    string::size_type incr = (chunkSize == 0) ? len : chunkSize;
    for (string::size_type i = 0; i < len; i += incr) {
      auto iobuf = IOBuf::copyBuffer(data + i, std::min(incr, len - i));
      channel->onH2BodyFrame(std::move(iobuf));
    }
    channel->onH2StreamEnd();
  });
  eventBase_->loop();
  // The loop exits when FakeResponseHandler::sendEOM() is called.
  outputHeaders = responseHandler_->getHeaders();
  outputPayload = responseHandler_->getBodyBuf();
}

string ChannelTestFixture::toString(IOBuf* buf) {
  return buf->to<std::string>();
}

} // namespace apache::thrift
