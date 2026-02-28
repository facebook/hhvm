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

#include <thrift/lib/cpp2/transport/http2/client/ThriftTransactionHandler.h>

#include <glog/logging.h>

#include <proxygen/lib/http/ProxygenErrorEnum.h>

namespace apache::thrift {

using proxygen::HTTPException;
using proxygen::HTTPMessage;
using proxygen::HTTPTransaction;
using proxygen::ProxygenError;

ThriftTransactionHandler::~ThriftTransactionHandler() {
  if (txn_) {
    txn_->setHandler(nullptr);
    txn_->setTransportCallback(nullptr);
  }
}

void ThriftTransactionHandler::setChannel(std::shared_ptr<H2Channel> channel) {
  channel_ = channel;
}

void ThriftTransactionHandler::setTransaction(HTTPTransaction* txn) noexcept {
  txn_ = txn;
  txn_->setTransportCallback(this);
}

void ThriftTransactionHandler::detachTransaction() noexcept {
  DCHECK(channel_);
  channel_->onH2StreamClosed(ProxygenError::kErrorNone, "");
  delete this;
}

void ThriftTransactionHandler::onHeadersComplete(
    std::unique_ptr<HTTPMessage> msg) noexcept {
  DCHECK(channel_);
  channel_->onH2StreamBegin(std::move(msg));
}

void ThriftTransactionHandler::onBody(
    std::unique_ptr<folly::IOBuf> body) noexcept {
  DCHECK(channel_);
  channel_->onH2BodyFrame(std::move(body));
}

void ThriftTransactionHandler::onEOM() noexcept {
  DCHECK(channel_);
  channel_->onH2StreamEnd();
}

void ThriftTransactionHandler::onError(const HTTPException& error) noexcept {
  DCHECK(channel_);
  channel_->onH2StreamClosed(error.getProxygenError(), error.describe());
}

void ThriftTransactionHandler::lastByteFlushed() noexcept {
  DCHECK(channel_);
  channel_->onMessageFlushed();
}

} // namespace apache::thrift
