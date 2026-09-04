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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/event_handler/Cpp2ContextAdapter.h>

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ssl/BasicTransportCertificate.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace apache::thrift::fast_thrift::thrift::server {

namespace {

// Stands in for what a resolver produces: the opaque object
// Cpp2ConnContext::getPeerIdentities hands back.
struct ResolvedIdentities {
  std::string identity;
  const folly::AsyncTransportCertificate* certSeen{nullptr};
};

std::unique_ptr<void, void (*)(void*)> resolveIdentities(
    const folly::AsyncTransportCertificate* cert,
    const folly::SocketAddress& /*peerAddress*/) {
  auto owned = std::make_unique<ResolvedIdentities>();
  owned->identity = cert == nullptr ? "plaintext" : cert->getIdentity();
  owned->certSeen = cert;
  return std::unique_ptr<void, void (*)(void*)>(
      owned.release(),
      +[](void* p) noexcept { delete static_cast<ResolvedIdentities*>(p); });
}

boost::intrusive_ptr<ThriftConnContext> makeConnContext(
    std::shared_ptr<const folly::AsyncTransportCertificate> cert) {
  boost::intrusive_ptr<ThriftConnContext> conn{new ThriftConnContext()};
  conn->setPeerAddress(folly::SocketAddress("127.0.0.1", 4321));
  conn->setSecurityProtocol("TLS1.3");
  conn->setPeerCertificate(std::move(cert));
  return conn;
}

// The slot plan a server that installed the bridge builds at start(). Static so
// it outlives every request context below, as the server's own layout does.
const ExtensionLayout& bridgeLayout() {
  static const ExtensionLayout layout = [] {
    ExtensionLayoutBuilder builder;
    builder.add(Cpp2BridgeExtension::kId);
    return std::move(builder).build();
  }();
  return layout;
}

} // namespace

TEST(Cpp2ContextAdapterTest, ConnectionContextCarriesWhatTheConnectionKnows) {
  auto conn = makeConnContext(nullptr);

  Cpp2ConnContextAdapter adapter(conn, &resolveIdentities);

  EXPECT_EQ(
      *adapter.get().getPeerAddress(), folly::SocketAddress("127.0.0.1", 4321));
  EXPECT_EQ(adapter.get().getSecurityProtocol(), "TLS1.3");
}

// The resolver is handed the certificate itself, not an X509 extracted from
// it: a resumed session has identities but no X509, so extracting first would
// silently produce none.
TEST(Cpp2ContextAdapterTest, ResolverSeesTheCertificateAndItsResultIsCached) {
  auto cert = std::make_shared<folly::ssl::BasicTransportCertificate>(
      "peer.identity", nullptr);
  auto conn = makeConnContext(cert);

  Cpp2ConnContextAdapter adapter(conn, &resolveIdentities);

  auto* identities =
      static_cast<ResolvedIdentities*>(adapter.get().getPeerIdentities());
  ASSERT_NE(identities, nullptr);
  EXPECT_EQ(identities->identity, "peer.identity");
  EXPECT_EQ(identities->certSeen, cert.get());
}

TEST(Cpp2ContextAdapterTest, NoResolverLeavesIdentitiesUnset) {
  auto conn = makeConnContext(nullptr);

  Cpp2ConnContextAdapter adapter(conn, /*resolver=*/nullptr);

  EXPECT_EQ(adapter.get().getPeerIdentities(), nullptr);
}

// A handler that goes to the transport — for the local address, exported
// keying material, or the peer — finds the connection's own, not a null it
// has to tolerate.
TEST(Cpp2ContextAdapterTest, ConnectionContextCarriesTheTransport) {
  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb);
  auto conn = makeConnContext(nullptr);
  conn->setTransport(socket.get());

  Cpp2ConnContextAdapter adapter(conn, /*resolver=*/nullptr);

  EXPECT_EQ(adapter.get().getTransport(), socket.get());
}

// The protocol is stamped from the handshake snapshot, so a connection that
// downgraded still reports what it negotiated even though its transport can
// no longer say.
TEST(Cpp2ContextAdapterTest, SecurityProtocolSurvivesADowngradedTransport) {
  folly::EventBase evb;
  // Plaintext, exactly as StopTLS leaves it.
  auto socket = folly::AsyncSocket::newSocket(&evb);
  auto conn = makeConnContext(nullptr);
  conn->setTransport(socket.get());
  ASSERT_TRUE(socket->getSecurityProtocol().empty());

  Cpp2ConnContextAdapter adapter(conn, /*resolver=*/nullptr);

  EXPECT_EQ(adapter.get().getSecurityProtocol(), "TLS1.3");
}

// A handler that reports on the client goes to the classic context for it, so
// what the setup latched has to arrive there.
TEST(Cpp2ContextAdapterTest, ClientMetadataReachesTheClassicContext) {
  auto conn = makeConnContext(nullptr);
  apache::thrift::ClientMetadata metadata;
  metadata.hostname() = "client.host";
  conn->setClientMetadata(std::move(metadata));

  Cpp2ConnContextAdapter adapter(conn, /*resolver=*/nullptr);

  const auto seen = adapter.get().getClientMetadataRef();
  ASSERT_TRUE(seen.has_value());
  EXPECT_EQ(seen->getHostname(), "client.host");
}

// A connection whose client sent none leaves the classic context saying so,
// rather than handing back an empty record that reads as if it had one.
TEST(Cpp2ContextAdapterTest, NoClientMetadataLeavesTheClassicContextEmpty) {
  auto conn = makeConnContext(nullptr);

  Cpp2ConnContextAdapter adapter(conn, /*resolver=*/nullptr);

  EXPECT_FALSE(adapter.get().getClientMetadataRef().has_value());
}

// Handlers written against the classic server dereference getWorker() without
// checking; its server is a different matter and they have to guard.
TEST(Cpp2ContextAdapterTest, WorkerIsPresentButBelongsToNoServer) {
  auto conn = makeConnContext(nullptr);

  Cpp2ConnContextAdapter adapter(conn, /*resolver=*/nullptr);

  ASSERT_NE(adapter.get().getWorker(), nullptr);
  EXPECT_EQ(adapter.get().getWorker()->getServer(), nullptr);
}

TEST(Cpp2ContextAdapterTest, RequestContextCarriesHeadersMethodAndConnection) {
  auto conn = makeConnContext(nullptr);
  Cpp2ConnContextAdapter connAdapter(conn, /*resolver=*/nullptr);

  // Declared before the request: the classic context points at this, and the
  // request reaches the classic context, so it has to outlive both.
  transport::THeader header;
  ThriftRequestContext request;
  request.installExtensions(bridgeLayout());
  request.setConnectionContext(conn);
  request.setHeaders(ThriftRequestContext::HeaderMap{{"cat", "token"}});

  Cpp2RequestContext cpp2Request(&connAdapter.get(), &header, "ping");
  Cpp2RequestContextAdapter requestAdapter(cpp2Request, header, request);

  EXPECT_EQ(requestAdapter.get().getMethodName(), "ping");
  EXPECT_EQ(requestAdapter.get().getConnectionContext(), &connAdapter.get());
  ASSERT_NE(requestAdapter.get().getHeader(), nullptr);

  // The classic header gets a copy for the whole request, and the request
  // keeps its own: a consumer reading through either finds them.
  const auto& read = requestAdapter.get().getHeader()->getHeaders();
  ASSERT_TRUE(read.contains("cat"));
  EXPECT_EQ(read.at("cat"), "token");
  EXPECT_TRUE(request.getHeaders().contains("cat"));
}

// What a handler writes on the way out is collected once, for the response to
// carry.
TEST(Cpp2ContextAdapterTest, WriteHeadersAreHandedBack) {
  auto conn = makeConnContext(nullptr);
  Cpp2ConnContextAdapter connAdapter(conn, /*resolver=*/nullptr);
  ThriftRequestContext request;
  request.installExtensions(bridgeLayout());
  request.setConnectionContext(conn);

  transport::THeader header;
  Cpp2RequestContext cpp2Request(&connAdapter.get(), &header, "ping");
  Cpp2RequestContextAdapter requestAdapter(cpp2Request, header, request);
  requestAdapter.header().setHeader("minted", "sat");

  auto written = requestAdapter.takeWriteHeaders();

  ASSERT_TRUE(written.contains("minted"));
  EXPECT_EQ(written.at("minted"), "sat");
  EXPECT_TRUE(requestAdapter.takeWriteHeaders().empty());
}

// fast_thrift installs no ambient context, so each request brings its own
// rather than sharing the process-global default.
TEST(Cpp2ContextAdapterTest, EachRequestOwnsADistinctAmbientContext) {
  auto conn = makeConnContext(nullptr);
  Cpp2ConnContextAdapter connAdapter(conn, /*resolver=*/nullptr);
  ThriftRequestContext request;
  request.installExtensions(bridgeLayout());
  request.setConnectionContext(conn);

  transport::THeader firstHeader;
  transport::THeader secondHeader;
  Cpp2RequestContext firstCpp2(&connAdapter.get(), &firstHeader, "ping");
  Cpp2RequestContext secondCpp2(&connAdapter.get(), &secondHeader, "ping");
  Cpp2RequestContextAdapter first(firstCpp2, firstHeader, request);
  Cpp2RequestContextAdapter second(secondCpp2, secondHeader, request);

  ASSERT_NE(first.ambientContext(), nullptr);
  ASSERT_NE(second.ambientContext(), nullptr);
  EXPECT_NE(first.ambientContext(), second.ambientContext());
  EXPECT_NE(first.ambientContext().get(), folly::RequestContext::try_get());
}

// The identities the resolver produced are reachable natively, so a reader
// does not need the classic context to find out who the peer is.
TEST(Cpp2ContextAdapterTest, PeerIdentitiesAreReachableFromTheConnection) {
  auto cert = std::make_shared<folly::ssl::BasicTransportCertificate>(
      "peer.identity", nullptr);
  auto conn = makeConnContext(cert);
  EXPECT_EQ(conn->getPeerIdentities(), nullptr);

  {
    Cpp2ConnContextAdapter connAdapter(conn, &resolveIdentities);

    ASSERT_NE(conn->getPeerIdentities(), nullptr);
    EXPECT_EQ(conn->getPeerIdentities(), connAdapter.get().getPeerIdentities());
    EXPECT_EQ(
        static_cast<ResolvedIdentities*>(conn->getPeerIdentities())->identity,
        "peer.identity");
  }
  EXPECT_EQ(conn->getPeerIdentities(), nullptr);
}

// The classic context is reachable from the request for as long as its storage
// lasts, which is what lets a consumer read it after the bridge has moved on.
TEST(Cpp2ContextAdapterTest, ClassicContextIsReachableThroughTheRequest) {
  auto conn = makeConnContext(nullptr);
  Cpp2ConnContextAdapter connAdapter(conn, /*resolver=*/nullptr);
  transport::THeader header;
  ThriftRequestContext request;
  request.installExtensions(bridgeLayout());
  request.setConnectionContext(conn);
  EXPECT_EQ(tryGetCpp2RequestContext(request), nullptr);

  Cpp2RequestContext cpp2Request(&connAdapter.get(), &header, "ping");
  {
    Cpp2RequestContextAdapter requestAdapter(cpp2Request, header, request);
    EXPECT_EQ(tryGetCpp2RequestContext(request), &requestAdapter.get());
  }
  // Borrowed, not owned: the slot outlives the adapter and still names the
  // caller's context, which nothing has reclaimed yet.
  ASSERT_EQ(tryGetCpp2RequestContext(request), &cpp2Request);
  EXPECT_EQ(tryGetCpp2RequestContext(request)->getMethodName(), "ping");
}

// Releasing a state empties the slot rather than leaving it pointing at
// storage the next request rebuilds.
TEST(Cpp2ContextAdapterTest, ClearingTheSlotLeavesNothingToRead) {
  auto conn = makeConnContext(nullptr);
  Cpp2ConnContextAdapter connAdapter(conn, /*resolver=*/nullptr);
  transport::THeader header;
  ThriftRequestContext request;
  request.installExtensions(bridgeLayout());
  request.setConnectionContext(conn);

  Cpp2RequestContext cpp2Request(&connAdapter.get(), &header, "ping");
  Cpp2RequestContextAdapter requestAdapter(cpp2Request, header, request);
  ASSERT_NE(tryGetCpp2RequestContext(request), nullptr);

  requestAdapter.ftContext().setState<Cpp2BridgeExtension>(nullptr);

  EXPECT_EQ(tryGetCpp2RequestContext(request), nullptr);
}

} // namespace apache::thrift::fast_thrift::thrift::server
