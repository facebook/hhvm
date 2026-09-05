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

#include <utility>

#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace apache::thrift::fast_thrift::thrift::server {

namespace {

/**
 * A worker for contexts that belong to no `ThriftServer`.
 *
 * `Cpp2ConnContext::getWorker()` is non-null on every classic connection, and
 * handlers written against that assumption dereference it without checking.
 * One dummy for the process is enough: nothing reads per-worker state through
 * it, and it is immutable once built. Its server is null, so
 * `getWorker()->getServer()` still is — a handler reaching that far has to
 * guard, and there is no honest value to give it.
 */
const Cpp2Worker* dummyWorker() {
  static const std::shared_ptr<Cpp2Worker> worker =
      Cpp2Worker::createDummy(/*eventBase=*/nullptr, /*server=*/nullptr);
  return worker.get();
}

apache::thrift::ClientIdentityHook adaptResolver(
    const ThriftConnContext& connContext, PeerIdentityResolver resolver) {
  if (resolver == nullptr) {
    return nullptr;
  }
  // The certificate outlives the connection context's own lifetime by being
  // shared, and this hook is invoked during construction below, so capturing
  // the raw pointer is bounded by that call.
  return [cert = connContext.getPeerCertificate(), resolver](
             const folly::AsyncTransport* /*transport*/,
             X509* /*x509*/,
             const folly::SocketAddress& peerAddress) {
    return resolver(cert, peerAddress);
  };
}

} // namespace

Cpp2ConnContextAdapter::Cpp2ConnContextAdapter(
    boost::intrusive_ptr<ThriftConnContext> connContext,
    PeerIdentityResolver resolver)
    : ftConnContext_(std::move(connContext)) {
  // Built into the connection's slot rather than held here, so a consumer
  // written against the classic context reads it there for as long as the
  // connection lasts.
  cpp2ConnContext_ = std::make_unique<apache::thrift::Cpp2ConnContext>(
      &ftConnContext_->getPeerAddress(),
      ftConnContext_->getTransport(),
      /*manager=*/nullptr,
      // What the peer proved, from the handshake snapshot rather than the
      // transport: after a StopTLS downgrade the transport has no certificate
      // to extract, and this is the only record left. When the transport does
      // still carry one the constructor prefers it, and the two are the same
      // certificate.
      folly::OpenSSLTransportCertificate::tryExtractX509(
          ftConnContext_->getPeerCertificate()),
      adaptResolver(*ftConnContext_, resolver),
      dummyWorker(),
      /*numServiceInterceptors=*/0);

  // Stamped after construction, not taken from the transport: a downgraded
  // transport reports no protocol, so letting the constructor derive it would
  // erase what was negotiated. Only reachable through the internal API.
  apache::thrift::detail::Cpp2ConnContextInternalAPI internalAPI(
      *cpp2ConnContext_);
  internalAPI.setSecurityProtocol(
      std::string(ftConnContext_->getSecurityProtocol()));
  // The setup handler latched this off the client's setup; a handler reading
  // it goes to the classic context, so it has to be carried across.
  if (const auto* metadata = ftConnContext_->getClientMetadata()) {
    internalAPI.setClientMetadata(*metadata);
  }
  // Resolved by the constructor above, from the hook or the certificate.
  ftConnContext_->setPeerIdentities(cpp2ConnContext_->getPeerIdentities());
}

Cpp2ConnContextAdapter::~Cpp2ConnContextAdapter() {
  ftConnContext_->setPeerIdentities(nullptr);
}

Cpp2RequestContextAdapter::Cpp2RequestContextAdapter(
    apache::thrift::Cpp2RequestContext& cpp2RequestContext,
    apache::thrift::transport::THeader& header,
    ThriftRequestContext& requestContext)
    : header_(header),
      requestContext_(requestContext),
      cpp2RequestContext_(&cpp2RequestContext),
      ambientContext_(std::make_shared<folly::RequestContext>()) {
  // Published on the request for its whole lifetime. Missing registration is a
  // wiring error, so setState aborts instead of silently exposing a null
  // classic context.
  requestContext_.setState<Cpp2BridgeExtension>(cpp2RequestContext_);

  // Copied, not moved: the request keeps its own headers for the service to
  // read, and the classic context gets them for as long as the request lasts,
  // so a consumer reading through either finds them at any point.
  header_.setReadHeaders(
      apache::thrift::transport::THeader::StringToStringMap(
          requestContext_.getHeaders()));
}

apache::thrift::transport::THeader::StringToStringMap
Cpp2RequestContextAdapter::takeWriteHeaders() {
  return header_.releaseWriteHeaders();
}

} // namespace apache::thrift::fast_thrift::thrift::server
