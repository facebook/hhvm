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

#pragma once
#include <optional>
#include <string_view>
#include <type_traits>

#include <folly/Try.h>
#include <folly/compression/Compression.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/logging/ThriftStreamLog.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::detail {

class StreamInterceptorContext;

using ServerStreamFactoryFn = folly::Function<void(
    FirstResponsePayload&&,
    StreamClientCallback*,
    folly::EventBase*,
    TilePtr&&,
    std::shared_ptr<ContextStack>,
    std::shared_ptr<StreamInterceptorContext>,
    std::unique_ptr<ThriftStreamLog>,
    std::optional<CompressionConfig>)>;

struct ServerStreamFactory {
  ServerStreamFactory() = default;

  /* implicit */ ServerStreamFactory(std::nullptr_t) : fn_(nullptr) {}

  template <
      typename F,
      std::enable_if_t<
          std::is_invocable_v<
              F,
              FirstResponsePayload&&,
              StreamClientCallback*,
              folly::EventBase*,
              TilePtr&&,
              std::shared_ptr<ContextStack>,
              std::shared_ptr<StreamInterceptorContext>,
              std::unique_ptr<ThriftStreamLog>,
              std::optional<CompressionConfig>>,
          int> = 0>
  explicit ServerStreamFactory(F&& fn) : fn_(std::forward<F>(fn)) {}

  void setInteraction(TilePtr&& interaction) {
    interaction_ = std::move(interaction);
  }

  void setContextStack(ContextStack::UniquePtr contextStack) {
    contextStack_ = std::shared_ptr<ContextStack>(std::move(contextStack));
  }

  std::shared_ptr<ContextStack> getContextStack() { return contextStack_; }

  void setInterceptorContext(std::shared_ptr<StreamInterceptorContext> ctx) {
    interceptorContext_ = std::move(ctx);
  }

  std::shared_ptr<StreamInterceptorContext> getInterceptorContext() const {
    return interceptorContext_;
  }

  void setMethodName(std::string_view methodName) { methodName_ = methodName; }

  std::string_view getMethodName() const { return methodName_; }

  void setStreamLog(std::unique_ptr<ThriftStreamLog> log) {
    streamLog_ = std::move(log);
  }

  void setCompressionConfig(std::optional<CompressionConfig> config) {
    compressionConfig_ = std::move(config);
  }

  void operator()(
      FirstResponsePayload&& payload,
      StreamClientCallback* cb,
      folly::EventBase* eb) {
    fn_(std::move(payload),
        cb,
        eb,
        std::move(interaction_),
        std::move(contextStack_),
        std::move(interceptorContext_),
        std::move(streamLog_),
        std::move(compressionConfig_));
  }

  explicit operator bool() { return !!fn_; }

 private:
  ServerStreamFactoryFn fn_;
  TilePtr interaction_;
  std::shared_ptr<ContextStack> contextStack_;
  std::shared_ptr<StreamInterceptorContext> interceptorContext_;
  std::string_view methodName_;
  std::unique_ptr<ThriftStreamLog> streamLog_;
  std::optional<CompressionConfig> compressionConfig_;
};

template <typename T>
using ServerStreamFn = folly::Function<ServerStreamFactory(
    folly::Executor::KeepAlive<>,
    apache::thrift::detail::StreamElementEncoder<T>*)>;

// Holds pre-resolved compression state so we don't re-resolve per item.
struct StreamCompressionContext {
  CompressionAlgorithm algorithm;
  std::unique_ptr<folly::compression::Codec> codec;
  size_t sizeLimit;
};

// Resolves the compression config into a cached codec + algorithm.
// Returns nullopt if compression should not be applied (no config,
// NONE/CUSTOM algorithm, etc.).
std::optional<StreamCompressionContext> makeCompressionContext(
    const CompressionConfig& config);

// Compress a stream item on the CPU thread if it exceeds the size limit.
// On failure, leaves the payload uncompressed.
void compressStreamItem(
    StreamPayload& sp, const StreamCompressionContext& ctx, size_t payloadSize);

} // namespace apache::thrift::detail
