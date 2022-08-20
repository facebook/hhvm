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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/VersionServicesMock.h>

namespace apache {
namespace thrift {

// Testing transport layers for resilience to changes in the service interface
class VersioningTest : public TestSetup {
 private:
 public:
  void SetUp() override {
    // Setup two servers, for VersionOld and VersionNew
    handlerOld_ = std::make_shared<StrictMock<OldServiceMock>>();
    serverOld_ = createServer(handlerOld_, portOld_);

    handlerNew_ = std::make_shared<StrictMock<NewServiceMock>>();
    serverNew_ = createServer(handlerNew_, portNew_);
  }

  void TearDown() override {
    if (serverOld_) {
      serverOld_->cleanUp();
      serverOld_.reset();
      handlerOld_.reset();
    }
    if (serverNew_) {
      serverNew_->cleanUp();
      serverNew_.reset();
      handlerNew_.reset();
    }
  }

  void connectToOldServer(
      folly::Function<void(std::unique_ptr<OldVersionAsyncClient>)> callMe,
      folly::Function<void()> onDetachable = nullptr) {
    auto channel = connectToServer(portOld_, std::move(onDetachable));
    callMe(std::make_unique<OldVersionAsyncClient>(std::move(channel)));
  }

  void connectToOldServer(
      folly::Function<void(std::unique_ptr<NewVersionAsyncClient>)> callMe,
      folly::Function<void()> onDetachable = nullptr) {
    auto channel = connectToServer(portOld_, std::move(onDetachable));
    callMe(std::make_unique<NewVersionAsyncClient>(std::move(channel)));
  }

  void connectToNewServer(
      folly::Function<void(std::unique_ptr<OldVersionAsyncClient>)> callMe,
      folly::Function<void()> onDetachable = nullptr) {
    auto channel = connectToServer(portNew_, std::move(onDetachable));
    callMe(std::make_unique<OldVersionAsyncClient>(std::move(channel)));
  }

  void connectToNewServer(
      folly::Function<void(std::unique_ptr<NewVersionAsyncClient>)> callMe,
      folly::Function<void()> onDetachable = nullptr) {
    auto channel = connectToServer(portNew_, std::move(onDetachable));
    callMe(std::make_unique<NewVersionAsyncClient>(std::move(channel)));
  }

 protected:
  std::unique_ptr<ThriftServer> serverOld_;
  std::unique_ptr<ThriftServer> serverNew_;
  std::shared_ptr<testing::StrictMock<OldServiceMock>> handlerOld_;
  std::shared_ptr<testing::StrictMock<NewServiceMock>> handlerNew_;
  uint16_t portOld_;
  uint16_t portNew_;

  folly::ScopedEventBaseThread executor_;
};

TEST_F(VersioningTest, SameRequestResponse) {
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    EXPECT_EQ(6, client->sync_AddOne(5));
  });
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    EXPECT_EQ(-1, client->sync_AddOne(-2));
  });
  connectToOldServer([](std::unique_ptr<NewVersionAsyncClient> client) {
    EXPECT_EQ(6, client->sync_AddOne(5));
  });
  connectToNewServer([](std::unique_ptr<NewVersionAsyncClient> client) {
    EXPECT_EQ(6, client->sync_AddOne(5));
  });
}

TEST_F(VersioningTest, SameStream) {
  auto oldLambda = [this](std::unique_ptr<OldVersionAsyncClient> client) {
    auto stream = client->sync_Range(5, 5);
    auto subscription = std::move(stream).subscribeExTry(
        &executor_, [init = 5](auto&& value) mutable {
          if (value.hasValue()) {
            EXPECT_EQ(init++, *value);
          } else if (value.hasException()) {
            FAIL() << "no error is expected";
          }
        });
    std::move(subscription).join();
  };

  auto newLambda = [this](std::unique_ptr<NewVersionAsyncClient> client) {
    auto stream = client->sync_Range(5, 5);
    auto subscription = std::move(stream).subscribeExTry(
        &executor_, [init = 5](auto&& value) mutable {
          if (value.hasValue()) {
            EXPECT_EQ(init++, *value);
          } else if (value.hasException()) {
            FAIL() << "no error is expected";
          }
        });
    std::move(subscription).join();
  };

  connectToOldServer(oldLambda);
  connectToNewServer(oldLambda);
  connectToOldServer(newLambda);
  connectToNewServer(newLambda);
}

TEST_F(VersioningTest, SameResponseAndStream) {
  auto oldLambda = [this](std::unique_ptr<OldVersionAsyncClient> client) {
    auto streamAndResponse = client->sync_RangeAndAddOne(5, 5, -2);
    EXPECT_EQ(-1, streamAndResponse.response);
    auto subscription =
        std::move(streamAndResponse.stream)
            .subscribeExTry(&executor_, [init = 5](auto&& value) mutable {
              if (value.hasValue()) {
                EXPECT_EQ(init++, *value);
              } else if (value.hasException()) {
                FAIL() << "no error is expected";
              }
            });
    std::move(subscription).join();
  };

  auto newLambda = [this](std::unique_ptr<NewVersionAsyncClient> client) {
    auto streamAndResponse = client->sync_RangeAndAddOne(5, 5, -2);
    EXPECT_EQ(-1, streamAndResponse.response);
    auto subscription =
        std::move(streamAndResponse.stream)
            .subscribeExTry(&executor_, [init = 5](auto&& value) mutable {
              if (value.hasValue()) {
                EXPECT_EQ(init++, *value);
              } else if (value.hasException()) {
                FAIL() << "no error is expected";
              }
            });
    std::move(subscription).join();
  };

  connectToOldServer(oldLambda);
  connectToNewServer(oldLambda);
  connectToOldServer(newLambda);
  connectToNewServer(newLambda);
}

TEST_F(VersioningTest, DeletedMethod) {
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    client->sync_DeletedMethod();
  });
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      client->sync_DeletedMethod();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

TEST_F(VersioningTest, DeletedStreamMethod) {
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    client->sync_DeletedStreamMethod();
  });
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      client->sync_DeletedStreamMethod();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

TEST_F(VersioningTest, StreamToRequestResponse) {
  // Old client connects to the old server, everything is fine
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    folly::ScopedEventBaseThread clientThread;
    auto messages = client->sync_StreamToRequestResponse();
    auto subs = std::move(messages).subscribeExTry(
        clientThread.getEventBase(), [](auto&&) {});
    std::move(subs).join();
  });

  // Old client connects to the new server, method type mismatch!
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      client->sync_StreamToRequestResponse();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });

  // New client connects to the old server, method type mismatch!
  connectToOldServer([](std::unique_ptr<NewVersionAsyncClient> client) {
    try {
      client->sync_StreamToRequestResponse();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

TEST_F(VersioningTest, ResponseandStreamToRequestResponse) {
  // Old client connects to the old server, everything is fine
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    folly::ScopedEventBaseThread clientThread;
    auto result = client->sync_ResponseandStreamToRequestResponse();
    auto messages = std::move(result.stream);
    auto subs = std::move(messages).subscribeExTry(
        clientThread.getEventBase(), [](auto&&) {});
    std::move(subs).join();
  });

  // Old client connects to the new server, method type mismatch!
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      client->sync_ResponseandStreamToRequestResponse();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });

  // New client connects to the old server, method type mismatch!
  connectToOldServer([](std::unique_ptr<NewVersionAsyncClient> client) {
    try {
      client->sync_ResponseandStreamToRequestResponse();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

TEST_F(VersioningTest, RequestResponseToStream) {
  // Old client connects to the old server, everything is fine
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    Message result;
    client->sync_RequestResponseToStream(result);
  });

  // Old client connects to the new server, method type mismatch!
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      Message result;
      client->sync_RequestResponseToStream(result);
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });

  // New client connects to the old server, method type mismatch!
  connectToOldServer([](std::unique_ptr<NewVersionAsyncClient> client) {
    try {
      client->sync_RequestResponseToStream();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

TEST_F(VersioningTest, RequestResponseToResponseandStream) {
  // Old client connects to the old server, everything is fine
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    Message result;
    client->sync_RequestResponseToResponseandStream(result);
  });

  // Old client connects to the new server, method type mismatch!
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      Message result;
      client->sync_RequestResponseToResponseandStream(result);
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });

  // New client connects to the old server, method type mismatch!
  connectToOldServer([](std::unique_ptr<NewVersionAsyncClient> client) {
    try {
      client->sync_RequestResponseToResponseandStream();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

TEST_F(VersioningTest, DeletedResponseAndStreamMethod) {
  connectToOldServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    client->sync_DeletedResponseAndStreamMethod();
  });
  connectToNewServer([](std::unique_ptr<OldVersionAsyncClient> client) {
    try {
      client->sync_DeletedResponseAndStreamMethod();
      FAIL() << "Should have thrown";
    } catch (const TApplicationException& e) {
      CHECK_EQ(
          TApplicationException::TApplicationExceptionType::UNKNOWN_METHOD,
          e.getType());
    }
  });
}

} // namespace thrift
} // namespace apache
