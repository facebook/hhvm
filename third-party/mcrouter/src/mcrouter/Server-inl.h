/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <signal.h>

#include <cstdio>

#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/server/ThriftServer.h>

#include "mcrouter/CarbonRouterClient.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/ExecutorObserver.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/OptionsUtil.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/ServerOnRequest.h"
#include "mcrouter/StandaloneConfig.h"
#include "mcrouter/ThriftAcceptor.h"
#include "mcrouter/ThriftObserver.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/network/AsyncMcServer.h"
#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/McSSLUtil.h"
#include "mcrouter/lib/network/Qos.h"
#include "mcrouter/standalone_options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

inline std::function<void(McServerSession&)> getAclChecker(
    const McrouterOptions& opts,
    const McrouterStandaloneOptions& standaloneOpts) {
  if (standaloneOpts.acl_checker_enable) {
    try {
      return getConnectionAclChecker(
          standaloneOpts.server_ssl_service_identity,
          standaloneOpts.acl_checker_enforce);
    } catch (const std::exception& ex) {
      MC_LOG_FAILURE(
          opts,
          failure::Category::kSystemError,
          "Error creating acl checker: {}",
          ex.what());
      LOG(WARNING) << "Disabling acl checker on all threads due to error.";
    }
  } else {
    LOG(WARNING) << "acl checker will not be enabled.";
  }
  return [](McServerSession&) {};
}

inline std::function<bool(const folly::AsyncTransportWrapper*)>
getThriftAclChecker(
    const McrouterOptions& opts,
    const McrouterStandaloneOptions& standaloneOpts) {
  if (standaloneOpts.acl_checker_enable) {
    try {
      return getThriftConnectionAclChecker(
          standaloneOpts.server_ssl_service_identity,
          standaloneOpts.acl_checker_enforce);
    } catch (const std::exception& ex) {
      MC_LOG_FAILURE(
          opts,
          failure::Category::kSystemError,
          "Error creating acl checker: {}",
          ex.what());
      LOG(WARNING) << "Disabling acl checker on all threads due to error.";
    }
  } else {
    LOG(WARNING) << "acl checker will not be enabled.";
  }
  return [](const folly::AsyncTransportWrapper*) { return true; };
}

template <class RouterInfo, template <class> class RequestHandler>
void serverInit(
    CarbonRouterInstance<RouterInfo>& router,
    size_t threadId,
    folly::EventBase& evb,
    AsyncMcServerWorker& worker,
    const McrouterStandaloneOptions& standaloneOpts,
    std::function<void(McServerSession&)>& aclChecker,
    CarbonRouterClient<RouterInfo>* routerClient) {
  using RequestHandlerType = RequestHandler<ServerOnRequest<RouterInfo>>;

  auto proxy = router.getProxy(threadId);
  // Manually override proxy assignment
  routerClient->setProxyIndex(threadId);
  // Set tlsWorkerThreadId which will be used by Thrift Observer
  tlsWorkerThreadId = threadId;

  worker.setOnRequest(RequestHandlerType(
      *routerClient,
      evb,
      standaloneOpts.retain_source_ip,
      standaloneOpts.enable_pass_through_mode,
      standaloneOpts.remote_thread,
      router.externalStatsHandler(),
      standaloneOpts.prefix_acl_checker_enable));

  worker.setOnConnectionAccepted(
      [proxy, &aclChecker](McServerSession& session) mutable {
        proxy->stats().increment(num_client_connections_stat);
        try {
          aclChecker(session);
        } catch (const std::exception& ex) {
          MC_LOG_FAILURE(
              proxy->router().opts(),
              failure::Category::kSystemError,
              "Error running acl checker: {}",
              ex.what());
          LOG(WARNING) << "Disabling acl checker on this thread.";
          aclChecker = [](McServerSession&) {};
        }
      });
  worker.setOnConnectionCloseFinish(
      [proxy](McServerSession&, bool onAcceptedCalled) {
        if (onAcceptedCalled) {
          proxy->stats().decrement(num_client_connections_stat);
        }
      });

  // Setup compression on each worker.
  if (standaloneOpts.enable_server_compression) {
    auto codecManager = router.getCodecManager();
    if (codecManager) {
      worker.setCompressionCodecMap(codecManager->getCodecMap(evb));
    } else {
      LOG(WARNING) << "Compression is enabled but couldn't find CodecManager. "
                   << "Compression will be disabled.";
    }
  }
}

template <class RouterInfo>
inline void startServerShutdown(
    std::shared_ptr<apache::thrift::ThriftServer> thriftServer,
    std::shared_ptr<AsyncMcServer> asyncMcServer,
    std::shared_ptr<std::atomic<bool>> shutdownStarted) {
  if (!shutdownStarted->exchange(true)) {
    LOG(INFO) << "Started server shutdown";
    if (asyncMcServer) {
      LOG(INFO) << "Started shutdown of AsyncMcServer";
      asyncMcServer->shutdown();
      asyncMcServer->join();
      LOG(INFO) << "Completed shutdown of AsyncMcServer";
    }
    if (thriftServer) {
      LOG(INFO) << "Calling stop on ThriftServer";
      thriftServer->stop();
      LOG(INFO) << "Called stop on ThriftServer";
    }
  }
}

inline AsyncMcServer::Options createAsyncMcServerOptions(
    const McrouterOptions& mcrouterOpts,
    const McrouterStandaloneOptions& standaloneOpts,
    const std::vector<folly::EventBase*>* evb = nullptr) {
  AsyncMcServer::Options opts;

  if (standaloneOpts.listen_sock_fd >= 0) {
    opts.existingSocketFds = {standaloneOpts.listen_sock_fd};
  } else if (!standaloneOpts.unix_domain_sock.empty()) {
    opts.unixDomainSockPath = standaloneOpts.unix_domain_sock;
  } else {
    opts.listenAddresses = standaloneOpts.listen_addresses;
    opts.ports = standaloneOpts.ports;
    opts.sslPorts = standaloneOpts.ssl_ports;
    opts.tlsTicketKeySeedPath = standaloneOpts.tls_ticket_key_seed_path;
    opts.pemCertPath = standaloneOpts.server_pem_cert_path;
    opts.pemKeyPath = standaloneOpts.server_pem_key_path;
    opts.pemCaPath = standaloneOpts.server_pem_ca_path;
    opts.sslRequirePeerCerts = standaloneOpts.ssl_require_peer_certs;
    opts.tfoEnabledForSsl = mcrouterOpts.enable_ssl_tfo;
    opts.tfoQueueSize = standaloneOpts.tfo_queue_size;
    opts.worker.useKtls12 = standaloneOpts.ssl_use_ktls12;
  }

  opts.numThreads = mcrouterOpts.num_proxies;
  opts.numListeningSockets = standaloneOpts.num_listening_sockets;
  opts.worker.tcpZeroCopyThresholdBytes =
      standaloneOpts.tcp_zero_copy_threshold;

  size_t maxConns =
      opts.setMaxConnections(standaloneOpts.max_conns, opts.numThreads);
  if (maxConns > 0) {
    VLOG(1) << "The system will allow " << maxConns
            << " simultaneos connections before start closing connections"
            << " using an LRU algorithm";
  }

  if (standaloneOpts.enable_qos) {
    uint64_t qos = 0;
    if (getQoS(
            standaloneOpts.default_qos_class,
            standaloneOpts.default_qos_path,
            qos)) {
      opts.worker.trafficClass = qos;
    } else {
      VLOG(1) << "Incorrect qos class / qos path. Accepted connections will not"
              << "be marked.";
    }
  }

  opts.tcpListenBacklog = standaloneOpts.tcp_listen_backlog;
  opts.worker.defaultVersionHandler = false;
  opts.worker.maxInFlight = standaloneOpts.max_client_outstanding_reqs;
  opts.worker.sendTimeout =
      std::chrono::milliseconds{standaloneOpts.client_timeout_ms};
  if (!mcrouterOpts.debug_fifo_root.empty()) {
    opts.worker.debugFifoPath = getServerDebugFifoFullPath(mcrouterOpts);
  }

  if (standaloneOpts.server_load_interval_ms > 0) {
    opts.cpuControllerOpts.dataCollectionInterval =
        std::chrono::milliseconds(standaloneOpts.server_load_interval_ms);
  }

  /* Default to one read per event to help latency-sensitive workloads.
     We can make this an option if this needs to be adjusted. */
  opts.worker.maxReadsPerEvent = 1;

  if (evb) {
    opts.eventBases = (*evb);
  }
  return opts;
}

} // namespace detail

template <class RouterInfo>
class ShutdownSignalHandler : public folly::AsyncSignalHandler {
 public:
  explicit ShutdownSignalHandler(
      folly::EventBase* evb,
      std::shared_ptr<apache::thrift::ThriftServer> thriftServer,
      std::shared_ptr<AsyncMcServer> asyncMcServer,
      std::shared_ptr<std::atomic<bool>> shutdownStarted)
      : AsyncSignalHandler(evb),
        thriftServer_(thriftServer),
        asyncMcServer_(asyncMcServer),
        shutdownStarted_(shutdownStarted) {}

  void signalReceived(int) noexcept override {
    detail::startServerShutdown<RouterInfo>(
        thriftServer_, asyncMcServer_, shutdownStarted_);
  }

 private:
  std::shared_ptr<apache::thrift::ThriftServer> thriftServer_;
  std::shared_ptr<AsyncMcServer> asyncMcServer_;
  std::shared_ptr<std::atomic<bool>> shutdownStarted_;
};

template <class RouterInfo>
void setupRouter(
    const McrouterOptions& mcrouterOpts,
    const McrouterStandaloneOptions& standaloneOpts,
    CarbonRouterInstance<RouterInfo>* router,
    StandalonePreRunCb preRunCb) {
  router->setStartupOpts(standaloneOpts.toDict());

  if (standaloneOpts.enable_server_compression &&
      !mcrouterOpts.enable_compression) {
    initCompression(*router);
  }

  if (preRunCb) {
    preRunCb(*router);
  }
}

template <
    class RouterInfo,
    template <class>
    class RequestHandler,
    template <class>
    class ThriftRequestHandler>
bool runServerDual(
    const McrouterOptions& mcrouterOpts,
    const McrouterStandaloneOptions& standaloneOpts,
    StandalonePreRunCb preRunCb) {
  using RequestHandlerType = RequestHandler<ServerOnRequest<RouterInfo>>;
  std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool;
  CarbonRouterInstance<RouterInfo>* router;
  std::shared_ptr<AsyncMcServer> asyncMcServer;
  std::shared_ptr<apache::thrift::ThriftServer> thriftServer;
  try {
    // Create thread pool for both AsyncMcServer and ThriftServer
    auto threadPrefix =
        folly::to<std::string>("mcrpxy-", mcrouterOpts.router_name);
    ioThreadPool = std::make_shared<folly::IOThreadPoolExecutor>(
        mcrouterOpts.num_proxies,
        mcrouterOpts.num_proxies,
        std::make_shared<folly::NamedThreadFactory>(threadPrefix));

    // extract event bases
    auto evbs = extractEvbs(*ioThreadPool);
    CHECK_EQ(evbs.size(), mcrouterOpts.num_proxies);

    // Create AsyncMcServer instance
    asyncMcServer =
        std::make_shared<AsyncMcServer>(detail::createAsyncMcServerOptions(
            mcrouterOpts, standaloneOpts, &evbs));

    // Create CarbonRouterInstance
    if (standaloneOpts.remote_thread) {
      router =
          CarbonRouterInstance<RouterInfo>::init("standalone", mcrouterOpts);
    } else {
      router = CarbonRouterInstance<RouterInfo>::init(
          "standalone", mcrouterOpts, ioThreadPool);
    }
    if (router == nullptr) {
      LOG(ERROR) << "CRITICAL: Failed to initialize mcrouter!";
      return false;
    }

    setupRouter<RouterInfo>(mcrouterOpts, standaloneOpts, router, preRunCb);

    // Create CarbonRouterClients for each worker thread
    std::vector<typename CarbonRouterClient<RouterInfo>::Pointer>
        carbonRouterClients;
    std::unordered_map<
        folly::EventBase*,
        std::shared_ptr<ServerOnRequest<RouterInfo>>>
        serverOnRequestMap;
    for (auto evb : evbs) {
      // Create CarbonRouterClients
      auto routerClient = standaloneOpts.remote_thread
          ? router->createClient(0 /* maximum_outstanding_requests */)
          : router->createSameThreadClient(
                0 /* maximum_outstanding_requests */);

      serverOnRequestMap.emplace(
          evb,
          std::make_shared<ServerOnRequest<RouterInfo>>(
              *routerClient,
              *evb,
              standaloneOpts.retain_source_ip,
              standaloneOpts.enable_pass_through_mode,
              standaloneOpts.remote_thread,
              router->externalStatsHandler(),
              standaloneOpts.prefix_acl_checker_enable));
      carbonRouterClients.push_back(std::move(routerClient));
    }
    CHECK_EQ(carbonRouterClients.size(), mcrouterOpts.num_proxies);
    CHECK_EQ(serverOnRequestMap.size(), mcrouterOpts.num_proxies);

    // Get local evb
    folly::EventBase* evb = ioThreadPool->getEventBaseManager()->getEventBase();

    // Thrift server setup
    apache::thrift::server::observerFactory_.reset();
    thriftServer = std::make_shared<apache::thrift::ThriftServer>();
    thriftServer->setIOThreadPool(ioThreadPool);
    thriftServer->setNumCPUWorkerThreads(1);

    // Shutdown state
    auto shutdownStarted = std::make_shared<std::atomic<bool>>(false);
    // Register signal handler which will handle ordered shutdown process of the
    // two servers
    ShutdownSignalHandler<RouterInfo> shutdownHandler(
        evb, thriftServer, asyncMcServer, shutdownStarted);
    shutdownHandler.registerSignalHandler(SIGTERM);
    shutdownHandler.registerSignalHandler(SIGINT);

    // Create thrift handler
    thriftServer->setInterface(
        std::make_shared<ThriftRequestHandler<ServerOnRequest<RouterInfo>>>(
            std::move(serverOnRequestMap)));

    // Add Identity Hook
    thriftServer->setClientIdentityHook(McSSLUtil::getClientIdentityHook());

    // ACL Checker for ThriftServer
    auto aclCheckerThrift =
        detail::getThriftAclChecker(mcrouterOpts, standaloneOpts);

    uint64_t qos = 0;
    if (standaloneOpts.enable_qos) {
      if (!getQoS(
              standaloneOpts.default_qos_class,
              standaloneOpts.default_qos_path,
              qos)) {
        LOG(ERROR)
            << "Incorrect qos class / qos path. Accepted connections will not"
            << "be marked.";
      }
    }

    thriftServer->setAcceptorFactory(std::make_shared<ThriftAcceptorFactory>(
        *thriftServer, std::move(aclCheckerThrift), qos));

    // Set listening port for cleartext and SSL connections
    if (standaloneOpts.thrift_port > 0) {
      thriftServer->setPort(standaloneOpts.thrift_port);
    } else {
      LOG(ERROR) << "Must specify thrift port";
      router->shutdown();
      freeAllRouters();
      return false;
    }
    thriftServer->disableActiveRequestsTracking();
    thriftServer->setSocketMaxReadsPerEvent(1);
    // Set observer for connection stats
    thriftServer->setObserver(
        std::make_shared<ThriftObserver<RouterInfo>>(*router, shutdownStarted));
    // Don't enforce default timeouts, unless Client forces them.
    thriftServer->setQueueTimeout(std::chrono::milliseconds(0));
    thriftServer->setTaskExpireTime(std::chrono::milliseconds(0));
    // Set idle and ssl handshake timeouts to 0 to be consistent with
    // AsyncMcServer
    thriftServer->setIdleServerTimeout(std::chrono::milliseconds(0));
    thriftServer->setSSLHandshakeTimeout(std::chrono::milliseconds(0));

    initStandaloneSSLDualServer(standaloneOpts, thriftServer);
    thriftServer->watchTicketPathForChanges(
        standaloneOpts.tls_ticket_key_seed_path);
    thriftServer->setStopWorkersOnStopListening(false);

    // Get acl checker for AsyncMcServer
    auto aclChecker = detail::getAclChecker(mcrouterOpts, standaloneOpts);
    // Start AsyncMcServer
    LOG(INFO) << "Starting AsyncMcServer in dual mode";
    asyncMcServer->startOnVirtualEB(
        [&carbonRouterClients,
         &router,
         &standaloneOpts,
         aclChecker = aclChecker](
            size_t threadId,
            folly::VirtualEventBase& vevb,
            AsyncMcServerWorker& worker) mutable {
          // Setup compression on each worker.
          if (standaloneOpts.enable_server_compression) {
            auto codecManager = router->getCodecManager();
            if (codecManager) {
              worker.setCompressionCodecMap(
                  codecManager->getCodecMap(vevb.getEventBase()));
            } else {
              LOG(WARNING)
                  << "Compression is enabled but couldn't find CodecManager. "
                  << "Compression will be disabled.";
            }
          }
          detail::serverInit<RouterInfo, RequestHandler>(
              *router,
              threadId,
              vevb.getEventBase(),
              worker,
              standaloneOpts,
              aclChecker,
              carbonRouterClients[threadId].get());
        },
        // Shutdown must be scheduled back to event base of main to ensure
        // that there we dont attempt to destruct a VirtualEventBase
        [evb = evb, &asyncMcServer, &thriftServer, &shutdownStarted] {
          evb->runInEventBaseThread([&]() {
            detail::startServerShutdown<RouterInfo>(
                thriftServer, asyncMcServer, shutdownStarted);
          });
        });

    LOG(INFO) << "Thrift Server and AsyncMcServer running.";
    // Run the ThriftServer; this blocks until the server is shut down.
    thriftServer->serve();
    thriftServer.reset();
    LOG(INFO) << "Started shutdown of CarbonRouterInstance";
    router->shutdown();
    freeAllRouters();
    // Now free iothread pool
    ioThreadPool.reset();
    LOG(INFO) << "Completed shutdown";
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error creating dual mode AsyncMcServer: " << e.what();
    exit(EXIT_FAILURE);
  }
  return true;
}

template <class RouterInfo, template <class> class RequestHandler>
bool runServer(
    const McrouterOptions& mcrouterOpts,
    const McrouterStandaloneOptions& standaloneOpts,
    StandalonePreRunCb preRunCb) {
  AsyncMcServer::Options opts =
      detail::createAsyncMcServerOptions(mcrouterOpts, standaloneOpts);

  std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool;
  CarbonRouterInstance<RouterInfo>* router = nullptr;
  std::shared_ptr<AsyncMcServer> asyncMcServer;
  try {
    LOG(INFO) << "Spawning AsyncMcServer";
    // Create thread pool for both AsyncMcServer and CarbonRouterInstance
    ioThreadPool = std::make_shared<folly::IOThreadPoolExecutor>(
        mcrouterOpts.num_proxies, mcrouterOpts.num_proxies);

    // extract event bases
    auto evbs = extractEvbs(*ioThreadPool);
    CHECK_EQ(evbs.size(), mcrouterOpts.num_proxies);

    // Get EVB of main thread
    auto localEvb = ioThreadPool->getEventBaseManager()->getEventBase();

    asyncMcServer =
        std::make_shared<AsyncMcServer>(detail::createAsyncMcServerOptions(
            mcrouterOpts, standaloneOpts, &evbs));

    if (standaloneOpts.remote_thread) {
      router =
          CarbonRouterInstance<RouterInfo>::init("standalone", mcrouterOpts);
    } else {
      router = CarbonRouterInstance<RouterInfo>::init(
          "standalone", mcrouterOpts, ioThreadPool);
    }
    if (router == nullptr) {
      LOG(ERROR) << "CRITICAL: Failed to initialize mcrouter!";
      return false;
    }

    setupRouter<RouterInfo>(mcrouterOpts, standaloneOpts, router, preRunCb);

    auto shutdownStarted = std::make_shared<std::atomic<bool>>(false);
    ShutdownSignalHandler<RouterInfo> shutdownHandler(
        localEvb, nullptr, asyncMcServer, shutdownStarted);
    shutdownHandler.registerSignalHandler(SIGTERM);
    shutdownHandler.registerSignalHandler(SIGINT);

    // Create CarbonRouterClients for each worker thread
    std::vector<typename CarbonRouterClient<RouterInfo>::Pointer>
        carbonRouterClients;
    for (size_t i = 0; i < mcrouterOpts.num_proxies; i++) {
      // Create CarbonRouterClients
      auto routerClient = standaloneOpts.remote_thread
          ? router->createClient(0 /* maximum_outstanding_requests */)
          : router->createSameThreadClient(
                0 /* maximum_outstanding_requests */);
      carbonRouterClients.push_back(std::move(routerClient));
    }
    CHECK_EQ(carbonRouterClients.size(), mcrouterOpts.num_proxies);

    auto aclChecker = detail::getAclChecker(mcrouterOpts, standaloneOpts);
    asyncMcServer->startOnVirtualEB(
        [&router,
         &carbonRouterClients,
         &standaloneOpts,
         aclChecker = aclChecker](
            size_t threadId,
            folly::VirtualEventBase& vevb,
            AsyncMcServerWorker& worker) mutable {
          detail::serverInit<RouterInfo, RequestHandler>(
              *router,
              threadId,
              vevb.getEventBase(),
              worker,
              standaloneOpts,
              aclChecker,
              carbonRouterClients[threadId].get());
        },
        [evb = localEvb, &asyncMcServer, &shutdownStarted] {
          evb->runInEventBaseThread([&]() {
            detail::startServerShutdown<RouterInfo>(
                nullptr, asyncMcServer, shutdownStarted);
          });
          evb->terminateLoopSoon();
        });
    localEvb->loopForever();
    LOG(INFO) << "Started shutdown of CarbonRouterInstance";
    router->shutdown();
    freeAllRouters();
    ioThreadPool.reset();
    if (!opts.unixDomainSockPath.empty()) {
      std::remove(opts.unixDomainSockPath.c_str());
    }
    LOG(INFO) << "Completed shutdown";
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
    exit(EXIT_FAILURE);
  }
  return true;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
