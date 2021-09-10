/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <memory>
#include <thread>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/logging/LogWriter.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include "hphp/runtime/ext/facts/logging.h"

using namespace ::testing;

using ::testing::_;
using ::testing::An;
using ::testing::Invoke;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Return;

namespace folly {

void PrintTo(StringPiece sp, std::ostream* os) {
  *os << "\"" << sp << "\"";
}

} // namespace folly

namespace HPHP {
namespace Facts {
namespace Test {

class MockLogWriter : public folly::LogWriter {
public:
  MockLogWriter() {
    // ttyOutput() is called by the constructor of the AsyncLogWriter and
    // generally isn't interesting.
    ON_CALL(*this, ttyOutput()).WillByDefault(Return(false));
  }

  MOCK_METHOD(void, writeMessage, (folly::StringPiece, uint32_t), (override));
  MOCK_METHOD(void, writeMessage, (std::string&&, uint32_t), (override));
  MOCK_METHOD(void, flush, (), (override));
  MOCK_METHOD(bool, ttyOutput, (), (const override));
};

TEST(Logging, asyncTtyPassThrough) {
  std::unique_ptr<MockLogWriter> t_writer = std::make_unique<MockLogWriter>();
  MockLogWriter* t_mock = t_writer.get();

  EXPECT_CALL(*t_mock, ttyOutput()).WillOnce(Return(true));

  AsyncLogWriter async_tty_true(std::move(t_writer));
  EXPECT_THAT(async_tty_true.ttyOutput(), IsTrue());

  std::unique_ptr<MockLogWriter> f_writer = std::make_unique<MockLogWriter>();
  MockLogWriter* f_mock = f_writer.get();

  EXPECT_CALL(*f_mock, ttyOutput()).WillOnce(Return(false));

  AsyncLogWriter async_tty_false(std::move(f_writer));
  EXPECT_THAT(async_tty_false.ttyOutput(), IsFalse());

  Mock::VerifyAndClearExpectations(t_mock);
  Mock::VerifyAndClearExpectations(f_mock);
}

TEST(LoggingTest, asyncFlush) {
  std::unique_ptr<MockLogWriter> writer = std::make_unique<MockLogWriter>();
  MockLogWriter* mock = writer.get();

  // If this happens asynchronously, the flush should get called from a
  // different thread.
  std::thread::id test_thread = std::this_thread::get_id();
  int flushes = 0;

  auto check_and_increment = [&]() {
    EXPECT_NE(test_thread, std::this_thread::get_id());
    flushes++;
  };

  InSequence seq;
  EXPECT_CALL(*mock, writeMessage(An<folly::StringPiece>(), _)).Times(0);
  EXPECT_CALL(*mock, writeMessage(An<std::string&&>(), _)).Times(1);
  EXPECT_CALL(*mock, flush()).WillOnce(check_and_increment);
  EXPECT_CALL(*mock, writeMessage(An<std::string&&>(), _)).Times(1);
  EXPECT_CALL(*mock, flush()).WillOnce(check_and_increment);

  AsyncLogWriter async_writer(std::move(writer));
  async_writer.writeMessage(folly::StringPiece{"Some Message"}, 1);
  async_writer.flush();
  async_writer.writeMessage(folly::StringPiece{"Some Other Message"}, 2);
  async_writer.flush();

  EXPECT_EQ(2, flushes);
  Mock::VerifyAndClearExpectations(mock);
}

TEST(LoggingTest, asyncWriteStringPiece) {
  std::unique_ptr<MockLogWriter> writer = std::make_unique<MockLogWriter>();
  MockLogWriter* mock = writer.get();

  std::thread::id test_thread = std::this_thread::get_id();

  int last_message = -1;

  auto process_write = [&](std::string&& message, uint32_t flags) {
    EXPECT_NE(test_thread, std::this_thread::get_id());
    EXPECT_EQ(folly::to<uint32_t>(message), flags);
    EXPECT_EQ(++last_message, flags);
  };

  InSequence seq;
  EXPECT_CALL(*mock, writeMessage(An<folly::StringPiece>(), _)).Times(0);
  EXPECT_CALL(*mock, writeMessage(An<std::string&&>(), _))
      .Times(5)
      .WillRepeatedly(Invoke(process_write));
  EXPECT_CALL(*mock, flush()).Times(1);
  EXPECT_CALL(*mock, writeMessage(An<std::string&&>(), _))
      .Times(5)
      .WillRepeatedly(Invoke(process_write));
  EXPECT_CALL(*mock, flush()).Times(1);

  AsyncLogWriter async_writer(std::move(writer));
  for (int i = 0; i < 5; i++) {
    std::string message = folly::to<std::string>(i);
    folly::StringPiece messagePiece{message};
    async_writer.writeMessage(messagePiece, i);
  }
  async_writer.flush();
  for (int i = 5; i < 10; i++) {
    std::string message = folly::to<std::string>(i);
    folly::StringPiece messagePiece{message};
    async_writer.writeMessage(messagePiece, i);
  }
  async_writer.flush();

  Mock::VerifyAndClearExpectations(mock);
}

TEST(LoggingTest, asyncWriteString) {
  std::unique_ptr<MockLogWriter> writer = std::make_unique<MockLogWriter>();
  MockLogWriter* mock = writer.get();

  std::thread::id test_thread = std::this_thread::get_id();

  int last_message = -1;

  auto process_write = [&](std::string&& message, uint32_t flags) {
    EXPECT_NE(test_thread, std::this_thread::get_id());
    EXPECT_EQ(folly::to<uint32_t>(message), flags);
    EXPECT_EQ(++last_message, flags);
  };

  InSequence seq;
  EXPECT_CALL(*mock, writeMessage(An<folly::StringPiece>(), _)).Times(0);
  EXPECT_CALL(*mock, writeMessage(An<std::string&&>(), _))
      .Times(5)
      .WillRepeatedly(Invoke(process_write));
  EXPECT_CALL(*mock, flush()).Times(1);
  EXPECT_CALL(*mock, writeMessage(An<std::string&&>(), _))
      .Times(5)
      .WillRepeatedly(Invoke(process_write));
  EXPECT_CALL(*mock, flush()).Times(1);

  AsyncLogWriter async_writer(std::move(writer));
  for (int i = 0; i < 5; i++) {
    std::string message = folly::to<std::string>(i);
    async_writer.writeMessage(message, i);
  }
  async_writer.flush();
  for (int i = 5; i < 10; i++) {
    std::string message = folly::to<std::string>(i);
    async_writer.writeMessage(message, i);
  }
  async_writer.flush();

  Mock::VerifyAndClearExpectations(mock);
}

TEST(LoggingTest, flushedOnDestruction) {
  std::atomic<bool> called_flush_during_destructor = false;

  std::unique_ptr<MockLogWriter> writer = std::make_unique<MockLogWriter>();
  MockLogWriter* mock = writer.get();

  EXPECT_CALL(*mock, flush()).WillOnce([&]() {
    called_flush_during_destructor = true;
  });

  auto logger = std::make_unique<AsyncLogWriter>(std::move(writer));

  ASSERT_FALSE(called_flush_during_destructor);
  logger.reset(nullptr);
  EXPECT_TRUE(called_flush_during_destructor);
}

} // namespace Test
} // namespace Facts
} // namespace HPHP
