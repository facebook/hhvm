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

#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/intrusive_ptr.hpp>

#include <glog/logging.h>

#include <fmt/core.h>
#include <folly/ExceptionWrapper.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/Request.h>
#include <folly/lang/Exception.h>
#include <folly/small_vector.h>

#include <thrift/lib/cpp/TProcessorEventHandler.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/event_handler/Cpp2ContextAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/event_handler/EventHandlerChain.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * What the bridge drives, and on whose behalf. Built once and shared by every
 * connection, so nothing here changes after install.
 *
 * `serviceName` names the service for the handlers' benefit: they are given
 * "Service.method", the same form the classic server passes.
 *
 * The handler lists are the ones an embedder would have installed on a
 * `ThriftServer`. Both may be empty; a bridge with no processor event handlers
 * forwards everything untouched.
 */
struct TProcessorEventHandlers {
  std::string serviceName;
  std::vector<std::shared_ptr<apache::thrift::TProcessorEventHandler>>
      processor;
  std::vector<std::shared_ptr<apache::thrift::server::TServerEventHandler>>
      server;
};

struct TProcessorEventHandlerBridgeConfig {
  // Shared rather than held by value: this config is copied into every
  // connection's handler, and one refcount is the whole cost of that copy.
  std::shared_ptr<const TProcessorEventHandlers> handlers;
  PeerIdentityResolver identityResolver{nullptr};
};

namespace event_handler_detail {

// The name handlers are given: the classic server passes "Service.method", and
// interface ACLs and per-method action lookups are keyed on that form.
inline std::string qualify(
    std::string_view serviceName, std::string_view method) {
  return fmt::format("{}.{}", serviceName, method);
}

inline std::string_view methodOf(
    const ThriftServerRequestMessage& request) noexcept {
  const auto* md = request.payload.getRequestRpcMetadata();
  if (md == nullptr || !md->name().has_value()) {
    return {};
  }
  const auto sp = md->name()->view();
  return std::string_view(sp.data(), sp.size());
}

inline std::uint32_t requestBytes(
    const ThriftServerRequestMessage& request) noexcept {
  if (!request.payload.is<ThriftRequestResponsePayload>()) {
    return 0;
  }
  const auto& rr = request.payload.get<ThriftRequestResponsePayload>();
  return rr.data == nullptr
      ? 0
      : static_cast<std::uint32_t>(rr.data->computeChainDataLength());
}

inline std::uint32_t payloadBytes(
    const std::unique_ptr<folly::IOBuf>& data) noexcept {
  return data == nullptr
      ? 0
      : static_cast<std::uint32_t>(data->computeChainDataLength());
}

// Moves what the handlers wrote onto the outgoing reply. Only the normal reply
// carries metadata; an error frame has nowhere to put them.
inline void stampWriteHeaders(
    ThriftInitialResponsePayload& reply,
    apache::thrift::transport::THeader::StringToStringMap headers) {
  if (headers.empty() || reply.metadata == nullptr) {
    return;
  }
  auto& other = reply.metadata->otherMetadata().ensure();
  for (auto& [key, value] : headers) {
    other[key] = std::move(value);
  }
}

} // namespace event_handler_detail

/**
 * Runs `TProcessorEventHandler`s / `TServerEventHandler`s on the
 * fast_thrift pipeline.
 *
 * These handlers are written against the classic server: they expect Cpp2
 * contexts, a `THeader` to read and write, an ambient `folly::RequestContext`,
 * and callbacks in a fixed order around a request. The bridge supplies all of
 * that at its own boundary and hands the pipeline back its own message types
 * unchanged — see Cpp2ContextAdapter.h for what is and is not
 * translatable.
 *
 * Mapping, per connection:
 *   SetupComplete    -> TServerEventHandler::newConnection
 *   ConnectionClosed -> TServerEventHandler::connectionDestroyed
 *
 * and per request:
 *   onRead   -> preRead, postRead across the chain, then forward
 *   onWrite  -> preWrite, postWrite across the chain, then forward
 *
 * A `preRead`/`postRead` that throws is the handler refusing the request: the
 * request is dropped, an application error carrying the exception's name and
 * message goes back to the client, and the write-side callbacks do NOT run —
 * matching the classic server, where an exception thrown before dispatch means
 * `postWrite` never fires. Any response headers the handler set before
 * throwing still reach the client.
 *
 * A `preWrite`/`postWrite` that throws terminates the process. The request has
 * been served by then, so there is no verdict left to honour and nothing to
 * turn the exception into; swallowing it would only hide the handler's bug.
 *
 * NOT supported, because fast_thrift has no equivalent: streams, sinks and
 * interactions (`onInteractionTerminate`), the serialized-message callbacks
 * (`onReadData` / `onWriteData`), and the server-lifecycle callbacks
 * (`preStart` / `preServe` / `postStop`). A handler relying on any of those
 * will not see them.
 *
 * Requires the server's `enableRequestContext`, and `enableRequestHeaders` if
 * any handler reads request headers. A request arriving without a context is
 * refused rather than forwarded: the bridge cannot tell an authorization
 * handler from a logging one, so quietly skipping them is not safe.
 *
 * Every callback runs synchronously on the connection's EventBase. That is
 * load-bearing, not incidental: the pipeline context does not guard against a
 * closed pipeline, so a callback that deferred work and wrote later would be a
 * use-after-free.
 */
template <typename Context>
class TProcessorEventHandlerBridge {
 public:
  explicit TProcessorEventHandlerBridge(
      TProcessorEventHandlerBridgeConfig config)
      : config_(std::move(config)),
        handlers_(config_.handlers.get()),
        drivesProcessorHandlers_(
            handlers_ != nullptr && !handlers_->processor.empty()) {
    if (drivesProcessorHandlers_) {
      // A pipelining client's steady-state depth. Deliberately small: a host
      // carries these by the hundred thousand, so headroom costs more memory
      // than the rehashes it saves.
      requests_.reserve(kInitialInFlight);
      idleStates_.reserve(kInitialInFlight);
    }
  }

  TProcessorEventHandlerBridge(const TProcessorEventHandlerBridge&) = delete;
  TProcessorEventHandlerBridge& operator=(const TProcessorEventHandlerBridge&) =
      delete;
  TProcessorEventHandlerBridge(TProcessorEventHandlerBridge&&) = delete;
  TProcessorEventHandlerBridge& operator=(TProcessorEventHandlerBridge&&) =
      delete;

  ~TProcessorEventHandlerBridge() = default;

  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();

    // The setup exchange is not a request. Latch the connection context it
    // carries — the only place the pipeline offers one — and forward.
    if (FOLLY_UNLIKELY(request.payload.is<ThriftConnectionSetupPayload>())) {
      ftConnContext_ = request.payload.get<ThriftConnectionSetupPayload>()
                           .setup->connContext;
      return ctx.fireRead(std::move(msg));
    }

    // No processor handlers means no per-request work, and so no classic
    // request context for anything downstream to read. A server configured
    // this way has no per-request security layer either, so a consumer that
    // finds nothing resolves nothing, which is the right answer. Note this is
    // narrower than the connection context, which is built for any handler at
    // all.
    if (!drivesProcessorHandlers_) {
      return ctx.fireRead(std::move(msg));
    }

    const uint32_t streamId = request.streamId;

    // Fail closed. A handler may be the thing authorizing this request, and
    // there is no way to tell from here, so a request the bridge cannot run
    // them for is refused rather than let through.
    if (FOLLY_UNLIKELY(
            request.requestContext == nullptr ||
            connectionContext_ == nullptr)) {
      return ctx.fireWrite(
          channel_pipeline::erase_and_box(makeAppErrorMessage(
              streamId,
              "TProcessorEventHandlerBridgeMisconfigured",
              "event handlers are installed but the server built no "
              "request context for this request; enableRequestContext is "
              "required")));
    }

    const auto methodName = event_handler_detail::methodOf(request);
    auto state = acquireState();
    state->cpp2Request.emplace(
        &connectionContext_->get(), &state->header, std::string(methodName));
    state->context.emplace(
        *state->cpp2Request, state->header, *request.requestContext);

    // Scoped across the forward as well as the callbacks: handlers stamp
    // identity onto the ambient context for the service to read, and an
    // executor hop downstream captures whatever is installed here.
    folly::RequestContextScopeGuard guard(state->context->ambientContext());

    // Binding is inside the refusal path with the callbacks: it runs
    // getServiceContext, which is handler code like any other, and an escape
    // out of a noexcept pipeline callback would take the process with it.
    if (auto refusal = folly::try_and_catch([&] {
          state->chain.bind(
              state->context->get(), qualifiedMethodName(*state, methodName));
          state->chain.preRead();
          state->chain.postRead(
              state->context->header(),
              event_handler_detail::requestBytes(request));
        })) {
      auto response = makeRejectionMessage(streamId, refusal);
      // A handler that set a response header before refusing meant it for the
      // client; the refusal is the only response there will be. A rejection is
      // always a reply payload, so it has somewhere to carry them.
      event_handler_detail::stampWriteHeaders(
          response.payload.template get<ThriftInitialResponsePayload>(),
          state->context->takeWriteHeaders());
      auto boxed = channel_pipeline::erase_and_box(std::move(response));
      releaseState(std::move(state));
      return ctx.fireWrite(std::move(boxed));
    }

    // Always an insert: the rocket layer errors a stream id that is already in
    // flight, so the bridge never sees the same one twice. Assigning over a
    // live entry would drop a bound chain, so it is not offered.
    [[maybe_unused]] const bool inserted =
        requests_.try_emplace(streamId, std::move(state)).second;
    DCHECK(inserted);
    return ctx.fireRead(std::move(msg));
  }

  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& response = msg.get<ThriftServerResponseMessage>();

    // Resolved once: the stream this answers, and the reply body if it carries
    // one. A frame that names no stream belongs to no request.
    ThriftInitialResponsePayload* reply = nullptr;
    uint32_t streamId = 0;
    if (response.payload.is<ThriftInitialResponsePayload>()) {
      reply = &response.payload.get<ThriftInitialResponsePayload>();
      streamId = reply->streamId;
    } else if (response.payload.is<ThriftErrorPayload>()) {
      streamId = response.payload.get<ThriftErrorPayload>().streamId;
    }

    auto it = requests_.find(streamId);
    if (it == requests_.end()) {
      // Setup responses, connection-level frames, and anything the bridge
      // refused — none of which a handler saw on the way in.
      return ctx.fireWrite(std::move(msg));
    }
    auto state = std::move(it->second);
    requests_.erase(it);

    // The write-side callbacks bracket the serialization of a reply body, so
    // a frame that carries none does not get them — the handlers' contexts are
    // still returned below, which is the pairing they are promised.
    if (reply != nullptr) {
      // A response that resolved inline is still under the read side's guard,
      // which installed this same context; re-installing swaps for nothing.
      const auto& ambient = state->context->ambientContext();
      std::optional<folly::RequestContextScopeGuard> guard;
      if (folly::RequestContext::try_get() != ambient.get()) {
        guard.emplace(ambient);
      }
      // Deliberately not caught: these run after the request has been served,
      // so there is no verdict left to honour and swallowing would hide a
      // handler bug. An escape terminates, as it would on a classic server.
      state->chain.preWrite();
      state->chain.postWrite(event_handler_detail::payloadBytes(reply->data));

      // After postWrite: handlers write response headers there, and the reply
      // has not been serialized yet.
      event_handler_detail::stampWriteHeaders(
          *reply, state->context->takeWriteHeaders());
    }
    releaseState(std::move(state));

    return ctx.fireWrite(std::move(msg));
  }

  static constexpr channel_pipeline::Subscriptions<
      ThriftServerEventType::SetupComplete,
      ThriftServerEventType::ConnectionClosed>
      kSubscribedEvents{};

  void onEvent(
      Context& /*ctx*/,
      ThriftServerEventType ev,
      const channel_pipeline::TypeErasedBox& /*evt*/) noexcept {
    // Nothing installed: the connection context exists only to be handed to
    // handlers, so there is none to build and nobody to tell about it.
    if (handlers_ == nullptr) {
      return;
    }
    if (ev == ThriftServerEventType::SetupComplete) {
      // The connection is answered and about to carry requests. Build the
      // context now so per-connection handler state exists before the first
      // request needs it.
      if (ftConnContext_ == nullptr || connectionContext_ != nullptr) {
        return;
      }
      connectionContext_ = std::make_unique<Cpp2ConnContextAdapter>(
          boost::intrusive_ptr<ThriftConnContext>(ftConnContext_),
          config_.identityResolver);
      for (const auto& handler : handlers_->server) {
        handler->newConnection(&connectionContext_->get());
      }
      return;
    }
    if (ev != ThriftServerEventType::ConnectionClosed ||
        connectionContext_ == nullptr || connectionDestroyed_) {
      return;
    }
    // The close is announced once even if the event arrives twice: a handler
    // is promised one connectionDestroyed for the newConnection it was told.
    connectionDestroyed_ = true;
    // Requests still outstanding never produced a response, so their
    // write-side callbacks do not run — as on a classic connection dropped
    // mid-flight. Their state is not dropped here though: the service may
    // still hold a request whose classic context this connection's one backs,
    // so the bridge's own destruction is the safe point to release them.
    for (const auto& handler : handlers_->server) {
      handler->connectionDestroyed(&connectionContext_->get());
    }
    return;
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onReadReady(Context&) noexcept {}
  void onWriteReady(Context&) noexcept {}
  void onPipelineActive(Context&) noexcept {}
  void onPipelineInactive(Context&) noexcept {}
  void handlerAdded(Context&) noexcept {}
  void handlerRemoved(Context&) noexcept {}

 private:
  static constexpr std::size_t kInitialInFlight = 8;
  // Wide enough for any service's own method set, and the longest the
  // per-request scan can ever get. See qualifiedMethodName().
  static constexpr std::size_t kMaxCachedMethods = 64;

  // One request's adapted state, from the point it enters the pipeline until
  // its response leaves. Only the context is per-request; the chain resolves
  // the handler list once and is rebound for whatever request holds this next.
  struct RequestState {
    // The two small members every acquire and release touches, kept at the
    // head so the bookkeeping does not reach past the bulk of the state.
    EventHandlerChain chain;
    // Engaged only while a request is in flight.
    std::optional<Cpp2RequestContextAdapter> context;
    // Pointed at by the classic context below, so it is declared ahead of it
    // and outlives it. Kept across requests: only its read headers are
    // per-request, and a binding context replaces those.
    apache::thrift::transport::THeader header;
    // Pooled rather than allocated per request: this is the largest thing a
    // request builds, and recycling the storage keeps it warm. Reconstructed
    // for each request so the security layer's fields never carry over.
    std::optional<apache::thrift::Cpp2RequestContext> cpp2Request;
    // Holds the qualified name for a request the connection's method cache
    // had no room for; see qualifiedMethodName(). Empty otherwise.
    std::string uncachedMethodName;

    RequestState(
        const EventHandlerChain::HandlerList& handlers,
        std::string_view serviceName)
        : chain(handlers, serviceName) {}
  };

  std::unique_ptr<RequestState> acquireState() {
    if (idleStates_.empty()) {
      return std::make_unique<RequestState>(
          handlers_->processor, handlers_->serviceName);
    }
    auto state = std::move(idleStates_.back());
    idleStates_.pop_back();
    return state;
  }

  // Returns the handlers' contexts and drops the request's, leaving the
  // resolved handler list and the header for the next request to rebind. The
  // idle list grows to the connection's peak concurrency and no further.
  void releaseState(std::unique_ptr<RequestState> state) {
    state->chain.unbind();
    // Emptied rather than left pointing at storage the next request rebuilds:
    // a reader that outlives the response finds nothing, which is what a
    // server that installed no bridge would give it.
    state->context->ftContext().template setState<Cpp2BridgeExtension>(nullptr);
    state->context.reset();
    state->cpp2Request.reset();
    // The header outlives the request: anything left in its write map would
    // otherwise reach the next response this state serves.
    state->header.clearHeaders();
    idleStates_.push_back(std::move(state));
  }

  // The "{Service}.{method}" name handlers are keyed on, built once per method
  // per connection rather than once per request. A request holds a view of the
  // value for its lifetime, which the node map keeps put across a rehash.
  //
  // A connection multiplexes a handful of methods, so remembering only the
  // last one thrashes and every alternation pays a hash. The resolved set is
  // small enough that scanning it outright is cheaper.
  //
  // Capped, because the name arrives on the wire: a peer sending distinct
  // names would otherwise grow the connection for as long as it holds it, and
  // lengthen the scan every request pays. Past the cap a request qualifies
  // into its own state.
  std::string_view qualifiedMethodName(
      RequestState& state, std::string_view method) {
    for (const auto& [name, qualified] : methodCache_) {
      if (name.size() == method.size() &&
          std::memcmp(name.data(), method.data(), name.size()) == 0) {
        return qualified;
      }
    }
    return resolveQualifiedMethodName(state, method);
  }

  FOLLY_NOINLINE std::string_view resolveQualifiedMethodName(
      RequestState& state, std::string_view method) {
    if (FOLLY_UNLIKELY(methodCache_.size() == kMaxCachedMethods)) {
      state.uncachedMethodName =
          event_handler_detail::qualify(handlers_->serviceName, method);
      return state.uncachedMethodName;
    }
    // Views into the node map, which keeps both put across a rehash.
    const auto& entry =
        *qualifiedMethodNames_
             .emplace(
                 method,
                 event_handler_detail::qualify(handlers_->serviceName, method))
             .first;
    methodCache_.push_back({entry.first, entry.second});
    return entry.second;
  }

  const TProcessorEventHandlerBridgeConfig config_;
  // Resolved once: the answer cannot change, and reaching it through the
  // shared_ptr on every request is two dependent loads into memory that every
  // connection shares.
  const TProcessorEventHandlers* const handlers_;
  const bool drivesProcessorHandlers_;
  // Sits here to land in the padding the flag above leaves, rather than widen
  // the connection by a word of its own.
  bool connectionDestroyed_{false};

  // Declared before the requests: their contexts borrow this one, and members
  // are destroyed in reverse.
  std::unique_ptr<Cpp2ConnContextAdapter> connectionContext_;
  // Latched from the setup message, the first thing the connection sends.
  // Non-owning; the connection-context handler outlives this pipeline.
  ThriftConnContext* ftConnContext_{nullptr};

  // Keyed by stream id rather than by the request context, because a
  // framework-generated error response carries no context to key on.
  folly::F14FastMap<uint32_t, std::unique_ptr<RequestState>> requests_;

  // Node map: a request holds a view of the value, which must survive a
  // rehash. See qualifiedMethodName().
  folly::F14NodeMap<std::string, std::string> qualifiedMethodNames_;
  // Contiguous mirror of the map above, scanned linearly; see
  // qualifiedMethodName().
  folly::small_vector<std::pair<std::string_view, std::string_view>, 8>
      methodCache_;

  // Contiguous spine: acquiring pops a pointer without dereferencing the state
  // it names, so the state's own cache miss overlaps the work that follows.
  // Grows to the connection's peak concurrency and no further.
  std::vector<std::unique_ptr<RequestState>> idleStates_;
};

} // namespace apache::thrift::fast_thrift::thrift::server
