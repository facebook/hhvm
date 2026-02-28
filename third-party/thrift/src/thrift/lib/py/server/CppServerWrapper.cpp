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

#include <map>

#include <glog/logging.h>

#include <boost/python.hpp>

#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/SocketAddress.h>
#include <folly/Utility.h>

#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/py/server/CppContextData.h>
#include <wangle/ssl/SSLContextConfig.h>

using namespace apache::thrift;
using apache::thrift::ThriftServer;
using apache::thrift::concurrency::PosixThreadFactory;
using apache::thrift::concurrency::ThreadManager;
using apache::thrift::server::TConnectionContext;
using apache::thrift::server::TServerEventHandler;
using apache::thrift::server::TServerObserver;
using apache::thrift::transport::THeader;
using folly::SSLContext;
using wangle::SSLCacheOptions;
using wangle::SSLContextConfig;
using namespace boost::python;

static bool isPyFinalizing() noexcept {
// If less than 3.7 offer no additional protection
#if PY_VERSION_HEX <= 0x03070000
  return false;
#elif PY_VERSION_HEX < 0x030D0000
  return _Py_IsFinalizing();
#else
  return Py_IsFinalizing();
#endif
}

THRIFT_FLAG_DEFINE_bool(allow_resource_pools_in_cpp_server_wrapper, false);

namespace {

const std::string kHeaderEx = "uex";
const std::string kHeaderExWhat = "uexw";

object makePythonHeaders(
    const THeader::StringToStringMap& cppheaders,
    const Cpp2RequestContext* context) {
  object headers = dict();
  for (const auto& it : cppheaders) {
    headers[it.first] = it.second;
  }
  headers[(std::string)apache::thrift::THeader::CLIENT_TIMEOUT_HEADER] =
      folly::to<std::string>(
          std::chrono::milliseconds(context->getRequestTimeout()).count());
  return headers;
}

object makePythonList(const std::vector<std::string>& vec) {
  list result;
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    result.append(*it);
  }
  return std::move(result);
}

std::string getStringAttrSafe(object& pyObject, const char* attrName) {
  object val = pyObject.attr(attrName);
  if (val.is_none()) {
    return "";
  }
  return extract<std::string>(str(val));
}

template <class T>
T getIntAttr(object& pyObject, const char* attrName) {
  object val = pyObject.attr(attrName);
  return extract<T>(val);
}

} // namespace

class CallbackWrapper {
 public:
  void call(object obj) { callback_(obj); }

  void setCallback(folly::Function<void(object)>&& callback) {
    callback_ = std::move(callback);
  }

 private:
  folly::Function<void(object)> callback_;
};

class CppServerEventHandler : public TServerEventHandler {
 public:
  explicit CppServerEventHandler(object serverEventHandler)
      : handler_(std::make_shared<object>(serverEventHandler)) {}

  void newConnection(TConnectionContext* ctx) override {
    callPythonHandler(ctx, "newConnection");
  }

  void connectionDestroyed(TConnectionContext* ctx) override {
    callPythonHandler(ctx, "connectionDestroyed");
  }

 private:
  void callPythonHandler(TConnectionContext* ctx, const char* method) {
    if (!isPyFinalizing()) {
      PyGILState_STATE state = PyGILState_Ensure();
      SCOPE_EXIT {
        PyGILState_Release(state);
      };

      // This cast always succeeds because it is called from Cpp2Connection.
      Cpp2ConnContext* cpp2Ctx = dynamic_cast<Cpp2ConnContext*>(ctx);
      auto cd_cls = handler_->attr("CONTEXT_DATA");
      object contextData = cd_cls();
      extract<CppContextData&>(contextData)().copyContextContents(cpp2Ctx);
      auto ctx_cls = handler_->attr("CPP_CONNECTION_CONTEXT");
      object cppConnContext = ctx_cls(contextData);
      handler_->attr(method)(cppConnContext);
    }
  }

  std::shared_ptr<object> handler_;
};

class PythonCallTimestamps : public TServerObserver::CallTimestamps {
 public:
  void set_readEndNow() { readEnd = clock::now(); }
  uint64_t get_readEndUsec() const noexcept {
    return to_microseconds(readEnd.time_since_epoch());
  }
  void set_processBeginNow() { processBegin = clock::now(); }
  uint64_t get_processBeginUsec() const noexcept {
    return to_microseconds(processBegin.time_since_epoch());
  }
  void set_processEndNow() { processEnd = clock::now(); }
  uint64_t get_processEndUsec() const noexcept {
    return to_microseconds(processEnd.time_since_epoch());
  }
  void set_writeBeginNow() { writeBegin = clock::now(); }
  uint64_t get_writeBeginUsec() const noexcept {
    return to_microseconds(writeBegin.time_since_epoch());
  }
  void set_writeEndNow() { writeEnd = clock::now(); }
  uint64_t get_writeEndUsec() const noexcept {
    return to_microseconds(writeEnd.time_since_epoch());
  }
};

class CppServerObserver : public TServerObserver {
 public:
  explicit CppServerObserver(object serverObserver)
      : observer_(serverObserver) {}

  void connAccepted(
      const wangle::TransportInfo& /* info */,
      const TServerObserver::ConnectionInfo& /* connInfo */) override {
    this->call("connAccepted");
  }
  void connDropped() override { this->call("connDropped"); }
  void connRejected() override { this->call("connRejected"); }
  void tlsError() override { this->call("tlsError"); }
  void tlsComplete() override { this->call("tlsComplete"); }
  void tlsFallback() override { this->call("tlsFallback"); }
  void tlsResumption() override { this->call("tlsResumption"); }
  void taskKilled() override { this->call("taskKilled"); }
  void taskTimeout() override { this->call("taskTimeout"); }
  void serverOverloaded(apache::thrift::LoadShedder /*loadShedder*/) override {
    this->call("serverOverloaded");
  }
  void receivedRequest(const std::string* /*method*/) override {
    this->call("receivedRequest");
  }
  void admittedRequest(const std::string* /*method*/) override {
    this->call("admittedRequest");
  }
  void queuedRequests(int32_t n) override { this->call("queuedRequests", n); }
  void queueTimeout() override { this->call("queueTimeout"); }
  void sentReply() override { this->call("sentReply"); }
  void activeRequests(int32_t n) override { this->call("activeRequests", n); }
  void callCompleted(const CallTimestamps& runtimes) override {
    this->call(
        "callCompleted",
        reinterpret_cast<const PythonCallTimestamps&>(runtimes));
  }
  void tlsWithClientCert() override { this->call("tlsWithClientCert"); }

 private:
  template <class... Types>
  void call(const char* method_name, Types... args) {
    PyGILState_STATE state = PyGILState_Ensure();
    SCOPE_EXIT {
      PyGILState_Release(state);
    };

    // check if the object has an attribute, because we want to be accepting
    // if we added a new listener callback and didn't yet update call the
    // people using this interface.
    if (!PyObject_HasAttrString(observer_.ptr(), method_name)) {
      return;
    }

    try {
      (void)observer_.attr(method_name)(args...);
    } catch (const error_already_set&) {
      // print the error to sys.stderr and carry on, because raising here
      // would break the server protocol, and raising in Python later
      // would be extremely disconnected and confusing since it would
      // happen in apparently unconnected Python code.
      PyErr_Print();
    }
  }

  object observer_;
};

class PythonAsyncProcessor : public AsyncProcessor {
 public:
  explicit PythonAsyncProcessor(std::shared_ptr<object> adapter)
      : adapter_(adapter) {
    getPythonOnewayMethods();
  }

  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedCompressedRequest,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES protType,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager* tm) override {
    auto fname = context->getMethodName();
    bool oneway = isOnewayMethod(fname);

    if (oneway && !req->isOneway()) {
      req->sendReply(ResponsePayload{});
    }

    auto task = [=,
                 this,
                 reqCaptured = std::move(req),
                 serializedCompressedRequestCaptured =
                     std::move(serializedCompressedRequest),
                 protTypeCaptured = protType,
                 contextCaptured = context,
                 ebCaptured = eb]() mutable {
      runTask(
          std::move(reqCaptured),
          std::move(serializedCompressedRequestCaptured),
          protTypeCaptured,
          contextCaptured,
          ebCaptured,
          false /* fromExecuteRequest  */);
    };

    using PriorityThreadManager =
        apache::thrift::concurrency::PriorityThreadManager;
    auto ptm = dynamic_cast<PriorityThreadManager*>(tm);
    if (ptm != nullptr) {
      ptm->add(
          getMethodPriority(context),
          std::make_shared<apache::thrift::concurrency::FunctionRunner>(
              std::move(task)));
      return;
    }
    tm->add(std::move(task));
  }

  void executeRequest(
      apache::thrift::ServerRequest&& request,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&) override {
    using ServerRequestHelper = apache::thrift::detail::ServerRequestHelper;
    auto req = ServerRequestHelper::request(std::move(request));
    auto serializedCompressedRequest =
        ServerRequestHelper::compressedRequest(std::move(request));
    auto protType = ServerRequestHelper::protocol(request);
    auto context = request.requestContext();
    auto eb = ServerRequestHelper::eventBase(request);

    runTask(
        std::move(req),
        std::move(serializedCompressedRequest),
        protType,
        context,
        eb,
        true /* fromExecuteRequest  */);
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {
    LOG(FATAL)
        << "This AsyncProcessor doesn't support Thrift interactions. "
        << "Please implement processInteraction to support interactions.";
  }

  /**
   * Get the priority of the request
   * Check the headers directly in C++ since noone seems to override that logic
   * Ask python if no priority headers were supplied with the request
   */
  concurrency::PRIORITY getMethodPriority(Cpp2RequestContext* ctx) {
    if (ctx) {
      auto requestPriority = ctx->getCallPriority();
      if (requestPriority != concurrency::PRIORITY::N_PRIORITIES) {
        VLOG(3) << "Request priority from headers";
        return requestPriority;
      }
    }

    PyGILState_STATE state = PyGILState_Ensure();
    SCOPE_EXIT {
      PyGILState_Release(state);
    };

    try {
      auto fname = ctx->getMethodName();
      return static_cast<concurrency::PRIORITY>(
          extract<int>(adapter_->attr("get_priority")(fname))());
    } catch (error_already_set&) {
      // get_priority doesn't exist, or it threw an exception
      LOG(ERROR) << "Error while calling _ProcessorAdapter.get_priority()";
      PyErr_Print();
    }

    return concurrency::PRIORITY::NORMAL;
  }

 private:
  void runTask(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedCompressedRequest,
      apache::thrift::protocol::PROTOCOL_TYPES protType,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb,
      bool fromExecuteRequest) {
    auto fname = context->getMethodName();
    bool oneway = isOnewayMethod(fname);

    auto buf = apache::thrift::LegacySerializedRequest(
                   protType,
                   context->getProtoSeqId(),
                   context->getMethodName(),
                   std::move(serializedCompressedRequest).uncompress())
                   .buffer;

    SCOPE_EXIT {
      eb->runInEventBaseThread([req = std::move(req)]() mutable { req = {}; });
    };

    if (!fromExecuteRequest && !oneway && !req->getShouldStartProcessing()) {
      return;
    }

    folly::ByteRange input_range = buf->coalesce();
    auto input_data = const_cast<unsigned char*>(input_range.data());
    auto clientType = context->getHeader()->getClientType();

    {
      PyGILState_STATE state = PyGILState_Ensure();
      SCOPE_EXIT {
        PyGILState_Release(state);
      };

#if PY_MAJOR_VERSION == 2
      auto input =
          handle<>(PyBuffer_FromMemory(input_data, input_range.size()));
#else
      auto input = handle<>(PyMemoryView_FromMemory(
          reinterpret_cast<char*>(input_data), input_range.size(), PyBUF_READ));
#endif

      auto cd_ctor = adapter_->attr("CONTEXT_DATA");
      object contextData = cd_ctor();
      extract<CppContextData&>(contextData)().copyContextContents(context);

      auto cb_ctor = adapter_->attr("CALLBACK_WRAPPER");
      object callbackWrapper = cb_ctor();
      extract<CallbackWrapper&>(callbackWrapper)().setCallback(
          [oneway,
           req = std::move(req),
           context,
           eb = eb,
           contextData,
           protType](object output) mutable {
            // Make sure the request is deleted in evb.
            SCOPE_EXIT {
              eb->runInEventBaseThread(
                  [req = std::move(req)]() mutable { req = {}; });
            };

            // Always called from python so no need to grab GIL.
            try {
              std::unique_ptr<folly::IOBuf> outbuf;
              if (output.is_none()) {
                throw std::runtime_error(
                    "Unexpected error in processor method");
              }
              PyObject* output_ptr = output.ptr();
#if PY_MAJOR_VERSION == 2
              if (PyString_Check(output_ptr)) {
                int len = extract<int>(output.attr("__len__")());
                if (len == 0) {
                  return;
                }
                outbuf =
                    folly::IOBuf::copyBuffer(extract<const char*>(output), len);
              } else
#endif
                  if (PyBytes_Check(output_ptr)) {
                int len = PyBytes_Size(output_ptr);
                if (len == 0) {
                  return;
                }
                outbuf =
                    folly::IOBuf::copyBuffer(PyBytes_AsString(output_ptr), len);
              } else {
                throw std::runtime_error(
                    "Return from processor "
                    "method is not string or bytes");
              }

              if (!req->isActive()) {
                return;
              }
              CppContextData& cppContextData =
                  extract<CppContextData&>(contextData);
              if (!cppContextData.getHeaderEx().empty()) {
                context->getHeader()->setHeader(
                    kHeaderEx, cppContextData.getHeaderEx());
              }
              if (!cppContextData.getHeaderExWhat().empty()) {
                context->getHeader()->setHeader(
                    kHeaderExWhat, cppContextData.getHeaderExWhat());
              }
              auto response = LegacySerializedResponse{std::move(outbuf)};
              auto [mtype, payload] = std::move(response).extractPayload(
                  req->includeEnvelope(), protType);
              payload.transform(context->getHeader()->getWriteTransforms());
              eb->runInEventBaseThread(
                  [mtype = mtype,
                   req = std::move(req),
                   payload = std::move(payload)]() mutable {
                    if (mtype == MessageType::T_REPLY) {
                      req->sendReply(std::move(payload));
                    } else if (mtype == MessageType::T_EXCEPTION) {
                      req->sendException(std::move(payload));
                    } else {
                      LOG(ERROR) << "Invalid type. type=" << uint16_t(mtype);
                    }
                  });
            } catch (const std::exception& e) {
              if (!oneway) {
                req->sendErrorWrapped(
                    folly::make_exception_wrapper<TApplicationException>(
                        folly::to<std::string>(
                            "Failed to read response from Python:", e.what())),
                    "python");
              }
            }
          });

      adapter_->attr("call_processor")(
          input,
          makePythonHeaders(context->getHeader()->getHeaders(), context),
          int(clientType),
          int(protType),
          contextData,
          callbackWrapper);
    }
  }

  bool isOnewayMethod(const std::string& fname) {
    return onewayMethods_.find(fname) != onewayMethods_.end();
  }

  void getPythonOnewayMethods() {
    PyGILState_STATE state = PyGILState_Ensure();
    SCOPE_EXIT {
      PyGILState_Release(state);
    };
    object ret = adapter_->attr("oneway_methods")();
    if (ret.is_none()) {
      LOG(ERROR) << "Unexpected error in processor method";
      return;
    }
    tuple t = extract<tuple>(ret);
    for (int i = 0; i < len(t); i++) {
      onewayMethods_.insert(extract<std::string>(t[i]));
    }
  }

  std::shared_ptr<object> adapter_;
  std::unordered_set<std::string> onewayMethods_;
};

class PythonAsyncProcessorFactory : public AsyncProcessorFactory {
 public:
  explicit PythonAsyncProcessorFactory(std::shared_ptr<object> adapter)
      : adapter_(adapter) {}

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::make_unique<PythonAsyncProcessor>(adapter_);
  }

  // TODO(T89004867): Call onStartServing() and onStopServing() hooks for
  // non-C++ thrift servers
  std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
      override {
    return {};
  }

  CreateMethodMetadataResult createMethodMetadata() override {
    WildcardMethodMetadataMap wildcardMap;
    // python tasks will be run on executor
    wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>(
        MethodMetadata::ExecutorType::ANY);

    wildcardMap.knownMethods = {};

    return wildcardMap;
  }

 private:
  std::shared_ptr<object> adapter_;
};

class CppServerWrapper : public ThriftServer {
 public:
  CppServerWrapper() {
    ThriftServer::metadata().wrapper = "CppServerWrapper-py";
  }

  void setAdapter(object adapter) {
    // We use a shared_ptr to manage the adapter so the processor
    // factory handing won't ever try to manipulate python reference
    // counts without the GIL.
    setInterface(
        std::make_unique<PythonAsyncProcessorFactory>(
            std::make_shared<object>(adapter)));
  }

  // peer to setObserver, but since we want a different argument, avoid
  // shadowing in our parent class.
  void setObserverFromPython(object observer) {
    setObserver(std::make_shared<CppServerObserver>(observer));
  }

  object getAddress() { return makePythonAddress(ThriftServer::getAddress()); }

  void loop() {
    PyThreadState* save_state = PyEval_SaveThread();
    SCOPE_EXIT {
      PyEval_RestoreThread(save_state);
    };

    // Thrift main loop.  This will run indefinitely, until stop() is
    // called.

    if (auto sslContextObserver = ThriftServer::getSSLConfig()) {
      // This mirrors what we do in ThriftServer::serve to handle listening for
      // cert/key changes and reloading SSLContextConfigs.
      ThriftServer::getSSLCallbackHandle();
    }
    getServeEventBase()->loopForever();
  }

  void setup() {
    PyThreadState* save_state = PyEval_SaveThread();
    SCOPE_EXIT {
      PyEval_RestoreThread(save_state);
    };

    // This check is only useful for C++-based Thrift servers.
    ThriftServer::setAllowCheckUnimplementedExtraInterfaces(false);
    ThriftServer::setup();
  }

  void setCppSSLConfig(object sslConfig) {
    auto certPath = getStringAttrSafe(sslConfig, "cert_path");
    auto keyPath = getStringAttrSafe(sslConfig, "key_path");
    if (certPath.empty() ^ keyPath.empty()) {
      PyErr_SetString(
          PyExc_ValueError, "certPath and keyPath must both be populated");
      throw_error_already_set();
      return;
    }
    auto cfg = std::make_shared<SSLContextConfig>();
    cfg->clientCAFiles = std::vector<std::string>{
        getStringAttrSafe(sslConfig, "client_ca_path")};
    if (!certPath.empty()) {
      auto keyPwPath = getStringAttrSafe(sslConfig, "key_pw_path");
      cfg->setCertificate(certPath, keyPath, keyPwPath);
    }
    cfg->clientVerification =
        extract<SSLContext::VerifyClientCertificate>(sslConfig.attr("verify"));
    auto eccCurve = getStringAttrSafe(sslConfig, "ecc_curve_name");
    if (!eccCurve.empty()) {
      cfg->eccCurveName = eccCurve;
    }
    object sessionContext = sslConfig.attr("session_context");
    if (!sessionContext.is_none()) {
      cfg->sessionContext = extract<std::string>(str(sessionContext));
    }

    object sslVersionAttr = sslConfig.attr("ssl_version");
    if (!sslVersionAttr.is_none()) {
      cfg->sslVersion =
          extract<SSLContext::SSLVersion>(sslConfig.attr("ssl_version"));
    }

    ThriftServer::setSSLConfig(
        folly::observer::makeObserver(
            [cfg,
             nextProtocolsObserver = ThriftServer::defaultNextProtocols()] {
              auto cfgWithNextProtocols = *cfg;
              cfgWithNextProtocols.setNextProtocols(**nextProtocolsObserver);
              return cfgWithNextProtocols;
            }));

    setSSLPolicy(extract<SSLPolicy>(sslConfig.attr("ssl_policy")));

    auto ticketFilePath = getStringAttrSafe(sslConfig, "ticket_file_path");
    ThriftServer::watchTicketPathForChanges(ticketFilePath);
  }

  void setCppFastOpenOptions(object enabledObj, object tfoMaxQueueObj) {
    bool enabled{extract<bool>(enabledObj)};
    uint32_t tfoMaxQueue{extract<uint32_t>(tfoMaxQueueObj)};
    ThriftServer::setFastOpenOptions(enabled, tfoMaxQueue);
  }

  void useCppExistingSocket(int socket) {
    ThriftServer::useExistingSocket(socket);
  }

  void setCppSSLCacheOptions(object cacheOptions) {
    SSLCacheOptions options = {
        .sslCacheTimeout = std::chrono::seconds(
            getIntAttr<uint32_t>(cacheOptions, "ssl_cache_timeout_seconds")),
        .maxSSLCacheSize =
            getIntAttr<uint64_t>(cacheOptions, "max_ssl_cache_size"),
        .sslCacheFlushSize =
            getIntAttr<uint64_t>(cacheOptions, "ssl_cache_flush_size"),
        .handshakeValidity = std::chrono::seconds(
            getIntAttr<uint32_t>(
                cacheOptions, "ssl_handshake_validity_seconds")),
    };
    ThriftServer::setSSLCacheOptions(std::move(options));
  }

  object getCppTicketSeeds() {
    auto seeds = getTicketSeeds();
    if (!seeds) {
      return boost::python::object();
    }
    boost::python::dict result;
    result["old"] = makePythonList(seeds->oldSeeds);
    result["current"] = makePythonList(seeds->currentSeeds);
    result["new"] = makePythonList(seeds->newSeeds);
    return std::move(result);
  }

  void cleanUp() {
    // Deadlock avoidance: consider a thrift worker thread is doing
    // something in C++-land having relinquished the GIL.  This thread
    // acquires the GIL, stops the workers, and waits for the worker
    // threads to complete.  The worker thread now finishes its work,
    // and tries to reacquire the GIL, but deadlocks with the current
    // thread, which holds the GIL and is waiting for the worker to
    // complete.  So we do cleanUp() without the GIL, and reacquire it
    // only once thrift is all cleaned up.

    PyThreadState* save_state = PyEval_SaveThread();
    SCOPE_EXIT {
      PyEval_RestoreThread(save_state);
    };
    ThriftServer::cleanUp();
  }

  void setIdleTimeout(int timeout) {
    std::chrono::milliseconds ms(timeout);
    ThriftServer::setIdleTimeout(ms);
  }

  void setTaskExpireTime(int timeout) {
    std::chrono::milliseconds ms(timeout);
    ThriftServer::setTaskExpireTime(ms);
  }

  void setCppServerEventHandler(object serverEventHandler) {
    setServerEventHandler(
        std::make_shared<CppServerEventHandler>(serverEventHandler));
  }

  void setNewSimpleThreadManager(size_t count, size_t) {
    if (THRIFT_FLAG(allow_resource_pools_in_cpp_server_wrapper)) {
      setNumCPUWorkerThreads(count);
      setThreadManagerType(
          apache::thrift::ThriftServer::ThreadManagerType::SIMPLE);
    } else {
      auto tm = ThreadManager::newSimpleThreadManager(count);
      auto poolThreadName = getCPUWorkerThreadName();
      if (!poolThreadName.empty()) {
        tm->setNamePrefix(poolThreadName);
      }

      tm->threadFactory(std::make_shared<PosixThreadFactory>());
      tm->start();
      setThreadManager_deprecated(std::move(tm));
    }
  }

  void setNewPriorityQueueThreadManager(size_t numThreads) {
    if (THRIFT_FLAG(allow_resource_pools_in_cpp_server_wrapper)) {
      setNumCPUWorkerThreads(numThreads);
      setThreadManagerType(
          apache::thrift::ThriftServer::ThreadManagerType::PRIORITY_QUEUE);
    } else {
      auto tm = ThreadManager::newPriorityQueueThreadManager(numThreads);
      auto poolThreadName = getCPUWorkerThreadName();
      if (!poolThreadName.empty()) {
        tm->setNamePrefix(poolThreadName);
      }

      tm->threadFactory(std::make_shared<PosixThreadFactory>());
      tm->start();
      setThreadManager_deprecated(std::move(tm));
    }
  }

  void setNewPriorityThreadManager(
      size_t high_important,
      size_t high,
      size_t important,
      size_t normal,
      size_t best_effort,
      size_t) {
    if (THRIFT_FLAG(allow_resource_pools_in_cpp_server_wrapper)) {
      setThreadManagerType(
          apache::thrift::ThriftServer::ThreadManagerType::PRIORITY);
      setThreadManagerPoolSizes(
          {{high_important, high, important, normal, best_effort}});
    } else {
      auto tm = PriorityThreadManager::newPriorityThreadManager(
          {{high_important, high, important, normal, best_effort}});
      tm->enableCodel(getEnableCodel());
      auto poolThreadName = getCPUWorkerThreadName();
      if (!poolThreadName.empty()) {
        tm->setNamePrefix(poolThreadName);
      }

      tm->threadFactory(std::make_shared<PosixThreadFactory>());
      tm->start();
      setThreadManager_deprecated(std::move(tm));
    }
  }

  // this adapts from a std::shared_ptr, which boost::python does not (yet)
  // support, to a boost::shared_ptr, which it has internal support for.
  //
  // the magic is in the custom deleter which takes and releases a refcount on
  // the std::shared_ptr, instead of doing any local deletion.
  boost::shared_ptr<ThreadManager> getThreadManagerHelper() {
    auto ptr = this->getThreadManager_deprecated();
    return boost::shared_ptr<ThreadManager>(ptr.get(), [ptr](void*) {});
  }

  void setWorkersJoinTimeout(int seconds) {
    ThriftServer::setWorkersJoinTimeout(std::chrono::seconds(seconds));
  }

  void setNumIOWorkerThreads(size_t numIOWorkerThreads) {
    ThriftServer::setNumIOWorkerThreads(numIOWorkerThreads);
  }

  void setListenBacklog(int listenBacklog) {
    ThriftServer::setListenBacklog(listenBacklog);
  }

  void setMaxConnections(uint32_t maxConnections) {
    ThriftServer::setMaxConnections(maxConnections);
  }

  void setNumCPUWorkerThreads(size_t numCPUWorkerThreads) {
    ThriftServer::setNumCPUWorkerThreads(numCPUWorkerThreads);
  }

  void setWrapperName(object wrapperName) {
    ThriftServer::metadata().wrapper = extract<std::string>(str(wrapperName));
  }

  void setLanguageFrameworkName(object languageFrameworkName) {
    ThriftServer::metadata().languageFramework =
        extract<std::string>(str(languageFrameworkName));
  }

  void setUnixSocketPath(const char* path) {
    setAddress(folly::SocketAddress::makeFromPath(path));
  }
};

BOOST_PYTHON_MODULE(CppServerWrapper) {
  PyEval_InitThreads();

  class_<CppContextData>("CppContextData")
      .def("getClientIdentity", &CppContextData::getClientIdentity)
      .def("getPeerAddress", &CppContextData::getPeerAddress)
      .def("getLocalAddress", &CppContextData::getLocalAddress)
      .def("setHeaderEx", &CppContextData::setHeaderEx)
      .def("setHeaderExWhat", &CppContextData::setHeaderExWhat);

  class_<CallbackWrapper, boost::noncopyable>("CallbackWrapper")
      .def("call", &CallbackWrapper::call);

  class_<ThriftServer, boost::noncopyable>("ThriftServer");

  class_<CppServerWrapper, bases<ThriftServer>, boost::noncopyable>(
      "CppServerWrapper")
      // methods added or customized for the python implementation
      .def("setAdapter", &CppServerWrapper::setAdapter)
      .def(
          "setAddress",
          static_cast<void (CppServerWrapper::*)(const std::string&, uint16_t)>(
              &CppServerWrapper::setAddress))
      .def("setUnixSocketPath", &CppServerWrapper::setUnixSocketPath)
      .def("setObserver", &CppServerWrapper::setObserverFromPython)
      .def("setIdleTimeout", &CppServerWrapper::setIdleTimeout)
      .def("setTaskExpireTime", &CppServerWrapper::setTaskExpireTime)
      .def("getAddress", &CppServerWrapper::getAddress)
      .def("getPort", &CppServerWrapper::getPort)
      .def("loop", &CppServerWrapper::loop)
      .def("cleanUp", &CppServerWrapper::cleanUp)
      .def(
          "setCppServerEventHandler",
          &CppServerWrapper::setCppServerEventHandler)
      .def(
          "setNewSimpleThreadManager",
          &CppServerWrapper::setNewSimpleThreadManager,
          (arg("count"), arg("pendingTaskCountMax")))
      .def(
          "setNewPriorityQueueThreadManager",
          &CppServerWrapper::setNewPriorityQueueThreadManager,
          (arg("numThreads")))
      .def(
          "setNewPriorityThreadManager",
          &CppServerWrapper::setNewPriorityThreadManager,
          (arg("high_important"),
           arg("high"),
           arg("important"),
           arg("normal"),
           arg("best_effort"),
           arg("maxQueueLen") = 0))
      .def("setCppSSLConfig", &CppServerWrapper::setCppSSLConfig)
      .def("setCppSSLCacheOptions", &CppServerWrapper::setCppSSLCacheOptions)
      .def("setCppFastOpenOptions", &CppServerWrapper::setCppFastOpenOptions)
      .def("getCppTicketSeeds", &CppServerWrapper::getCppTicketSeeds)
      .def("setWorkersJoinTimeout", &CppServerWrapper::setWorkersJoinTimeout)
      .def("useCppExistingSocket", &CppServerWrapper::useCppExistingSocket)

      // methods directly passed to the C++ impl
      .def("setup", &CppServerWrapper::setup)
      .def("setNumCPUWorkerThreads", &CppServerWrapper::setNumCPUWorkerThreads)
      .def("setNumIOWorkerThreads", &CppServerWrapper::setNumIOWorkerThreads)
      .def("setListenBacklog", &CppServerWrapper::setListenBacklog)
      .def("setPort", &CppServerWrapper::setPort)
      .def("setReusePort", &CppServerWrapper::setReusePort)
      .def("stop", &CppServerWrapper::stop)
      .def("setMaxConnections", &CppServerWrapper::setMaxConnections)
      .def("getMaxConnections", &CppServerWrapper::getMaxConnections)
      .def("setEnabled", &CppServerWrapper::setEnabled)

      .def("getLoad", &CppServerWrapper::getLoad)
      .def("getActiveRequests", &CppServerWrapper::getActiveRequests)
      .def("getThreadManager", &CppServerWrapper::getThreadManagerHelper)
      .def("setWrapperName", &CppServerWrapper::setWrapperName)
      .def(
          "setLanguageFrameworkName",
          &CppServerWrapper::setLanguageFrameworkName);

  class_<ThreadManager, boost::shared_ptr<ThreadManager>, boost::noncopyable>(
      "ThreadManager", no_init)
      .def("idleWorkerCount", &ThreadManager::idleWorkerCount)
      .def("workerCount", &ThreadManager::workerCount)
      .def("pendingTaskCount", &ThreadManager::pendingTaskCount)
      .def("pendingUpstreamTaskCount", &ThreadManager::pendingUpstreamTaskCount)
      .def("totalTaskCount", &ThreadManager::totalTaskCount)
      .def("expiredTaskCount", &ThreadManager::expiredTaskCount)
      .def("clearPending", &ThreadManager::clearPending);

  class_<PythonCallTimestamps>("CallTimestamps")
      .def("getReadEnd", &PythonCallTimestamps::get_readEndUsec)
      .def("setReadEndNow", &PythonCallTimestamps::set_readEndNow)
      .def("getProcessBegin", &PythonCallTimestamps::get_processBeginUsec)
      .def("setProcessBeginNow", &PythonCallTimestamps::set_processBeginNow)
      .def("getProcessEnd", &PythonCallTimestamps::get_processEndUsec)
      .def("setProcessEndNow", &PythonCallTimestamps::set_processEndNow)
      .def("getWriteBegin", &PythonCallTimestamps::get_writeBeginUsec)
      .def("setWriteBeginNow", &PythonCallTimestamps::set_writeBeginNow)
      .def("getWriteEnd", &PythonCallTimestamps::get_writeEndUsec)
      .def("setWriteEndNow", &PythonCallTimestamps::set_writeEndNow);

  enum_<SSLPolicy>("SSLPolicy")
      .value("DISABLED", SSLPolicy::DISABLED)
      .value("PERMITTED", SSLPolicy::PERMITTED)
      .value("REQUIRED", SSLPolicy::REQUIRED);

  enum_<folly::SSLContext::VerifyClientCertificate>("VerifyClientCertificate")
      .value(
          "IF_PRESENTED",
          folly::SSLContext::VerifyClientCertificate::IF_PRESENTED)
      .value(
          "ALWAYS_VERIFY", folly::SSLContext::VerifyClientCertificate::ALWAYS)
      .value(
          "NONE_DO_NOT_REQUEST",
          folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST);

  enum_<folly::SSLContext::SSLVersion>("SSLVersion")
      .value("TLSv1_2", folly::SSLContext::SSLVersion::TLSv1_2);
}
