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

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

#include <folly/ScopeGuard.h>
#include <folly/SocketAddress.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

using std::shared_ptr;
using std::weak_ptr;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;

namespace apache {
namespace thrift {
namespace util {

/**
 * ScopedServerThread::Helper runs the server loop in the new server thread.
 *
 * It also provides a waitUntilStarted() method that the main thread can use to
 * block until the server has started listening for new connections.
 */
class ScopedServerThread::Helper : public Runnable, public TServerEventHandler {
 public:
  Helper() : state_(STATE_NOT_STARTED) {}

  void init(
      shared_ptr<ThriftServer> server, shared_ptr<Helper> self, Func onExit);

  void run() override;

  /**
   * Stop the server.
   *
   * This may be called from the main thread.
   */
  void stop();

  /**
   * Get the address that the server is listening on.
   *
   * This may be called from the main thread.
   */
  const folly::SocketAddress* getAddress() const { return &address_; }

  /**
   * Wait until the server has started.
   *
   * Raises TException if the server failed to start.
   */
  void waitUntilStarted();

  void preServe(const folly::SocketAddress* address) override;

  const shared_ptr<ThriftServer>& getServer() const { return server_; }

  void releaseServer() { server_.reset(); }

 private:
  enum StateEnum {
    STATE_NOT_STARTED,
    STATE_RUNNING,
    STATE_START_ERROR,
  };

  class EventHandler : public TServerEventHandler {
   public:
    explicit EventHandler(const shared_ptr<Helper>& outer) : outer_(outer) {}
    void preServe(const folly::SocketAddress* address) override;

   private:
    weak_ptr<Helper> outer_;
  };

  void handleServeError(const std::exception&) override {
    // TODO(praihan): This function is no longer called by the Thrift server
    // runtime. It should be removed.
    std::terminate();
  }

  void handleServeErrorImpl(folly::exception_wrapper ex) {
    std::unique_lock<std::mutex> l(stateMutex_);

    if (state_ == STATE_NOT_STARTED) {
      // If the error occurred before the server started,
      // save a copy of the error and notify the main thread
      savedError_ = ex;
      state_ = STATE_START_ERROR;
      stateCondVar_.notify_one();
    } else {
      // The error occurred during normal server execution.
      // Just log an error message.
      T_ERROR(
          "ScopedServerThread: serve() raised a %s while running: %s",
          ex.class_name().c_str(),
          ex.what().c_str());
    }
  }

  StateEnum state_;
  std::mutex stateMutex_;
  std::condition_variable stateCondVar_;

  shared_ptr<ThriftServer> server_;
  Func onExit_;
  folly::exception_wrapper savedError_;
  folly::SocketAddress address_;
};

void ScopedServerThread::Helper::init(
    shared_ptr<ThriftServer> server, shared_ptr<Helper> self, Func onExit) {
  server_ = std::move(server);
  onExit_ = std::move(onExit);

  // Install ourself as the server event handler, so that our preServe() method
  // will be called once we've successfully bound to the port and are about to
  // start listening.
  // This function takes self as an argument since we need a shared_ptr to
  // ourselves.
  server_->addServerEventHandler(std::make_shared<EventHandler>(self));
}

void ScopedServerThread::Helper::run() {
  // Call serve()
  //
  // If an error occurs before the server finishes starting, save the error and
  // notify the main thread of the failure.
  try {
    server_->serve();
  } catch (...) {
    handleServeErrorImpl(folly::exception_wrapper(std::current_exception()));
  }
  if (onExit_) {
    onExit_();
  }
}

void ScopedServerThread::Helper::stop() {
  if (server_) {
    server_->stop();
  }
}

void ScopedServerThread::Helper::waitUntilStarted() {
  std::unique_lock<std::mutex> l(stateMutex_);
  while (state_ == STATE_NOT_STARTED) {
    stateCondVar_.wait(l);
  }

  // If an error occurred starting the server,
  // throw the saved error in the main thread
  if (state_ == STATE_START_ERROR) {
    assert(savedError_.has_exception_ptr());
    savedError_.throw_exception();
  }
}

void ScopedServerThread::Helper::preServe(const folly::SocketAddress* address) {
  // Save a copy of the address
  address_ = *address;

  // Inform the main thread that the server has started
  std::unique_lock<std::mutex> l(stateMutex_);
  assert(state_ == STATE_NOT_STARTED);
  state_ = STATE_RUNNING;
  stateCondVar_.notify_one();
}

void ScopedServerThread::Helper::EventHandler::preServe(
    const folly::SocketAddress* address) {
  auto outer = outer_.lock();
  if (outer) {
    outer->preServe(address);
  }
}

/*
 * ScopedServerThread methods
 */

ScopedServerThread::ScopedServerThread() {}

ScopedServerThread::ScopedServerThread(shared_ptr<ThriftServer> server) {
  start(std::move(server));
}

ScopedServerThread::~ScopedServerThread() {
  stop();
}

void ScopedServerThread::start(shared_ptr<ThriftServer> server, Func onExit) {
  if (helper_) {
    throw TLibraryException("ScopedServerThread is already running");
  }

  // Thrift forces us to use shared_ptr for both Runnable and
  // TServerEventHandler objects, so we have to allocate Helper on the heap,
  // rather than having it as a member variable or deriving from Runnable and
  // TServerEventHandler ourself.
  auto helper = std::make_shared<Helper>();

  helper->init(std::move(server), helper, std::move(onExit));

  // Start the thread
  PosixThreadFactory threadFactory;
  threadFactory.setDetached(false);
  auto thread = threadFactory.newThread(helper);
  thread->start();
  auto threadGuard = folly::makeGuard([&] {
    helper->stop();
    thread->join();
  });

  // Wait for the server to start
  helper->waitUntilStarted();

  helper_ = std::move(helper);
  thread_ = std::move(thread);
  threadGuard.dismiss();
}

void ScopedServerThread::stop() {
  if (!helper_) {
    return;
  }

  helper_->stop();
  join();
  helper_.reset();
}

void ScopedServerThread::join() {
  if (thread_) {
    thread_->join();
    // Make sure we don't try to join the thread again from the destructor.
    thread_ = nullptr;
  }
  if (helper_) {
    helper_->releaseServer();
  }
}

const folly::SocketAddress* ScopedServerThread::getAddress() const {
  if (!helper_) {
    throw TTransportException(
        TTransportException::NOT_OPEN,
        "attempted to get address of stopped "
        "ScopedServerThread");
  }

  return helper_->getAddress();
}

weak_ptr<ThriftServer> ScopedServerThread::getServer() const {
  if (!helper_) {
    return weak_ptr<ThriftServer>();
  }
  return helper_->getServer();
}

bool ScopedServerThread::setServeThreadName(const std::string& name) {
  return thread_->setName(name);
}

} // namespace util
} // namespace thrift
} // namespace apache
