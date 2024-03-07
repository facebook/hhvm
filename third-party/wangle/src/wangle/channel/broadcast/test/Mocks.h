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

#include <folly/MoveWrapper.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <wangle/channel/broadcast/BroadcastHandler.h>
#include <wangle/channel/broadcast/BroadcastPool.h>
#include <wangle/channel/broadcast/ObservingHandler.h>
#include <wangle/codec/ByteToMessageDecoder.h>
#include <wangle/codec/MessageToByteEncoder.h>

namespace wangle {

template <typename T, typename R>
class MockSubscriber : public Subscriber<T, R> {
 public:
  MOCK_METHOD1_T(onNext, void(const T&));
  MOCK_METHOD1(onError, void(const folly::exception_wrapper ex));
  MOCK_METHOD0(onCompleted, void());

  MOCK_METHOD0_T(routingData, R&());

  folly::Future<BroadcastHandler<int, std::string>*> getHandler();
};

template <typename T>
class MockByteToMessageDecoder : public ByteToMessageDecoder<T> {
 public:
  using Context = typename ByteToMessageDecoder<T>::Context;

  MOCK_METHOD4_T(decode, bool(Context*, folly::IOBufQueue&, T&, size_t&));
};

template <typename T>
class MockMessageToByteEncoder : public MessageToByteEncoder<T> {
 public:
  using Context = typename MessageToByteEncoder<T>::Context;

  MOCK_METHOD1_T(
      mockEncode,
      folly::MoveWrapper<std::unique_ptr<folly::IOBuf>>(T&));

  std::unique_ptr<folly::IOBuf> encode(T& data) override {
    return mockEncode(data).move();
  }
};

class MockServerPool : public ServerPool<std::string> {
 public:
  explicit MockServerPool(std::shared_ptr<folly::SocketAddress> addr)
      : ServerPool(), addr_(addr) {}

  folly::Future<DefaultPipeline*> connect(
      BaseClientBootstrap<DefaultPipeline>* client,
      const std::string& /* routingData */) noexcept override {
    return failConnect_ ? folly::makeFuture<DefaultPipeline*>(std::exception())
                        : client->connect(*addr_);
  }

  void failConnect() {
    failConnect_ = true;
  }

 private:
  std::shared_ptr<folly::SocketAddress> addr_;
  bool failConnect_{false};
};

class MockBroadcastPool : public BroadcastPool<int, std::string> {
 public:
  MockBroadcastPool()
      : BroadcastPool<int, std::string>(
            nullptr,
            nullptr,
            std::make_shared<ClientBootstrapFactory>()) {}

  MOCK_METHOD1_T(
      mockGetHandler,
      folly::MoveWrapper<folly::Future<BroadcastHandler<int, std::string>*>>(
          const std::string&));

  folly::Future<BroadcastHandler<int, std::string>*> getHandler(
      const std::string& routingData) override {
    return mockGetHandler(routingData).move();
  }
};

class MockObservingHandler : public ObservingHandler<int, std::string> {
 public:
  explicit MockObservingHandler(BroadcastPool<int, std::string>* broadcastPool)
      : ObservingHandler<int, std::string>("", broadcastPool) {}

  MOCK_METHOD2(
      mockWrite,
      folly::MoveWrapper<folly::Future<folly::Unit>>(Context*, int));
  MOCK_METHOD1(
      mockClose,
      folly::MoveWrapper<folly::Future<folly::Unit>>(Context*));

  folly::Future<folly::Unit> write(Context* ctx, int data) override {
    return mockWrite(ctx, data).move();
  }

  folly::Future<folly::Unit> close(Context* ctx) override {
    return mockClose(ctx).move();
  }
};

class MockBroadcastHandler : public BroadcastHandler<int, std::string> {
 public:
  MOCK_METHOD1(subscribe, uint64_t(Subscriber<int, std::string>*));
  MOCK_METHOD1(unsubscribe, void(uint64_t));
};

class MockBroadcastPipelineFactory
    : public BroadcastPipelineFactory<int, std::string> {
 public:
  DefaultPipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransport> socket) override {
    auto pipeline = DefaultPipeline::create();
    pipeline->addBack(AsyncSocketHandler(socket));
    pipeline->addBack(std::make_shared<MockByteToMessageDecoder<int>>());
    pipeline->addBack(BroadcastHandler<int, std::string>());
    pipeline->finalize();

    return pipeline;
  }

  BroadcastHandler<int, std::string>* getBroadcastHandler(
      DefaultPipeline* pipeline) noexcept override {
    return pipeline->getHandler<BroadcastHandler<int, std::string>>(2);
  }

  MOCK_METHOD2(setRoutingData, void(DefaultPipeline*, const std::string&));
};

class MockObservingPipelineFactory
    : public ObservingPipelineFactory<int, std::string> {
 public:
  MockObservingPipelineFactory(
      std::shared_ptr<ServerPool<std::string>> serverPool,
      std::shared_ptr<BroadcastPipelineFactory<int, std::string>>
          broadcastPipelineFactory)
      : ObservingPipelineFactory(serverPool, broadcastPipelineFactory) {}

  ObservingPipeline<int>::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransport>,
      const std::string& routingData,
      RoutingDataHandler<std::string>*,
      std::shared_ptr<TransportInfo>) override {
    auto pipeline = ObservingPipeline<int>::create();
    pipeline->addBack(std::make_shared<wangle::BytesToBytesHandler>());
    pipeline->addBack(std::make_shared<MockMessageToByteEncoder<int>>());
    auto handler = std::make_shared<ObservingHandler<int, std::string>>(
        routingData, broadcastPool());
    pipeline->addBack(handler);
    pipeline->finalize();

    return pipeline;
  }
};

} // namespace wangle
