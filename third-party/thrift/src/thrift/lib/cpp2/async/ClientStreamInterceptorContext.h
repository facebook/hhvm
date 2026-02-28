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

#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp2/async/ClientInterceptorBase.h>
#include <thrift/lib/cpp2/async/ClientInterceptorStorage.h>
#include <thrift/lib/cpp2/util/TypeErasedRef.h>

#include <memory>
#include <string>
#include <vector>

namespace apache::thrift {

class ContextStack;

/**
 * Manages interceptor state for client streaming RPCs.
 *
 * This context is created when a stream begins and destroyed when it ends.
 * It maintains request state storage (moved from ContextStack) for each
 * registered interceptor and invokes the appropriate lifecycle hooks as
 * payloads flow through. RequestState persists through the entire stream
 * lifetime, mirroring the server-side design.
 *
 */
class ClientStreamInterceptorContext {
 public:
  using InterceptorList = std::vector<std::shared_ptr<ClientInterceptorBase>>;

  // Constructor that takes ownership of interceptors and request storages.
  // Use this when the source (e.g., ContextStack) will be destroyed before
  // stream consumption completes.
  ClientStreamInterceptorContext(
      std::shared_ptr<InterceptorList> interceptors,
      std::vector<detail::ClientInterceptorOnRequestStorage> requestStorages);

  ~ClientStreamInterceptorContext();

  // Non-copyable, non-assignable, but movable
  ClientStreamInterceptorContext(const ClientStreamInterceptorContext&) =
      delete;
  ClientStreamInterceptorContext& operator=(
      const ClientStreamInterceptorContext&) = delete;
  ClientStreamInterceptorContext(
      ClientStreamInterceptorContext&& other) noexcept;
  ClientStreamInterceptorContext& operator=(ClientStreamInterceptorContext&&) =
      delete;

  bool hasEnded() const { return ended_; }
  std::size_t payloadCount() const { return payloadIndex_; }

  // Low-level lifecycle methods - prefer passing context to
  // toAsyncGeneratorImpl() for typical usage.
  // These are public for testing and advanced use cases.
  void onStreamBegin();

  template <typename T>
  void onStreamPayload(const T& payload) {
    onStreamPayloadImpl(util::TypeErasedRef::of<T>(payload));
  }

  void onStreamEnd(
      details::STREAM_ENDING_TYPES reason,
      const folly::exception_wrapper& error = {});

  /**
   * Factory: create from a ContextStack, extracting (moving) the necessary
   * data so the context can outlive the ContextStack.
   *
   * Request storages are moved from the ContextStack into the stream context,
   * allowing interceptors to access request state throughout the stream
   * lifetime.
   *
   * @param ctx The ContextStack containing interceptors and request storages
   * @return A shared_ptr to the context, or nullptr if no interceptors
   */
  static std::shared_ptr<ClientStreamInterceptorContext> fromContextStack(
      ContextStack& ctx);

 private:
  void onStreamPayloadImpl(util::TypeErasedRef payload);

  const InterceptorList& getInterceptors() const;
  detail::ClientInterceptorOnRequestStorage* getRequestStorage(
      std::size_t index);

  std::shared_ptr<InterceptorList> ownedInterceptors_;
  std::vector<detail::ClientInterceptorOnRequestStorage> ownedRequestStorages_;

  std::size_t payloadIndex_ = 0;
  bool ended_ = false;
};

// Free function wrapper for use in generated code.
inline std::shared_ptr<ClientStreamInterceptorContext>
makeClientStreamInterceptorContextFromContextStack(ContextStack& ctx) {
  return ClientStreamInterceptorContext::fromContextStack(ctx);
}

} // namespace apache::thrift
