/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <exception>

#include <thrift/lib/cpp/Thrift.h>

namespace folly {
class SocketAddress;
class RequestContext;
} // namespace folly

namespace apache {
namespace thrift {
namespace server {

class TConnectionContext;

/**
 * Virtual interface class that can handle events from the server core. To
 * use this you should subclass it and implement the methods that you care
 * about. Your subclass can also store local data that you may care about,
 * such as additional "arguments" to these methods (stored in the object
 * instance's state).
 */
class TServerEventHandler {
 public:
  virtual ~TServerEventHandler() {}

  /**
   * Called before the onStartServing is called.
   *
   * @param address The address on which the server is listening.
   */
  virtual void preStart(const folly::SocketAddress* /*address*/) {}

  /**
   * Called before the server begins.
   *
   * @param address The address on which the server is listening.
   */
  virtual void preServe(const folly::SocketAddress* /*address*/) {}

  /**
   * Called if the server will not begin.
   *
   * @param e The exception that caused the failure, if any.
   */
  virtual void handleServeError(const std::exception& x) { (void)x; }

  void handleServeError() {
    handleServeError(TLibraryException("serve() threw non-exception type"));
  }

  /**
   * Called when a new client has connected and is about to begin processing.
   *
   * @param ctx A pointer to the connection context.  The context will remain
   *            valid until the corresponding connectionDestroyed() call.
   */
  virtual void newConnection(TConnectionContext* ctx) { (void)ctx; }

  /**
   * Called when a client has finished request-handling to delete server
   * context.
   *
   * @param ctx A pointer to the connection context.  The context will be
   *            destroyed after connectionDestroyed() returns.
   */
  virtual void connectionDestroyed(TConnectionContext* ctx) { (void)ctx; }

  /**
   * Called after the server stops.
   */
  virtual void postStop() {}

 protected:
  /**
   * Prevent direct instantiation.
   */
  TServerEventHandler() {}
};

} // namespace server
} // namespace thrift
} // namespace apache
