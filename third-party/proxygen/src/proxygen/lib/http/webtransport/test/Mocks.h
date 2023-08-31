/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>

namespace proxygen::test {

using GenericApiRet = folly::Expected<folly::Unit, WebTransport::ErrorCode>;

class MockStreamReadHandle : public WebTransport::StreamReadHandle {
 public:
  explicit MockStreamReadHandle(uint64_t inId) : id(inId) {
  }

  MOCK_METHOD(uint64_t, getID, ());
  MOCK_METHOD(folly::CancellationToken, getCancelToken, ());
  using ReadStreamDataRet = folly::SemiFuture<WebTransport::StreamData>;
  MOCK_METHOD(ReadStreamDataRet, readStreamData, ());
  MOCK_METHOD(GenericApiRet, stopSending, (uint32_t));
  uint64_t id{0};
};

class MockStreamWriteHandle : public WebTransport::StreamWriteHandle {
 public:
  explicit MockStreamWriteHandle(uint64_t inId) : id(inId) {
    ON_CALL(*this, getID()).WillByDefault(testing::Return(id));
  }

  MOCK_METHOD(uint64_t, getID, ());
  MOCK_METHOD(folly::CancellationToken, getCancelToken, ());
  using WriteStreamDataRet =
      folly::Expected<folly::SemiFuture<folly::Unit>, WebTransport::ErrorCode>;
  MOCK_METHOD(WriteStreamDataRet,
              writeStreamData,
              (std::unique_ptr<folly::IOBuf>, bool));

  MOCK_METHOD(GenericApiRet, resetStream, (uint32_t));
  uint64_t id{0};
};

class MockWebTransport : public WebTransport {
 public:
  MockWebTransport() {
    EXPECT_CALL(*this, createUniStream()).WillRepeatedly([&] {
      auto id = nextUniStreamId_;
      nextUniStreamId_ += 4;
      auto handle = new testing::NiceMock<MockStreamWriteHandle>(id);
      writeHandles_.emplace(id, handle);
      return handle;
    });
    EXPECT_CALL(*this, createBidiStream()).WillRepeatedly([&] {
      auto id = nextBidiStreamId_;
      nextBidiStreamId_ += 4;
      BidiStreamHandle handle(
          {.readHandle = new MockStreamReadHandle(id),
           .writeHandle = new testing::NiceMock<MockStreamWriteHandle>(id)});
      readHandles_.emplace(id, handle.readHandle);
      writeHandles_.emplace(id, handle.writeHandle);
      return handle;
    });
  }
  using CreateUniStreamRet = folly::Expected<StreamWriteHandle*, ErrorCode>;
  MOCK_METHOD(CreateUniStreamRet, createUniStream, ());

  using CreateBidiStreamRet = folly::Expected<BidiStreamHandle, ErrorCode>;
  MOCK_METHOD(CreateBidiStreamRet, createBidiStream, ());

  using AwaitStreamCreditRet = folly::SemiFuture<folly::Unit>;
  MOCK_METHOD(AwaitStreamCreditRet, awaitUniStreamCredit, ());
  MOCK_METHOD(AwaitStreamCreditRet, awaitBidiStreamCredit, ());

  using ReadStreamDataRet =
      folly::Expected<folly::SemiFuture<StreamData>, WebTransport::ErrorCode>;
  MOCK_METHOD(ReadStreamDataRet, readStreamData, (uint64_t));
  MOCK_METHOD(MockStreamWriteHandle::WriteStreamDataRet,
              writeStreamData,
              (uint64_t, std::unique_ptr<folly::IOBuf>, bool));
  MOCK_METHOD(GenericApiRet, resetStream, (uint64_t, uint32_t));
  MOCK_METHOD(GenericApiRet, stopSending, (uint64_t, uint32_t));
  MOCK_METHOD(GenericApiRet, sendDatagram, (std::unique_ptr<folly::IOBuf>));
  MOCK_METHOD(GenericApiRet, closeSession, (folly::Optional<uint32_t>));

  void cleanupStream(uint64_t id) {
    auto handleIt = writeHandles_.find(id);
    if (handleIt != writeHandles_.end()) {
      delete handleIt->second;
      writeHandles_.erase(handleIt);
    }
  }

  void cleanupReadHandle(uint64_t id) {
    auto handleIt = readHandles_.find(id);
    if (handleIt != readHandles_.end()) {
      delete handleIt->second;
      readHandles_.erase(handleIt);
    }
  }

  uint64_t nextBidiStreamId_{0};
  uint64_t nextUniStreamId_{2};
  std::map<uint64_t, StreamHandleBase*> writeHandles_;
  std::map<uint64_t, StreamHandleBase*> readHandles_;
};

} // namespace proxygen::test
