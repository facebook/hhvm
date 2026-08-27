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
  ThriftRequestContext(ThriftRequestContext&&) = default;
  ThriftRequestContext& operator=(ThriftRequestContext&&) = default;

  void setConnectionContext(
      boost::intrusive_ptr<ThriftConnContext> ctx) noexcept {
    connContext_ = std::move(ctx);
  }

  ThriftConnContext* getConnectionContext() const noexcept {
    return connContext_.get();
  }

  // Inbound custom request headers (RequestRpcMetadata.otherMetadata),
  // stamped by ThriftServerRequestHeadersHandler. Empty when the handler is
  // not wired or the request carried no custom headers.
  void setHeaders(HeaderMap headers) noexcept { headers_ = std::move(headers); }

  const HeaderMap& getHeaders() const noexcept { return headers_; }

  // Returns a pointer to the header value for `key`, or nullptr if absent.
  const std::string* getHeader(std::string_view key) const noexcept {
    auto it = headers_.find(key);
    return it == headers_.end() ? nullptr : &it->second;
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

 private:
  boost::intrusive_ptr<ThriftConnContext> connContext_;
  HeaderMap headers_;
  apache::thrift::ChecksumAlgorithm checksumAlgorithm_{
      apache::thrift::ChecksumAlgorithm::NONE};
  detail::InternalFieldsT internalFields_;
};

} // namespace apache::thrift::fast_thrift::thrift
