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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

namespace apache::thrift::fast_thrift::channel_pipeline::detail {

ContextImpl::ContextImpl(
    PipelineImpl* pipeline,
    folly::EventBase* eventBase,
    void* allocator,
    size_t handlerIndex,
    HandlerId handlerId) noexcept
    : pipeline_(pipeline),
      eventBase_(eventBase),
      allocator_(allocator),
      handlerIndex_(handlerIndex),
      handlerId_(handlerId) {}

void ContextImpl::activate() noexcept {
  // onPipelineActive is propagated via PipelineImpl::activate() which
  // iterates through all handlers. No-op here as handlers don't chain
  // activation.
}

void ContextImpl::deactivate() noexcept {
  pipeline_->deactivateFromIndex(handlerIndex_);
}
BytesPtr ContextImpl::allocate(size_t size) noexcept {
  return pipeline_->allocate(size);
}

BytesPtr ContextImpl::copyBuffer(const void* data, size_t size) noexcept {
  return folly::IOBuf::copyBuffer(data, size);
}

void ContextImpl::close() noexcept {
  pipeline_->close();
}

void ContextImpl::awaitWriteReady() noexcept {
  auto* hook = pipeline_->handlerWriteReadyHook(handlerIndex_);
  if (hook && !hook->hook.is_linked()) {
    pipeline_->writeReadyList().push_back(*hook);
  }
}

void ContextImpl::cancelAwaitWriteReady() noexcept {
  auto* hook = pipeline_->handlerWriteReadyHook(handlerIndex_);
  if (hook && hook->hook.is_linked()) {
    hook->hook.unlink();
  }
}

bool ContextImpl::isAwaitingWriteReady() const noexcept {
  auto* hook = pipeline_->handlerWriteReadyHook(handlerIndex_);
  return hook && hook->hook.is_linked();
}

void ContextImpl::awaitReadReady() noexcept {
  auto* hook = pipeline_->handlerReadReadyHook(handlerIndex_);
  if (hook && !hook->hook.is_linked()) {
    pipeline_->readReadyList().push_back(*hook);
  }
}

void ContextImpl::cancelAwaitReadReady() noexcept {
  auto* hook = pipeline_->handlerReadReadyHook(handlerIndex_);
  if (hook && hook->hook.is_linked()) {
    hook->hook.unlink();
  }
}

bool ContextImpl::isAwaitingReadReady() const noexcept {
  auto* hook = pipeline_->handlerReadReadyHook(handlerIndex_);
  return hook && hook->hook.is_linked();
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::detail
