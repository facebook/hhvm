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

#include <folly/io/async/test/SocketPair.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/ThriftRocketServerHandler.h>

namespace apache::thrift::rocket::test {

class FakeTransport final : public folly::AsyncTransport {
 public:
  explicit FakeTransport(folly::EventBase* e) : eventBase_(e) {}
  void setReadCB(ReadCallback*) override {}
  ReadCallback* getReadCallback() const override { return nullptr; }
  void write(
      WriteCallback* cb, const void*, size_t, folly::WriteFlags) override {
    cb->writeSuccess();
  }
  void writev(
      WriteCallback* cb, const iovec*, size_t, folly::WriteFlags) override {
    cb->writeSuccess();
  }
  void writeChain(
      WriteCallback* cb,
      std::unique_ptr<folly::IOBuf>&&,
      folly::WriteFlags) override {
    cb->writeSuccess();
  }
  folly::EventBase* getEventBase() const override { return eventBase_; }
  void getAddress(folly::SocketAddress*) const override {}
  void close() override {}
  void closeNow() override {}
  void shutdownWrite() override {}
  void shutdownWriteNow() override {}
  bool good() const override { return true; }
  bool readable() const override { return true; }
  bool connecting() const override { return true; }
  bool error() const override { return true; }
  void attachEventBase(folly::EventBase*) override {}
  void detachEventBase() override {}
  bool isDetachable() const override { return true; }
  void setSendTimeout(uint32_t) override {}
  uint32_t getSendTimeout() const override { return 0u; }
  void getLocalAddress(folly::SocketAddress*) const override {}
  void getPeerAddress(folly::SocketAddress*) const override {}
  bool isEorTrackingEnabled() const override { return true; }
  void setEorTracking(bool) override {}
  size_t getAppBytesWritten() const override { return 0u; }
  size_t getRawBytesWritten() const override { return 0u; }
  size_t getAppBytesReceived() const override { return 0u; }
  size_t getRawBytesReceived() const override { return 0u; }

 private:
  folly::EventBase* eventBase_;
};

class FakeProcessor final : public apache::thrift::AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&&,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {
    req->sendErrorWrapped(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::TApplicationExceptionType::
                INTERNAL_ERROR,
            "place holder"),
        "1" /* doesnt matter */);
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {
    LOG(FATAL)
        << "This AsyncProcessor doesn't support Thrift interactions. "
        << "Please implement processInteraction to support interactions.";
  }

  void executeRequest(
      ServerRequest&& req,
      const AsyncProcessorFactory::MethodMetadata&) override {
    req.request()->sendErrorWrapped(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::TApplicationExceptionType::
                INTERNAL_ERROR,
            "place holder"),
        "1" /* doesnt matter */);
  }
};

class FakeProcessorFactory final
    : public apache::thrift::AsyncProcessorFactory {
 public:
  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::make_unique<FakeProcessor>();
  }

  CreateMethodMetadataResult createMethodMetadata() override {
    WildcardMethodMetadataMap wildcardMap;
    wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>(
        MethodMetadata::ExecutorType::ANY);
    wildcardMap.knownMethods = {};
    return wildcardMap;
  }

  std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
      override {
    return {};
  }
};

void testServerOneInput(const uint8_t* Data, size_t Size) {
  folly::EventBase evb;
  auto sock = folly::AsyncTransport::UniquePtr(
      new apache::thrift::rocket::test::FakeTransport(&evb));
  auto* const sockPtr = sock.get();
  apache::thrift::ThriftServer server;
  server.setInterface(std::make_shared<FakeProcessorFactory>());
  auto worker = apache::thrift::Cpp2Worker::create(&server, &evb);
  std::vector<std::unique_ptr<apache::thrift::rocket::SetupFrameHandler>> v;
  std::vector<std::unique_ptr<apache::thrift::rocket::SetupFrameInterceptor>> i;
  folly::SocketAddress address;
  MemoryTracker memoryTracker;

  auto connection = new apache::thrift::rocket::RocketServerConnection(
      std::move(sock),
      std::make_unique<apache::thrift::rocket::ThriftRocketServerHandler>(
          worker, address, sockPtr, v, i),
      memoryTracker, // (ingress)
      memoryTracker); // (egress)

  folly::DelayedDestruction::DestructorGuard dg(connection);
  apache::thrift::rocket::Parser<apache::thrift::rocket::RocketServerConnection>
      p(*connection, THRIFT_FLAG(rocket_frame_parser));
  evb.runInEventBaseThread([&]() {
    size_t left = Size;
    while (left != 0) {
      void* buffer;
      size_t length;
      p.getReadBuffer(&buffer, &length);
      size_t lenToRead = std::min(left, length);
      memcpy(buffer, Data, lenToRead);
      p.readDataAvailable(lenToRead);
      Data += lenToRead;
      left -= lenToRead;
    }
    connection->close(folly::exception_wrapper());
  });
  evb.loop();
}

void testClientOneInput(const uint8_t* Data, size_t Size) {
  folly::EventBase evb;
  folly::SocketPair sp;
  auto sock = folly::AsyncTransport::UniquePtr(
      new folly::AsyncSocket(&evb, sp.extractNetworkSocket0()));
  auto channel =
      apache::thrift::RocketClientChannel::newChannel(std::move(sock));
  apache::thrift::RpcOptions rpcOptions;
  apache::thrift::SerializedRequest request(std::make_unique<folly::IOBuf>());
  auto tHeader = std::make_shared<apache::thrift::transport::THeader>();
  apache::thrift::ClientReceiveState _returnState;
  apache::thrift::ClientSyncCallback<false> callback(&_returnState);
  channel->sendRequestResponse(
      rpcOptions,
      "",
      std::move(request),
      std::move(tHeader),
      apache::thrift::RequestClientCallback::Ptr(&callback),
      nullptr /* frameworkMetadata */);
  evb.drive();
  int fd = sp.extractFD1();
  write(fd, Data, Size);
  close(fd);
  callback.waitUntilDone(&evb);
}

} // namespace apache::thrift::rocket::test
