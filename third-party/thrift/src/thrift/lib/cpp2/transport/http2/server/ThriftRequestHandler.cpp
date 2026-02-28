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

#include <thrift/lib/cpp2/transport/http2/server/ThriftRequestHandler.h>

#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>

namespace apache::thrift {

using proxygen::HTTPMessage;
using proxygen::ProxygenError;
using proxygen::UpgradeProtocol;

ThriftRequestHandler::ThriftRequestHandler(
    ThriftProcessor* processor, std::shared_ptr<Cpp2Worker> worker)
    : processor_(processor), worker_(std::move(worker)) {}

ThriftRequestHandler::~ThriftRequestHandler() {}

void ThriftRequestHandler::onHeadersComplete(
    std::unique_ptr<HTTPMessage> headers) noexcept {
  channel_ = std::make_shared<SingleRpcChannel>(txn_, processor_, worker_);
  channel_->onH2StreamBegin(std::move(headers));
}

void ThriftRequestHandler::onBody(std::unique_ptr<IOBuf> body) noexcept {
  channel_->onH2BodyFrame(std::move(body));
}

void ThriftRequestHandler::onEOM() noexcept {
  channel_->onH2StreamEnd();
}

void ThriftRequestHandler::onUpgrade(UpgradeProtocol /*prot*/) noexcept {}

void ThriftRequestHandler::detachTransaction() noexcept {
  if (channel_) {
    channel_->onH2StreamClosed(ProxygenError::kErrorNone, "");
  }
  delete this;
}

void ThriftRequestHandler::onError(
    const proxygen::HTTPException& error) noexcept {
  if (error.getProxygenError() == proxygen::kErrorTimeout) {
    deliverChannelError(proxygen::kErrorTimeout);

    if (!txn_->canSendHeaders()) {
      txn_->sendAbort();
    } else {
      HTTPMessage resp;
      resp.setStatusCode(408 /* client timeout */);
      txn_->sendHeadersWithOptionalEOM(resp, true);
    }
  } else if (
      error.getDirection() == proxygen::HTTPException::Direction::INGRESS) {
    deliverChannelError(proxygen::kErrorRead);

    if (!txn_->canSendHeaders()) {
      txn_->sendAbort();
    } else {
      HTTPMessage resp;
      resp.setStatusCode(400 /* bad request */);
      txn_->sendHeadersWithOptionalEOM(resp, true);
    }

  } else {
    deliverChannelError(
        error.hasProxygenError() ? error.getProxygenError()
                                 : proxygen::kErrorWrite);
  }
}

void ThriftRequestHandler::deliverChannelError(proxygen::ProxygenError error) {
  if (channel_) {
    channel_->onH2StreamClosed(error, "");
    channel_ = nullptr;
  }
}

} // namespace apache::thrift
