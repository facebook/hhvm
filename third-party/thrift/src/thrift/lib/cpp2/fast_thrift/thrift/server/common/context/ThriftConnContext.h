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

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

namespace apache::thrift::fast_thrift::thrift {

// Per-connection context. Lives for the duration of one accepted connection.
//
// Refcount is non-atomic (boost::thread_unsafe_counter): a ThriftConnContext
// is per-connection and per-IO-thread — all increments/decrements happen on
// the owning EventBase. Using a non-atomic counter avoids the per-bump
// atomic op when the context is shared into per-request handles.
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
  void setUserData(rocket::TypeErasedPtr userData) noexcept {
    userData_ = std::move(userData);
  }
  void* getUserData() const noexcept { return userData_.get(); }

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
  folly::SocketAddress peerAddress_{};
  std::string securityProtocol_;
  std::shared_ptr<const folly::AsyncTransportCertificate> peerCertificate_;
  rocket::TypeErasedPtr userData_{};
};

} // namespace apache::thrift::fast_thrift::thrift
