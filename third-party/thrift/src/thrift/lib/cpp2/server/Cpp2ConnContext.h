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

#ifndef THRIFT_ASYNC_CPP2CONNCONTEXT_H_
#define THRIFT_ASYNC_CPP2CONNCONTEXT_H_ 1

#include <memory>
#include <string_view>

#include <folly/CancellationToken.h>
#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/server/TConnectionContext.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/util/TypeErasedStorage.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <wangle/ssl/SSLUtil.h>

using apache::thrift::concurrency::PriorityThreadManager;

namespace apache {
namespace thrift {

namespace rocket {
class ThriftRocketServerHandler;
}

using ClientIdentityHook = std::function<std::unique_ptr<void, void (*)(void*)>(
    const folly::AsyncTransport* transport,
    X509* cert,
    const folly::SocketAddress& peerAddress)>;

class RequestChannel;
class TClientBase;
class Cpp2Worker;
class IOWorkerContext;

class ClientMetadataRef {
 public:
  explicit ClientMetadataRef(const ClientMetadata& md) : md_(md) {}
  std::optional<std::string_view> getAgent();
  std::optional<std::string_view> getHostname();
  std::optional<std::string_view> getOtherMetadataField(std::string_view key);
  const folly::F14NodeMap<std::string, std::string>& getFields() const;

 private:
  const ClientMetadata& md_;
};

namespace detail {
using ConnectionInternalFieldsT =
    util::TypeErasedValue<64, alignof(std::max_align_t)>;
THRIFT_PLUGGABLE_FUNC_DECLARE(
    ConnectionInternalFieldsT, createPerConnectionInternalFields);
using RequestInternalFieldsT =
    util::TypeErasedValue<128, alignof(std::max_align_t)>;
THRIFT_PLUGGABLE_FUNC_DECLARE(
    RequestInternalFieldsT, createPerRequestInternalFields);

/*
 * Cache the exported keying material from the transport per connection
 * and per label (key). Note we expect the number of labels to be very small
 * Computing this per request may be too expensive which is why we cache this
 * here. This is stored as a vector for efficiency
 */
struct EkmInfo {
  std::string label;
  uint16_t length;
  std::unique_ptr<folly::IOBuf> buf{nullptr};
};

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::vector<EkmInfo>, populateCachedEkms, const folly::AsyncTransport&);
} // namespace detail

class Cpp2ConnContext : public apache::thrift::server::TConnectionContext {
  Cpp2ConnContext(
      const folly::SocketAddress* address,
      const folly::AsyncTransport* transport,
      folly::EventBaseManager* manager,
      std::shared_ptr<X509> peerCert,
      apache::thrift::ClientIdentityHook clientIdentityHook,
      const Cpp2Worker* worker,
      const IOWorkerContext* workerContext,
      detail::ConnectionInternalFieldsT internalFields)
      : transport_(transport),
        manager_(manager),
        worker_(worker),
        workerContext_(workerContext),
        internalFields_(std::move(internalFields)) {
    if (address) {
      transportInfo_.peerAddress = *address;
    }
    if (transport) {
      // require worker to be passed when wrapping a real connection
      DCHECK(worker != nullptr);
      DCHECK(workerContext != nullptr);
      transport->getLocalAddress(&transportInfo_.localAddress);
      if (auto osslCert = folly::OpenSSLTransportCertificate::tryExtractX509(
              transport->getPeerCertificate())) {
        peerCert = std::move(osslCert);
      }
      transportInfo_.securityProtocol = transport->getSecurityProtocol();
      transportInfo_.cachedEkms = detail::populateCachedEkms(*transport);

      if (transportInfo_.localAddress.getFamily() == AF_UNIX) {
        auto wrapper = transport->getUnderlyingTransport<folly::AsyncSocket>();
        if (wrapper) {
          peerCred_ = PeerCred::queryFromSocket(wrapper->getNetworkSocket());
        }
      }
    }
    transportInfo_.peerCertificate = std::move(peerCert);

    if (clientIdentityHook) {
      transportInfo_.peerIdentities = clientIdentityHook(
          transport,
          transportInfo_.peerCertificate.get(),
          transportInfo_.peerAddress);
    }
  }

 protected:
  /** DO NOT use this ctor to override internal fields if you are not sure
   * because this may result in a security SEV**/
  template <typename WorkerT>
  explicit Cpp2ConnContext(
      const folly::SocketAddress* address,
      const folly::AsyncTransport* transport,
      folly::EventBaseManager* manager,
      std::shared_ptr<X509> peerCert /*overridden from socket*/,
      apache::thrift::ClientIdentityHook clientIdentityHook,
      const WorkerT* worker,
      detail::ConnectionInternalFieldsT internalFields)
      : Cpp2ConnContext(
            address,
            transport,
            manager,
            std::move(peerCert),
            std::move(clientIdentityHook),
            worker,
            worker,
            std::move(internalFields)) {}

 public:
  enum class TransportType {
    HEADER,
    ROCKET,
    HTTP2,
  };

  explicit Cpp2ConnContext(
      const folly::SocketAddress* address = nullptr,
      const folly::AsyncTransport* transport = nullptr,
      folly::EventBaseManager* manager = nullptr,
      std::shared_ptr<X509> peerCert = nullptr /*overridden from socket*/,
      apache::thrift::ClientIdentityHook clientIdentityHook = nullptr)
      : Cpp2ConnContext(
            address,
            transport,
            manager,
            std::move(peerCert),
            std::move(clientIdentityHook),
            nullptr,
            nullptr,
            detail::createPerConnectionInternalFields()) {}

  template <typename WorkerT>
  explicit Cpp2ConnContext(
      const folly::SocketAddress* address,
      const folly::AsyncTransport* transport,
      folly::EventBaseManager* manager,
      std::shared_ptr<X509> peerCert /*overridden from socket*/,
      apache::thrift::ClientIdentityHook clientIdentityHook,
      const WorkerT* worker)
      : Cpp2ConnContext(
            address,
            transport,
            manager,
            std::move(peerCert),
            std::move(clientIdentityHook),
            worker,
            worker,
            detail::createPerConnectionInternalFields()) {}

  ~Cpp2ConnContext() override { DCHECK(tiles_.empty()); }
  Cpp2ConnContext(Cpp2ConnContext&&) = default;
  Cpp2ConnContext& operator=(Cpp2ConnContext&&) = default;

  void reset() {
    transportInfo_.peerAddress.reset();
    transportInfo_.localAddress.reset();
    userData_.reset();
  }

  const folly::SocketAddress* getPeerAddress() const final {
    return &transportInfo_.peerAddress;
  }

  const folly::SocketAddress* getLocalAddress() const {
    return &transportInfo_.localAddress;
  }

  void setLocalAddress(const folly::SocketAddress& localAddress) {
    transportInfo_.localAddress = localAddress;
  }

  void setRequestHeader(apache::thrift::transport::THeader* header) {
    header_ = header;
  }

  folly::EventBaseManager* getEventBaseManager() override { return manager_; }

  std::string getPeerCommonName() const {
    if (!transport_) {
      return "";
    }
    auto osslCert = dynamic_cast<const folly::OpenSSLTransportCertificate*>(
        transport_->getPeerCertificate());
    if (!osslCert) {
      return "";
    }
    return folly::ssl::OpenSSLUtils::getCommonName(osslCert->getX509().get());
  }

  virtual const std::string& getSecurityProtocol() const {
    return transportInfo_.securityProtocol;
  }

  virtual void* getPeerIdentities() const {
    return transportInfo_.peerIdentities.get();
  }

  const X509* FOLLY_NULLABLE getPeerCertificate() const {
    return transportInfo_.peerCertificate.get();
  }

  virtual const folly::AsyncTransport* getTransport() const {
    return transport_;
  }

  /*
   * The context may have prepopulated some cached ekms. This method
   * will check against the cache and return ekm on a hit. The caller is
   * expected to try to generate the ekm themselves on a cache miss.
   */
  std::unique_ptr<folly::IOBuf> getCachedEkm(
      folly::StringPiece label, uint16_t length) const {
    for (const auto& ekm : transportInfo_.cachedEkms) {
      if (ekm.label == label && ekm.length == length) {
        // We may have negatively cached (e.g. for a plaintext transport), this
        // is a valid value so return.
        return (ekm.buf != nullptr) ? ekm.buf->clone() : nullptr;
      }
    }
    return nullptr;
  }

  /**
   * Get the user data field.
   */
  void* getUserData() const override { return userData_.get(); }

  /**
   * Set the user data field.
   *
   * @param data         The new value for the user data field.
   *
   * @return Returns the old user data value.
   */
  void* setUserData(folly::erased_unique_ptr data) override {
    auto oldData = userData_.release();
    userData_ = std::move(data);
    return oldData;
  }
  using TConnectionContext::setUserData;

#ifndef _WIN32
  struct PeerEffectiveCreds {
    pid_t pid;
    uid_t uid;
    gid_t gid;
  };

  /**
   * Returns the connecting process ID, effective user ID, and effective user ID
   * of the unix socket peer. The connection may have terminated since that
   * time, so the PID may no longer map to a running process or the same process
   * that initially connected. Returns nullopt for TCP, on Windows, and if there
   * was an error retrieving the peer creds. In that case, call
   * `getPeerCredError` for the reason.
   *
   * On macOS, the pid field contains the effective pid. On Linux, there is no
   * distinction.
   */
  folly::Optional<PeerEffectiveCreds> getPeerEffectiveCreds() const {
    return peerCred_.getPeerEffectiveCreds();
  }
#endif

  /**
   * If the peer effective pid or uid are not available, it's possible
   * retrieving the information failed. Produce an error message with the
   * reason.
   */
  folly::Optional<std::string> getPeerCredError() const {
    return peerCred_.getError();
  }

  /**
   * Retrieve a new folly::CancellationToken that will be signaled when the
   * connection is closed.
   */
  folly::CancellationToken getCancellationToken() const {
    return cancellationSource_.getToken();
  }

  /**
   * Signal that the connection has been closed.
   *
   * This is intended to be called by the thrift server implementation code.
   *
   * Note that this will cause any CancellationCallback functions that have been
   * registered to run immediately in this thread.  If any of these callbacks
   * throw this will cause program termination.
   */
  void connectionClosed() { cancellationSource_.requestCancellation(); }

  const Cpp2Worker* getWorker() const { return worker_; }

  const IOWorkerContext* getWorkerContext() const { return workerContext_; }

  std::optional<TransportType> getTransportType() const {
    return transportType_;
  }

  std::optional<CLIENT_TYPE> getClientType() const { return clientType_; }

  std::optional<ClientMetadataRef> getClientMetadataRef() const {
    if (!clientMetadata_) {
      return {};
    }
    return ClientMetadataRef{*clientMetadata_};
  }

  template <class T>
  T& getInternalFields() noexcept {
    return internalFields_.value_unchecked<T>();
  }

  template <class T>
  const T& getInternalFields() const noexcept {
    return internalFields_.value_unchecked<T>();
  }

  InterfaceKind getInterfaceKind() const { return interfaceKind_; }

  /**
   * This is intended to be called by the thrift server implementation code.
   *
   * Replaces interaction if id is present in map.
   * Destroys passed-in tile otherwise.
   */
  void tryReplaceTile(int64_t id, TilePtr tile) {
    auto it = tiles_.find(id);
    if (it != tiles_.end()) {
      it->second = std::move(tile);
    }
  }

 private:
  /**
   * Adds interaction to interaction map
   * Returns false and destroys tile if id is in use
   */
  bool addTile(int64_t id, TilePtr tile) {
    return tiles_.try_emplace(id, std::move(tile)).second;
  }
  /**
   * Removes interaction from map
   * Returns old value
   */
  TilePtr removeTile(int64_t id) {
    auto it = tiles_.find(id);
    if (it == tiles_.end()) {
      return {};
    }
    auto ret = std::move(it->second);
    tiles_.erase(it);
    return ret;
  }
  /**
   * Gets tile from map
   * Throws std::out_of_range if not found
   */
  Tile& getTile(int64_t id) { return *tiles_.at(id); }
  friend class GeneratedAsyncProcessorBase;
  friend class Tile;
  friend class TilePromise;
  friend class HandlerCallbackBase;

  void setTransportType(TransportType transportType) {
    transportType_ = transportType;
  }

  void setClientType(CLIENT_TYPE clientType) { clientType_ = clientType; }

  void readSetupMetadata(const RequestSetupMetadata& meta) {
    if (const auto& md = meta.clientMetadata()) {
      setClientMetadata(*md);
    }
    if (auto interfaceKind = meta.interfaceKind()) {
      interfaceKind_ = *interfaceKind;
    }
  }

  void setClientMetadata(const ClientMetadata& md) { clientMetadata_ = md; }

  friend class Cpp2Connection;
  friend class rocket::ThriftRocketServerHandler;
  friend class HTTP2RoutingHandler;

  /**
   * Platform-independent representation of unix domain socket peer credentials,
   * e.g. ucred on Linux and xucred on macOS.
   *
   * Null implementation on Windows.
   */
  class PeerCred {
   public:
#ifndef _WIN32
    using StatusOrPid = pid_t;
#else
    // Even on Windows, differentiate between not initialized (not unix
    // domain socket), and unsupported platform.
    using StatusOrPid = int;
#endif

    /**
     * pid_t is guaranteed to be signed, so reserve non-positive values as
     * sentinels that indicate credential validity.
     * While negative pid_t values are possible, they are used to refer
     * to process groups and thus cannot occur in a process identifier.
     * Linux and macOS allow user IDs to span the entire range of a uint32_t,
     * so sentinal values must be stored in pid_t.
     */
    enum Validity : StatusOrPid {
      NotInitialized = -1,
      ErrorRetrieving = -2,
      UnsupportedPlatform = -3,
    };

    PeerCred() = default;
    PeerCred(const PeerCred&) = default;
    PeerCred& operator=(const PeerCred&) = default;

    /**
     * Query a socket for peer credentials.
     */
    static PeerCred queryFromSocket(folly::NetworkSocket socket);

#ifndef _WIN32
    folly::Optional<PeerEffectiveCreds> getPeerEffectiveCreds() const {
      return hasCredentials()
          ? folly::make_optional(PeerEffectiveCreds{pid_, uid_, gid_})
          : folly::none;
    }
#endif

    /**
     * If retrieving the effective credentials failed, return a string
     * containing the reason.
     */
    folly::Optional<std::string> getError() const;

   private:
    explicit PeerCred(Validity validity) : pid_{validity} {}

#ifndef _WIN32
    explicit PeerCred(pid_t pid, uid_t uid, gid_t gid)
        : pid_{pid}, uid_{uid}, gid_{gid} {}
#endif

    bool hasCredentials() const { return pid_ >= 0; }

    StatusOrPid pid_ = Validity::NotInitialized;
#ifndef _WIN32
    uid_t uid_ = 0;
    gid_t gid_ = 0;
#endif
  };

  folly::erased_unique_ptr userData_{folly::empty_erased_unique_ptr()};

  // Snapshot of info from AsyncTransport since this object may outlive it
  struct TransportInfo {
    folly::SocketAddress peerAddress;
    folly::SocketAddress localAddress;
    folly::erased_unique_ptr peerIdentities{folly::empty_erased_unique_ptr()};
    std::string securityProtocol;
    std::shared_ptr<X509> peerCertificate;
    std::vector<detail::EkmInfo> cachedEkms;
  } transportInfo_;
  const folly::AsyncTransport* transport_;

  folly::EventBaseManager* manager_;
  PeerCred peerCred_;
  // A CancellationSource that will be signaled when the connection is closed.
  folly::CancellationSource cancellationSource_;
  folly::F14FastMap<int64_t, TilePtr> tiles_;
  const Cpp2Worker* worker_;
  const IOWorkerContext* workerContext_;
  InterfaceKind interfaceKind_{InterfaceKind::USER};
  std::optional<TransportType> transportType_;
  std::optional<CLIENT_TYPE> clientType_;
  std::optional<ClientMetadata> clientMetadata_;
  detail::ConnectionInternalFieldsT internalFields_;
};

class Cpp2ClientRequestContext
    : public apache::thrift::server::TConnectionContext {
 public:
  explicit Cpp2ClientRequestContext(transport::THeader* header)
      : TConnectionContext(header) {}

  void setRequestHeader(transport::THeader* header) { header_ = header; }
};

// Request-specific context
class Cpp2RequestContext : public apache::thrift::server::TConnectionContext {
 protected:
  /** DO NOT use this ctor to override internal fields if you are not sure
   * because this may result in a security SEV**/
  Cpp2RequestContext(
      Cpp2ConnContext* ctx,
      detail::RequestInternalFieldsT internalFields,
      apache::thrift::transport::THeader* header = nullptr,
      std::string methodName = std::string{})
      : TConnectionContext(header),
        ctx_(ctx),
        methodName_(std::move(methodName)),
        internalFields_(std::move(internalFields)) {}

 public:
  explicit Cpp2RequestContext(
      Cpp2ConnContext* ctx,
      apache::thrift::transport::THeader* header = nullptr,
      std::string methodName = std::string{})
      : Cpp2RequestContext(
            ctx,
            detail::createPerRequestInternalFields(),
            header,
            std::move(methodName)) {}

  void setConnectionContext(Cpp2ConnContext* ctx) { ctx_ = ctx; }

  // Forward all connection-specific information
  const folly::SocketAddress* getPeerAddress() const override {
    return ctx_->getPeerAddress();
  }

  const folly::SocketAddress* getLocalAddress() const {
    return ctx_->getLocalAddress();
  }

  void reset() { ctx_->reset(); }

  concurrency::PRIORITY getCallPriority() const {
    return header_->getCallPriority();
  }

  concurrency::ThreadManager::ExecutionScope getRequestExecutionScope() const {
    return executionScope_;
  }

  void setRequestExecutionScope(
      concurrency::ThreadManager::ExecutionScope scope) {
    executionScope_ = std::move(scope);
  }

  virtual std::vector<uint16_t>& getTransforms() {
    return header_->getWriteTransforms();
  }

  folly::EventBaseManager* getEventBaseManager() override {
    return ctx_->getEventBaseManager();
  }

  void setException() { isException_ = true; }

  bool isException() const { return isException_; }

  void* getUserData() const override { return ctx_->getUserData(); }

  void* setUserData(folly::erased_unique_ptr data) override {
    return ctx_->setUserData(std::move(data));
  }
  using TConnectionContext::setUserData;

  // This data is set on a per request basis.
  void* getRequestData() const { return requestData_.get(); }

  // Returns the old request data context so the caller can clean up
  folly::erased_unique_ptr setRequestData(
      void* data, void (*destructor)(void*) = no_op_destructor) {
    return std::exchange(requestData_, {data, destructor});
  }
  folly::erased_unique_ptr setRequestData(folly::erased_unique_ptr data) {
    return std::exchange(requestData_, std::move(data));
  }

  template <class T>
  T& getInternalFields() noexcept {
    return internalFields_.value_unchecked<T>();
  }

  template <class T>
  const T& getInternalFields() const noexcept {
    return internalFields_.value_unchecked<T>();
  }

  virtual Cpp2ConnContext* getConnectionContext() const { return ctx_; }

  std::chrono::milliseconds getRequestTimeout() const {
    return requestTimeout_;
  }

  void setRequestTimeout(std::chrono::milliseconds requestTimeout) {
    requestTimeout_ = requestTimeout;
  }

  const std::string& getMethodName() const { return methodName_; }

  std::string releaseMethodName() { return std::move(methodName_); }

  void setRpcKind(RpcKind rpcKind) { rpcKind_ = rpcKind; }

  RpcKind getRpcKind() const { return rpcKind_; }

  void setProtoSeqId(int32_t protoSeqId) { protoSeqId_ = protoSeqId; }

  int32_t getProtoSeqId() { return protoSeqId_; }

  void setInteractionId(int64_t id) { interactionId_ = id; }

  int64_t getInteractionId() { return interactionId_; }

  void setFrameworkMetadata(folly::IOBuf frameworkMetadata) {
    frameworkMetadata_ = std::move(frameworkMetadata);
  }

  folly::IOBuf& getFrameworkMetadata() { return frameworkMetadata_; }

  void setInteractionCreate(InteractionCreate interactionCreate) {
    interactionCreate_ = std::move(interactionCreate);
  }

  folly::Optional<InteractionCreate>& getInteractionCreate() {
    return interactionCreate_;
  }

  server::TServerObserver::PreHandlerTimestamps& getTimestamps() {
    return timestamps_;
  }

  // This lets us avoid having different signatures in the processor map.
  // Should remove if we decide to split out interaction methods.
  void setTile(TilePtr&& tile) {
    DCHECK(tile);
    tile_ = std::move(tile);
  }
  TilePtr releaseTile() { return std::move(tile_); }

  const std::string* clientId() const {
    if (auto header = getHeader(); header && header->clientId()) {
      return &*header->clientId();
    }
    if (auto headers = getHeadersPtr()) {
      return folly::get_ptr(*headers, transport::THeader::kClientId);
    }
    return nullptr;
  }

  const std::string* getClientRequestId() const {
    if (auto* header = getHeader()) {
      if (const auto& loggingContext = header->loggingContext()) {
        if (const auto requestIdRef = loggingContext->requestId();
            requestIdRef.has_value() && !requestIdRef->empty()) {
          return &*requestIdRef;
        }
      }
    }
    return nullptr;
  }

  const std::string* getRoutingTarget() const {
    if (auto* header = getHeader()) {
      if (const auto& loggingContext = header->loggingContext()) {
        if (const auto routingTargetRef = loggingContext->routingTarget();
            routingTargetRef.has_value() && !routingTargetRef->empty()) {
          return &*routingTargetRef;
        }
      }
    }
    return nullptr;
  }

  const std::string* tenantId() const {
    if (auto header = getHeader(); header && header->tenantId()) {
      return &*header->tenantId();
    }
    if (auto headers = getHeadersPtr()) {
      return folly::get_ptr(*headers, transport::THeader::kTenantId);
    }
    return nullptr;
  }

 protected:
  apache::thrift::server::TServerObserver::CallTimestamps timestamps_;

 private:
  Cpp2ConnContext* ctx_;
  folly::erased_unique_ptr requestData_{nullptr, nullptr};
  std::chrono::milliseconds requestTimeout_{0};
  std::string methodName_;
  RpcKind rpcKind_{RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  int32_t protoSeqId_{0};
  int64_t interactionId_{0};
  bool isException_{false};
  folly::Optional<InteractionCreate> interactionCreate_;
  TilePtr tile_;
  concurrency::ThreadManager::ExecutionScope executionScope_{
      concurrency::PRIORITY::NORMAL};
  folly::IOBuf frameworkMetadata_;
  detail::RequestInternalFieldsT internalFields_;
};

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_ASYNC_CPP2CONNCONTEXT_H_
