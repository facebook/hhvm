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

#ifndef THRIFT_UTIL_SCOPEDSERVERTHREAD_H_
#define THRIFT_UTIL_SCOPEDSERVERTHREAD_H_ 1

#include <memory>
#include <string>

#include <folly/Function.h>

namespace folly {
class SocketAddress;
}

namespace apache {
namespace thrift {

class BaseThriftServer;

namespace concurrency {
class Thread;
}
namespace util {

/**
 * ScopedServerThread spawns a thrift server in a new thread.
 *
 * The server is stopped automatically when the ScopedServerThread is
 * destroyed.
 */
class ScopedServerThread {
 public:
  /**
   * Create a new, unstarted ScopedServerThread object.
   */
  ScopedServerThread();

  /**
   * Create a ScopedServerThread object and automatically start it.
   */
  explicit ScopedServerThread(std::shared_ptr<BaseThriftServer> server);

  ScopedServerThread(const ScopedServerThread&) = delete;
  ScopedServerThread& operator=(const ScopedServerThread&) = delete;

  virtual ~ScopedServerThread();

  /**
   * Start the server thread.
   *
   * This method does not return until the server has successfully started.
   *
   * @param server The server to run in the new thread.
   */
  using Func = folly::Function<void()>;
  void start(std::shared_ptr<BaseThriftServer> server, Func onExit = {});

  /**
   * Stop the server thread.
   */
  void stop();

  /**
   * Waits for the server thread to finish.
   * Note that this doesn't stop the thread.
   */
  void join();

  /**
   * Get the address on which the server is listening.
   */
  const folly::SocketAddress* getAddress() const;

  /**
   * Get the server.
   */
  std::weak_ptr<BaseThriftServer> getServer() const;

  /**
   * Set name on the underlying server thread.
   *
   * @param name The name for the thread.
   */
  bool setServeThreadName(const std::string& name);

 private:
  class Helper;

  std::shared_ptr<Helper> helper_;
  std::shared_ptr<concurrency::Thread> thread_;
};

} // namespace util
} // namespace thrift
} // namespace apache

#endif // THRIFT_UTIL_SCOPEDSERVERTHREAD_H_
