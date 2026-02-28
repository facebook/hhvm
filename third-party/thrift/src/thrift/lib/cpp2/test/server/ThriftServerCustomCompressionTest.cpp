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

#include <folly/test/TestUtils.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/test/server/ThriftServerTestUtils.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorRegistry.h>

using namespace apache::thrift;
using namespace apache::thrift::test;

/*
 * This compressor does the following:
 * # Negotiation
 * 1. client uses payload to propose a prefix
 * 2. server sends back the final prefix
 *
 * # Compressor
 * 1. compress -> append the final prefix to the beginning
 * 2. uncompress -> strip the final prefix from the beginning
 */
class TestCustomCompressor : public rocket::CustomCompressor {
 public:
  enum class CompressBehavior { Success, Throw, Unreachable };
  enum class UncompressBehavior { Success, Throw, Unreachable };

  using Behaviors = std::tuple<CompressBehavior, UncompressBehavior>;

  explicit TestCustomCompressor(
      std::string const& prefix, Behaviors const& behaviors)
      : prefix_(prefix),
        compressBehavior_(std::get<0>(behaviors)),
        uncompressBehavior_(std::get<1>(behaviors)) {}

  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer) override {
    switch (compressBehavior_) {
      case CompressBehavior::Success: {
        std::string bufferStr = std::move(buffer)->toString();
        compressHistory_.emplace_back(bufferStr);
        std::string newBuffer = prefix_ + bufferStr;
        return folly::IOBuf::fromString(std::move(newBuffer));
      }
      case CompressBehavior::Throw: {
        throw std::runtime_error("error during compress");
      }
      case CompressBehavior::Unreachable: {
        LOG(FATAL) << "compress should not be called";
      }
    }
  }

  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer) override {
    switch (uncompressBehavior_) {
      case UncompressBehavior::Success: {
        std::string bufferStr = std::move(buffer)->toString();
        EXPECT_EQ(bufferStr.substr(0, prefix_.size()), prefix_);
        uncompressHistory_.emplace_back(bufferStr);
        return folly::IOBuf::fromString(bufferStr.substr(prefix_.size()));
      }
      case UncompressBehavior::Throw: {
        throw std::runtime_error("error during uncompress");
      }
      case UncompressBehavior::Unreachable: {
        LOG(FATAL) << "uncompress should not be called";
      }
    }
  }

  std::vector<std::string> const& getCompressHistory() {
    LOG(INFO) << " getting compress history of " << this;
    return compressHistory_;
  }

  std::vector<std::string> const& getUncompressHistory() {
    LOG(INFO) << " getting uncompress history of " << this;
    return uncompressHistory_;
  }

 private:
  std::vector<std::string> compressHistory_;
  std::vector<std::string> uncompressHistory_;
  std::string prefix_;
  CompressBehavior compressBehavior_;
  UncompressBehavior uncompressBehavior_;
};

class TestCustomThriftCompressorFactory
    : public rocket::CustomCompressorFactory {
 public:
  enum class GetCustomCompressorNegotiationResponseBehavior {
    Success,
    Fail,
    Throw,
    Unreachable,
  };

  enum class ClientMakeBehavior {
    Success,
    Fail,
    Throw,
    Unreachable,
  };

  enum class ServerMakeBehavior {
    Success,
    Fail,
    Throw,
    Unreachable,
  };

  struct AutoRegistry {
    explicit AutoRegistry(
        std::shared_ptr<TestCustomThriftCompressorFactory> factory)
        : factory_(factory) {
      LOG(INFO) << "Registering compressor factory with name "
                << factory->getCompressorName();
      apache::thrift::rocket::CustomCompressorRegistry::registerFactory(
          factory);
    }

    AutoRegistry(AutoRegistry const&) = delete;
    AutoRegistry(AutoRegistry&&) = delete;
    AutoRegistry& operator=(AutoRegistry const&) = delete;
    AutoRegistry& operator=(AutoRegistry&&) = delete;

    ~AutoRegistry() {
      LOG(INFO) << "Unregistering compressor factory with name "
                << factory_->getCompressorName();
      apache::thrift::rocket::CustomCompressorRegistry::unregister(
          factory_->getCompressorName());
    }

    std::shared_ptr<TestCustomThriftCompressorFactory> getFactory() const {
      return factory_;
    }

   private:
    std::shared_ptr<TestCustomThriftCompressorFactory> factory_;
  };

  explicit TestCustomThriftCompressorFactory(
      std::string const& compressorName,
      GetCustomCompressorNegotiationResponseBehavior
          getCustomCompressorNegotiationResponseBehavior,
      ServerMakeBehavior serverMakeBehavior,
      ClientMakeBehavior clientMakeBehavior,
      TestCustomCompressor::Behaviors serverCompressBehaviors,
      TestCustomCompressor::Behaviors clientCompressBehaviors)
      : compressorName_(compressorName),
        getCustomCompressorNegotiationResponseBehavior_(
            getCustomCompressorNegotiationResponseBehavior),
        serverMakeBehavior_(serverMakeBehavior),
        clientMakeBehavior_(clientMakeBehavior),
        serverCompressBehaviors_(serverCompressBehaviors),
        clientCompressBehaviors_(clientCompressBehaviors) {}

  std::string getCompressorName() const override { return compressorName_; }

  std::optional<apache::thrift::CustomCompressionSetupResponse>
  createCustomCompressorNegotiationResponse(
      apache::thrift::CustomCompressionSetupRequest const& request)
      const override {
    switch (getCustomCompressorNegotiationResponseBehavior_) {
      case GetCustomCompressorNegotiationResponseBehavior::Success: {
        CustomCompressionSetupResponse res;
        res.compressorName() = *request.compressorName();
        res.payload() = *request.payload() + "_final_";
        return std::make_optional<CustomCompressionSetupResponse>(
            std::move(res));
      }
      case GetCustomCompressorNegotiationResponseBehavior::Fail: {
        return std::nullopt;
      }
      case GetCustomCompressorNegotiationResponseBehavior::Throw: {
        throw std::runtime_error(
            "exception during getCustomCompressorNegotiationResponse");
      }
      case GetCustomCompressorNegotiationResponseBehavior::Unreachable: {
        LOG(FATAL) << "Should not be reachable";
      }
    }
  }

  std::shared_ptr<apache::thrift::rocket::CustomCompressor> make(
      apache::thrift::CustomCompressionSetupRequest const& /*request*/,
      apache::thrift::CustomCompressionSetupResponse const& response,
      CustomCompressorFactory::CompressorLocation location) const override {
    switch (location) {
      case CustomCompressorFactory::CompressorLocation::CLIENT: {
        switch (clientMakeBehavior_) {
          case ClientMakeBehavior::Success: {
            clientCompressor_ = std::make_shared<TestCustomCompressor>(
                response.payload().value_or(""), clientCompressBehaviors_);
            return clientCompressor_;
          }
          case ClientMakeBehavior::Fail: {
            return nullptr;
          }
          case ClientMakeBehavior::Throw: {
            throw std::runtime_error("exception during client make");
          }
          case ClientMakeBehavior::Unreachable: {
            LOG(FATAL) << "Should not be reachable on client";
          }
        }
      }
      case CustomCompressorFactory::CompressorLocation::SERVER: {
        switch (serverMakeBehavior_) {
          case ServerMakeBehavior::Success: {
            serverCompressor_ = std::make_shared<TestCustomCompressor>(
                response.payload().value_or(""), serverCompressBehaviors_);
            return serverCompressor_;
          }
          case ServerMakeBehavior::Fail: {
            return nullptr;
          }
          case ServerMakeBehavior::Throw: {
            throw std::runtime_error("exception during server make");
          }
          case ServerMakeBehavior::Unreachable: {
            LOG(FATAL) << "Should not be reachable on server";
          }
        }
      }
    };
  }

  std::shared_ptr<TestCustomCompressor> getServerCompressor() const {
    return serverCompressor_;
  }

  std::shared_ptr<TestCustomCompressor> getClientCompressor() const {
    return clientCompressor_;
  }

 private:
  std::string compressorName_;
  GetCustomCompressorNegotiationResponseBehavior
      getCustomCompressorNegotiationResponseBehavior_;
  ServerMakeBehavior serverMakeBehavior_;
  ClientMakeBehavior clientMakeBehavior_;
  TestCustomCompressor::Behaviors serverCompressBehaviors_;
  TestCustomCompressor::Behaviors clientCompressBehaviors_;
  std::shared_ptr<TestCustomCompressor> mutable serverCompressor_;
  std::shared_ptr<TestCustomCompressor> mutable clientCompressor_;
};

class RocketCustomCompressionTest : public HeaderOrRocketTest {
 public:
  std::unique_ptr<TestCustomThriftCompressorFactory::AutoRegistry>
      compressorAutoRegistry = nullptr;

  void setupCommon() {
    compression = Compression::Custom; // need this to set up client channel
  }

  void TearDown() override { compressorAutoRegistry = nullptr; }

  std::unique_ptr<AsyncProcessorFactory> makeFactory() {
    return std::make_unique<TestHandler>();
  }
};

class RocketCustomCompressionSuccess : public RocketCustomCompressionTest {
  void SetUp() override {
    setupCommon();

    compressorAutoRegistry =
        std::make_unique<TestCustomThriftCompressorFactory::AutoRegistry>(
            std::make_shared<TestCustomThriftCompressorFactory>(
                getCustomCompressorNameForCurrentTestCase(),
                TestCustomThriftCompressorFactory::
                    GetCustomCompressorNegotiationResponseBehavior::Success,
                TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
                TestCustomThriftCompressorFactory::ClientMakeBehavior::Success,
                std::make_tuple(
                    TestCustomCompressor::CompressBehavior::Success,
                    TestCustomCompressor::UncompressBehavior::Success),
                std::make_tuple(
                    TestCustomCompressor::CompressBehavior::Success,
                    TestCustomCompressor::UncompressBehavior::Success)
                //
                ));
  }
};

TEST_F(RocketCustomCompressionSuccess, _) {
  folly::EventBase base;

  ScopedServerInterfaceThread runner(makeFactory());
  auto client = makeStickyClient(
      runner,
      &base); // use a sticky client to ensure we reuse the same connection
              // across requests
  std::string response;

  // first call establish a new connection and retains compression preference.
  // However, first call is not using custom compression itself
  // (request setup is piggy-backed with the first request for each
  // connection).
  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");

  // second call uses custom compression on both server and client,
  // which results in two compress and two uncompress calls
  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");

  {
    const auto& serverCompressor =
        compressorAutoRegistry->getFactory()->getServerCompressor();

    const auto& compressHistory = serverCompressor->getCompressHistory();
    EXPECT_EQ(compressHistory.size(), 1);

    const auto& uncompressHistory = serverCompressor->getUncompressHistory();
    EXPECT_EQ(uncompressHistory.size(), 1);
    for (const auto& buf : uncompressHistory) {
      EXPECT_TRUE(buf.starts_with("magic_prefix_final_"));
    }
  }

  {
    const auto& clientCompressor =
        compressorAutoRegistry->getFactory()->getClientCompressor();

    const auto& compressHistory = clientCompressor->getCompressHistory();
    EXPECT_EQ(compressHistory.size(), 1);

    const auto& uncompressHistory = clientCompressor->getUncompressHistory();
    EXPECT_EQ(uncompressHistory.size(), 1);
    for (const auto& buf : uncompressHistory) {
      EXPECT_TRUE(buf.starts_with("magic_prefix_final_"));
    }
  }
}

class RocketCustomCompressionWithParams
    : public RocketCustomCompressionTest,
      public ::testing::WithParamInterface<std::tuple<
          TestCustomThriftCompressorFactory::
              GetCustomCompressorNegotiationResponseBehavior,
          TestCustomThriftCompressorFactory::ServerMakeBehavior,
          TestCustomThriftCompressorFactory::ClientMakeBehavior,
          TestCustomCompressor::Behaviors, /*server*/
          TestCustomCompressor::Behaviors, /*client*/
          std::string /*errorPattern*/
          >> {
 public:
  std::string errorPattern_;

  void SetUp() override {
    setupCommon();

    auto
        [negotiationResponseBehavior,
         serverMakeBehavior,
         clientMakeBehavior,
         serverCompressorBehavior,
         clientCompressorBehavior,
         errorPattern] = GetParam();
    errorPattern_ = errorPattern;

    compressorAutoRegistry =
        std::make_unique<TestCustomThriftCompressorFactory::AutoRegistry>(
            std::make_shared<TestCustomThriftCompressorFactory>(
                getCustomCompressorNameForCurrentTestCase(),
                negotiationResponseBehavior,
                serverMakeBehavior,
                clientMakeBehavior,
                serverCompressorBehavior,
                clientCompressorBehavior));
  }
};

class RocketCustomCompressionNegotiationCloseConnection
    : public RocketCustomCompressionWithParams {};

TEST_P(RocketCustomCompressionNegotiationCloseConnection, _) {
  folly::EventBase base;

  ScopedServerInterfaceThread runner(makeFactory());
  auto client = makeStickyClient(
      runner,
      &base); // use a sticky client to ensure we reuse the same connection
              // across requests
  std::string response;

  EXPECT_THROW_RE(
      client->sync_sendResponse(response, 64),
      apache::thrift::transport::TTransportException,
      errorPattern_);

  // channel will be broken after this
  EXPECT_THROW_RE(
      client->sync_sendResponse(response, 64),
      apache::thrift::transport::TTransportException,
      "Connection not open:");
}

INSTANTIATE_TEST_CASE_P(
    RocketCustomCompressionNegotiationCloseConnection,
    RocketCustomCompressionNegotiationCloseConnection,
    ::testing::Values(
        // When client fails to create custom compressor, server wouldn't know
        // it, and there is a potential mismatch between client and server,
        // and thus it is the safest option to close the connection.
        //
        // Technically speaking, the client can choose to use basic compression
        // in the remainer of the connection. However, that is not robust, and
        // is not going to help with server-initiated messages.
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Throw,
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"(Failed to make custom compressor on client due to: exception during client make)"),
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Fail,
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"(Failed to make custom compressor on client.)")));

class RocketCustomCompressionNegotiationGracefulFallback
    : public RocketCustomCompressionWithParams {};

TEST_P(RocketCustomCompressionNegotiationGracefulFallback, _) {
  folly::EventBase base;

  ScopedServerInterfaceThread runner(makeFactory());
  auto client = makeStickyClient(
      runner,
      &base); // use a sticky client to ensure we reuse the same connection
              // across requests

  std::string response;

  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");

  // subsequent requests on the same connection should still be successful,
  // using basic compression as fallback, even though the request is still
  // requesting 'CUSTOM' compression.
  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");
}

INSTANTIATE_TEST_CASE_P(
    RocketCustomCompressionNegotiationGracefulFallback,
    RocketCustomCompressionNegotiationGracefulFallback,
    ::testing::Values(
        // When negotiation fails or throws, it means the server does not
        // recognize the custom compressor. It will fall back to basic
        // compression in that case.
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Fail,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Unreachable,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Unreachable,
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"()"),
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Throw,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Unreachable,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Unreachable,
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"()"),
        // When server fails to create a custom compressor (or throws),
        // it will behave as if it does not recognize the custom
        // compressor at all, so that client will just stick to basic
        // compression afterwards.
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Fail,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Unreachable,
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"()"),
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Throw,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Unreachable,
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"()")
        //
        ));

class RocketCustomCompressionCompressorClientFailures
    : public RocketCustomCompressionWithParams {};

TEST_P(RocketCustomCompressionCompressorClientFailures, _) {
  folly::EventBase base;

  ScopedServerInterfaceThread runner(makeFactory());
  auto client = makeStickyClient(
      runner,
      &base); // use a sticky client to ensure we reuse the same connection
              // across requests

  std::string response;

  // does negotiation successfully
  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");

  // uses custom compression and fails in various ways
  EXPECT_THROW_RE(
      client->sync_sendResponse(response, 64),
      // payload packing / compression is a transport layer feature,
      // so TTransportException makes sense
      apache::thrift::transport::TTransportException,
      errorPattern_);
}

INSTANTIATE_TEST_CASE_P(
    RocketCustomCompressionCompressorClientFailures,
    RocketCustomCompressionCompressorClientFailures,
    ::testing::Values(
        // When client uncompress fails, it is able to get back that error
        // locally. Server wouldn't get anything.
        // But get the following?
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Success,
            /*server*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            /*client*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Throw,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"(Failed to pack request payload: error during compress)"),
        // When client uncompress fails, it is able to get back that error
        // locally.
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Success,
            /*server*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Success,
                TestCustomCompressor::UncompressBehavior::Success),
            /*client*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Success,
                TestCustomCompressor::UncompressBehavior::Throw),
            R"(Failed to unpack payload with custom compression: error during uncompress)")
        //
        ));

class RocketCustomCompressionCompressorServerFailures
    : public RocketCustomCompressionWithParams {};

TEST_P(RocketCustomCompressionCompressorServerFailures, _) {
  folly::EventBase base;

  ScopedServerInterfaceThread runner(makeFactory());
  auto client = makeStickyClient(
      runner,
      &base); // use a sticky client to ensure we reuse the same connection
              // across requests

  std::string response;

  // does negotiation successfully
  client->sync_sendResponse(response, 64);
  EXPECT_EQ(response, "test64");

  // uses custom compression and fails in various ways
  EXPECT_THROW_RE(
      client->sync_sendResponse(response, 64),
      // It should have been a TTransportException.
      // However, 'RocketThriftRequests' will convert all exception to
      // TApplicationException. So we live with it for now.
      apache::thrift::TApplicationException,
      errorPattern_);
}

INSTANTIATE_TEST_CASE_P(
    RocketCustomCompressionCompressorServerFailures,
    RocketCustomCompressionCompressorServerFailures,
    ::testing::Values(
        // When server fails, it will get an exception, and will
        // attempt to send that exception back to client.
        // Exceptions are not compressed with custom compression, so
        // there is no risk of infinite recursion
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Success,
            /*server*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Unreachable,
                TestCustomCompressor::UncompressBehavior::Throw),
            /*client*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Success,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"(std::runtime_error: error during uncompress)"),
        std::make_tuple(
            TestCustomThriftCompressorFactory::
                GetCustomCompressorNegotiationResponseBehavior::Success,
            TestCustomThriftCompressorFactory::ServerMakeBehavior::Success,
            TestCustomThriftCompressorFactory::ClientMakeBehavior::Success,
            /*server*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Throw,
                TestCustomCompressor::UncompressBehavior::Success),
            /*client*/
            std::make_tuple(
                TestCustomCompressor::CompressBehavior::Success,
                TestCustomCompressor::UncompressBehavior::Unreachable),
            R"(Failed to pack or send payload due to: error during compress)")
        //
        ));
