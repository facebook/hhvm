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

#include <thrift/lib/cpp2/server/Cpp2Worker.h>

#include <vector>

#include <folly/GLog.h>
#include <folly/Overload.h>
#include <folly/String.h>
#include <folly/experimental/io/AsyncIoUringSocketFactory.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBaseLocal.h>
#include <folly/io/async/fdsock/AsyncFdSocket.h>
#include <folly/portability/Sockets.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/server/Cpp2Connection.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/peeking/PeekingManager.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <wangle/acceptor/EvbHandshakeHelper.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>
#include <wangle/acceptor/UnencryptedAcceptorHandshakeHelper.h>

// DANGER: If you disable this overly broadly, this can completely break
// workloads that rely on passing FDs over Unix sockets + Thrift.
THRIFT_FLAG_DEFINE_bool(enable_server_async_fd_socket, /* default = */ true);
THRIFT_FLAG_DEFINE_bool(fizz_server_enable_inplace_decryption, false);

namespace apache {
namespace thrift {

namespace {
folly::LeakySingleton<folly::EventBaseLocal<RequestsRegistry>> registry;
} // namespace

void Cpp2Worker::initRequestsRegistry() {
  auto* evb = getEventBase();
  auto memPerReq = server_->getMaxDebugPayloadMemoryPerRequest();
  auto memPerWorker = server_->getMaxDebugPayloadMemoryPerWorker();
  auto maxFinished = server_->getMaxFinishedDebugPayloadsPerWorker();
  std::weak_ptr<Cpp2Worker> self_weak = shared_from_this();
  evb->runInEventBaseThread([=, self_weak = std::move(self_weak)]() {
    if (auto self = self_weak.lock()) {
      self->requestsRegistry_ = &registry.get().try_emplace(
          *evb, memPerReq, memPerWorker, maxFinished);
      self->workerProvider_ =
          detail::createIOWorkerProvider(evb, requestsRegistry_);
      // add mapping from evb to worker
      self->server_->evbToWorker_.emplace(*evb, this);
    }
  });
}

void Cpp2Worker::onNewConnection(
    folly::AsyncTransport::UniquePtr sock,
    const folly::SocketAddress* addr,
    const std::string& nextProtocolName,
    wangle::SecureTransportType secureTransportType,
    const wangle::TransportInfo& tinfo) {
  /**
   * Most commonly, we are called in a noexcept callback sequence, which means
   * that `onNewConnection` is effectively `noexcept`.
   *
   * Since `Cpp2Worker` delegates to a routing handler, and the
   * `TransportRoutingHandler::handleConnection` interface is not marked
   * `noexcept`, it is very easy for an implementation to introduce code that
   * may throw (e.g. calling `socket_->getPeerAddress(...)`), thus crashing
   * the process.
   *
   * Don't you simply love exceptions?
   */

  try {
    onNewConnectionThatMayThrow(
        std::move(sock), addr, nextProtocolName, secureTransportType, tinfo);
  } catch (...) {
    FB_LOG_EVERY_MS(WARNING, 1000)
        << "Cpp2Worker::onNewConnection(...) caught an unhandled exception: "
        << folly::exceptionStr(std::current_exception());
  }
}

void Cpp2Worker::onNewConnectionThatMayThrow(
    folly::AsyncTransport::UniquePtr sock,
    const folly::SocketAddress* addr,
    const std::string& nextProtocolName,
    wangle::SecureTransportType secureTransportType,
    const wangle::TransportInfo& tinfo) {
  // This is possible if the connection was accepted before stopListening()
  // call, but handshake was finished after stopCPUWorkers() call.
  if (stopping_) {
    return;
  }

  auto* observer = server_->getObserver();
  uint32_t maxConnection = server_->getMaxConnections();
  if (maxConnection > 0 &&
      (getConnectionManager()->getNumConnections() >=
       maxConnection / server_->getNumIOWorkerThreads())) {
    if (observer) {
      observer->connDropped();
      observer->connRejected();
    }
    return;
  }

  const auto& func = server_->getZeroCopyEnableFunc();
  if (func && sock) {
    sock->setZeroCopy(true);
    sock->setZeroCopyEnableFunc(func);
  }

  // Check the security protocol
  switch (secureTransportType) {
    // If no security, peek into the socket to determine type
    case wangle::SecureTransportType::NONE: {
      if (server_->preferIoUring() &&
          folly::AsyncIoUringSocketFactory::supports(sock->getEventBase())) {
        sock = folly::AsyncIoUringSocketFactory::create<
            folly::AsyncTransport::UniquePtr>(std::move(sock));
      }
      new TransportPeekingManager(
          shared_from_this(), *addr, tinfo, server_, std::move(sock));
      break;
    }
    case wangle::SecureTransportType::TLS:
      if (auto fizz =
              sock->getUnderlyingTransport<fizz::server::AsyncFizzServer>()) {
        fizz->setDecryptInplace(
            THRIFT_FLAG(fizz_server_enable_inplace_decryption));
      }
      // Use the announced protocol to determine the correct handler
      if (!nextProtocolName.empty()) {
        for (auto& routingHandler : *server_->getRoutingHandlers()) {
          if (routingHandler->canAcceptEncryptedConnection(nextProtocolName)) {
            VLOG(4) << "Cpp2Worker: Routing encrypted connection for protocol "
                    << nextProtocolName;
            routingHandler->handleConnection(
                getConnectionManager(),
                std::move(sock),
                addr,
                tinfo,
                shared_from_this());
            return;
          }
        }
      }
      new TransportPeekingManager(
          shared_from_this(), *addr, tinfo, server_, std::move(sock));
      break;
    default:
      LOG(ERROR) << "Unsupported Secure Transport Type";
      break;
  }
}

void Cpp2Worker::handleHeader(
    folly::AsyncTransport::UniquePtr sock,
    const folly::SocketAddress* addr,
    const wangle::TransportInfo& tinfo) {
  folly::AsyncSocket* underlying =
      sock->getUnderlyingTransport<folly::AsyncSocket>();
  auto fd = underlying ? underlying->getNetworkSocket().toFd() : -1;
  VLOG(4) << "Cpp2Worker: Creating connection for socket " << fd;

  auto thriftTransport = createThriftTransport(std::move(sock));
  auto connection = std::make_shared<Cpp2Connection>(
      std::move(thriftTransport), addr, shared_from_this());
  Acceptor::addConnection(connection.get());
  connection->addConnection(connection, tinfo);
  connection->start();

  VLOG(4) << "Cpp2Worker: created connection for socket " << fd;

  auto observer = server_->getObserver();
  if (observer) {
    observer->activeConnections(
        getConnectionManager()->getNumConnections() *
        server_->getNumIOWorkerThreads());
  }
}

std::shared_ptr<folly::AsyncTransport> Cpp2Worker::createThriftTransport(
    folly::AsyncTransport::UniquePtr sock) {
  folly::AsyncSocket* tsock =
      sock->getUnderlyingTransport<folly::AsyncSocket>();
  if (tsock) {
    markSocketAccepted(tsock);
  }
  // use custom deleter for std::shared_ptr<folly::AsyncTransport> to allow
  // socket transfer from header to rocket (if enabled by ThriftFlags)
  return apache::thrift::transport::detail::convertToShared(std::move(sock));
}

void Cpp2Worker::markSocketAccepted(folly::AsyncSocket* sock) {
  sock->setShutdownSocketSet(server_->wShutdownSocketSet_);
}

void Cpp2Worker::plaintextConnectionReady(
    folly::AsyncSocket::UniquePtr sock,
    const folly::SocketAddress& clientAddr,
    wangle::TransportInfo& tinfo) {
  sock->setShutdownSocketSet(server_->wShutdownSocketSet_);
  new CheckTLSPeekingManager(
      shared_from_this(),
      clientAddr,
      tinfo,
      server_,
      std::move(sock),
      server_->getObserverShared());
}

folly::AsyncSocket::UniquePtr Cpp2Worker::makeNewAsyncSocket(
    folly::EventBase* base, int fd, const folly::SocketAddress* peerAddress) {
  if (THRIFT_FLAG(enable_server_async_fd_socket) &&
      peerAddress->getFamily() == AF_UNIX) {
    VLOG(4) << "Enabling AsyncFdSocket"; // peerAddress is always anonymous
    // Enable passing FDs over Unix sockets, see `man cmsg`.
    return folly::AsyncSocket::UniquePtr(new folly::AsyncFdSocket(
        base, folly::NetworkSocket::fromFd(fd), peerAddress));
  }
  return Acceptor::makeNewAsyncSocket(base, fd, peerAddress);
}

void Cpp2Worker::updateSSLStats(
    const folly::AsyncTransport* sock,
    std::chrono::milliseconds /* acceptLatency */,
    wangle::SSLErrorEnum error,
    const folly::exception_wrapper& /*ex*/) noexcept {
  if (!sock) {
    return;
  }

  auto observer = getServer()->getObserver();
  if (!observer) {
    return;
  }

  auto fizz = sock->getUnderlyingTransport<fizz::server::AsyncFizzServer>();
  if (fizz) {
    if (sock->good() && error == wangle::SSLErrorEnum::NO_ERROR) {
      observer->tlsComplete();
      auto pskType = fizz->getState().pskType();
      if (pskType && *pskType == fizz::PskType::Resumption) {
        observer->tlsResumption();
      }
      if (fizz->getPeerCertificate()) {
        observer->tlsWithClientCert();
      }
    } else {
      observer->tlsError();
    }
  } else {
    auto socket = sock->getUnderlyingTransport<folly::AsyncSSLSocket>();
    if (!socket) {
      return;
    }
    if (socket->good() && error == wangle::SSLErrorEnum::NO_ERROR) {
      observer->tlsComplete();
      if (socket->getSSLSessionReused()) {
        observer->tlsResumption();
      }
      if (socket->getPeerCertificate()) {
        observer->tlsWithClientCert();
      }
    } else {
      observer->tlsError();
    }
  }
}

wangle::AcceptorHandshakeHelper::UniquePtr Cpp2Worker::createSSLHelper(
    const std::vector<uint8_t>& bytes,
    const folly::SocketAddress& clientAddr,
    std::chrono::steady_clock::time_point acceptTime,
    wangle::TransportInfo& tInfo) {
  if (accConfig_.fizzConfig.enableFizz) {
    auto helper =
        fizzPeeker_.getThriftHelper(bytes, clientAddr, acceptTime, tInfo);
    if (!helper) {
      return nullptr;
    }
    if (auto parametersContext = getThriftParametersContext(clientAddr)) {
      helper->setThriftParametersContext(
          folly::copy_to_shared_ptr(*parametersContext));
    }
    return helper;
  }
  return defaultPeekingCallback_.getHelper(
      bytes, clientAddr, acceptTime, tInfo);
}

bool Cpp2Worker::shouldPerformSSL(
    const std::vector<uint8_t>& bytes, const folly::SocketAddress& clientAddr) {
  auto sslPolicy = getSSLPolicy();
  if (sslPolicy == SSLPolicy::REQUIRED) {
    if (isPlaintextAllowedOnLoopback()) {
      // loopback clients may still be sending TLS so we need to ensure that
      // it doesn't appear that way in addition to verifying it's loopback.
      return !(
          clientAddr.isLoopbackAddress() && !TLSHelper::looksLikeTLS(bytes));
    }
    return true;
  } else {
    return sslPolicy != SSLPolicy::DISABLED && TLSHelper::looksLikeTLS(bytes);
  }
}

std::optional<ThriftParametersContext> Cpp2Worker::getThriftParametersContext(
    const folly::SocketAddress& clientAddr) {
  auto thriftConfigBase =
      folly::get_ptr(accConfig_.customConfigMap, "thrift_tls_config");
  if (!thriftConfigBase) {
    return std::nullopt;
  }
  assert(static_cast<ThriftTlsConfig*>((*thriftConfigBase).get()));
  auto thriftConfig = static_cast<ThriftTlsConfig*>((*thriftConfigBase).get());
  if (!thriftConfig->enableThriftParamsNegotiation) {
    return std::nullopt;
  }

  auto thriftParametersContext = ThriftParametersContext();
  thriftParametersContext.setUseStopTLS(
      clientAddr.getFamily() == AF_UNIX || thriftConfig->enableStopTLS ||
      **ThriftServer::enableStopTLS());
  return thriftParametersContext;
}

wangle::AcceptorHandshakeHelper::UniquePtr Cpp2Worker::getHelper(
    const std::vector<uint8_t>& bytes,
    const folly::SocketAddress& clientAddr,
    std::chrono::steady_clock::time_point acceptTime,
    wangle::TransportInfo& ti) {
  if (!shouldPerformSSL(bytes, clientAddr)) {
    return wangle::AcceptorHandshakeHelper::UniquePtr(
        new wangle::UnencryptedAcceptorHandshakeHelper());
  }
  return createSSLHelper(bytes, clientAddr, acceptTime, ti);
}

void Cpp2Worker::requestStop() {
  getEventBase()->runInEventBaseThreadAndWait([&] {
    if (isStopping()) {
      return;
    }
    cancelQueuedRequests();
    stopping_.store(true, std::memory_order_relaxed);
    server_->evbToWorker_.erase(*getEventBase());
    if (activeRequests_ == 0) {
      stopBaton_.post();
    }
  });
}

bool Cpp2Worker::waitForStop(std::chrono::steady_clock::time_point deadline) {
  if (!stopBaton_.try_wait_until(deadline)) {
    LOG(ERROR) << "Failed to join outstanding requests.";
    return false;
  }
  return true;
}

void Cpp2Worker::cancelQueuedRequests() {
  auto eb = getEventBase();
  eb->dcheckIsInEventBaseThread();
  for (auto& stub : requestsRegistry_->getActive()) {
    if (stub.stateMachine_.isActive() &&
        stub.stateMachine_.tryStopProcessing()) {
      stub.req_->sendQueueTimeoutResponse();
    }
  }
}

void Cpp2Worker::handleServerRequestRejection(
    const ServerRequest& serverRequest, ServerRequestRejection& reject) {
  auto errorCode = kAppOverloadedErrorCode;
  if (reject.applicationException().getType() ==
      TApplicationException::UNKNOWN_METHOD) {
    errorCode = kMethodUnknownErrorCode;
  }
  serverRequest.request()->sendErrorWrapped(
      folly::exception_wrapper(
          folly::in_place, std::move(reject).applicationException()),
      errorCode);
}

Cpp2Worker::ActiveRequestsGuard Cpp2Worker::getActiveRequestsGuard() {
  DCHECK(!isStopping() || activeRequests_);
  ++activeRequests_;
  return Cpp2Worker::ActiveRequestsGuard(this);
}

Cpp2Worker::PerServiceMetadata::FindMethodResult
Cpp2Worker::PerServiceMetadata::findMethod(std::string_view methodName) const {
  if (const auto* map =
          std::get_if<AsyncProcessorFactory::MethodMetadataMap>(&methods_)) {
    if (auto* m = folly::get_ptr(*map, methodName)) {
      DCHECK(m->get());
      return MetadataFound{**m};
    }
    return MetadataNotFound{};
  }
  if (const auto* wildcard =
          std::get_if<AsyncProcessorFactory::WildcardMethodMetadataMap>(
              &methods_)) {
    if (auto* m = folly::get_ptr(wildcard->knownMethods, methodName)) {
      DCHECK(m->get());
      return MetadataFound{**m};
    }
    return MetadataFound{*wildcard->wildcardMetadata};
  }

  LOG(FATAL) << "Invalid CreateMethodMetadataResult from service";
  folly::assume_unreachable();
}

std::shared_ptr<folly::RequestContext>
Cpp2Worker::PerServiceMetadata::getBaseContextForRequest(
    const Cpp2Worker::PerServiceMetadata::FindMethodResult& findMethodResult)
    const {
  if (const auto* found =
          std::get_if<PerServiceMetadata::MetadataFound>(&findMethodResult)) {
    return processorFactory_.getBaseContextForRequest(found->metadata);
  }
  return nullptr;
}

void Cpp2Worker::dispatchRequest(
    AsyncProcessor* processor,
    ResponseChannelRequest::UniquePtr request,
    SerializedCompressedRequest&& serializedCompressedRequest,
    const PerServiceMetadata::FindMethodResult& methodMetadataResult,
    protocol::PROTOCOL_TYPES protocolId,
    Cpp2RequestContext* cpp2ReqCtx,
    concurrency::ThreadManager* tm,
    server::ServerConfigs* serverConfigs) {
  auto eb = cpp2ReqCtx->getConnectionContext()
                ->getWorkerContext()
                ->getWorkerEventBase();
  try {
    if (auto* found = std::get_if<PerServiceMetadata::MetadataFound>(
            &methodMetadataResult);
        LIKELY(found != nullptr)) {
      if (serverConfigs->resourcePoolEnabled() &&
          !serverConfigs->resourcePoolSet().empty()) {
        if (!found->metadata.isWildcard() && !found->metadata.rpcKind) {
          std::string_view methodName = cpp2ReqCtx->getMethodName();
          AsyncProcessorHelper::sendUnknownMethodError(
              std::move(request), methodName);
          return;
        }

        auto priority = cpp2ReqCtx->getCallPriority();
        if (priority == concurrency::N_PRIORITIES) {
          priority = found->metadata.priority.value_or(concurrency::NORMAL);
        }
        cpp2ReqCtx->setRequestExecutionScope(
            serverConfigs->getRequestExecutionScope(cpp2ReqCtx, priority));

        ServerRequest serverRequest(
            std::move(request),
            std::move(serializedCompressedRequest),
            cpp2ReqCtx,
            protocolId,
            folly::RequestContext::saveContext(),
            processor,
            &found->metadata);

        // Once we remove the old code we'll move validateRpcKind to a helper.
        if (!found->metadata.isWildcard() &&
            !GeneratedAsyncProcessorBase::validateRpcKind(
                serverRequest.request(), *found->metadata.rpcKind)) {
          return;
        }

        SelectPoolResult poolResult =
            serverConfigs->resourcePoolSet().selectResourcePool(serverRequest);

        ResourcePool* resourcePool;

        if (auto resourcePoolHandle =
                std::get_if<std::reference_wrapper<const ResourcePoolHandle>>(
                    &poolResult)) {
          DCHECK(serverConfigs->resourcePoolSet().hasResourcePool(
              *resourcePoolHandle));
          resourcePool = &serverConfigs->resourcePoolSet().resourcePool(
              *resourcePoolHandle);
        } else if (
            auto* reject = std::get_if<ServerRequestRejection>(&poolResult)) {
          handleServerRequestRejection(serverRequest, *reject);
          return;
        } else {
          // std::monostate So a ResourcePool is picked based on default logic
          if (found->metadata.isWildcard() &&
              found->metadata.executorType ==
                  AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB) {
            // if this is a wildcard method enabled for using Sync path of
            // ResourcePool
            poolResult = ResourcePoolHandle::defaultSync();
          } else {
            poolResult = AsyncProcessorHelper::selectResourcePool(
                serverRequest, found->metadata);
            // poolResult can't be std::monostate
            DCHECK(!std::holds_alternative<std::monostate>(poolResult));
          }

          if (auto* reject = std::get_if<ServerRequestRejection>(&poolResult)) {
            handleServerRequestRejection(serverRequest, *reject);
            return;
          }

          auto resourcePoolHandle =
              std::get_if<std::reference_wrapper<const ResourcePoolHandle>>(
                  &poolResult);
          DCHECK(serverConfigs->resourcePoolSet().hasResourcePool(
              *resourcePoolHandle));
          resourcePool = &serverConfigs->resourcePoolSet().resourcePool(
              *resourcePoolHandle);
          // Allow the priority to override the default resource pool
          if (priority != concurrency::NORMAL &&
              resourcePoolHandle->get().index() ==
                  ResourcePoolHandle::kDefaultAsyncIndex) {
            resourcePool = &serverConfigs->resourcePoolSet()
                                .resourcePoolByPriority_deprecated(priority);
          }
        }

        auto executor = resourcePool->executor();
        apache::thrift::detail::ServerRequestHelper::setExecutor(
            serverRequest, executor ? &executor.value().get() : nullptr);
        apache::thrift::detail::ServerRequestHelper::setResourcePool(
            serverRequest, resourcePool);

        if (cpp2ReqCtx->getInteractionId()) {
          processor->processInteraction(std::move(serverRequest));
          return;
        }

        // This will be used to put the request on the right queue on the
        // executor
        apache::thrift::detail::ServerRequestHelper::setInternalPriority(
            serverRequest, folly::Executor::LO_PRI);

        auto result = resourcePool->accept(std::move(serverRequest));
        if (result) {
          auto errorCode = errorCodeFromTapplicationException(
              result.value().applicationException().getType());
          serverRequest.request()->sendErrorWrapped(
              folly::exception_wrapper(
                  folly::in_place,
                  std::move(std::move(result).value()).applicationException()),
              errorCode);
          return;
        }
      } else if (
          // wildcard metadata do not specify rpcKind
          (found->metadata.rpcKind ||
           (found->metadata.isWildcard() &&
            THRIFT_FLAG(allow_wildcard_process_via_execute_request))) &&
          // executorType is defaulted to UNKNOWN for wildcard metadata
          // so only processors that implement createMethodMetadata can
          // pass this test
          found->metadata.executorType !=
              AsyncProcessor::MethodMetadata::ExecutorType::UNKNOWN &&
          (found->metadata.interactionType ==
               AsyncProcessor::MethodMetadata::InteractionType::NONE ||
           found->metadata.isWildcard()) &&
          !cpp2ReqCtx->getInteractionId()) {
        if (found->metadata.executorType ==
                AsyncProcessor::MethodMetadata::ExecutorType::ANY &&
            tm) {
          cpp2ReqCtx->setRequestExecutionScope(
              serverConfigs->getRequestExecutionScope(
                  cpp2ReqCtx,
                  found->metadata.priority.value_or(concurrency::NORMAL)));
        }
        detail::ap::processViaExecuteRequest(
            processor,
            std::move(request),
            std::move(serializedCompressedRequest),
            found->metadata,
            protocolId,
            cpp2ReqCtx,
            tm);
      } else {
        processor->processSerializedCompressedRequestWithMetadata(
            std::move(request),
            std::move(serializedCompressedRequest),
            found->metadata,
            protocolId,
            cpp2ReqCtx,
            eb,
            tm);
      }
    } else if (std::holds_alternative<PerServiceMetadata::MetadataNotFound>(
                   methodMetadataResult)) {
      std::string_view methodName = cpp2ReqCtx->getMethodName();
      AsyncProcessorHelper::sendUnknownMethodError(
          std::move(request), methodName);
    } else {
      LOG(FATAL) << "Invalid PerServiceMetadata from Cpp2Worker";
    }
  } catch (...) {
    LOG(DFATAL) << "AsyncProcessor::process exception: "
                << folly::exceptionStr(std::current_exception());
  }
}

const std::string& Cpp2Worker::errorCodeFromTapplicationException(
    TApplicationException::TApplicationExceptionType exceptionType) {
  switch (exceptionType) {
    case TApplicationException::TApplicationExceptionType::
        TENANT_QUOTA_EXCEEDED:
      return kTenantQuotaExceededErrorCode;
    default:
      return kQueueOverloadedErrorCode;
  }
}

} // namespace thrift
} // namespace apache
