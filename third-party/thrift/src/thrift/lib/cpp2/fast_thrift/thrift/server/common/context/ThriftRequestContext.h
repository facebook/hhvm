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
#include <string_view>
#include <utility>

#include <boost/intrusive_ptr.hpp>

#include <folly/container/F14Map.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

// Per-request context. Lives for the duration of one in-flight RPC.
//
// Default-constructed empty. Pipeline handlers populate individual fields as
// the request traverses the chain: RequestContextHandler creates the object,
// then ConnectionContextHandler stamps in the per-connection context, and so
// on as additional handlers come online (metadata, headers, etc.).
class ThriftRequestContext {
 public:
  // Same type as RequestRpcMetadata.otherMetadata, so headers move in with no
  // copy. F14's default hasher/equal are transparent, enabling find() by
  // string_view without building a std::string.
  using HeaderMap = folly::F14NodeMap<std::string, std::string>;

  // The security layer's per-request fields are constructed by whoever
  // creates the request context and moved in here; a server with no security
  // layer leaves the slot empty.
  explicit ThriftRequestContext(detail::InternalFieldsT internalFields = {})
      : internalFields_(std::move(internalFields)) {}

  ThriftRequestContext(const ThriftRequestContext&) = delete;
  ThriftRequestContext& operator=(const ThriftRequestContext&) = delete;
  // Extension storage hands out interior pointers, so a context stays where it
  // was built.
  ThriftRequestContext(ThriftRequestContext&&) = delete;
  ThriftRequestContext& operator=(ThriftRequestContext&&) = delete;

  void setConnectionContext(
      boost::intrusive_ptr<ThriftConnContext> ctx) noexcept {
    connContext_ = std::move(ctx);
  }

  ThriftConnContext* getConnectionContext() const noexcept {
    return connContext_.get();
  }

  // Invoked method name (RequestRpcMetadata.name), stamped by
  // ThriftServerRequestContextHandler. Empty when the request carried no
  // method metadata.
  void setMethodName(std::string methodName) noexcept {
    methodName_ = std::move(methodName);
  }

  const std::string& getMethodName() const noexcept { return methodName_; }

  // Inbound custom request headers (RequestRpcMetadata.otherMetadata),
  // stamped by ThriftServerRequestHeadersHandler. Empty when the handler is
  // not wired or the request carried no custom headers.
  void setHeaders(HeaderMap headers) noexcept { headers_ = std::move(headers); }

  // Adds to (or overwrites) a single inbound header. Used by extensions
  // injecting a header for the service to read; the bulk setter above is the
  // one the headers handler uses to move the client's map in wholesale.
  void setHeader(std::string key, std::string value) {
    headers_[std::move(key)] = std::move(value);
  }

  const HeaderMap& getHeaders() const noexcept { return headers_; }

  // Returns a pointer to the header value for `key`, or nullptr if absent.
  const std::string* getHeader(std::string_view key) const noexcept {
    auto it = headers_.find(key);
    return it == headers_.end() ? nullptr : &it->second;
  }

  // Custom headers to send back on this request's response, set by the service
  // handler. Kept separate from the inbound headers above: the two directions
  // are independent, and echoing what the client sent is never the intent.
  //
  // The map is the same type as ResponseRpcMetadata.otherMetadata, so it moves
  // onto the outgoing metadata with no copy or rehash.
  //
  // Only an initial response carries metadata to put these on. A request
  // answered by a rocket ERROR frame, or by a setup rejection, has nowhere on
  // the wire for them and drops them.
  void setResponseHeader(std::string key, std::string value) {
    writeHeaders_[std::move(key)] = std::move(value);
  }

  // Only reports what is still on the context. Once the response has been
  // handed its headers this reads empty, so it answers "what will this
  // response carry", not "what did it carry".
  const std::string* getResponseHeader(std::string_view key) const noexcept {
    auto it = writeHeaders_.find(key);
    return it == writeHeaders_.end() ? nullptr : &it->second;
  }

  bool hasResponseHeaders() const noexcept { return !writeHeaders_.empty(); }

  // Hands the accumulated headers to the response being built, leaving the
  // context with none. Called once per response, on the write path.
  HeaderMap extractResponseHeaders() noexcept {
    return std::exchange(writeHeaders_, {});
  }

  // Checksum algorithm the response must echo, captured from the inbound
  // request's checksum by ThriftServerChecksumHandler. NONE when the request
  // carried no checksum, so the response is left unchecksummed.
  void setChecksumAlgorithm(
      apache::thrift::ChecksumAlgorithm algorithm) noexcept {
    checksumAlgorithm_ = algorithm;
  }

  apache::thrift::ChecksumAlgorithm getChecksumAlgorithm() const noexcept {
    return checksumAlgorithm_;
  }

  // The security layer's per-request fields. The connection's own fields are
  // on getConnectionContext(). Unchecked: only valid for the `T` the slot was
  // constructed with, and only once something has filled it.
  template <class T>
  T& getInternalFields() noexcept {
    return internalFields_.value_unchecked<T>();
  }

  template <class T>
  const T& getInternalFields() const noexcept {
    return internalFields_.value_unchecked<T>();
  }

  bool hasInternalFields() const noexcept {
    return internalFields_.has_value();
  }

  // Builds this request's extension storage from the server's request-scope
  // layout, which must outlive the request. Called once, by whoever creates
  // the context, before any handler sees it.
  void installExtensions(const ExtensionLayout& layout) {
    extensionSlots_.install(layout);
  }

  // `Ext`'s per-request state, or null when `Ext` is not installed on this
  // server. An extension that declares no `RequestState` does not compile here,
  // so asking for a scope an extension does not have is caught at build time
  // rather than read as absent.
  template <class Ext>
  typename Ext::RequestState* FOLLY_NULLABLE tryState() const noexcept {
    return extensionSlots_.find<typename Ext::RequestState>(Ext::kId);
  }

  // Publishes `Ext`'s state on this context. Non-owning: the extension keeps
  // it alive while the context can reach it, and clears the slot when it does
  // not.
  template <class Ext>
  void setState(typename Ext::RequestState* FOLLY_NULLABLE state) noexcept {
    extensionSlots_.set(Ext::kId, state);
  }

 private:
  boost::intrusive_ptr<ThriftConnContext> connContext_;
  std::string methodName_;
  HeaderMap headers_;
  HeaderMap writeHeaders_;
  apache::thrift::ChecksumAlgorithm checksumAlgorithm_{
      apache::thrift::ChecksumAlgorithm::NONE};
  detail::InternalFieldsT internalFields_;
  ExtensionSlots extensionSlots_;
};

} // namespace apache::thrift::fast_thrift::thrift
