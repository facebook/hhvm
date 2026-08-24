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
#include <cstdint>
#include <string_view>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransportCertificate.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Connection-lifecycle callbacks for FastThriftServer extensions.
 *
 * An extension registered through FastServerModule::addThriftExtension may
 * implement any subset of these alongside — or instead of — the request and
 * response callbacks in ThriftExtension.h. One extension instance is
 * constructed per connection, so whatever an extension resolves during setup is
 * simply a member its later callbacks read; nothing needs threading through the
 * framework.
 *
 * Ordering, per connection:
 *   onConnectionAttempted    — the client's SETUP metadata has been parsed and
 *                              version-negotiated; the SETUP response has not
 *                              been built yet.
 *   onConnectionAnswering    — the SETUP response has been assembled and is on
 *                              its way out; the fields an extension owns can
 *                              still be set on it.
 *   onConnectionEstablished  — the SETUP response is on the write path; the
 *                              next thing on the wire is the first request.
 *   onConnectionClosed       — the connection has finished settling and no
 *                              handler callbacks remain in flight.
 *
 * The two setup-phase callbacks are rejection points and return a
 * ConnectionVerdict; a rejection reaches the client as a REJECTED_SETUP error
 * frame followed by a close. Rejecting at onConnectionAttempted
 * short-circuits: no SETUP response is sent and onConnectionEstablished does
 * not run. onConnectionClosed cannot reject and returns void; it runs only for
 * connections that reached onConnectionEstablished, so setup and teardown work
 * pair up without the extension tracking whether setup ran.
 */

/**
 * Outcome of an extension's connection-lifecycle callback: proceed with the
 * connection, or refuse it with a cause.
 */
class ConnectionVerdict {
 public:
  static ConnectionVerdict proceed() noexcept { return ConnectionVerdict(); }

  static ConnectionVerdict reject(folly::exception_wrapper cause) noexcept {
    return ConnectionVerdict(std::move(cause));
  }

  // noexcept despite allocating: constructing Ex can throw bad_alloc, which
  // terminates rather than unwinds. OOM ⇒ terminate is the pipeline-wide
  // contract, not a hazard introduced here.
  template <typename Ex, typename... Args>
  static ConnectionVerdict reject(Args&&... args) noexcept {
    return ConnectionVerdict(
        folly::make_exception_wrapper<Ex>(std::forward<Args>(args)...));
  }

  bool isRejected() const noexcept { return bool(cause_); }

  // Empty unless isRejected().
  const folly::exception_wrapper& cause() const& noexcept { return cause_; }
  folly::exception_wrapper&& cause() && noexcept { return std::move(cause_); }

 private:
  ConnectionVerdict() = default;
  explicit ConnectionVerdict(folly::exception_wrapper cause) noexcept
      : cause_(std::move(cause)) {}

  folly::exception_wrapper cause_;
};

/**
 * Read-only view of a connection, limited to what is true at every point in
 * its life. Valid only for the duration of the callback — do not retain it or
 * anything it hands out.
 */
class ThriftConnectionView {
 public:
  explicit ThriftConnectionView(const ThriftConnContext& connContext) noexcept
      : connContext_(connContext) {}

  ThriftConnectionView(const ThriftConnectionView&) = delete;
  ThriftConnectionView& operator=(const ThriftConnectionView&) = delete;
  ThriftConnectionView(ThriftConnectionView&&) = delete;
  ThriftConnectionView& operator=(ThriftConnectionView&&) = delete;
  ~ThriftConnectionView() = default;

  // Peer address of the accepted socket. Empty if it could not be read.
  const folly::SocketAddress& peerAddress() const noexcept {
    return connContext_.getPeerAddress();
  }

  // Negotiated TLS protocol ("TLS1.3", ...). Empty on plaintext.
  std::string_view securityProtocol() const noexcept {
    return connContext_.getSecurityProtocol();
  }

  // Peer's TLS leaf certificate, or null on plaintext.
  const folly::AsyncTransportCertificate* peerCertificate() const noexcept {
    return connContext_.getPeerCertificate();
  }

  // The opaque per-connection slot the embedder populated at accept time via
  // ThriftConnContext::setUserData, or null if it set none. This is where a
  // service hangs the state its per-connection decisions are made against.
  void* userData() const noexcept { return connContext_.getUserData(); }

 private:
  const ThriftConnContext& connContext_;
};

/**
 * Read-only view during the setup exchange: everything above, plus what the
 * client asked for and what was agreed with it. Only the setup-phase callbacks
 * receive one, because outside that exchange these fields do not exist.
 */
class ThriftSetupConnectionView : public ThriftConnectionView {
 public:
  explicit ThriftSetupConnectionView(const ConnectionSetupData& setup) noexcept
      : ThriftConnectionView(*setup.connContext), setup_(setup) {}

  // The thrift protocol version that will be agreed with this client. Derived
  // rather than read: the server settles on it downstream of every extension,
  // so at this point it has not been recorded anywhere yet. Zero when the
  // client's range does not overlap the server's, i.e. when the connection is
  // about to be refused over it.
  std::int32_t negotiatedVersion() const noexcept {
    return negotiateVersion(setup_.clientSetup).value_or(0);
  }

  // The client's SETUP metadata — client identity, requested interface kind,
  // QoS hints.
  const apache::thrift::RequestSetupMetadata& clientSetup() const noexcept {
    return setup_.clientSetup;
  }

 private:
  const ConnectionSetupData& setup_;
};

/**
 * Write access during the connection-attempted callback: every
 * ThriftSetupConnectionView accessor, plus whatever an extension may set on
 * the exchange itself before it is answered.
 *
 * Contributing to the answer is not done here — the response does not exist
 * yet at this point. It is assembled downstream of every extension and passes
 * back out on the write path, which is where onConnectionAnswering stamps it.
 * This type carries the inbound-settable fields; it has none today.
 */
class ThriftSetupConnectionMutator final : public ThriftSetupConnectionView {
 public:
  explicit ThriftSetupConnectionMutator(ConnectionSetupData& setup) noexcept
      : ThriftSetupConnectionView(setup), setup_(setup) {}

 private:
  ConnectionSetupData& setup_;
};

/**
 * Write access to the answer while it is on its way to the client.
 *
 * Handed to onConnectionAnswering, which runs as the SETUP response passes on
 * the write path — after the server has settled what it negotiated and before
 * the response is serialized. That is the only window in which an extension
 * can put something in front of the client, and the only point at which what
 * the server decided is visible.
 *
 * Extensions set the fields they own. The ones the server negotiated are
 * readable but not writable, so a contribution cannot disturb them.
 */
class ThriftSetupResponseMutator final {
 public:
  explicit ThriftSetupResponseMutator(
      apache::thrift::SetupResponse& response) noexcept
      : response_(response) {}

  // The thrift protocol version the server settled on for this connection.
  std::int32_t negotiatedVersion() const noexcept {
    return response_.version().value_or(0);
  }

  // Declare the server's security policy to the client. The client reads this
  // off the SETUP response to decide whether the server enforces the policies
  // it expects.
  void setSecurityPolicy(apache::thrift::SecurityPolicy policy) noexcept {
    response_.securityPolicy() = std::move(policy);
  }

 private:
  apache::thrift::SetupResponse& response_;
};

// === Concepts ===
//
// Each callback is independently optional; an extension implements the ones it
// needs and pays for nothing else. An extension implementing none of them is
// never linked into a connection event list at all.

// onConnectionAttempted comes in two forms, resolved the same way as the
// request callbacks: the mutator derives from the view, so a view-taking
// callback also accepts a mutator, and the adapter tests the view form first so
// each extension is handed exactly the access its signature asks for.

template <typename H>
concept HasConnectionAttemptedViewCallback =
    requires(H& h, const ThriftSetupConnectionView& v) {
      {
        h.onConnectionAttempted(v)
      } noexcept -> std::same_as<ConnectionVerdict>;
    };

template <typename H>
concept HasConnectionAttemptedMutatorCallback =
    requires(H& h, ThriftSetupConnectionMutator& m) {
      {
        h.onConnectionAttempted(m)
      } noexcept -> std::same_as<ConnectionVerdict>;
    };

template <typename H>
concept HasConnectionAttemptedCallback =
    HasConnectionAttemptedViewCallback<H> ||
    HasConnectionAttemptedMutatorCallback<H>;

template <typename H>
concept HasConnectionAnsweringCallback =
    requires(H& h, ThriftSetupResponseMutator& m) {
      { h.onConnectionAnswering(m) } noexcept -> std::same_as<void>;
    };

// Takes the plain connection view, not the setup one: by the time this fires
// the exchange is over and its fields are gone. What is left is the
// connection.
template <typename H>
concept HasConnectionEstablishedCallback =
    requires(H& h, const ThriftConnectionView& v) {
      {
        h.onConnectionEstablished(v)
      } noexcept -> std::same_as<ConnectionVerdict>;
    };

template <typename H>
concept HasConnectionClosedCallback =
    requires(H& h, const ThriftConnectionView& v) {
      { h.onConnectionClosed(v) } noexcept -> std::same_as<void>;
    };

/**
 * True iff H hooks the connection lifecycle at all. Such an extension observes
 * per-connection state, so the server requires enableRequestContext when one is
 * registered.
 */
template <typename H>
concept ThriftConnectionExtensionHandler =
    HasConnectionAttemptedCallback<H> || HasConnectionAnsweringCallback<H> ||
    HasConnectionEstablishedCallback<H> || HasConnectionClosedCallback<H>;

} // namespace apache::thrift::fast_thrift::thrift
