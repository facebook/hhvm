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

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/HandlerNode.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/NativeThriftHandlerAllowlist.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * Context type seen by handlers registered into the thrift-layer pipeline. This
 * is the concrete pipeline context; a registered handler's onRead / onWrite /
 * lifecycle methods take a `ThriftPipelineHandlerContext&`.
 */
// NOLINTNEXTLINE(facebook-hte-DetailCall)
using ThriftPipelineHandlerContext = channel_pipeline::detail::ContextImpl;

/**
 * A per-connection factory that produces a fresh, type-erased handler node for
 * the thrift pipeline. The factory is invoked once per accepted connection when
 * that connection's thrift pipeline is built, so each connection gets its own
 * handler instance (handlers may hold per-connection state).
 */
using ThriftPipelineHandlerFactory =
    std::function<channel_pipeline::detail::HandlerNode()>;

/**
 * Derive a pipeline handler id from a namespace (e.g. a module name, or empty
 * for top-level framework handlers) and a within-namespace index.
 *
 * The id is the FNV-1a hash of the composed tag "<ns>/<index>" — the same
 * hash-of-a-tag-string scheme HANDLER_TAG uses, so these ids live in the same
 * id space as the built-in handler tags. Composing before hashing (rather than
 * `hash(ns) + index`) makes it a genuine two-level tag: distinct
 * (namespace, index) pairs are distinct strings and hash independently, with no
 * arithmetic-adjacency collisions. Callers that reserve disjoint namespaces
 * (one per module; empty for top-level) get non-overlapping id streams. Ids key
 * PipelineImpl::handlerMap_, so this keeps context() lookups unambiguous;
 * dispatch itself is positional and unaffected either way.
 */
inline channel_pipeline::HandlerId deriveThriftPipelineHandlerId(
    std::string_view ns, std::size_t index) {
  std::string tag;
  tag.reserve(ns.size() + 21); // '/' + up to 20 digits
  tag.append(ns);
  tag.push_back('/');
  tag.append(std::to_string(index));
  return channel_pipeline::fnv1a_hash(tag);
}

/**
 * Build a ThriftPipelineHandlerFactory for handler type T.
 *
 * T's concrete type and constructor arguments are captured here, where T is in
 * scope; the returned factory erases T and constructs a fresh T per connection
 * from copies of `args`. The node is created with ThriftServerEventType so T
 * may subscribe to server events (e.g. connection drain/close) exactly like the
 * built-in thrift pipeline handlers.
 *
 * @param id A pipeline-unique handler id (e.g. from a HANDLER_TAG or a
 *           registration-order-derived id). Used for context lookup and
 *           diagnostics.
 * @param args Constructor arguments, copied into every per-connection instance.
 */
template <typename T, typename... Args>
ThriftPipelineHandlerFactory makeThriftPipelineHandlerFactory(
    channel_pipeline::HandlerId id, Args... args) {
  static_assert(
      channel_pipeline::InboundHandler<T, ThriftPipelineHandlerContext> ||
          channel_pipeline::OutboundHandler<T, ThriftPipelineHandlerContext> ||
          channel_pipeline::DuplexHandler<T, ThriftPipelineHandlerContext>,
      "makeThriftPipelineHandlerFactory<T>: T must satisfy the Inbound, "
      "Outbound, or Duplex handler concept over ThriftPipelineHandlerContext");
  static_assert(
      kIsAllowedNativeThriftHandler<T>,
      "makeThriftPipelineHandlerFactory<T>: T is not on the native thrift "
      "pipeline handler allowlist. Native handlers are gated on an as-needed "
      "basis in framework/NativeThriftHandlerAllowlist.h (Thrift-owned). Prefer "
      "the constrained observer/modifier extension API "
      "(FastServerModule::addThriftExtension) for user handlers.");
  return [id, args...]() {
    return channel_pipeline::detail::makeHandlerNode<T, ThriftServerEventType>(
        id, std::make_unique<T>(args...));
  };
}

} // namespace apache::thrift::fast_thrift::thrift::server
