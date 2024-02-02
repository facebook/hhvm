/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsyncMcServer.h"

#include <signal.h>
#include <sys/eventfd.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <folly/Conv.h>
#include <folly/IPAddress.h>
#include <folly/SharedMutex.h>
#include <folly/String.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/VirtualEventBase.h>
#include <wangle/ssl/TLSCredProcessor.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

#include "mcrouter/lib/network/AsyncMcServerWorker.h"
#include "mcrouter/lib/network/ThreadLocalSSLContextProvider.h"

namespace facebook {
namespace memcache {

namespace {

/* Global pointer to the server for signal handlers */
facebook::memcache::AsyncMcServer* gServer;

class ShutdownPipe : public folly::EventHandler {
 public:
  ShutdownPipe(AsyncMcServer& server, folly::EventBase& evb)
      : folly::EventHandler(&evb), evb_(evb), server_(server) {
    fd_ = eventfd(0, 0);
    if (FOLLY_UNLIKELY(fd_ == -1)) {
      throw std::runtime_error(
          "Unexpected file descriptor (-1) in ShutdownPipe");
    }
    changeHandlerFD(folly::NetworkSocket::fromFd(fd_));
    registerHandler(EV_READ);
  }

  void shutdownFromSignalHandler() {
    uint64_t val = 1;
    auto res = write(fd_, &val, 8);
    if (FOLLY_UNLIKELY(res != 8)) {
      throw std::system_error(
          errno,
          std::system_category(),
          folly::sformat("Unexpected return of write: {}", res));
    }
  }

 private:
  folly::EventBase& evb_;
  AsyncMcServer& server_;
  int fd_;

  void handlerReady(uint16_t /* events */) noexcept final {
    evb_.runInEventBaseThreadAlwaysEnqueue(
        [&, s = &server_] { s->shutdown(); });
  }
};

} // anonymous namespace

/**
 * Class responsible for orchestrating the startup of McServerThreads.
 */
class McServerThreadSpawnController {
 public:
  explicit McServerThreadSpawnController(size_t numListeningSockets)
      : numListeningSockets_(numListeningSockets) {}

  /**
   * Blocks the current thread until it's ready to start running.
   */
  void waitToStart() {
    runningFuture_.get();
  }

  /**
   * Blocks until the acceptor thread is ready to accept new connections.
   *
   * @throw  If anything was thrown when starting to accept connections.
   */
  void waitForAcceptor() {
    acceptorFuture_.get();
  }

  /**
   * Wake up all threads to start running.
   */
  void startRunning() {
    runningPromise_.set_value();
  }

  /**
   * Abort execution.
   * Used only when we want to shutdown the server before we started running.
   */
  void abort() {
    runningPromise_.set_exception(
        std::make_exception_ptr(std::runtime_error("Execution aborted.")));
  }

  /**
   * Starts accepting (if it's an acceptor thread), or blocks until accetor
   * thread has started accepting.
   *
   * @param acceptorFn        The acceptor function. This function should throw
   *                          if something goes wrong.
   * @param isAcceptorThread  Whether or not this is the acceptor thread.
   *
   * @throw  If anything was thrown when starting to accept connections.
   */
  template <class F>
  void startAccepting(F&& acceptorFn, bool isAcceptorThread) {
    if (isAcceptorThread) {
      try {
        acceptorFn();
        if (++listeningThreadCount == numListeningSockets_) {
          acceptorPromise_.set_value();
        } else {
          // Last listening socket fulfills the promise, the others wait
          waitForAcceptor();
        }
      } catch (...) {
        auto exception = std::current_exception();
        acceptorPromise_.set_exception(exception);
        std::rethrow_exception(exception);
      }
    } else {
      waitForAcceptor();
    }
  }

 private:
  std::atomic<size_t> listeningThreadCount{0};
  size_t numListeningSockets_{1};

  std::promise<void> runningPromise_;
  std::shared_future<void> runningFuture_{runningPromise_.get_future()};

  std::promise<void> acceptorPromise_;
  std::shared_future<void> acceptorFuture_{acceptorPromise_.get_future()};
};

class McServerThread {
 public:
  explicit McServerThread(AsyncMcServer& server, size_t id)
      : server_(server),
        evb_(std::make_unique<folly::EventBase>(
            server.opts_.worker.enableEventBaseTimeMeasurement)),
        vevb_(nullptr),
        id_(id),
        worker_(server.opts_.worker, *evb_),
        acceptCallback_(this, false),
        sslAcceptCallback_(this, true),
        accepting_(false) {
    startThread();
  }

  explicit McServerThread(
      AsyncMcServer& server,
      size_t id,
      folly::VirtualEventBase* vevb)
      : server_(server),
        evb_(nullptr),
        vevb_(vevb),
        id_(id),
        worker_(server.opts_.worker, vevb_),
        acceptCallback_(this, false),
        sslAcceptCallback_(this, true),
        accepting_(false) {
    startRemote();
  }

  enum AcceptorT { Acceptor };

  McServerThread(AcceptorT, AsyncMcServer& server, size_t id, bool reusePort)
      : server_(server),
        evb_(std::make_unique<folly::EventBase>(
            server.opts_.worker.enableEventBaseTimeMeasurement)),
        vevb_(nullptr),
        id_(id),
        worker_(server.opts_.worker, *evb_),
        acceptCallback_(this, false),
        sslAcceptCallback_(this, true),
        accepting_(true),
        reusePort_(reusePort),
        shutdownPipe_(std::make_unique<ShutdownPipe>(server, eventBase())) {
    startThread();
  }

  McServerThread(
      AcceptorT,
      AsyncMcServer& server,
      size_t id,
      bool reusePort,
      folly::VirtualEventBase* vevb)
      : server_(server),
        evb_(nullptr),
        vevb_(vevb),
        id_(id),
        worker_(server.opts_.worker, vevb_),
        acceptCallback_(this, false),
        sslAcceptCallback_(this, true),
        accepting_(true),
        reusePort_(reusePort),
        shutdownPipe_(std::make_unique<ShutdownPipe>(server, eventBase())) {
    startRemote();
  }

  folly::EventBase& eventBase() {
    return vevb_ ? vevb_->getEventBase() : *evb_;
  }

  void startRemote() {
    CHECK(isVirtualEventBase());
    if (accepting_) {
      vevb_->runOnDestruction([&]() {
        socket_.reset();
        sslSocket_.reset();
        acceptorsKeepAlive_.clear();
      });
    }
    vevb_->runInEventBaseThread([&]() {
      try {
        worker_.setOnShutdownOperation([&]() { server_.shutdown(); });

        server_.threadsSpawnController_->waitToStart();
        server_.threadsSpawnController_->startAccepting(
            [this]() { startAccepting(); }, accepting_);
      } catch (...) {
        // if an exception is thrown, something went wrong before startup.
        return;
      }
      initFn_(id_, *vevb_, worker_);
    });
  }

  void startThread() {
    CHECK(!isVirtualEventBase());
    worker_.setOnShutdownOperation([&]() { server_.shutdown(); });

    thread_ = std::thread{[this]() {
      SCOPE_EXIT {
        // We must detroy the EventBase in it's own thread.
        // The reason is that we might have already scheduled something
        // using some VirtualEventBase, and it won't allow the EventBase
        // to be destroyed until all keepAlive tokens have being released.
        // In this case evb_.reset() will loop the EventBase until we are done
        // releasing all keepAlive tokens, so we can safely destroy
        // VirtualEventBase from main thread.
        std::unique_ptr<folly::EventBase> evbToDestroy;
        {
          std::lock_guard<std::mutex> lock(evbDestroyMutex_);
          evbToDestroy.swap(evb_);
        }
        evbToDestroy.reset();
      };

      try {
        server_.threadsSpawnController_->waitToStart();

        server_.threadsSpawnController_->startAccepting(
            [this]() { startAccepting(); }, accepting_);
      } catch (...) {
        // if an exception is thrown, something went wrong before startup.
        return;
      }

      loopFn_(id_, eventBase(), worker_);

      // Detach the server sockets from the acceptor thread.
      // If we don't do this, the TAsyncSSLServerSocket destructor
      // will try to do it, and a segfault will result if the
      // socket destructor runs after the threads' destructors.
      if (accepting_) {
        socket_.reset();
        sslSocket_.reset();
        for (auto& keepAlive : acceptorsKeepAlive_) {
          auto evb = keepAlive.get();
          evb->add([ka = std::move(keepAlive)]() {});
        }
        acceptorsKeepAlive_.clear();
      }
    }};
  }

  void shutdownVirtualEventBase() {
    if (isVirtualEventBase()) {
      shutdownPromise_.set_value();
    }
  }

  /**
   * The first of 2 shutdown steps will release the sockets from acceptor
   * threads to guarantee that no new connections are established once shutdown
   * has proceeded to the workers. shutdownAcceptor() must be called on all
   * threads before calling shutdownWorker() on any thread.
   *
   * Safe to call from other threads.
   */
  void shutdownAcceptor() {
    if (!accepting_) {
      return;
    }
    auto keepAlive = getKeepAlive();
    if (!keepAlive) {
      // The worker must have already shutdown and closed the sockets.
      return;
    }
    auto shutdownFn = [&]() {
      socket_.reset();
      sslSocket_.reset();
      for (auto& acceptorKeepAlive : acceptorsKeepAlive_) {
        auto evb = acceptorKeepAlive.get();
        evb->add([ka = std::move(acceptorKeepAlive)]() {});
      }
      acceptorsKeepAlive_.clear();
    };
    // We must wait for sockets to be released to guarantee that there are no
    // new connections once we proceed to the worker shutdown step.
    if (keepAlive->inRunningEventBaseThread()) {
      shutdownFn();
    } else {
      // Since this is blocking, it can be scheduled directly on the EventBase
      // and does not need to go through the VirtualEventBase.
      keepAlive->runInEventBaseThreadAndWait(std::move(shutdownFn));
    }
  }

  /**
   * The second of 2 shutdown steps will close all existing connections and
   * possibly begin releasing worker resources. There must be no new connections
   * established once worker shutdown has started. shutdownAcceptor() must be
   * called on all threads before calling shutdownWorker() on any thread.
   *
   * Safe to call from other threads.
   */
  void shutdownWorker() {
    auto shutdownFn = [&]() {
      if (shutdownPipe_) {
        shutdownPipe_->unregisterHandler();
      }
      worker_.shutdown();
      shutdownVirtualEventBase();
    };
    if (isVirtualEventBase()) {
      // Schedule the shutdown function through the VirtualEventBase so that
      // we can wait for its completion on VirtualEventBase destruction.
      vevb_->runInEventBaseThread(std::move(shutdownFn));
    } else if (auto keepAlive = getKeepAlive()) {
      // Schedule the shutdown function on the EventBase if it is still alive,
      // otherwise the worker must have already shutdown.
      keepAlive->runInEventBaseThread(std::move(shutdownFn));
    }
  }

  void shutdownFromSignalHandler() {
    if (shutdownPipe_) {
      shutdownPipe_->shutdownFromSignalHandler();
    }
  }

  void join() {
    if (isVirtualEventBase()) {
      shutdownFuture_.get();
    } else {
      if (thread_.joinable()) {
        thread_.join();
      }
    }
  }

  void setLoopFn(AsyncMcServer::LoopFn fn) {
    loopFn_ = std::move(fn);
  }

  void setInitFn(AsyncMcServer::InitFn fn) {
    initFn_ = std::move(fn);
  }

 private:
  class AcceptCallback : public folly::AsyncServerSocket::AcceptCallback {
   public:
    AcceptCallback(McServerThread* mcServerThread, bool secure)
        : mcServerThread_(mcServerThread), secure_(secure) {}
    void connectionAccepted(
        folly::NetworkSocket fdNetworkSocket,
        const folly::SocketAddress& /* clientAddr */,
        AcceptInfo /* info */) noexcept final {
      int fd = fdNetworkSocket.toFd();

      if (secure_) {
        const auto& server = mcServerThread_->server_;
        auto& opts = server.opts_;

        folly::Optional<wangle::TLSTicketKeySeeds> ticketKeySeeds =
            server.getTicketKeySeeds();
        if (ticketKeySeeds && ticketKeySeeds->isEmpty()) {
          ticketKeySeeds = folly::none;
        }

        auto contextPair = getServerContexts(
            opts.pemCertPath,
            opts.pemKeyPath,
            opts.pemCaPath,
            opts.sslRequirePeerCerts,
            std::move(ticketKeySeeds),
            opts.tlsPreferOcbCipher);

        if (contextPair.first) {
          mcServerThread_->worker_.addSecureClientSocket(
              fd, std::move(contextPair));
        } else {
          ::close(fd);
        }
      } else {
        mcServerThread_->worker_.addClientSocket(fd);
      }
    }
    void acceptError(folly::exception_wrapper ex) noexcept final {
      LOG(ERROR) << "Connection accept error: " << ex.what();
    }

   private:
    McServerThread* mcServerThread_{nullptr};
    bool secure_{false};
  };

  AsyncMcServer& server_;
  std::unique_ptr<folly::EventBase> evb_;
  folly::VirtualEventBase* vevb_;
  size_t id_;
  AsyncMcServerWorker worker_;
  AcceptCallback acceptCallback_;
  AcceptCallback sslAcceptCallback_;
  bool accepting_{false};
  bool reusePort_{false};
  std::mutex evbDestroyMutex_;
  std::thread thread_;

  folly::AsyncServerSocket::UniquePtr socket_;
  folly::AsyncServerSocket::UniquePtr sslSocket_;
  std::vector<folly::Executor::KeepAlive<folly::EventBase>> acceptorsKeepAlive_;
  std::unique_ptr<ShutdownPipe> shutdownPipe_;

  // Shutdown promise for virtual event bases
  std::promise<void> shutdownPromise_;
  std::future<void> shutdownFuture_{shutdownPromise_.get_future()};

  AsyncMcServer::InitFn initFn_;
  AsyncMcServer::LoopFn loopFn_;

  folly::Executor::KeepAlive<folly::EventBase> getKeepAlive() {
    std::lock_guard<std::mutex> lock(evbDestroyMutex_);
    if (vevb_) {
      return getKeepAliveToken(vevb_->getEventBase());
    } else if (evb_) {
      return getKeepAliveToken(*evb_);
    } else {
      return folly::Executor::KeepAlive<folly::EventBase>();
    }
  }

  bool isVirtualEventBase() {
    return (vevb_ != nullptr);
  }

  /**
   * Start accepting new connections.
   *
   * @throw   If anything goes wrong when start accepting connections.
   */
  void startAccepting() {
    CHECK(accepting_);
    auto& opts = server_.opts_;

    if (!opts.existingSocketFds.empty()) {
      checkLogic(
          opts.ports.empty() && opts.sslPorts.empty(),
          "Can't use ports if using existing socket");
      checkLogic(
          !reusePort_,
          "Can't use multiple listening sockets option if using existing socket");

      // Don't enable tcpZeroCopy here as it will be inherited. It has to be
      // enabled when the socket is in a TCP_CLOSE state, afterwards its too
      // late.
      if (opts.worker.tcpZeroCopyThresholdBytes > 0) {
        for (auto fd : opts.existingSocketFds) {
          int val = 0;
          socklen_t optlen = sizeof(val);
          int ret = getsockopt(fd, SOL_SOCKET, SO_ZEROCOPY, &val, &optlen);
          checkLogic(!ret, "Failed to getsockopt existing FD");
          checkLogic(val, "SO_ZEROCOPY must be enabled on existing socket.");
        }
      }
      if (!opts.pemCertPath.empty() || !opts.pemKeyPath.empty() ||
          !opts.pemCaPath.empty()) {
        checkLogic(
            !opts.pemCertPath.empty() && !opts.pemKeyPath.empty() &&
                !opts.pemCaPath.empty(),
            "All of pemCertPath, pemKeyPath and pemCaPath are required "
            "if at least one of them set");

        sslSocket_.reset(new folly::AsyncServerSocket());
        std::vector<folly::NetworkSocket> sockets;
        for (auto fd : opts.existingSocketFds) {
          sockets.push_back(folly::NetworkSocket::fromFd(fd));
        }

        sslSocket_->useExistingSockets(sockets);
      } else {
        socket_.reset(new folly::AsyncServerSocket());
        std::vector<folly::NetworkSocket> sockets;
        for (auto fd : opts.existingSocketFds) {
          sockets.push_back(folly::NetworkSocket::fromFd(fd));
        }
        socket_->useExistingSockets(sockets);
      }
    } else if (!opts.unixDomainSockPath.empty()) {
      checkLogic(
          opts.ports.empty() && opts.sslPorts.empty() &&
              opts.existingSocketFds.empty(),
          "Can't listen on port and unix domain socket at the same time");
      checkLogic(
          !reusePort_,
          "Can't use multiple listening sockets option with unix domain sockets.");
      checkLogic(
          !opts.worker.tcpZeroCopyThresholdBytes,
          "Can't use tcp zero copy with unix domain sockets.");
      std::remove(opts.unixDomainSockPath.c_str());
      socket_.reset(new folly::AsyncServerSocket());
      folly::SocketAddress serverAddress;
      serverAddress.setFromPath(opts.unixDomainSockPath);
      socket_->bind(serverAddress);
    } else {
      checkLogic(
          !server_.opts_.ports.empty() || !server_.opts_.sslPorts.empty(),
          "At least one port (plain or SSL) must be speicified");
      if (!server_.opts_.ports.empty()) {
        socket_.reset(new folly::AsyncServerSocket());
        socket_->setReusePortEnabled(reusePort_);
        for (auto port : server_.opts_.ports) {
          if (server_.opts_.listenAddresses.empty()) {
            socket_->bind(port);
          } else {
            std::vector<folly::IPAddress> ipAddresses;
            for (auto& listenAddress : server_.opts_.listenAddresses) {
              auto maybeIp = folly::IPAddress::tryFromString(listenAddress);
              checkLogic(
                  maybeIp.hasValue(),
                  "Invalid listen address: {}",
                  listenAddress);
              auto ip = std::move(maybeIp).value();
              ipAddresses.push_back(std::move(ip));
            }
            socket_->bind(ipAddresses, port);
          }
        }
        if (opts.worker.tcpZeroCopyThresholdBytes > 0) {
          bool zeroCopyApplied = socket_->setZeroCopy(true);
          checkLogic(zeroCopyApplied, "Failed to set TCP zero copy on socket");
        }
        socket_->setTosReflect(server_.opts_.worker.tosReflection);
      }
      if (!server_.opts_.sslPorts.empty()) {
        checkLogic(
            !server_.opts_.pemCertPath.empty() &&
                !server_.opts_.pemKeyPath.empty() &&
                !server_.opts_.pemCaPath.empty(),
            "All of pemCertPath, pemKeyPath, pemCaPath required"
            " with sslPorts");

        sslSocket_.reset(new folly::AsyncServerSocket());
        sslSocket_->setReusePortEnabled(reusePort_);
        for (auto sslPort : server_.opts_.sslPorts) {
          sslSocket_->bind(sslPort);
        }
        if (opts.worker.tcpZeroCopyThresholdBytes > 0) {
          bool zeroCopyApplied = sslSocket_->setZeroCopy(true);
          checkLogic(
              zeroCopyApplied, "Failed to set TCP zero copy on ssl socket");
        }
        sslSocket_->setTosReflect(server_.opts_.worker.tosReflection);
      }
    }

    if (socket_) {
      socket_->listen(server_.opts_.tcpListenBacklog);
      socket_->startAccepting();
      socket_->attachEventBase(&eventBase());
    }
    if (sslSocket_) {
      if (server_.opts_.tfoEnabledForSsl) {
        sslSocket_->setTFOEnabled(true, server_.opts_.tfoQueueSize);
      } else {
        sslSocket_->setTFOEnabled(false, 0);
      }
      sslSocket_->listen(server_.opts_.tcpListenBacklog);
      sslSocket_->startAccepting();
      sslSocket_->attachEventBase(&eventBase());
    }

    for (auto& t : server_.threads_) {
      if (socket_ != nullptr) {
        socket_->addAcceptCallback(&t->acceptCallback_, &t->eventBase());
      }
      if (sslSocket_ != nullptr) {
        sslSocket_->addAcceptCallback(&t->sslAcceptCallback_, &t->eventBase());
      }
      if (socket_ != nullptr || sslSocket_ != nullptr || t.get() != this) {
        acceptorsKeepAlive_.emplace_back(getKeepAliveToken(&t->eventBase()));
      }
    }
  }
};

size_t AsyncMcServer::Options::setMaxConnections(
    size_t globalMaxConns,
    size_t nThreads) {
  if (globalMaxConns == 0) {
    worker.maxConns = 0;
    return 0;
  }
  assert(nThreads > 0);

  rlimit rlim;
  auto rlimRes = getrlimit(RLIMIT_NOFILE, &rlim);

  if (globalMaxConns == 1) {
    if (rlimRes != 0) {
      LOG(ERROR) << "getrlimit failed. Errno: " << folly::errnoStr(errno)
                 << ". Disabling connection limits.";
      worker.maxConns = 0;
      return 0;
    }

    size_t softLimit = rlim.rlim_cur;
    globalMaxConns =
        std::max<size_t>(softLimit, worker.reservedFDs) - worker.reservedFDs;
    VLOG(2) << "Setting max conns to " << globalMaxConns
            << " based on soft resource limit of " << softLimit;

    worker.maxConns = globalMaxConns / nThreads;
    return globalMaxConns;
  }

  // globalMaxConns > 1

  if (rlimRes != 0) {
    // if the call to getrlimit fails, just set maxConns to what was specified.
    LOG(ERROR) << "getrlimit failed. Errno: " << folly::errnoStr(errno)
               << ". Using the number provided by the user"
               << " without raising rlimit";
    worker.maxConns = globalMaxConns / nThreads;
    return globalMaxConns;
  }

  size_t desiredRlim = globalMaxConns + worker.reservedFDs;

  // if the hard rlimit is not large enough, try and raise it.
  // this call will fail for unprivileged services.
  if (rlim.rlim_max < desiredRlim) {
    rlimit newRlim;
    newRlim.rlim_cur = desiredRlim;
    newRlim.rlim_max = desiredRlim;
    if (setrlimit(RLIMIT_NOFILE, &newRlim) == 0) {
      VLOG(2) << "Successfully updated hard and soft rlimit to " << desiredRlim;
      rlim = newRlim;
    } else {
      LOG(WARNING) << "Setting hard rlimit failed. Errno: "
                   << folly::errnoStr(errno);
      // we failed to set hard limt, lower the globalMaxConns to the current
      // hard limit.
      globalMaxConns = std::max<size_t>(rlim.rlim_max, worker.reservedFDs) -
          worker.reservedFDs;
    }
  }

  // if the soft limit is not large enough, increase it.
  if (rlim.rlim_cur < desiredRlim && rlim.rlim_cur < rlim.rlim_max) {
    auto newRlim = rlim;
    newRlim.rlim_cur = std::min<size_t>(rlim.rlim_max, desiredRlim);
    if (setrlimit(RLIMIT_NOFILE, &newRlim) == 0) {
      VLOG(2) << "Successfully updated soft rlimit to " << newRlim.rlim_cur;
      // setrlimit succeeded, update globalMaxConns.
      globalMaxConns = std::max<size_t>(newRlim.rlim_cur, worker.reservedFDs) -
          worker.reservedFDs;
    } else {
      LOG(ERROR) << "setrlimit for soft limit failed. "
                 << "Errno: " << folly::errnoStr(errno)
                 << ". Disabling connection limits.";
      worker.maxConns = 0;
      return 0;
    }
  }

  worker.maxConns = globalMaxConns / nThreads;
  return globalMaxConns;
}

AsyncMcServer::AsyncMcServer(Options opts) : opts_(std::move(opts)) {
  if (opts_.cpuControllerOpts.shouldEnable()) {
    auxiliaryEvbThread_ = std::make_unique<folly::ScopedEventBaseThread>();

    opts_.worker.cpuController = std::make_shared<CpuController>(
        opts_.cpuControllerOpts, *auxiliaryEvbThread_->getEventBase());
    opts_.worker.cpuController->start();
  }

  if (!opts_.tlsTicketKeySeedPath.empty()) {
    if (auto initialSeeds = wangle::TLSCredProcessor::processTLSTickets(
            opts_.tlsTicketKeySeedPath)) {
      tlsTicketKeySeeds_ = std::move(*initialSeeds);
      VLOG(0) << folly::to<std::string>(
          "Successfully loaded ticket key seeds from ",
          opts_.tlsTicketKeySeedPath);
    } else {
      LOG(ERROR) << folly::to<std::string>(
          "Unable to load ticket key seeds from ", opts_.tlsTicketKeySeedPath);
    }
    startPollingTicketKeySeeds();
  }

  if (opts_.eventBases.size() > 0) {
    virtualEventBaseMode_ = true;
    if (opts_.numListeningSockets == 0 ||
        opts_.numListeningSockets > opts_.eventBases.size()) {
      throw std::invalid_argument(folly::sformat(
          "Unexpected option: opts_.numListeningSockets={}",
          opts_.numListeningSockets));
    }

    for (auto evb : opts_.eventBases) {
      virtualEventBases_.push_back(
          std::make_unique<folly::VirtualEventBase>(*evb));
    }

    threadsSpawnController_ = std::make_unique<McServerThreadSpawnController>(
        opts_.numListeningSockets);

    // First construct the McServerThreads with listening sockets.
    size_t id;
    for (id = 0; id < opts_.numListeningSockets; id++) {
      threads_.emplace_back(std::make_unique<McServerThread>(
          McServerThread::Acceptor,
          *this,
          id,
          (opts_.numListeningSockets > 1),
          virtualEventBases_[id].get()));
    }
    // Now the rest
    for (; id < opts_.numThreads; ++id) {
      threads_.emplace_back(std::make_unique<McServerThread>(
          *this, id, virtualEventBases_[id].get()));
    }
  } else {
    if (opts_.numThreads == 0) {
      throw std::invalid_argument(folly::sformat(
          "Unexpected option: opts_.numThreads={}, virtualEventBaseMode={}",
          opts_.numThreads,
          virtualEventBaseMode_));
    }

    if (opts_.numListeningSockets == 0 ||
        opts_.numListeningSockets > opts_.numThreads) {
      throw std::invalid_argument(folly::sformat(
          "Unexpected option: opts_.numListeningSockets={}",
          opts_.numListeningSockets));
    }

    threadsSpawnController_ = std::make_unique<McServerThreadSpawnController>(
        opts_.numListeningSockets);
    size_t id;
    // First construct the McServerThreads with listening sockets.
    for (id = 0; id < opts_.numListeningSockets; id++) {
      threads_.emplace_back(std::make_unique<McServerThread>(
          McServerThread::Acceptor,
          *this,
          /*id*/ id,
          (opts_.numListeningSockets > 1)));
    }
    // Now the rest
    for (; id < opts_.numThreads; ++id) {
      threads_.emplace_back(std::make_unique<McServerThread>(*this, id));
    }
  }
}

std::vector<folly::EventBase*> AsyncMcServer::eventBases() const {
  std::vector<folly::EventBase*> out;
  for (auto& t : threads_) {
    out.push_back(&t->eventBase());
  }
  return out;
}

AsyncMcServer::~AsyncMcServer() {
  /* Need to place the destructor here, since this is the only
     translation unit that knows about McServerThread */
  if (opts_.worker.cpuController) {
    opts_.worker.cpuController->stop();
  }

  /* In case some signal handlers are still registered */
  gServer = nullptr;
}

void AsyncMcServer::start(std::function<void()> onShutdown) {
  onShutdown_ = std::move(onShutdown);

  threadsSpawnController_->startRunning();
  spawned_ = true;

  // this call will throw if something went wrong with acceptor.
  threadsSpawnController_->waitForAcceptor();

  /* We atomically attempt to change the state STARTUP -> SPAWNED.
     If we see the state SHUTDOWN, it means a signal handler ran
     concurrently with us (maybe even on this thread),
     so we just shutdown threads. */
  auto state = signalShutdownState_.load();
  do {
    if (state == SignalShutdownState::SHUTDOWN) {
      shutdown();
      return;
    }
  } while (!signalShutdownState_.compare_exchange_weak(
      state, SignalShutdownState::SPAWNED));
}

void AsyncMcServer::spawn(LoopFn fn, std::function<void()> onShutdown) {
  CHECK(!virtualEventBaseMode_);

  // Set loop callback on thread object
  CHECK(threads_.size() == opts_.numThreads);
  for (size_t i = 0; i < threads_.size(); ++i) {
    threads_[i]->setLoopFn(fn);
  }

  start(onShutdown);
}

void AsyncMcServer::startOnVirtualEB(
    InitFn fn,
    std::function<void()> onShutdown) {
  CHECK(virtualEventBaseMode_);

  // Set init callback on thread object
  CHECK(threads_.size() == opts_.numThreads);
  for (size_t i = 0; i < threads_.size(); ++i) {
    threads_[i]->setInitFn(fn);
  }

  start(onShutdown);
}

void AsyncMcServer::shutdown() {
  if (!alive_.exchange(false)) {
    return;
  }

  if (onShutdown_) {
    onShutdown_();
  }

  // We must shutdown all acceptor sockets before shutting down any workers to
  // guarantee that no new connections can be established after worker resources
  // have been released.
  ensureAcceptorsShutdown();
  for (auto& thread : threads_) {
    thread->shutdownWorker();
  }
}

void AsyncMcServer::ensureAcceptorsShutdown() {
  std::lock_guard<std::mutex> guard(shutdownAcceptorsMutex_);
  if (acceptorsAlive_) {
    for (auto& thread : threads_) {
      thread->shutdownAcceptor();
    }
    acceptorsAlive_ = false;
  }
}

void AsyncMcServer::installShutdownHandler(const std::vector<int>& signals) {
  gServer = this;

  /* prevent the above write being reordered with installing the handler */
  std::atomic_signal_fence(std::memory_order_seq_cst);

  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = [](int) {
    if (gServer) {
      gServer->shutdownFromSignalHandler();
    }
  };
  act.sa_flags = SA_RESETHAND;

  for (auto sig : signals) {
    if (sigaction(sig, &act, nullptr) == -1) {
      throw std::system_error(
          errno,
          std::system_category(),
          "Unexpected error returned by sigaction");
    }
  }
}

void AsyncMcServer::shutdownFromSignalHandler() {
  /* We atomically attempt to change the state STARTUP -> SHUTDOWN.
     If we see that the state is already SPAWNED,
     we can just issue a shutdown request. */

  auto state = signalShutdownState_.load();
  do {
    if (state == SignalShutdownState::SPAWNED) {
      threads_[0]->shutdownFromSignalHandler();
      return;
    }
  } while (!signalShutdownState_.compare_exchange_weak(
      state, SignalShutdownState::SHUTDOWN));
}

void AsyncMcServer::join() {
  if (!spawned_) {
    threadsSpawnController_->abort();
  }
  for (auto& thread : threads_) {
    thread->join();
  }
  if (virtualEventBaseMode_) {
    // Once all threads joined, clear virtual event bases
    virtualEventBases_.clear();
  }
}

void AsyncMcServer::setTicketKeySeeds(wangle::TLSTicketKeySeeds seeds) {
  std::unique_lock writeGuard(tlsTicketKeySeedsLock_);
  tlsTicketKeySeeds_ = std::move(seeds);
}

wangle::TLSTicketKeySeeds AsyncMcServer::getTicketKeySeeds() const {
  std::shared_lock readGuard(tlsTicketKeySeedsLock_);
  return tlsTicketKeySeeds_;
}

void AsyncMcServer::startPollingTicketKeySeeds() {
  // Caller assumed to have checked opts_.tlsTicketKeySeedPath is non-empty
  ticketKeySeedPoller_ = std::make_unique<wangle::TLSCredProcessor>();
  ticketKeySeedPoller_->setTicketPathToWatch(opts_.tlsTicketKeySeedPath);
  ticketKeySeedPoller_->addTicketCallback(
      [this](wangle::TLSTicketKeySeeds updatedSeeds) {
        setTicketKeySeeds(std::move(updatedSeeds));
        VLOG(0) << "Updated TLSTicketKeySeeds";
      });
}

} // namespace memcache
} // namespace facebook
