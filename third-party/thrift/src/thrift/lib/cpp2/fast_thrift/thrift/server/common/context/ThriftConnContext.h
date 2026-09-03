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

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/AsyncTransportCertificate.h>

#include <folly/CppAttributes.h>

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace detail {

// The security layer's internal-fields slot, stored inline in the context
// that owns it. Same type and size as the classic server's.
using InternalFieldsT = apache::thrift::util::TypeErasedValue<128>;

} // namespace detail

// Per-connection context. Lives for the duration of one accepted connection.
//
// Refcount is non-atomic (boost::thread_unsafe_counter), which avoids an
// atomic op every time the context is shared into a per-request handle. The
// invariant that makes this sound is that every increment and decrement
// happens on the connection's EventBase.
//
// That is not automatic once user handlers run on a CPU pool. It holds
// because the only reference outside the pipeline lives in
// ThriftRequestContext, which is owned by FastHandlerCallback, and the
// callback defers its own destruction to the EventBase. Moving a context —
// as the response path does — never touches the refcount, so building a
// response off-EventBase stays safe.
//
// Handler code must therefore not copy a boost::intrusive_ptr to this object
// off the EventBase. getConnectionContext() hands back a raw pointer
// precisely so the safe usage is the obvious one.
class ThriftConnContext : public boost::intrusive_ref_counter<
                              ThriftConnContext,
                              boost::thread_unsafe_counter> {
 public:
  // The security layer's per-connection fields are constructed by whoever
  // accepts the connection and moved in here; a server with no security layer
  // leaves the slot empty.
  explicit ThriftConnContext(detail::InternalFieldsT internalFields = {})
      : internalFields_(std::move(internalFields)) {}

  ThriftConnContext(const ThriftConnContext&) = delete;
  ThriftConnContext& operator=(const ThriftConnContext&) = delete;
  ThriftConnContext(ThriftConnContext&&) = delete;
  ThriftConnContext& operator=(ThriftConnContext&&) = delete;

  // Peer address of the accepted socket. May be empty.
  const folly::SocketAddress& getPeerAddress() const noexcept {
    return peerAddress_;
  }

  // Negotiated TLS protocol ("TLS1.3", ...). Empty on plaintext.
  std::string_view getSecurityProtocol() const noexcept {
    return securityProtocol_;
  }

  // Peer's TLS leaf certificate, or null if none.
  const folly::AsyncTransportCertificate* getPeerCertificate() const noexcept {
    return peerCertificate_.get();
  }

  // The connection's transport. Non-owning, and valid only while the
  // connection is open — the transport outlives this context, but neither
  // outlives the connection, so nothing may retain it.
  //
  // EventBase-affine: only touch it from the connection's own EventBase, which
  // rules out a service that has been handed off to a CPU executor. Prefer the
  // snapshotted peer certificate above, which is safe from anywhere and, after
  // a StopTLS downgrade, is the only thing that still knows what the peer
  // proved — by then this reports a plaintext socket.
  const folly::AsyncTransport* getTransport() const noexcept {
    return transport_;
  }

  // What the client said about itself in its setup: its hostname, its agent,
  // and whatever else it chose to send. Client-supplied and unverified — good
  // for triage, not evidence.
  void setClientMetadata(apache::thrift::ClientMetadata metadata) noexcept {
    clientMetadata_ = std::move(metadata);
  }

  // Null when the client sent none.
  const apache::thrift::ClientMetadata* FOLLY_NULLABLE
  getClientMetadata() const noexcept {
    return clientMetadata_.has_value() ? &clientMetadata_.value() : nullptr;
  }

  // Opaque per-connection slot. The deleter runs at connection close.
  //
  // There is one, so it has one owner. Anything shared between handlers, or
  // published from a handler to the service, belongs in the connection's
  // ExtensionStateStore instead — that is keyed by type and so cannot be
  // claimed twice.
  void setUserData(rocket::TypeErasedPtr userData) noexcept {
    userData_ = std::move(userData);
  }
  void* getUserData() const noexcept { return userData_.get(); }

  // The security layer's per-connection fields. Unchecked: only valid for the
  // `T` the slot was constructed with, and only once something has filled it.
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

  void setPeerAddress(folly::SocketAddress addr) noexcept {
    peerAddress_ = std::move(addr);
  }
  void setSecurityProtocol(std::string proto) noexcept {
    securityProtocol_ = std::move(proto);
  }
  void setPeerCertificate(
      std::shared_ptr<const folly::AsyncTransportCertificate> cert) noexcept {
    peerCertificate_ = std::move(cert);
  }
  void setTransport(const folly::AsyncTransport* transport) noexcept {
    transport_ = transport;
  }

 private:
  folly::SocketAddress peerAddress_{};
  std::string securityProtocol_;
  std::shared_ptr<const folly::AsyncTransportCertificate> peerCertificate_;
  const folly::AsyncTransport* transport_{nullptr};
  rocket::TypeErasedPtr userData_{};
  detail::InternalFieldsT internalFields_;
  std::optional<apache::thrift::ClientMetadata> clientMetadata_;
};

} // namespace apache::thrift::fast_thrift::thrift
