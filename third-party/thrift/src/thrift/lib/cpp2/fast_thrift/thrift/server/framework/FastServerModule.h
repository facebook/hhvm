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
    // Two-level id keyed on the module name (its namespace) and the handler's
    // within-module index — distinct modules get non-overlapping id streams.
    // Empty module names are rejected at FastThriftServer::addModule, so the
    // empty namespace stays reserved for top-level (loose) handlers.
    auto id = server::deriveThriftPipelineHandlerId(name_, factories_.size());
    factories_.push_back(
        server::makeThriftPipelineHandlerFactory<T>(id, std::move(args)...));
    return *this;
  }

  /**
   * Append a constrained extension to this module, in order. `H` must declare
   * an onRequest taking a request view (read-only) or mutator (read/write) —
   * see ThriftExtension.h — the recommended, safe way to hook the
   * request/response path. `args` are copied and used to construct a fresh H
   * per connection. Returns *this for chaining.
   *
   * Sugar over addNativeThriftHandler that wraps H in the framework's extension
   * adapter, which owns message lifetime and enforces the forwarding /
   * rejection contract on H's behalf. The adapter is always allowlisted, so
   * extensions never need an allowlist entry.
   */
  template <typename H, typename... Args>
  FastServerModule& addThriftExtension(Args... args) {
    return addNativeThriftHandler<server::ThriftExtensionPipelineHandler<H>>(
        std::move(args)...);
  }

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
  std::string name_;
  std::vector<server::ThriftPipelineHandlerFactory> factories_;
};

} // namespace apache::thrift::fast_thrift::thrift
