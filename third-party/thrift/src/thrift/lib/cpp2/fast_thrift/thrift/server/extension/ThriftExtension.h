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

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftRequestPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Safe, curated view/mutator types and concepts for FastThriftServer
 * extensions.
 *
 * Extensions are the recommended way to hook the request/response path. Rather
 * than a raw channel_pipeline handler (which owns message lifetime and the raw
 * pipeline context), an extension implements exactly one small callback over a
 * restricted view of the request. The framework adapts it into a pipeline
 * handler (see ThriftExtensionPipelineHandler) and enforces the forwarding /
 * rejection contract, so an extension can neither retain, replace, nor manually
 * forward a message.
 *
 * An extension declares the access it needs through its parameter type: take a
 * ThriftRequestView to read, or a ThriftRequestMutator to read and write. The
 * mutator *is* a view, so a read/write extension keeps every read accessor.
 */

/**
 * Outcome of an extension's request callback: proceed with the request, or
 * reject it with a cause.
 *
 * On reject the request is dropped and a protocol-correct error response
 * carrying the cause is emitted back to the client.
 */
class RequestVerdict {
 public:
  static RequestVerdict proceed() noexcept { return RequestVerdict(); }

  static RequestVerdict reject(folly::exception_wrapper cause) noexcept {
    return RequestVerdict(std::move(cause));
  }

  // noexcept despite allocating: constructing Ex can throw bad_alloc, which
  // terminates rather than unwinds. OOM ⇒ terminate is the pipeline-wide
  // contract, not a hazard introduced here.
  template <typename Ex, typename... Args>
  static RequestVerdict reject(Args&&... args) noexcept {
    return RequestVerdict(
        folly::make_exception_wrapper<Ex>(std::forward<Args>(args)...));
  }

  /**
   * Serve this request, then stop admitting further ones on the connection
   * until the extension resumes it.
   *
   * Backpressure is expressed here, on the request itself, because this is
   * where the result can still reach the transport: the verdict travels back
   * out of the pipeline as Result::Backpressure and stops the socket being
   * read before the next request arrives. An extension deciding out-of-band
   * instead — off a write completion, say — could not take effect until a
   * request had already been let through.
   *
   * Only honoured for an extension wired for backpressure (one declaring
   * onBackpressureAttached, so it holds a ReadResumer); returning it from any
   * other extension is a programming error and fails a DCHECK, since nothing
   * would ever lift the pause.
   */
  static RequestVerdict backpressure() noexcept {
    RequestVerdict verdict;
    verdict.backpressure_ = true;
    return verdict;
  }

  bool isRejected() const noexcept { return bool(cause_); }

  bool appliesBackpressure() const noexcept { return backpressure_; }

  // Empty unless isRejected().
  const folly::exception_wrapper& cause() const& noexcept { return cause_; }
  folly::exception_wrapper&& cause() && noexcept { return std::move(cause_); }

 private:
  RequestVerdict() = default;
  explicit RequestVerdict(folly::exception_wrapper cause) noexcept
      : cause_(std::move(cause)) {}

  folly::exception_wrapper cause_;
  bool backpressure_{false};
};

/**
 * Read-only view of an inbound request. Accessors are lazy over the underlying
 * request metadata and never copy. The view is valid only for the duration of
 * the callback — do not retain it.
 *
 * Base of ThriftRequestMutator. The destructor is non-virtual: views are
 * stack-constructed by the framework adapter and never destroyed through a
 * base pointer.
 */
class ThriftRequestView {
 public:
  explicit ThriftRequestView(const ThriftServerRequestMessage& request) noexcept
      : request_(request) {}

  ThriftRequestView(const ThriftRequestView&) = delete;
  ThriftRequestView& operator=(const ThriftRequestView&) = delete;
  ThriftRequestView(ThriftRequestView&&) = delete;
  ThriftRequestView& operator=(ThriftRequestView&&) = delete;
  ~ThriftRequestView() = default;

  // Invoked method name, or empty if the request carries no method metadata.
  std::string_view methodName() const noexcept {
    const auto* md = request_.payload.getRequestRpcMetadata();
    if (md != nullptr && md->name().has_value()) {
      const auto sp = md->name()->view();
      return std::string_view(sp.data(), sp.size());
    }
    return {};
  }

  // Taken from the payload variant's discriminator rather than the optional
  // metadata field, so it is always well-defined.
  apache::thrift::RpcKind rpcKind() const noexcept {
    return request_.payload.rpcKind();
  }

  // Protocol the client encoded with. `protocol` is an optional metadata field
  // and the server tolerates its absence, so this reports the same COMPACT
  // default the server itself applies when decoding.
  apache::thrift::ProtocolId protocol() const noexcept {
    const auto* md = request_.payload.getRequestRpcMetadata();
    if (md != nullptr && md->protocol().has_value()) {
      return *md->protocol();
    }
    return apache::thrift::ProtocolId::COMPACT;
  }

  // Size in bytes of the serialized request arguments (0 if absent).
  std::size_t serializedSize() const noexcept {
    if (request_.payload.is<ThriftRequestResponsePayload>()) {
      const auto& rr = request_.payload.get<ThriftRequestResponsePayload>();
      if (rr.data != nullptr) {
        return rr.data->computeChainDataLength();
      }
    }
    return 0;
  }

  // Value of custom request header `key`, or nullptr if absent.
  //
  // The per-request context is the only place headers live. Declaring
  // kUsesHeaders is what guarantees one is there: without it a server may be
  // configured with no header support and this reads empty. The lookup is free
  // of temporaries — the context's map is searchable by string_view.
  const std::string* header(std::string_view key) const noexcept {
    if (request_.requestContext == nullptr) {
      return nullptr;
    }
    return request_.requestContext->getHeader(key);
  }

  // Rocket stream id correlating this request with its response.
  uint32_t streamId() const noexcept { return request_.streamId; }

 private:
  const ThriftServerRequestMessage& request_;
};

/**
 * Read/write access to an inbound request: every ThriftRequestView accessor
 * plus header mutation. It intentionally does not expose the raw payload, so an
 * extension cannot swap out or take ownership of the message. Valid only for
 * the duration of the callback.
 */
class ThriftRequestMutator final : public ThriftRequestView {
 public:
  explicit ThriftRequestMutator(ThriftServerRequestMessage& request) noexcept
      : ThriftRequestView(request), request_(request) {}

  // Set (or overwrite) a custom request header seen by downstream extensions
  // and the service.
  //
  // Writes to the per-request context, the one place headers are read from.
  // Dropped on a server with no request context — see header() for why
  // kUsesHeaders is what keeps that from happening.
  //
  // noexcept despite allocating: extension callbacks are themselves noexcept,
  // so an escaping bad_alloc terminates either way. OOM ⇒ terminate is the
  // pipeline-wide contract, not a hazard introduced here.
  void setHeader(std::string key, std::string value) noexcept {
    if (request_.requestContext == nullptr) {
      return;
    }
    request_.requestContext->setHeader(std::move(key), std::move(value));
  }

 private:
  ThriftServerRequestMessage& request_;
};

/**
 * Read-only view of an outbound response. Header access is only meaningful for
 * a response that has a per-request context behind it: framework-generated
 * responses (parse errors, wrong RPC kind) have none and report empty.
 *
 * Base of ThriftResponseMutator; see ThriftRequestView for the non-virtual
 * destructor rationale.
 */
class ThriftResponseView {
 public:
  explicit ThriftResponseView(
      const ThriftServerResponseMessage& response) noexcept
      : response_(response) {}

  ThriftResponseView(const ThriftResponseView&) = delete;
  ThriftResponseView& operator=(const ThriftResponseView&) = delete;
  ThriftResponseView(ThriftResponseView&&) = delete;
  ThriftResponseView& operator=(ThriftResponseView&&) = delete;
  ~ThriftResponseView() = default;

  // True if this response carries a terminal error frame rather than a normal
  // reply.
  bool isError() const noexcept {
    return response_.payload.is<ThriftErrorPayload>();
  }

  // Value of custom response header `key`, or nullptr if absent.
  //
  // Reads the headers the response is going to carry, off the per-request
  // context — the extensions run while the response is still accumulating them
  // and before they are handed to the outgoing metadata.
  const std::string* header(std::string_view key) const noexcept {
    if (response_.requestContext == nullptr) {
      return nullptr;
    }
    return response_.requestContext->getResponseHeader(key);
  }

 private:
  const ThriftServerResponseMessage& response_;
};

/**
 * Read/write access to an outbound response: every ThriftResponseView accessor
 * plus header mutation on the normal reply. Valid only for the duration of the
 * callback.
 */
class ThriftResponseMutator final : public ThriftResponseView {
 public:
  explicit ThriftResponseMutator(ThriftServerResponseMessage& response) noexcept
      : ThriftResponseView(response), response_(response) {}

  // Set (or overwrite) a custom response header, on the same context the
  // service handler writes its own to. Dropped for a response with no
  // per-request context, and for a Rocket ERROR frame, which has no metadata to
  // carry headers on. See ThriftRequestMutator::setHeader for the noexcept
  // rationale.
  void setHeader(std::string key, std::string value) noexcept {
    if (response_.requestContext == nullptr) {
      return;
    }
    response_.requestContext->setResponseHeader(
        std::move(key), std::move(value));
  }

 private:
  ThriftServerResponseMessage& response_;
};

// === Concepts ===
//
// An extension declares the access it needs by parameter type: a const view for
// read-only, a mutator for read/write. Because the mutator derives from the
// view, a view-taking callback also accepts a mutator; the adapter resolves
// this by testing the view form first, so each extension is handed exactly the
// access its signature asks for. Declaring both forms is redundant — the view
// form wins and the mutator overload is never called.

template <typename H>
concept HasRequestViewCallback = requires(H& h, const ThriftRequestView& v) {
  { h.onRequest(v) } noexcept -> std::same_as<RequestVerdict>;
};

template <typename H>
concept HasRequestMutatorCallback = requires(H& h, ThriftRequestMutator& m) {
  { h.onRequest(m) } noexcept -> std::same_as<RequestVerdict>;
};

template <typename H>
concept ThriftExtensionHandler =
    HasRequestViewCallback<H> || HasRequestMutatorCallback<H>;

// Optional response-side callbacks. The response path is not a rejection point,
// so these return void.

template <typename H>
concept HasResponseViewCallback = requires(H& h, const ThriftResponseView& v) {
  { h.onResponse(v) } noexcept -> std::same_as<void>;
};

template <typename H>
concept HasResponseMutatorCallback = requires(H& h, ThriftResponseMutator& m) {
  { h.onResponse(m) } noexcept -> std::same_as<void>;
};

template <typename H>
concept HasResponseCallback =
    HasResponseViewCallback<H> || HasResponseMutatorCallback<H>;

/**
 * True iff H reads or writes request/response headers, by declaring
 *
 *   static constexpr bool kUsesHeaders = true;
 *
 * Headers live only on the per-request context, and a server populates that
 * context only when configured to. FastThriftServer::addModule refuses a module
 * carrying such an extension unless enableRequestHeaders is set, so a
 * misconfigured server fails at startup rather than handing the extension
 * silently empty headers.
 *
 * One declaration covers both directions: enableRequestHeaders implies
 * enableRequestContext, which is all the response side needs.
 */
template <typename H>
concept UsesHeaders = requires {
  { H::kUsesHeaders } -> std::convertible_to<bool>;
} && H::kUsesHeaders;

/**
 * True iff H shares per-connection state with its peer extensions, by defining
 *
 *   using ConnState = SomeType;
 *
 * The adapter then resolves the connection's `ConnState` from the
 * ExtensionStateStore and passes it as H's first constructor argument, ahead of
 * the arguments given to addThriftExtension. Extensions naming the same type
 * share one object per connection; an extension that declares none is
 * constructed from its own arguments alone.
 *
 * The state is default-constructed — whichever extension names it first brings
 * it into being, so no one extension's arguments could construct it. A
 * ConnState that is not default-constructible is a compile error.
 */
template <typename H>
concept HasConnState = requires { typename H::ConnState; };

} // namespace apache::thrift::fast_thrift::thrift
