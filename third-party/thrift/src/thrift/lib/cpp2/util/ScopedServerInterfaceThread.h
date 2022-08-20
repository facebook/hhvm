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

#include <memory>

#include <folly/Function.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/system/HardwareConcurrency.h>

#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

namespace apache {
namespace thrift {

class AsyncProcessorFactory;
class BaseThriftServer;
class ThriftServer;

/**
 * ScopedServerInterfaceThread spawns a thrift cpp2 server in a new thread.
 *
 * The server is stopped automatically when the instance is destroyed.
 */
class ScopedServerInterfaceThread {
 public:
  using ServerConfigCb = folly::Function<void(ThriftServer&)>;
  using MakeChannelFunc =
      folly::Function<RequestChannel::Ptr(folly::AsyncSocket::UniquePtr)>;
  using FaultInjectionFunc =
      folly::Function<folly::exception_wrapper(folly::StringPiece methodName)>;

  ScopedServerInterfaceThread(
      std::shared_ptr<AsyncProcessorFactory> apf,
      const folly::SocketAddress& addr,
      ServerConfigCb configCb = {});

  explicit ScopedServerInterfaceThread(
      std::shared_ptr<AsyncProcessorFactory> apf,
      const std::string& host = "::1",
      uint16_t port = 0,
      ServerConfigCb configCb = {});

  explicit ScopedServerInterfaceThread(
      std::shared_ptr<AsyncProcessorFactory> apf, ServerConfigCb configCb);

  explicit ScopedServerInterfaceThread(std::shared_ptr<BaseThriftServer> ts);

  BaseThriftServer& getThriftServer() const;
  const folly::SocketAddress& getAddress() const;
  uint16_t getPort() const;

  /**
   * Creates and returns a new AsyncClientT. Uses the given channelFunc to
   * create a channel for the client by passing it a connected, plaintext
   * folly::AsyncSocket.
   */
  template <class AsyncClientT>
  std::unique_ptr<AsyncClientT> newClient(
      folly::Executor* callbackExecutor = nullptr,
      MakeChannelFunc channelFunc = RocketClientChannel::newChannel) const;

  /**
   * Like newClient but invokes injectFault before each request and
   * short-circuits the request if it returns an exception.
   * Useful for testing handling of e.g. TTransportException.
   */
  template <class AsyncClientT>
  std::unique_ptr<AsyncClientT> newClientWithFaultInjection(
      FaultInjectionFunc injectFault,
      folly::Executor* callbackExecutor = nullptr,
      MakeChannelFunc channelFunc = RocketClientChannel::newChannel) const;

  /**
   * Like newClient but sends all requests over a single internal channel
   */
  template <class AsyncClientT>
  std::unique_ptr<AsyncClientT> newStickyClient(
      folly::Executor* callbackExecutor = nullptr,
      MakeChannelFunc channelFunc = RocketClientChannel::newChannel) const;

  static std::shared_ptr<RequestChannel> makeTestClientChannel(
      std::shared_ptr<AsyncProcessorFactory> apf,
      ScopedServerInterfaceThread::FaultInjectionFunc injectFault);

 private:
  std::shared_ptr<BaseThriftServer> ts_;
  util::ScopedServerThread sst_;

  RequestChannel::Ptr newChannel(
      folly::Executor* callbackExecutor,
      MakeChannelFunc channelFunc,
      size_t numThreads = folly::hardware_concurrency()) const;
};

namespace detail {
template <typename T>
T get_service_tag(ServiceHandler<T>*);

template <typename ServiceHandler>
using get_service_tag_t =
    decltype(get_service_tag(std::declval<ServiceHandler*>()));
} // namespace detail

/**
 * Creates a AsyncClientT for a given handler, backed by an internal
 * ScopedServerInterfaceThread, with optional fault injection
 * (if set, invokes injectFault before each request and
 * short-circuits the request if it returns an exception.
 * Useful for testing handling of e.g. TTransportException.)
 *
 * This is more convenient but offers less control than managing
 * your own ScopedServerInterfaceThread.
 */
template <
    class ServiceHandler,
    class ServiceTag =
        apache::thrift::detail::get_service_tag_t<ServiceHandler>>
std::unique_ptr<Client<ServiceTag>> makeTestClient(
    std::shared_ptr<ServiceHandler> handler,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault = nullptr);

template <class AsyncClientT>
std::unique_ptr<AsyncClientT> makeTestClient(
    std::shared_ptr<AsyncProcessorFactory> apf,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault = nullptr);
} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread-inl.h>
