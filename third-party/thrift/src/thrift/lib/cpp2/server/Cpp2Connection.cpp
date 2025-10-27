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

#include <thrift/lib/cpp2/server/Cpp2Connection.h>

#include <algorithm>

#include <folly/Overload.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEventHelper.h>
#include <thrift/lib/cpp2/server/MonitoringMethodNames.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/core/SendCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>

THRIFT_FLAG_DEFINE_bool(server_header_reject_framed, true);
THRIFT_FLAG_DEFINE_bool(server_header_reject_unframed, true);
THRIFT_FLAG_DEFINE_bool(server_header_reject_all, true);

THRIFT_FLAG_DEFINE_int64(monitoring_over_header_logging_sample_rate, 1'000'000);

namespace apache::thrift {

using namespace std;

namespace {
// This is a SendCallback used for transport upgrade from header to rocket
class TransportUpgradeSendCallback : public MessageChannel::SendCallback {
 public:
  TransportUpgradeSendCallback(
      RocketRoutingHandler* rocketHandler,
      const std::shared_ptr<folly::AsyncTransport>& transport,
      const folly::SocketAddress* peerAddress,
      Cpp2Worker* cpp2Worker,
      Cpp2Connection* cpp2Conn,
      HeaderServerChannel* headerChannel)
      : rocketHandler_(rocketHandler),
        transport_(transport),
        peerAddress_(peerAddress),
        cpp2Worker_(cpp2Worker),
        cpp2Conn_(cpp2Conn),
        headerChannel_(headerChannel) {}

  void sendQueued() override {}

  void messageSent() override {
    // Close the channel, since the transport is transferring to rocket
    DCHECK(headerChannel_);
    // This should clear all but one shared references to the transport, so that
    // it can be stolen below and wrapped in a unique_ptr.
    headerChannel_->setCallback(nullptr);
    headerChannel_->setTransport(nullptr);
    headerChannel_->closeNow();
    DCHECK(transport_.use_count() == 1);

    // Only do upgrade if transport_ is the only one managing the socket.
    // Otherwise close the connection.
    if (transport_.use_count() == 1) {
      // Steal the transport from header channel
      auto uPtr =
          std::get_deleter<apache::thrift::transport::detail::ReleaseDeleter<
              folly::AsyncTransport,
              folly::DelayedDestruction::Destructor>>(transport_)
              ->stealPtr();

      // Let RocketRoutingHandler handle the connection from here
      rocketHandler_->handleConnection(
          cpp2Worker_->getConnectionManager(),
          std::move(uPtr),
          peerAddress_,
          wangle::TransportInfo(),
          cpp2Worker_->getWorkerShared());
    }
    DCHECK(cpp2Conn_);
    cpp2Conn_->stop();
  }

  void messageSendError(folly::exception_wrapper&& ew) override {
    VLOG(4) << "Failed to send rocket upgrade response: " << ew.what();
  }

 private:
  RocketRoutingHandler* rocketHandler_;
  const std::shared_ptr<folly::AsyncTransport>& transport_;
  const folly::SocketAddress* peerAddress_;
  Cpp2Worker* cpp2Worker_;
  Cpp2Connection* cpp2Conn_;
  HeaderServerChannel* headerChannel_;
};

std::shared_ptr<apache::thrift::AsyncProcessorFactory>
getProcessorFactoryOverride(folly::AsyncTransport& transport) {
  if (auto newConnectionContext =
          ThriftServer::extractNewConnectionContext(transport)) {
    return newConnectionContext->processorFactory;
  }
  return {};
}
} // namespace

Cpp2Connection::Cpp2Connection(
    const std::shared_ptr<folly::AsyncTransport>& transport,
    const folly::SocketAddress* address,
    std::shared_ptr<Cpp2Worker> worker)
    : processorFactoryOverride_(
          transport ? getProcessorFactoryOverride(*transport) : nullptr),
      processorFactory_(
          processorFactoryOverride_
              ? *processorFactoryOverride_
              : worker->getServer()->getDecoratedProcessorFactory()),
      serviceMetadata_(worker->getMetadataForService(
          processorFactory_, processorFactoryOverride_)),
      processor_(processorFactory_.getProcessor()),
      channel_(HeaderServerChannel::newChannel(transport)),
      worker_(std::move(worker)),
      context_(
          address,
          transport.get(),
          worker_->getServer()->getEventBaseManager(),
          nullptr,
          worker_->getServer()->getClientIdentityHook(),
          worker_.get(),
          worker_->getServer()->getServiceInterceptors().size()),
      transport_(transport),
      executor_(worker_->getServer()->getHandlerExecutor_deprecated().get()) {
  processor_->coalesceWithServerScopedLegacyEventHandlers(
      *worker_->getServer());
  if (worker_->getServer()->resourcePoolSet().empty()) {
    threadManager_ = worker_->getServer()->getThreadManager_deprecated();
  }
  context_.setTransportType(Cpp2ConnContext::TransportType::HEADER);

  if (auto* observer = worker_->getServer()->getObserver()) {
    channel_->setSampleRate(observer->getSampleRate());
  }

  for (const auto& handler : worker_->getServer()->getEventHandlersUnsafe()) {
    handler->newConnection(&context_);
  }
}

Cpp2Connection::~Cpp2Connection() {
  channel_.reset();
}

void Cpp2Connection::invokeServiceInterceptorsOnConnectionClosed() noexcept {
#if FOLLY_HAS_COROUTINES
  auto* server = worker_->getServer();
  const auto& serviceInterceptors = server->getServiceInterceptors();
  for (std::size_t i = 0; i < serviceInterceptors.size(); ++i) {
    ServiceInterceptorBase::ConnectionInfo connectionInfo{
        &context_,
        context_.getStorageForServiceInterceptorOnConnectionByIndex(i)};
    serviceInterceptors[i]->internal_onConnectionClosed(
        connectionInfo, server->getInterceptorMetricCallback());
  }
#endif // FOLLY_HAS_COROUTINES
}

void Cpp2Connection::stop() {
  invokeServiceInterceptorsOnConnectionClosed();

  for (const auto& handler : worker_->getServer()->getEventHandlersUnsafe()) {
    handler->connectionDestroyed(&context_);
  }

  if (getConnectionManager()) {
    getConnectionManager()->removeConnection(this);
  }
  context_.connectionClosed();

  for (auto req : activeRequests_) {
    VLOG(1) << "Task killed due to channel close: "
            << context_.getPeerAddress()->describe();
    if (!req->isOneway()) {
      req->cancelRequest();
      if (auto* observer = worker_->getServer()->getObserver()) {
        observer->taskKilled();
      }
    }
  }

  if (channel_) {
    channel_->setCallback(nullptr);

    // Release the socket to avoid long CLOSE_WAIT times
    channel_->closeNow();
  }

  if (connectionAdded_) {
    if (auto* observer = worker_->getServer()->getObserver()) {
      observer->connClosed(
          server::TServerObserver::ConnectionInfo(
              reinterpret_cast<uint64_t>(transport_.get()),
              context_.getSecurityProtocol()));
      connectionAdded_ = false;
    }
  }

  transport_.reset();

  this_.reset();
}

void Cpp2Connection::timeoutExpired() noexcept {
  // Only disconnect if there are no active requests. No need to set another
  // timeout here because it's going to be set when all the requests are
  // handled.
  if (activeRequests_.empty()) {
    disconnect("idle timeout");
  }
}

void Cpp2Connection::disconnect(const char* comment) noexcept {
  // This must be the last call, it may delete this.
  auto guard = folly::makeGuard([&] { stop(); });

  VLOG(1) << "ERROR: Disconnect: " << comment
          << " on channel: " << context_.getPeerAddress()->describe();
  if (auto* observer = worker_->getServer()->getObserver()) {
    observer->connDropped();
  }
}

void Cpp2Connection::setServerHeaders(
    transport::THeader::StringToStringMap& writeHeaders) {
  if (getWorker()->isStopping()) {
    writeHeaders["connection"] = "goaway";
  }
}

void Cpp2Connection::setServerHeaders(
    HeaderServerChannel::HeaderRequest& request) {
  auto& writeHeaders = request.getHeader()->mutableWriteHeaders();
  setServerHeaders(writeHeaders);
  const auto& readHeaders = request.getHeader()->getHeaders();
  // set load header
  auto ptr = folly::get_ptr(readHeaders, THeader::QUERY_LOAD_HEADER);
  if (ptr) {
    auto load = getWorker()->getServer()->getLoad(*ptr);
    writeHeaders[THeader::QUERY_LOAD_HEADER] = folly::to<std::string>(load);
  }

  // set secondary load header
  auto ptrSl =
      folly::get_ptr(readHeaders, THeader::QUERY_SECONDARY_LOAD_HEADER);
  if (ptrSl) {
    auto load = getWorker()->getServer()->getLoad(*ptrSl);
    writeHeaders[THeader::QUERY_SECONDARY_LOAD_HEADER] =
        folly::to<std::string>(load);
  }
}

void Cpp2Connection::requestTimeoutExpired() {
  VLOG(1) << "ERROR: Task expired on channel: "
          << context_.getPeerAddress()->describe();
  if (auto* observer = worker_->getServer()->getObserver()) {
    observer->taskTimeout();
  }
}

void Cpp2Connection::queueTimeoutExpired() {
  VLOG(1) << "ERROR: Queue timeout on channel: "
          << context_.getPeerAddress()->describe();
  if (auto* observer = worker_->getServer()->getObserver()) {
    observer->queueTimeout();
  }
}

bool Cpp2Connection::pending() {
  return transport_ ? transport_->isPending() : false;
}

void Cpp2Connection::handleAppError(
    std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
    const std::string& name,
    const std::string& message,
    bool isClientError) {
  static const std::string headerEx = "uex";
  static const std::string headerExWhat = "uexw";
  req->getHeader()->setHeader(headerEx, name);
  req->getHeader()->setHeader(headerExWhat, message);
  killRequest(
      std::move(req),
      TApplicationException::UNKNOWN,
      isClientError ? kAppClientErrorCode : kAppServerErrorCode,
      message.c_str());
}

void Cpp2Connection::killRequest(
    std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
    TApplicationException::TApplicationExceptionType reason,
    const std::string& errorCode,
    const char* comment) {
  DCHECK(
      reason != TApplicationException::TApplicationExceptionType::LOADSHEDDING)
      << "Use killRequestServerOverloaded instead";
  VLOG(1) << "ERROR: Task killed: " << comment << ": "
          << context_.getPeerAddress()->getAddressStr();

  auto server = worker_->getServer();
  if (auto* observer = server->getObserver()) {
    observer->taskKilled();
  }

  // Nothing to do for Thrift oneway request.
  if (req->isOneway()) {
    return;
  }

  setServerHeaders(*req);

  req->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(reason, comment),
      errorCode);
}

void Cpp2Connection::killRequestServerOverloaded(
    std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
    OverloadResult&& overloadResult) {
  auto server = worker_->getServer();
  if (auto* observer = server->getObserver()) {
    observer->serverOverloaded(overloadResult.loadShedder);
  }

  // Nothing to do for Thrift oneway request.
  if (req->isOneway()) {
    return;
  }

  setServerHeaders(*req);

  req->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::TApplicationExceptionType::LOADSHEDDING,
          std::move(overloadResult.errorMessage)),
      std::move(overloadResult.errorCode));
}

// Response Channel callbacks
void Cpp2Connection::requestReceived(
    unique_ptr<HeaderServerChannel::HeaderRequest>&& hreq) {
  auto& samplingStatus = hreq->getSamplingStatus();
  std::chrono::steady_clock::time_point readEnd{
      std::chrono::steady_clock::now()};

  folly::call_once(clientInfoFlag_, [&] {
    if (const auto& m = hreq->getHeader()->extractClientMetadata()) {
      context_.setClientMetadata(*m);
    }
  });

  if (hreq->getHeader()->getClientType() == THRIFT_HTTP_SERVER_TYPE ||
      hreq->getHeader()->getClientType() == THRIFT_HTTP_CLIENT_TYPE ||
      hreq->getHeader()->getClientType() == THRIFT_HTTP_GET_CLIENT_TYPE) {
    disconnect("Rejecting HTTP connection over Header");
    return;
  }
  if (THRIFT_FLAG(server_header_reject_framed) &&
      (hreq->getHeader()->getClientType() == THRIFT_FRAMED_DEPRECATED ||
       hreq->getHeader()->getClientType() == THRIFT_FRAMED_COMPACT)) {
    disconnect("Rejecting framed connection over Header");
    return;
  }
  if (THRIFT_FLAG(server_header_reject_unframed) &&
      (hreq->getHeader()->getClientType() == THRIFT_UNFRAMED_DEPRECATED ||
       hreq->getHeader()->getClientType() ==
           THRIFT_UNFRAMED_COMPACT_DEPRECATED)) {
    disconnect("Rejecting unframed connection over Header");
    return;
  }

  auto protoId = static_cast<apache::thrift::protocol::PROTOCOL_TYPES>(
      hreq->getHeader()->getProtocolId());
  auto msgBegin = apache::thrift::detail::ap::deserializeMessageBegin(
      *hreq->getBuf(), protoId);

  if (!msgBegin.metadata.isValid) {
    disconnect("Rejecting unparseable message begin");
    return;
  }

  std::string& methodName = msgBegin.methodName;
  const auto& meta = msgBegin.metadata;

  // If the transport upgrade has begun, we should reject any requests made
  // before it's completed.
  if (upgradeToRocketCallback_ != nullptr) {
    // In essence, we've already swapped to Rocket even if the current message
    // was still in the buffer. Handle this as a protocol error, as a
    // misbehaving client.
    disconnect("Unexpected Header message after Rocket upgrade");
    return;
  }
  // Transport upgrade: check if client requested transport upgrade from header
  // to rocket. If yes, check if we support Rocket, then reply immediately and
  // upgrade the transport after sending the reply.
  if (methodName == "upgradeToRocket") {
    auto handlers = worker_->getServer()->getRoutingHandlers();
    auto rocketHandlerIterator =
        std::find_if(handlers->begin(), handlers->end(), [](auto& handler) {
          return dynamic_cast<RocketRoutingHandler*>(handler.get()) != nullptr;
        });
    if (rocketHandlerIterator == handlers->end()) {
      killRequest(
          std::move(hreq),
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          kMethodUnknownErrorCode,
          "Rocket upgrade attempted but RocketRoutingHandler not found");
      return;
    }
    // We assume the referenced RocketRoutingHandler object will remain alive
    // while we need it, but there's nothing guaranteeing that.
    apache::thrift::RocketRoutingHandler* rocketHandler =
        dynamic_cast<RocketRoutingHandler*>(rocketHandlerIterator->get());
    ResponsePayload response;
    switch (protoId) {
      case apache::thrift::protocol::T_BINARY_PROTOCOL:
        response = upgradeToRocketReply<apache::thrift::BinaryProtocolWriter>(
            meta.seqId);
        break;
      case apache::thrift::protocol::T_COMPACT_PROTOCOL:
        response = upgradeToRocketReply<apache::thrift::CompactProtocolWriter>(
            meta.seqId);
        break;
      default:
        LOG(DFATAL) << "Unsupported protocol found";
        // This should never occur. Unsupported protocols should be caught and
        // handled higher up in the stack. If we somehow still reach this, and
        // the protocol is neither binary nor compact, disconnect the client and
        // abort the upgrade.
        disconnect("Unsupported protocol found during Rocket upgrade");
        return;
    }

    upgradeToRocketCallback_ = std::make_unique<TransportUpgradeSendCallback>(
        rocketHandler,
        transport_,
        context_.getPeerAddress(),
        getWorker(),
        this,
        channel_.get());
    hreq->sendReply(std::move(response), upgradeToRocketCallback_.get());
    return;
  }

  if ((worker_->getServer()->getLegacyTransport() ==
           ThriftServer::LegacyTransport::DEFAULT &&
       THRIFT_FLAG(server_header_reject_all)) ||
      worker_->getServer()->getLegacyTransport() ==
          ThriftServer::LegacyTransport::DISABLED) {
    THRIFT_CONNECTION_EVENT(connection_rejected.header).log(context_);

    disconnect("Rejecting Header connection");
    return;
  }

  using PerServiceMetadata = Cpp2Worker::PerServiceMetadata;
  const PerServiceMetadata::FindMethodResult methodMetadataResult =
      serviceMetadata_.findMethod(methodName);

  auto baseReqCtx =
      serviceMetadata_.getBaseContextForRequest(methodMetadataResult);
  auto rootid = worker_->getRequestsRegistry()->genRootId();
  auto reqCtx = baseReqCtx
      ? folly::RequestContext::copyAsRoot(*baseReqCtx, rootid)
      : std::make_shared<folly::RequestContext>(rootid);

  folly::RequestContextScopeGuard rctx(reqCtx);

  auto server = worker_->getServer();
  server->touchRequestTimestamp();

  auto* observer = server->getObserver();
  if (observer) {
    observer->receivedRequest(&methodName);
  }

  auto injectedFailure = server->maybeInjectFailure();
  switch (injectedFailure) {
    case ThriftServer::InjectedFailure::NONE:
      break;
    case ThriftServer::InjectedFailure::ERROR:
      killRequest(
          std::move(hreq),
          TApplicationException::TApplicationExceptionType::INJECTED_FAILURE,
          kInjectedFailureErrorCode,
          "injected failure");
      return;
    case ThriftServer::InjectedFailure::DROP:
      VLOG(1) << "ERROR: injected drop: "
              << context_.getPeerAddress()->getAddressStr();
      return;
    case ThriftServer::InjectedFailure::DISCONNECT:
      disconnect("injected failure");
      return;
  }

  if (server->getGetHeaderHandler()) {
    server->getGetHeaderHandler()(hreq->getHeader(), context_.getPeerAddress());
  }

  if (auto overloadResult =
          server->checkOverload(hreq->getHeader()->getHeaders(), methodName)) {
    killRequestServerOverloaded(std::move(hreq), std::move(*overloadResult));
    return;
  }

  if (auto preprocessResult = server->preprocess(
          {hreq->getHeader()->getHeaders(), methodName, context_});
      !std::holds_alternative<std::monostate>(preprocessResult)) {
    folly::variant_match(
        preprocessResult,
        [&](AppClientException& ace) {
          handleAppError(std::move(hreq), ace.name(), ace.getMessage(), true);
        },
        [&](AppOverloadedException& aoe) {
          killRequestServerOverloaded(
              std::move(hreq),
              OverloadResult{
                  kAppOverloadedErrorCode,
                  aoe.getMessage(),
                  LoadShedder::CUSTOM,
              });
        },
        [&](AppQuotaExceededException& aqe) {
          killRequest(
              std::move(hreq),
              TApplicationException::TENANT_QUOTA_EXCEEDED,
              kTenantQuotaExceededErrorCode,
              aqe.getMessage().c_str());
        },
        [&](AppServerException& ase) {
          handleAppError(std::move(hreq), ase.name(), ase.getMessage(), false);
        },
        [](std::monostate&) { folly::assume_unreachable(); });
    return;
  }

  if (worker_->isStopping()) {
    killRequest(
        std::move(hreq),
        TApplicationException::TApplicationExceptionType::INTERNAL_ERROR,
        kQueueOverloadedErrorCode,
        "server shutting down");
    return;
  }

  if (!server->shouldHandleRequestForMethod(methodName)) {
    killRequest(
        std::move(hreq),
        TApplicationException::TApplicationExceptionType::INTERNAL_ERROR,
        kQueueOverloadedErrorCode,
        "server not ready");
    return;
  }

  // After this, the request buffer is no longer owned by the request
  // and will be released after deserializeRequest.
  auto serializedRequest = [&] {
    folly::IOBufQueue bufQueue;
    bufQueue.append(hreq->extractBuf());
    bufQueue.trimStart(meta.size);
    if (bufQueue.empty()) {
      // If the request only contained metadata, trimStart could clear the
      // entire queue. Return an empty IOBuf if that happens (move() otherwise
      // returns nullptr).
      return SerializedRequest(folly::IOBuf::create(0));
    }
    return SerializedRequest(bufQueue.move());
  }();

  // We keep a clone of the request payload buffer for debugging purposes, but
  // the lifetime of payload should not necessarily be the same as its request
  // object's.
  auto debugPayload =
      rocket::Payload::makeCombined(serializedRequest.buffer->clone(), 0);

  std::chrono::milliseconds queueTimeout;
  std::chrono::milliseconds taskTimeout;
  std::chrono::milliseconds clientQueueTimeout =
      hreq->getHeader()->getClientQueueTimeout();
  std::chrono::milliseconds clientTimeout =
      hreq->getHeader()->getClientTimeout();
  auto differentTimeouts = server->getTaskExpireTimeForRequest(
      clientQueueTimeout, clientTimeout, queueTimeout, taskTimeout);

  context_.setClientType(hreq->getHeader()->getClientType());

  auto t2r = RequestsRegistry::makeRequest<Cpp2Request>(
      *server,
      std::move(hreq),
      std::move(reqCtx),
      this_,
      std::move(debugPayload),
      std::move(methodName));

  server->incActiveRequests();
  // Expensive operations; happens only when sampling is enabled
  auto& timestamps = t2r->getTimestamps();
  timestamps.setStatus(samplingStatus);
  timestamps.readEnd = readEnd;
  timestamps.processBegin = std::chrono::steady_clock::now();
  if (samplingStatus.isServerSamplingEnabled() && observer) {
    if (threadManager_) {
      observer->queuedRequests(threadManager_->pendingUpstreamTaskCount());
    } else if (!server->resourcePoolSet().empty()) {
      observer->queuedRequests(server->resourcePoolSet().numQueued());
    }
    observer->activeRequests(server->getActiveRequests());
  }

  activeRequests_.insert(t2r.get());

  auto reqContext = t2r->getContext();
  if (observer) {
    observer->admittedRequest(&reqContext->getMethodName());
  }

  if (differentTimeouts) {
    if (queueTimeout > std::chrono::milliseconds(0)) {
      scheduleTimeout(&t2r->queueTimeout_, queueTimeout);
    }
  }
  if (taskTimeout > std::chrono::milliseconds(0)) {
    scheduleTimeout(&t2r->taskTimeout_, taskTimeout);
  }

  if (clientTimeout > std::chrono::milliseconds::zero()) {
    reqContext->setRequestTimeout(clientTimeout);
  } else {
    reqContext->setRequestTimeout(taskTimeout);
  }

  // Log monitoring methods that are called over header interface so that they
  // can be migrated to rocket monitoring interface.
  LoggingSampler monitoringLogSampler{
      THRIFT_FLAG(monitoring_over_header_logging_sample_rate)};
  if (monitoringLogSampler.isSampled()) {
    if (isMonitoringMethodName(reqContext->getMethodName())) {
      THRIFT_CONNECTION_EVENT(monitoring_over_header)
          .logSampled(context_, monitoringLogSampler, [&] {
            return folly::dynamic::object(
                "method_name", reqContext->getMethodName());
          });
    }
  }

  ResponseChannelRequest::UniquePtr req = std::move(t2r);
  if (!apache::thrift::detail::ap::setupRequestContextWithMessageBegin(
          meta, protoId, req, reqContext, worker_->getEventBase())) {
    return;
  }

  if (!std::get_if<PerServiceMetadata::MetadataNotFound>(
          &methodMetadataResult)) {
    logSetupConnectionEventsOnce(setupLoggingFlag_, context_);
  }

  Cpp2Worker::dispatchRequest(
      processorFactory_,
      processor_.get(),
      std::move(req),
      SerializedCompressedRequest(std::move(serializedRequest)),
      methodMetadataResult,
      protoId,
      reqContext,
      threadManager_.get(),
      worker_->getServer());
}

void Cpp2Connection::channelClosed(folly::exception_wrapper&& ex) {
  // This must be the last call, it may delete this.
  auto guard = folly::makeGuard([&] { stop(); });

  VLOG(4) << "Channel " << context_.getPeerAddress()->describe()
          << " closed: " << ex.what();
}

void Cpp2Connection::removeRequest(Cpp2Request* req) {
  activeRequests_.erase(req);
  if (activeRequests_.empty()) {
    resetTimeout();
  }
}

Cpp2Connection::Cpp2Request::Cpp2Request(
    ColocatedConstructionParams colocationParams,
    apache::thrift::ThriftServer& server,
    std::unique_ptr<HeaderServerChannel::HeaderRequest> req,
    std::shared_ptr<folly::RequestContext> rctx,
    std::shared_ptr<Cpp2Connection> con,
    rocket::Payload&& debugPayload,
    std::string&& methodName)
    : req_(std::move(req)),
      connection_(std::move(con)),
      // Note: tricky ordering here; see the note on connection_ in the class
      // definition.
      reqContext_(
          &connection_->context_,
          req_->getHeader(),
          std::move(methodName),
          std::move(colocationParams.data.interceptorStorage),
          std::move(colocationParams.data.decoratorDataStorage)),
      stateMachine_(
          util::includeInRecentRequestsCount(reqContext_.getMethodName()),
          server.getAdaptiveConcurrencyController(),
          server.getCPUConcurrencyController()),
      activeRequestsGuard_(connection_->getWorker()->getActiveRequestsGuard()) {
  new (colocationParams.debugStubToInit) RequestsRegistry::DebugStub(
      *connection_->getWorker()->getRequestsRegistry(),
      *this,
      reqContext_,
      std::move(rctx),
      protocol::PROTOCOL_TYPES(req_->getHeader()->getProtocolId()),
      std::move(debugPayload),
      stateMachine_);
  queueTimeout_.request_ = this;
  taskTimeout_.request_ = this;
}

MessageChannel::SendCallback* Cpp2Connection::Cpp2Request::prepareSendCallback(
    MessageChannel::SendCallback* sendCallback,
    apache::thrift::server::TServerObserver* observer) {
  // If we are sampling this call, wrap it with a Cpp2Sample, which also
  // implements MessageChannel::SendCallback. Callers of sendReply/sendError
  // are responsible for cleaning up their own callbacks.
  MessageChannel::SendCallback* cb = sendCallback;
  auto& timestamps = getTimestamps();
  if (stateMachine_.getStartedProcessing() &&
      timestamps.getSamplingStatus().isServerSamplingEnabled()) {
    // Cpp2Sample will delete itself when it's callback is called.
    cb = new Cpp2Sample(timestamps, observer, sendCallback);
  }
  return cb;
}

void Cpp2Connection::Cpp2Request::sendReply(
    ResponsePayload&& response,
    MessageChannel::SendCallback* sendCallback,
    folly::Optional<uint32_t>) {
  if (tryCancel()) {
    connection_->setServerHeaders(*req_);
    markProcessEnd();
    auto* server = connection_->getWorker()->getServer();
    const auto& serverConfig = server->getThriftServerConfig();
    auto maxResponseWriteTime = serverConfig.getMaxResponseWriteTime().get();
    auto maxResponseSize = serverConfig.getMaxResponseSize().get();
    auto* observer = server->getObserver();

    if (maxResponseWriteTime > std::chrono::milliseconds{0}) {
      sendCallback = new ResponseWriteTimeoutSendCallback(
          *connection_, maxResponseWriteTime, sendCallback);
    }

    if (maxResponseSize != 0 && response.length() > maxResponseSize) {
      req_->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::TApplicationExceptionType::INTERNAL_ERROR,
              "Response size too big"),
          kResponseTooBigErrorCode,
          reqContext_.getMethodName(),
          reqContext_.getProtoSeqId(),
          prepareSendCallback(sendCallback, observer));
    } else {
      req_->sendReply(
          std::move(response), prepareSendCallback(sendCallback, observer));
    }
    cancelTimeout();
    if (observer) {
      observer->sentReply();
    }
  }
}

void Cpp2Connection::Cpp2Request::sendException(
    ResponsePayload&& response, MessageChannel::SendCallback* sendCallback) {
  if (tryCancel()) {
    connection_->setServerHeaders(*req_);
    markProcessEnd();
    auto* observer = connection_->getWorker()->getServer()->getObserver();
    auto maxResponseSize =
        connection_->getWorker()->getServer()->getMaxResponseSize();
    if (maxResponseSize != 0 && response.length() > maxResponseSize) {
      req_->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              TApplicationException::TApplicationExceptionType::INTERNAL_ERROR,
              "Response size too big"),
          kResponseTooBigErrorCode,
          reqContext_.getMethodName(),
          reqContext_.getProtoSeqId(),
          prepareSendCallback(sendCallback, observer));
    } else {
      req_->sendException(
          std::move(response), prepareSendCallback(sendCallback, observer));
    }
    cancelTimeout();
    if (observer) {
      observer->sentReply();
    }
  }
}

void Cpp2Connection::Cpp2Request::sendErrorWrapped(
    folly::exception_wrapper ew, std::string exCode) {
  if (tryCancel()) {
    connection_->setServerHeaders(*req_);
    markProcessEnd();
    auto* observer = connection_->getWorker()->getServer()->getObserver();
    req_->sendErrorWrapped(
        std::move(ew),
        std::move(exCode),
        reqContext_.getMethodName(),
        reqContext_.getProtoSeqId(),
        prepareSendCallback(nullptr, observer));
    cancelTimeout();
  }
}

void Cpp2Connection::Cpp2Request::sendTimeoutResponse(
    HeaderServerChannel::HeaderRequest::TimeoutResponseType responseType) {
  if (!tryCancel()) {
    // Timeout was not properly cancelled when request was previously
    // cancelled
    DCHECK(false);
  }
  auto* observer = connection_->getWorker()->getServer()->getObserver();
  transport::THeader::StringToStringMap headers;
  connection_->setServerHeaders(headers);
  markProcessEnd();
  req_->sendTimeoutResponse(
      reqContext_.getMethodName(),
      reqContext_.getProtoSeqId(),
      prepareSendCallback(nullptr, observer),
      headers,
      responseType);
  cancelTimeout();
}

void Cpp2Connection::Cpp2Request::sendQueueTimeoutResponse(
    bool /*interactionIsTerminated*/) {
  sendTimeoutResponse(
      HeaderServerChannel::HeaderRequest::TimeoutResponseType::QUEUE);
  connection_->queueTimeoutExpired();
}

void Cpp2Connection::Cpp2Request::TaskTimeout::timeoutExpired() noexcept {
  request_->sendTimeoutResponse(
      HeaderServerChannel::HeaderRequest::TimeoutResponseType::TASK);
  request_->connection_->requestTimeoutExpired();
}

void Cpp2Connection::Cpp2Request::QueueTimeout::timeoutExpired() noexcept {
  if (request_->stateMachine_.tryStopProcessing()) {
    request_->sendQueueTimeoutResponse();
  }
}

void Cpp2Connection::Cpp2Request::markProcessEnd() {
  auto& timestamps = getTimestamps();
  timestamps.processEnd = std::chrono::steady_clock::now();
}

void Cpp2Connection::Cpp2Request::setLatencyHeader(
    const std::string& key,
    const std::string& value,
    transport::THeader::StringToStringMap* newHeaders) const {
  // newHeaders is used timeout exceptions, where req->header cannot be
  // mutated.
  if (newHeaders) {
    (*newHeaders)[key] = value;
  } else {
    req_->getHeader()->setHeader(key, value);
  }
}

Cpp2Connection::Cpp2Request::~Cpp2Request() {
  connection_->removeRequest(this);
  cancelTimeout();
  connection_->getWorker()->getServer()->decActiveRequests();
}

void Cpp2Connection::Cpp2Request::cancelRequest() {
  if (tryCancel()) {
    cancelTimeout();
  }
}

Cpp2Connection::Cpp2Sample::Cpp2Sample(
    apache::thrift::server::TServerObserver::CallTimestamps& timestamps,
    apache::thrift::server::TServerObserver* observer,
    MessageChannel::SendCallback* chainedCallback)
    : timestamps_(timestamps),
      observer_(observer),
      chainedCallback_(chainedCallback) {
  DCHECK(observer != nullptr);
}

void Cpp2Connection::Cpp2Sample::sendQueued() {
  if (chainedCallback_ != nullptr) {
    chainedCallback_->sendQueued();
  }
  timestamps_.writeBegin = std::chrono::steady_clock::now();
}

void Cpp2Connection::Cpp2Sample::messageSent() {
  if (chainedCallback_ != nullptr) {
    chainedCallback_->messageSent();
  }
  timestamps_.writeEnd = std::chrono::steady_clock::now();
  delete this;
}

void Cpp2Connection::Cpp2Sample::messageSendError(
    folly::exception_wrapper&& e) {
  if (chainedCallback_ != nullptr) {
    chainedCallback_->messageSendError(std::move(e));
  }
  timestamps_.writeEnd = std::chrono::steady_clock::now();
  delete this;
}

Cpp2Connection::Cpp2Sample::~Cpp2Sample() {
  if (observer_) {
    observer_->callCompleted(timestamps_);
  }
}

} // namespace apache::thrift
