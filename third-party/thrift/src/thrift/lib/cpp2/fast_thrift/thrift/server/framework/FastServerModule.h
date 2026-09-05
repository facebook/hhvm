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

#include <string>
#include <utility>
#include <vector>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftConnectionExtension.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftExtensionPipelineHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/ThriftPipelineHandler.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * FastServerModule — a named, ordered bundle of thrift pipeline handlers.
 *
 * A module is a *value*, not a polymorphic base: it carries only data (a name
 * plus an ordered list of handler factories), so it needs no virtuals. Unlike
 * the legacy apache::thrift::ServerModule, it contributes only pipeline
 * handlers (no interceptors / event handlers).
 *
 * Distribute a reusable bundle as a free function returning a populated module:
 *
 *   FastServerModule makeLoggingModule(LogConfig cfg) {
 *     return FastServerModule("logging")
 *         .addThriftExtension<RequestLogObserver>()
 *         .addThriftExtension<TimingObserver>(std::move(cfg));
 *   }
 *
 * addThriftExtension is the recommended surface; addNativeThriftHandler is the
 * advanced, allowlist-gated path for handlers that need raw pipeline access.
 *
 * Hand it to FastThriftServer::addModule; its handlers are spliced into the
 * pipeline in call order relative to other addModule /
 * addNativeThriftPipelineHandlers calls, with intra-module order preserved.
 */
class FastServerModule {
 public:
  explicit FastServerModule(std::string name) : name_(std::move(name)) {}

  const std::string& name() const { return name_; }

  /**
   * Append a raw ("native") pipeline handler to this module, in order. This is
   * the advanced path: T owns message lifetime and the raw pipeline context, so
   * T must be on the Thrift-governed allowlist (see
   * NativeThriftHandlerAllowlist.h) or this fails to compile. Prefer
   * addThriftExtension unless raw access is genuinely required. `args` are
   * copied and used to construct a fresh T per connection. Returns *this for
   * chaining.
   *
   * T must satisfy the Inbound, Outbound, or Duplex handler concept over
   * server::ThriftPipelineHandlerContext.
   */
  template <typename T, typename... Args>
  FastServerModule& addNativeThriftHandler(Args... args) {
    return addFactory([&](channel_pipeline::HandlerId id) {
      return server::makeThriftPipelineHandlerFactory<T>(
          id, std::move(args)...);
    });
  }

  /**
   * Append a constrained extension to this module, in order. `H` must declare
   * an onRequest taking a request view (read-only) or mutator (read/write) —
   * see ThriftExtension.h — the recommended, safe way to hook the
   * request/response path. `args` are copied and used to construct a fresh H
   * per connection. Returns *this for chaining.
   *
   * H is wrapped in the framework's extension adapter, which owns message
   * lifetime and enforces the forwarding / rejection contract on H's behalf.
   * The factory is built directly rather than through addNativeThriftHandler,
   * because the adapter also needs the connection's shared extension state.
   * H is constrained by the adapter's own static_assert on the extension
   * callback contract rather than by the native-handler allowlist, so
   * extensions never need an allowlist entry.
   */
  template <typename H, typename... Args>
  FastServerModule& addThriftExtension(Args... args) {
    requiresConnectionContext_ |= ThriftConnectionExtensionHandler<H> ||
        ThriftBackpressureExtensionHandler<H>;
    controlsReads_ |= ThriftBackpressureExtensionHandler<H>;
    requiresHeaders_ |= UsesHeaders<H>;
    return addFactory([&](channel_pipeline::HandlerId id) {
      return server::makeThriftExtensionHandlerFactory<H>(
          id, std::move(args)...);
    });
  }

  /**
   * Whether any extension in this module hooks the connection lifecycle, and so
   * needs the server to build a per-connection context. Checked by
   * FastThriftServer::addModule against the server's enableRequestContext
   * setting: without it there is no ThriftConnContext to observe, and a
   * connection extension would silently see nothing.
   */
  bool requiresConnectionContext() const { return requiresConnectionContext_; }

  /**
   * Whether any extension in this module pauses and resumes reads on a
   * connection. Checked by FastThriftServer::addModule against
   * enableWriteBufferBackpressure, which resumes reads the moment its own
   * buffer drains: the two arbitrate nothing between them, so a write-buffer
   * drain would silently lift an extension's pause.
   */
  bool controlsReads() const { return controlsReads_; }

  /**
   * Whether any extension in this module declares kUsesHeaders. Checked by
   * FastThriftServer::addModule against enableRequestHeaders: headers are
   * reachable only through the per-request context, which the server populates
   * only under that setting, so without it the extension would read empty.
   */
  bool requiresHeaders() const { return requiresHeaders_; }

  /**
   * The module's handler factories, in call order. FastThriftServer::addModule
   * consumes these when splicing the module into every connection's pipeline.
   */
  const std::vector<server::ThriftPipelineHandlerFactory>& handlers() const& {
    return factories_;
  }
  std::vector<server::ThriftPipelineHandlerFactory>&& handlers() && {
    return std::move(factories_);
  }

 private:
  // Append the factory `makeFactory` builds for the next registration slot.
  //
  // The id is two-level, keyed on the module name (its namespace) and the
  // handler's within-module index, so distinct modules get non-overlapping id
  // streams. Empty module names are rejected at FastThriftServer::addModule, so
  // the empty namespace stays reserved for top-level (loose) handlers. Every
  // registration path derives its id here, which is what keeps the indices
  // dense and in registration order.
  template <typename MakeFactory>
  FastServerModule& addFactory(MakeFactory&& makeFactory) {
    auto id = server::deriveThriftPipelineHandlerId(name_, factories_.size());
    factories_.push_back(std::forward<MakeFactory>(makeFactory)(id));
    return *this;
  }

  std::string name_;
  std::vector<server::ThriftPipelineHandlerFactory> factories_;
  bool requiresConnectionContext_{false};
  bool controlsReads_{false};
  bool requiresHeaders_{false};
};

} // namespace apache::thrift::fast_thrift::thrift
