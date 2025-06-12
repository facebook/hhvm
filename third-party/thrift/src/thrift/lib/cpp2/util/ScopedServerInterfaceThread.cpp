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

#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <folly/SocketAddress.h>

#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

using namespace std;
using namespace folly;
using namespace apache::thrift::concurrency;

namespace apache::thrift {

ScopedServerInterfaceThread::ScopedServerInterfaceThread(
    shared_ptr<AsyncProcessorFactory> apf,
    const SocketAddress& addr,
    ServerConfigCb configCb) {
  auto ts = make_shared<ThriftServer>();
  ts->setAddress(addr);
  // Allow plaintext on loopback so the plaintext clients created by default
  // by the newClient methods can still connect.
  ts->setAllowPlaintextOnLoopback(true);
  ts->setInterface(std::move(apf));
  ts->setNumIOWorkerThreads(1);
  ts->setNumCPUWorkerThreads(1);
  auto tf = make_shared<PosixThreadFactory>(PosixThreadFactory::ATTACHED);
  ts->setThreadFactory(std::move(tf));
  ts->setThreadManagerType(
      apache::thrift::ThriftServer::ThreadManagerType::SIMPLE);

  // The default behavior is to keep N recent requests per IO worker in
  // memory. In unit-tests, this defers memory reclamation and potentially
  // masks use-after-free bugs. Because this facility is used mostly in tests,
  // it is better not to keep any recent requests in memory.
  ts->setMaxFinishedDebugPayloadsPerWorker(0);
  if (configCb) {
    configCb(*ts);
  }
  ts_ = ts;
  sst_.start(ts_, [ts]() { ts->getEventBaseManager()->clearEventBase(); });
}

ScopedServerInterfaceThread::ScopedServerInterfaceThread(
    shared_ptr<AsyncProcessorFactory> apf,
    const string& host,
    uint16_t port,
    ServerConfigCb configCb)
    : ScopedServerInterfaceThread(
          std::move(apf), SocketAddress(host, port), std::move(configCb)) {}

ScopedServerInterfaceThread::ScopedServerInterfaceThread(
    shared_ptr<AsyncProcessorFactory> apf, ServerConfigCb configCb)
    : ScopedServerInterfaceThread(
          std::move(apf), "::1", 0, std::move(configCb)) {}

ScopedServerInterfaceThread::ScopedServerInterfaceThread(
    shared_ptr<ThriftServer> bts) {
  ts_ = bts;
  sst_.start(ts_);
}

ThriftServer& ScopedServerInterfaceThread::getThriftServer() const {
  return *ts_;
}

const SocketAddress& ScopedServerInterfaceThread::getAddress() const {
  return *sst_.getAddress();
}

uint16_t ScopedServerInterfaceThread::getPort() const {
  return getAddress().getPort();
}

RequestChannel::Ptr ScopedServerInterfaceThread::newChannel(
    folly::Executor* callbackExecutor,
    MakeChannelFunc makeChannel,
    size_t numThreads,
    protocol::PROTOCOL_TYPES prot) const {
  return PooledRequestChannel::newChannel(
      callbackExecutor,
      [makeChannel = std::move(makeChannel),
       address = getAddress()](folly::EventBase& eb) mutable {
        return makeChannel(folly::AsyncSocket::UniquePtr(
            new folly::AsyncSocket(&eb, address)));
      },
      numThreads,
      prot);
}

namespace {
struct TestClientRunner {
  ScopedServerInterfaceThread runner;
  RequestChannel::Ptr channel;

  explicit TestClientRunner(std::shared_ptr<AsyncProcessorFactory> apf)
      : runner(std::move(apf)) {}
};
} // namespace

std::shared_ptr<RequestChannel>
ScopedServerInterfaceThread::makeTestClientChannel(
    std::shared_ptr<AsyncProcessorFactory> apf,
    ScopedServerInterfaceThread::FaultInjectionFunc injectFault,
    ScopedServerInterfaceThread::StreamFaultInjectionFunc streamInjectFault,
    protocol::PROTOCOL_TYPES prot) {
  auto runner = std::make_shared<TestClientRunner>(std::move(apf));
  auto makeChannel = [prot](folly::AsyncSocket::UniquePtr socket) {
    auto channel = RocketClientChannel::newChannel(std::move(socket));
    channel->setProtocolId(prot);
    return channel;
  };
  auto innerChannel = runner->runner.newChannel(
      folly::getGlobalCPUExecutor().get(),
      makeChannel,
      folly::hardware_concurrency(),
      prot);
  if (injectFault || streamInjectFault) {
    runner->channel.reset(new apache::thrift::detail::FaultInjectionChannel(
        std::move(innerChannel),
        std::move(injectFault),
        std::move(streamInjectFault)));
  } else {
    runner->channel = std::move(innerChannel);
  }
  auto* channel = runner->channel.get();
  return folly::to_shared_ptr_aliasing(std::move(runner), channel);
}

namespace detail {
void validateServiceName(
    AsyncProcessorFactory& apf, std::string_view serviceName) {
  if (auto* service = dynamic_cast<ServerInterface*>(&apf)) {
    std::string actualServiceName{service->getGeneratedName()};
    CHECK_EQ(actualServiceName, serviceName)
        << "Client and handler type mismatch";
  }
}
} // namespace detail
} // namespace apache::thrift
