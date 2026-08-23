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
#include <string>
#include <string_view>
#include <utility>

#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransportCertificate.h>

#include <folly/CppAttributes.h>

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>

namespace apache::thrift::fast_thrift::thrift {

namespace detail {

// The classic server's internal-fields slot, which fast_thrift points at
// rather than owning: the object lives inline in whichever context owns it, so
// two contexts can only reach one object by one of them holding a pointer.
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
  ThriftConnContext() = default;

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

  // Points this context at the per-connection fields the security layer keeps
  // for this connection. Non-owning: the fields live on the Cpp2ConnContext
  // built for the connection, which outlives this call, and nothing is copied
  // — both contexts hand out the one object.
  void setInternalFields(detail::InternalFieldsT* fields) noexcept {
    internalFields_ = fields;
  }

  // The security layer's per-connection fields, or null while nothing holds
  // any — a server with no security layer never fills the slot, unlike the
  // classic server, where the fields always exist.
  //
  // Unchecked: only valid for the `T` the owner constructed.
  template <class T>
  T* FOLLY_NULLABLE getInternalFields() noexcept {
    return hasInternalFields() ? &internalFields_->value_unchecked<T>()
                               : nullptr;
  }

  template <class T>
  const T* FOLLY_NULLABLE getInternalFields() const noexcept {
    return hasInternalFields() ? &internalFields_->value_unchecked<T>()
                               : nullptr;
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

 private:
  bool hasInternalFields() const noexcept {
    return internalFields_ != nullptr && internalFields_->has_value();
  }

  folly::SocketAddress peerAddress_{};
  std::string securityProtocol_;
  std::shared_ptr<const folly::AsyncTransportCertificate> peerCertificate_;
  rocket::TypeErasedPtr userData_{};
  detail::InternalFieldsT* internalFields_{nullptr};
};

} // namespace apache::thrift::fast_thrift::thrift
