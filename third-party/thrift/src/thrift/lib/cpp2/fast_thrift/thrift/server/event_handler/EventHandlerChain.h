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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include <glog/logging.h>

#include <folly/CPortability.h>
#include <folly/tracing/StaticTracepoint.h>

#include <thrift/lib/cpp/TProcessorEventHandler.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * Drives a fixed list of `TProcessorEventHandler`s across one request.
 *
 * The list is fixed for the server's life, so a chain resolves it once and
 * thereafter walks a flat array of handler/context pairs. Only the context
 * half is per-request: `bind` takes one from every handler and `unbind`
 * returns it, which is the pairing `getServiceContext` / `freeContext`
 * define. `unbind` leaves the chain as `bind` found it, so one chain can
 * serve request after request, for any method, without rebuilding.
 *
 * Only the callbacks the bridge can honour are driven. Streams, sinks,
 * interactions and the serialized-message callbacks have no fast_thrift
 * equivalent, and client interceptors are not a server concern.
 */
class EventHandlerChain {
 public:
  using HandlerList =
      std::vector<std::shared_ptr<apache::thrift::TProcessorEventHandler>>;

  // `serviceName` and the handlers must outlive the chain.
  EventHandlerChain(const HandlerList& handlers, std::string_view serviceName)
      : serviceName_(serviceName) {
    callees_.reserve(handlers.size());
    for (const auto& handler : handlers) {
      CHECK(handler != nullptr);
      callees_.push_back(Callee{handler.get(), nullptr});
    }
  }

  EventHandlerChain(const EventHandlerChain&) = delete;
  EventHandlerChain& operator=(const EventHandlerChain&) = delete;
  EventHandlerChain(EventHandlerChain&&) = delete;
  EventHandlerChain& operator=(EventHandlerChain&&) = delete;

  ~EventHandlerChain() { unbind(); }

  bool empty() const noexcept { return callees_.empty(); }

  /**
   * Takes each handler's context for one request. `method` is the
   * "{service}.{method}" form handlers are keyed on, and must outlive the
   * request.
   *
   * A handler that throws from here leaves the chain bound, so `unbind`
   * returns the contexts the handlers ahead of it already gave up. Those
   * behind it are freed with the null context a handler that supplies none
   * would have left.
   */
  void bind(
      apache::thrift::Cpp2RequestContext& requestContext,
      std::string_view method) {
    DCHECK(!bound_);
    method_ = method;
    bound_ = true;
    const auto serviceName = serviceName_;
    forEachCallee([&](Callee& callee) {
      callee.context = callee.handler->getServiceContext(
          serviceName, method, &requestContext);
    });
  }

  /** Returns the contexts `bind` took. Idempotent. */
  void unbind() noexcept {
    if (!bound_) {
      return;
    }
    bound_ = false;
    const auto method = method_;
    forEachCallee([&](Callee& callee) {
      callee.handler->freeContext(callee.context, method);
      callee.context = nullptr;
    });
    method_ = {};
  }

  void preRead() {
    DCHECK(bound_);
    const auto method = method_;
    FOLLY_SDT(thrift, thrift_context_stack_pre_read, serviceName_, method);
    forEachCallee([&](Callee& callee) {
      callee.handler->preRead(callee.context, method);
    });
  }

  void postRead(
      apache::thrift::transport::THeader& header, std::uint32_t bytes) {
    DCHECK(bound_);
    const auto method = method_;
    FOLLY_SDT(
        thrift, thrift_context_stack_post_read, serviceName_, method, bytes);
    forEachCallee([&](Callee& callee) {
      callee.handler->postRead(callee.context, method, &header, bytes);
    });
  }

  void preWrite() {
    DCHECK(bound_);
    const auto method = method_;
    FOLLY_SDT(thrift, thrift_context_stack_pre_write, serviceName_, method);
    forEachCallee([&](Callee& callee) {
      callee.handler->preWrite(callee.context, method);
    });
  }

  void postWrite(std::uint32_t bytes) {
    DCHECK(bound_);
    const auto method = method_;
    FOLLY_SDT(
        thrift, thrift_context_stack_post_write, serviceName_, method, bytes);
    forEachCallee([&](Callee& callee) {
      callee.handler->postWrite(callee.context, method, bytes);
    });
  }

 private:
  struct Callee {
    apache::thrift::TProcessorEventHandler* handler;
    void* context;
  };

  // The bounds are read into locals before the loop: a handler call is opaque,
  // so leaving them as members costs a reload of each on every iteration.
  template <typename F>
  FOLLY_ALWAYS_INLINE void forEachCallee(F&& fn) {
    auto* const callees = callees_.data();
    const auto count = callees_.size();
    for (std::size_t i = 0; i < count; ++i) {
      fn(callees[i]);
    }
  }

  const std::string_view serviceName_;
  std::string_view method_;
  std::vector<Callee> callees_;
  bool bound_{false};
};

} // namespace apache::thrift::fast_thrift::thrift::server
