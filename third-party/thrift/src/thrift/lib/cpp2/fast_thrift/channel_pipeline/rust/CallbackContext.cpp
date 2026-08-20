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
#include <new>
#include <type_traits>
#include <utility>

#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustMessageAdapter.h>

namespace channel_pipeline_rust {
namespace {

using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl;

struct RustContextHandleToken final {
  using IsRelocatable = std::true_type;

  explicit RustContextHandleToken(ContextImpl& context) noexcept
      : context(&context), pipelineGuard(context.pipeline()) {}

  RustContextHandleToken(const RustContextHandleToken&) = delete;
  RustContextHandleToken& operator=(const RustContextHandleToken&) = delete;

  RustContextHandleToken(RustContextHandleToken&& other) noexcept
      : context(std::exchange(other.context, nullptr)),
        pipelineGuard(std::move(other.pipelineGuard)) {}

  RustContextHandleToken& operator=(RustContextHandleToken&&) = delete;

  ~RustContextHandleToken() {
    CHECK(context == nullptr || context->eventBase()->isInEventBaseThread());
  }

  ContextImpl* context;
  folly::DelayedDestruction::DestructorGuard pipelineGuard;
};

static_assert(sizeof(RustContextHandleToken) == 2 * sizeof(void*));
static_assert(alignof(RustContextHandleToken) == alignof(void*));
static_assert(folly::IsRelocatable<RustContextHandleToken>::value);
static_assert(!std::is_copy_constructible_v<RustContextHandleToken>);
static_assert(!std::is_copy_assignable_v<RustContextHandleToken>);
static_assert(std::is_nothrow_move_constructible_v<RustContextHandleToken>);
static_assert(!std::is_move_assignable_v<RustContextHandleToken>);

int32_t toInt(Result result) noexcept {
  return static_cast<int32_t>(result);
}

RustContextHandleToken consumeContextHandle(uint8_t* storage) noexcept {
  CHECK(storage != nullptr);
  CHECK(
      reinterpret_cast<uintptr_t>(storage) % alignof(RustContextHandleToken) ==
      0);
  auto* token = reinterpret_cast<RustContextHandleToken*>(storage);
  CHECK(token->context != nullptr);
  auto consumed = std::move(*token);
  token->~RustContextHandleToken();
  return consumed;
}

template <typename Fire>
void fireContextHandle(
    uint8_t* storage, BytesPtr message, Fire&& fire) noexcept {
  auto token = consumeContextHandle(storage);
  auto* eventBase = token.context->eventBase();
  if (eventBase->isInEventBaseThread()) {
    folly::RequestContextSaverScopeGuard requestContextGuard;
    auto boxed = RustMessageAdapter<BytesPtr>::tryBox(std::move(message));
    if (!boxed || token.context->pipeline()->isClosed()) {
      return;
    }
    fire(*token.context, std::move(*boxed));
    return;
  }

  auto continuation = [token = std::move(token),
                       message = std::move(message),
                       fire = std::forward<Fire>(fire)]() mutable noexcept {
    auto boxed = RustMessageAdapter<BytesPtr>::tryBox(std::move(message));
    if (!boxed || token.context->pipeline()->isClosed()) {
      return;
    }
    fire(*token.context, std::move(*boxed));
  };
  static_assert(
      sizeof(continuation) <= sizeof(folly::detail::function::Data),
      "ContextHandle continuation must fit folly::Function inline storage");
  eventBase->runInEventBaseThreadAlwaysEnqueue(std::move(continuation));
}

} // namespace

void CallbackContext::initContextHandle(uint8_t* storage) noexcept {
  CHECK(context_.eventBase()->isInEventBaseThread());
  CHECK(storage != nullptr);
  CHECK(
      reinterpret_cast<uintptr_t>(storage) % alignof(RustContextHandleToken) ==
      0);
  ::new (storage) RustContextHandleToken(context_);
}

folly::EventBase* CallbackContext::eventBase() const noexcept {
  return context_.eventBase();
}

bool isInEventBaseThread(folly::EventBase* eventBase) noexcept {
  DCHECK(eventBase != nullptr);
  return eventBase != nullptr && eventBase->isInEventBaseThread();
}

void enqueueInEventBase(
    folly::EventBase* eventBase,
    uintptr_t task,
    rust::Fn<void(uintptr_t)> call,
    rust::Fn<void(uintptr_t)> drop) noexcept {
  CHECK(eventBase != nullptr);
  struct Task final {
    uintptr_t value;
    rust::Fn<void(uintptr_t)> call;
    rust::Fn<void(uintptr_t)> drop;

    Task(
        uintptr_t value,
        rust::Fn<void(uintptr_t)> call,
        rust::Fn<void(uintptr_t)> drop)
        : value(value), call(std::move(call)), drop(std::move(drop)) {}
    Task(Task&& other) noexcept
        : value(std::exchange(other.value, 0)),
          call(std::move(other.call)),
          drop(std::move(other.drop)) {}
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    ~Task() {
      if (auto owned = std::exchange(value, 0)) {
        drop(owned);
      }
    }
    void operator()() {
      if (auto owned = std::exchange(value, 0)) {
        call(owned);
      }
    }
  };

  eventBase->runInEventBaseThreadAlwaysEnqueue(
      [owned =
           Task(task, std::move(call), std::move(drop))]() mutable noexcept {
        owned();
      });
}

void fireContextHandleRead(uint8_t* storage, BytesPtr message) noexcept {
  fireContextHandle(
      storage,
      std::move(message),
      [](ContextImpl& context, auto boxed) noexcept {
        (void)context.fireRead(std::move(boxed));
      });
}

void fireContextHandleWrite(uint8_t* storage, BytesPtr message) noexcept {
  fireContextHandle(
      storage,
      std::move(message),
      [](ContextImpl& context, auto boxed) noexcept {
        (void)context.fireWrite(std::move(boxed));
      });
}

void fireContextHandleException(
    uint8_t* storage, const uint8_t* messageData, size_t messageSize) noexcept {
  CHECK(messageData != nullptr || messageSize == 0);
  auto token = consumeContextHandle(storage);
  auto exception = folly::make_exception_wrapper<std::runtime_error>(
      std::string(reinterpret_cast<const char*>(messageData), messageSize));
  auto* eventBase = token.context->eventBase();
  if (eventBase->isInEventBaseThread()) {
    folly::RequestContextSaverScopeGuard requestContextGuard;
    if (!token.context->pipeline()->isClosed()) {
      token.context->fireException(std::move(exception));
    }
    return;
  }

  auto continuation = [token = std::move(token),
                       exception = std::move(exception)]() mutable noexcept {
    if (!token.context->pipeline()->isClosed()) {
      token.context->fireException(std::move(exception));
    }
  };
  static_assert(
      sizeof(continuation) <= sizeof(folly::detail::function::Data),
      "ContextHandle exception must fit folly::Function inline storage");
  eventBase->runInEventBaseThreadAlwaysEnqueue(std::move(continuation));
}

void destroyContextHandle(uint8_t* storage) noexcept {
  auto token = consumeContextHandle(storage);
  auto* eventBase = token.context->eventBase();
  if (eventBase->isInEventBaseThread()) {
    return;
  }

  eventBase->runInEventBaseThread(
      [token = std::move(token)]() mutable noexcept {});
}

int32_t CallbackContext::fireRead(
    std::unique_ptr<folly::IOBuf> message) noexcept {
  if (!message_ || !message || forwarded_) {
    return toInt(Result::Error);
  }
  forwarded_ = true;
  // The Rust handler took the value out of the box, leaving it empty; restore
  // the returned message into that same box to keep the same-box fast path.
  if (!RustMessageAdapter<BytesPtr>::tryRestore(
          *message_, std::move(message))) {
    return toInt(Result::Error);
  }
  return toInt(context_.fireRead(std::move(*message_)));
}

int32_t CallbackContext::fireWrite(
    std::unique_ptr<folly::IOBuf> message) noexcept {
  if (!message_ || !message || forwarded_) {
    return toInt(Result::Error);
  }
  forwarded_ = true;
  if (!RustMessageAdapter<BytesPtr>::tryRestore(
          *message_, std::move(message))) {
    return toInt(Result::Error);
  }
  return toInt(context_.fireWrite(std::move(*message_)));
}

int32_t CallbackContext::forwardRead() noexcept {
  if (!message_ || forwarded_ || message_->empty()) {
    return toInt(Result::Error);
  }
  forwarded_ = true;
  return toInt(context_.fireRead(std::move(*message_)));
}

int32_t CallbackContext::forwardWrite() noexcept {
  if (!message_ || forwarded_ || message_->empty()) {
    return toInt(Result::Error);
  }
  forwarded_ = true;
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
      return context_.allocate(0);
    }
    if (!data) {
      return nullptr;
    }
    return context_.copyBuffer(data, size);
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
      return context_.allocate(0);
    }
    auto out = context_.allocate(total);
    if (!out) {
      return nullptr;
    }
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
