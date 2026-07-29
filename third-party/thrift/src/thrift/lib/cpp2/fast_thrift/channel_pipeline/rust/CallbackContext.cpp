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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/CallbackContext.h>

#include <cstring>

namespace channel_pipeline_rust {
namespace {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::Result;

int32_t toInt(Result result) noexcept {
  return static_cast<int32_t>(result);
}

} // namespace

int32_t CallbackContext::fireRead(
    std::unique_ptr<folly::IOBuf> message) noexcept {
  if (!message_ || !message || forwarded_) {
    return toInt(Result::Error);
  }
  forwarded_ = true;
  message_->get<BytesPtr>() = std::move(message);
  return toInt(context_.fireRead(std::move(*message_)));
}

int32_t CallbackContext::fireWrite(
    std::unique_ptr<folly::IOBuf> message) noexcept {
  if (!message_ || !message || forwarded_) {
    return toInt(Result::Error);
  }
  forwarded_ = true;
  message_->get<BytesPtr>() = std::move(message);
  return toInt(context_.fireWrite(std::move(*message_)));
}

void CallbackContext::awaitReadReady() noexcept {
  context_.awaitReadReady();
}

void CallbackContext::cancelReadReady() noexcept {
  context_.cancelAwaitReadReady();
}

bool CallbackContext::isAwaitingReadReady() const noexcept {
  return context_.isAwaitingReadReady();
}

void CallbackContext::awaitWriteReady() noexcept {
  context_.awaitWriteReady();
}

void CallbackContext::cancelWriteReady() noexcept {
  context_.cancelAwaitWriteReady();
}

bool CallbackContext::isAwaitingWriteReady() const noexcept {
  return context_.isAwaitingWriteReady();
}

uint64_t CallbackContext::handlerId() const noexcept {
  try {
    return static_cast<uint64_t>(context_.handlerId());
  } catch (...) {
    return 0;
  }
}

std::unique_ptr<folly::IOBuf> CallbackContext::allocate(size_t size) noexcept {
  try {
    return context_.allocate(size);
  } catch (...) {
    return nullptr;
  }
}

std::unique_ptr<folly::IOBuf> CallbackContext::copyBuffer(
    const uint8_t* data, size_t size) noexcept {
  try {
    if (size == 0) {
      return folly::IOBuf::create(0);
    }
    if (!data) {
      return nullptr;
    }
    return folly::IOBuf::copyBuffer(data, size);
  } catch (...) {
    return nullptr;
  }
}

std::unique_ptr<folly::IOBuf> CallbackContext::cloneBufferChain(
    const folly::IOBuf& buffer) noexcept {
  try {
    return buffer.clone();
  } catch (...) {
    return nullptr;
  }
}

std::unique_ptr<folly::IOBuf> CallbackContext::cloneOne(
    const folly::IOBuf& buffer) noexcept {
  try {
    return buffer.cloneOne();
  } catch (...) {
    return nullptr;
  }
}

std::unique_ptr<folly::IOBuf> CallbackContext::coalescedCopy(
    const folly::IOBuf& buffer) noexcept {
  try {
    const size_t total = buffer.computeChainDataLength();
    if (total == 0) {
      return folly::IOBuf::create(0);
    }
    auto out = folly::IOBuf::create(total);
    out->append(total);
    size_t offset = 0;
    for (auto* curr = &buffer;; curr = curr->next()) {
      const size_t len = curr->length();
      if (len > 0) {
        std::memcpy(out->writableData() + offset, curr->data(), len);
        offset += len;
      }
      if (curr->next() == &buffer) {
        break;
      }
    }
    return out;
  } catch (...) {
    return nullptr;
  }
}

void CallbackContext::close() noexcept {
  try {
    context_.close();
  } catch (...) {
  }
}

bool CallbackContext::isClosed() const noexcept {
  try {
    auto* pipeline = context_.pipeline();
    if (!pipeline) {
      return true;
    }
    return pipeline->isClosed();
  } catch (...) {
    return true;
  }
}

} // namespace channel_pipeline_rust
