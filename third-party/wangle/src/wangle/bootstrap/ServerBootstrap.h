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

#include <folly/io/async/AsyncTransport.h>
#include <folly/synchronization/Baton.h>
#include <wangle/bootstrap/ServerBootstrap-inl.h>
#include <wangle/channel/Pipeline.h>
#include <iostream>
#include <thread>

namespace wangle {

/*
 * ServerBootstrap is a parent class intended to set up a
 * high-performance TCP accepting server.  It will manage a pool of
 * accepting threads, any number of accepting sockets, a pool of
 * IO-worker threads, and connection pool for each IO thread for you.
 *
 * The output is given as a Pipeline template: given a
 * PipelineFactory, it will create a new pipeline for each connection,
 * and your server can handle the incoming bytes.
 *
 * BACKWARDS COMPATIBLITY: for servers already taking a pool of
 * Acceptor objects, an AcceptorFactory can be given directly instead
 * of a pipeline factory.
 */
template <typename Pipeline = wangle::DefaultPipeline>
class ServerBootstrap {
 public:
  ServerBootstrap(const ServerBootstrap& that) = delete;
  ServerBootstrap(ServerBootstrap&& that) = default;

  ServerBootstrap() = default;

  ~ServerBootstrap() {
    stop();
    join();
  }

  /*
   * Pipeline used to add connections to event bases.
   * This is used for UDP or for load balancing
   * TCP connections to IO threads explicitly
   */
  ServerBootstrap* pipeline(std::shared_ptr<AcceptPipelineFactory> factory) {
    acceptPipelineFactory_ = factory;
    return this;
  }

  ServerBootstrap* channelFactory(
      std::shared_ptr<ServerSocketFactory> factory) {
    socketFactory_ = factory;
    return this;
  }

  ServerBootstrap* acceptorConfig(const ServerSocketConfig& accConfig) {
    accConfig_ = accConfig;
    return this;
  }

  ServerBootstrap* useZeroCopy(bool zc) {
    socketConfig.useZeroCopy = zc;
    return this;
  }

  /*
   * BACKWARDS COMPATIBILITY - an acceptor factory can be set.  Your
   * Acceptor is responsible for managing the connection pool.
   *
   * @param childHandler - acceptor factory to call for each IO thread
   */
  ServerBootstrap* childHandler(std::shared_ptr<AcceptorFactory> h) {
    acceptorFactory_ = h;
    return this;
  }

  /*
   * Set a pipeline factory that will be called for each new connection
   *
   * @param factory pipeline factory to use for each new connection
   */
  ServerBootstrap* childPipeline(
      std::shared_ptr<PipelineFactory<Pipeline>> factory) {
    childPipelineFactory_ = factory;
    return this;
  }

  /*
   * Set the IO executor.  If not set, a default one will be created
   * with one thread per core.
   *
   * @param io_group - io executor to use for IO threads.
   */
  ServerBootstrap* group(
      std::shared_ptr<folly::IOThreadPoolExecutorBase> io_group) {
    return group(nullptr, io_group);
  }

  /*
   * Set the acceptor executor, and IO executor.
   *
   * If no acceptor executor is set, a single thread will be created for accepts
   * If no IO executor is set, a default of one thread per core will be created
   *
   * @param group - acceptor executor to use for acceptor threads.
   * @param io_group - io executor to use for IO threads.
   */
  ServerBootstrap* group(
      std::shared_ptr<folly::IOThreadPoolExecutorBase> accept_group,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> io_group) {
    if (!accept_group) {
      accept_group = std::make_shared<folly::IOThreadPoolExecutor>(
          1, std::make_shared<folly::NamedThreadFactory>("Acceptor Thread"));
    }
    if (!io_group) {
      auto threads = std::thread::hardware_concurrency();
      if (threads <= 0) {
        // Reasonable mid-point for concurrency when actual value unknown
        threads = 8;
      }
      io_group = std::make_shared<folly::IOThreadPoolExecutor>(
          threads, std::make_shared<folly::NamedThreadFactory>("IO Thread"));
    }

    // TODO better config checking
    // CHECK(acceptorFactory_ || childPipelineFactory_);
    CHECK(!(acceptorFactory_ && childPipelineFactory_));

    if (acceptorFactory_) {
      workerFactory_ = std::make_shared<ServerWorkerPool>(
          acceptorFactory_, sockets_, socketFactory_);
    } else {
      auto acceptorFactory = std::make_shared<ServerAcceptorFactory<Pipeline>>(
          acceptPipelineFactory_, childPipelineFactory_, accConfig_);
      acceptorFactory->enableSharedSSLContext(useSharedSSLContextManager_);
      sharedSSLContextManager_ = acceptorFactory->getSharedSSLContextManager();
      workerFactory_ = std::make_shared<ServerWorkerPool>(
          acceptorFactory, sockets_, socketFactory_);
    }

    io_group->addObserver(workerFactory_);

    acceptor_group_ = accept_group;
    io_group_ = io_group;

    return this;
  }

  /*
   * Bind to an existing socket
   *
   * @param sock Existing socket to use for accepting
   */
  void bind(folly::AsyncServerSocket::UniquePtr s) {
    if (!workerFactory_) {
      group(nullptr);
    }

    // Since only a single socket is given,
    // we can only accept on a single thread
    CHECK(acceptor_group_->numThreads() == 1);

    std::shared_ptr<folly::AsyncServerSocket> socket(
        s.release(), AsyncServerSocketFactory::ThreadSafeDestructor());
    socket->setMaxNumMessagesInQueue(
        accConfig_.maxNumPendingConnectionsPerWorker);

    folly::via(acceptor_group_.get(), [&] {
      if (socketConfig.useZeroCopy) {
        socket->setZeroCopy(true);
      }
      socket->attachEventBase(folly::EventBaseManager::get()->getEventBase());
      socket->listen(socketConfig.acceptBacklog);
      socket->startAccepting();
    }).get();

    // Startup all the threads
    workerFactory_->forEachWorker([this, socket](Acceptor* worker) {
      socket->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
          [this, worker, socket]() {
            socketFactory_->addAcceptCB(socket, worker, worker->getEventBase());
          });
    });

    sockets_->push_back(socket);
  }

  void bind(folly::SocketAddress& address) {
    bindImpl(address);
  }

  /*
   * Bind to a port and start listening.
   * One of childPipeline or childHandler must be called before bind
   *
   * @param port Port to listen on
   */
  void bind(int port) {
    CHECK(port >= 0);
    folly::SocketAddress address;
    address.setFromLocalPort(port);
    bindImpl(address);
  }

  void bindImpl(folly::SocketAddress& address) {
    if (!workerFactory_) {
      group(nullptr);
    }

    bool reusePort = reusePort_ || (acceptor_group_->numThreads() > 1) ||
        accConfig_.reusePort;

    std::mutex sock_lock;
    std::vector<std::shared_ptr<folly::AsyncSocketBase>> new_sockets;

    std::exception_ptr exn;

    auto startupFunc = [&](std::shared_ptr<folly::Baton<>> barrier) {
      try {
        auto socket = socketFactory_->newSocket(
            address, socketConfig.acceptBacklog, reusePort, socketConfig);
        sock_lock.lock();
        new_sockets.push_back(socket);
        sock_lock.unlock();
        socket->getAddress(&address);

        barrier->post();
      } catch (...) {
        exn = std::current_exception();
        barrier->post();

        return;
      }
    };

    auto wait0 = std::make_shared<folly::Baton<>>();
    acceptor_group_->add(std::bind(startupFunc, wait0));
    wait0->wait();

    for (size_t i = 1; i < acceptor_group_->numThreads(); i++) {
      auto barrier = std::make_shared<folly::Baton<>>();
      acceptor_group_->add(std::bind(startupFunc, barrier));
      barrier->wait();
    }

    if (exn) {
      std::rethrow_exception(exn);
    }

    for (auto& socket : new_sockets) {
      // Startup all the threads
      workerFactory_->forEachWorker([this, socket](Acceptor* worker) {
        socket->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
            [this, worker, socket]() {
              socketFactory_->addAcceptCB(
                  socket, worker, worker->getEventBase());
            });
      });

      sockets_->push_back(socket);
    }
  }

  /*
   * Stop listening on all sockets.
   */
  void stop() {
    // sockets_ may be null if ServerBootstrap has been std::move'd
    if (sockets_) {
      sockets_->clear();
    }
    if (!stopped_) {
      stopped_ = true;
      // stopBaton_ may be null if ServerBootstrap has been std::move'd
      if (stopBaton_) {
        stopBaton_->post();
      }
    }
  }

  void join() {
    if (acceptor_group_) {
      acceptor_group_->join();
    }
    if (io_group_) {
      io_group_->join();
    }
  }

  void waitForStop() {
    if (stopBaton_) {
      stopBaton_->wait();
      stopBaton_.reset();
    }
    CHECK(stopped_);
  }

  /*
   * Get the list of listening sockets
   */
  const std::vector<std::shared_ptr<folly::AsyncSocketBase>>& getSockets()
      const {
    return *sockets_;
  }

  std::shared_ptr<SharedSSLContextManager> getSharedSSLContextManager() const {
    return sharedSSLContextManager_;
  }

  std::shared_ptr<folly::IOThreadPoolExecutorBase> getIOGroup() const {
    return io_group_;
  }

  template <typename F>
  void forEachWorker(F&& f) const {
    if (!workerFactory_) {
      return;
    }
    workerFactory_->forEachWorker(f);
  }

  ServerSocketConfig socketConfig;

  ServerBootstrap* setReusePort(bool reusePort) {
    reusePort_ = reusePort;
    return this;
  }

  ServerBootstrap* setUseSharedSSLContextManager(bool enabled) {
    useSharedSSLContextManager_ = enabled;
    return this;
  }

 protected:
  void acceptConnection(
      folly::NetworkSocket fd,
      const folly::SocketAddress& clientAddr,
      folly::AsyncServerSocket::AcceptCallback::AcceptInfo info,
      folly::AsyncSocket::LegacyLifecycleObserver* observer) {
    workerFactory_->forRandomWorker([&](Acceptor* acceptor) {
      acceptor->getEventBase()->runInEventBaseThread(
          [=] { acceptor->acceptConnection(fd, clientAddr, info, observer); });
    });
  }

 private:
  std::shared_ptr<folly::IOThreadPoolExecutorBase> acceptor_group_;
  std::shared_ptr<folly::IOThreadPoolExecutorBase> io_group_;
  std::shared_ptr<SharedSSLContextManager> sharedSSLContextManager_;

  std::shared_ptr<ServerWorkerPool> workerFactory_;
  std::shared_ptr<std::vector<std::shared_ptr<folly::AsyncSocketBase>>>
      sockets_{std::make_shared<
          std::vector<std::shared_ptr<folly::AsyncSocketBase>>>()};

  std::shared_ptr<AcceptorFactory> acceptorFactory_;
  std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory_;
  std::shared_ptr<AcceptPipelineFactory> acceptPipelineFactory_{
      std::make_shared<DefaultAcceptPipelineFactory>()};
  std::shared_ptr<ServerSocketFactory> socketFactory_{
      std::make_shared<AsyncServerSocketFactory>()};

  ServerSocketConfig accConfig_;

  bool reusePort_{false};

  std::unique_ptr<folly::Baton<>> stopBaton_{
      std::make_unique<folly::Baton<>>()};
  bool stopped_{false};
  bool useSharedSSLContextManager_{false};
};

} // namespace wangle
