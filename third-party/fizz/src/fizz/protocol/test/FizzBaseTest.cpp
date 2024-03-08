/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/protocol/FizzBase.h>
#include <fizz/util/Variant.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace test {

using namespace folly;
using namespace folly::test;
using namespace testing;

static WriteNewSessionTicket writeNewSessionTicket(const std::string& str) {
  WriteNewSessionTicket write;
  write.appToken = IOBuf::copyBuffer(str);
  return write;
}

static AppWrite appWrite(const std::string& str) {
  AppWrite write;
  write.data = IOBuf::copyBuffer(str);
  return write;
}

MATCHER_P(WriteNewSessionTicketMatches, expected, "") {
  return IOBufEqualTo()(IOBuf::copyBuffer(expected), arg.appToken);
}

MATCHER_P(WriteMatches, expected, "") {
  return IOBufEqualTo()(IOBuf::copyBuffer(expected), arg.data);
}

enum class StateEnum { NotError, Closed, Error };

struct State {
  StateEnum state() const {
    return state_;
  }

  StateEnum state_{StateEnum::NotError};
};

struct A1 {};
struct A2 {};

#define TEST_ACTIONS(F, ...) \
  F(A1, __VA_ARGS__)         \
  F(A2, __VA_ARGS__)

FIZZ_DECLARE_VARIANT_TYPE(Action, TEST_ACTIONS)

using Actions = std::vector<Action>;

class TestStateMachine {
 public:
  using StateType = State;
  using ProcessingActions = Future<Actions>;
  using CompletedActions = Actions;

  TestStateMachine() {
    instance = this;
  }

  MOCK_METHOD(
      Future<Actions>,
      processSocketData,
      (const State&, IOBufQueue&, Aead::AeadOptions));

  Future<Actions> processAppWrite(const State& state, AppWrite&& write) {
    return processAppWrite_(state, write);
  }

  Future<Actions> processEarlyAppWrite(
      const State& state,
      EarlyAppWrite&& write) {
    return processEarlyAppWrite_(state, write);
  }

  MOCK_METHOD(
      Future<Actions>,
      processWriteNewSessionTicket_,
      (const State&, WriteNewSessionTicket&));
  Future<Actions> processWriteNewSessionTicket(
      const State& state,
      WriteNewSessionTicket ticket) {
    return processWriteNewSessionTicket_(state, ticket);
  }
  MOCK_METHOD(
      Future<Actions>,
      processKeyUpdateInitiation_,
      (const State&, KeyUpdateInitiation&));
  Future<Actions> processKeyUpdateInitiation(
      const State& state,
      KeyUpdateInitiation kui) {
    return processKeyUpdateInitiation_(state, kui);
  }
  MOCK_METHOD(Future<Actions>, processAppWrite_, (const State&, AppWrite&));
  MOCK_METHOD(
      Future<Actions>,
      processEarlyAppWrite_,
      (const State&, EarlyAppWrite&));
  MOCK_METHOD(Future<Actions>, processAppClose, (const State&));
  MOCK_METHOD(Future<Actions>, processAppCloseImmediate, (const State&));

  static TestStateMachine* instance;
};
TestStateMachine* TestStateMachine::instance;

class ActionMoveVisitor {
 public:
  MOCK_METHOD(void, a1, ());
  MOCK_METHOD(void, a2, ());

  void operator()(A1&) {
    a1();
  }
  void operator()(A2&) {
    a2();
  }
};

class TestFizzBase
    : public FizzBase<TestFizzBase, ActionMoveVisitor, TestStateMachine>,
      public DelayedDestruction {
 public:
  TestFizzBase()
      : FizzBase<TestFizzBase, ActionMoveVisitor, TestStateMachine>(
            state_,
            queue_,
            readAeadOptions_,
            visitor_,
            this) {}

  State state_;
  IOBufQueue queue_;
  ActionMoveVisitor visitor_;
  Aead::AeadOptions readAeadOptions_;

  void startActions(Future<Actions> actions) {
    std::move(actions).then(
        &TestFizzBase::processActions,
        static_cast<
            FizzBase<TestFizzBase, ActionMoveVisitor, TestStateMachine>*>(
            this));
  }

  void visitActions(TestStateMachine::CompletedActions& actions) override {
    for (auto& action : actions) {
      switch (action.type()) {
        case Action::Type::A1_E:
          visitor_(*action.asA1());
          break;
        case Action::Type::A2_E:
          visitor_(*action.asA2());
          break;
      }
    }
  }
};

struct Callback {
  MOCK_METHOD(
      void,
      cb,
      (int bytesWritten,
       const folly::AsyncSocketException& ex,
       void* writeToken));
};

class FizzBaseTest : public Test {
 public:
  void SetUp() override {
    testFizz_.reset(new TestFizzBase());
  }

  static Callback errCallback;

 protected:
  std::unique_ptr<TestFizzBase, DelayedDestruction::Destructor> testFizz_;
  Sequence s_;
  MockWriteCallback writeCallback_;
  MockWriteCallback earlyWriteCallback_;
}; // namespace test

Callback FizzBaseTest::errCallback;

constexpr auto writeErrorCallback = [](const folly::AsyncSocketException& ex,
                                       void* writeToken) {
  FizzBaseTest::errCallback.cb(0, ex, writeToken);
};

TEST_F(FizzBaseTest, TestReadSingle) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1()).WillOnce(Invoke([this]() {
    testFizz_->waitForData();
  }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestReadMulti) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        actions.push_back(A2());
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1()).InSequence(s_);
  EXPECT_CALL(testFizz_->visitor_, a2()).InSequence(s_);
  EXPECT_CALL(testFizz_->visitor_, a1()).InSequence(s_);
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2()).InSequence(s_);
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_->waitForData(); }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestReadNoActions) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_->waitForData(); }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestWriteNewSessionTicket) {
  EXPECT_CALL(*TestStateMachine::instance, processWriteNewSessionTicket_(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1());
  testFizz_->writeNewSessionTicket(WriteNewSessionTicket());
}

TEST_F(FizzBaseTest, TestWrite) {
  EXPECT_CALL(*TestStateMachine::instance, processAppWrite_(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1());
  testFizz_->appWrite(AppWrite());
}

TEST_F(FizzBaseTest, TestEarlyWrite) {
  EXPECT_CALL(*TestStateMachine::instance, processEarlyAppWrite_(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1());
  testFizz_->earlyAppWrite(EarlyAppWrite());
}

TEST_F(FizzBaseTest, TestWriteMulti) {
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1());
  testFizz_->appWrite(appWrite("write1"));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2());
  testFizz_->appWrite(appWrite("write2"));
}

TEST_F(FizzBaseTest, TestInitiateKeyUpdate) {
  EXPECT_CALL(*TestStateMachine::instance, processAppWrite_(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  bool inCallback = false;
  EXPECT_CALL(testFizz_->visitor_, a1())
      .WillOnce(InvokeWithoutArgs([this, &inCallback] {
        inCallback = true;
        SCOPE_EXIT {
          inCallback = false;
        };
        testFizz_->initiateKeyUpdate(KeyUpdateInitiation());
      }));
  EXPECT_CALL(*TestStateMachine::instance, processKeyUpdateInitiation_(_, _))
      .WillOnce(InvokeWithoutArgs([&inCallback]() {
        EXPECT_FALSE(inCallback);
        return Actions();
      }));
  testFizz_->appWrite(AppWrite());
}

TEST_F(FizzBaseTest, TestAppClose) {
  EXPECT_CALL(*TestStateMachine::instance, processAppClose(_))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1());
  testFizz_->appClose();
}

TEST_F(FizzBaseTest, TestWriteNewSessionTicketInCallback) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() {
        testFizz_->waitForData();
        testFizz_->writeNewSessionTicket(writeNewSessionTicket("appToken"));
      }));
  EXPECT_CALL(
      *TestStateMachine::instance,
      processWriteNewSessionTicket_(
          _, WriteNewSessionTicketMatches("appToken")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_->appWrite(appWrite("write")); }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestWriteInCallback) {
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() {
        testFizz_->appWrite(appWrite("write2"));
        testFizz_->appWrite(appWrite("write3"));
      }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_->appWrite(appWrite("write4")); }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write3")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write4")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  testFizz_->appWrite(appWrite("write1"));
}

TEST_F(FizzBaseTest, TestAppCloseInCallback) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_->appClose(); }));
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_->waitForData(); }));
  EXPECT_CALL(*TestStateMachine::instance, processAppClose(_))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestWriteThenCloseInCallback) {
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() {
        testFizz_->appWrite(appWrite("write2"));
        testFizz_->appClose();
      }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  EXPECT_CALL(*TestStateMachine::instance, processAppClose(_))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  testFizz_->appWrite(appWrite("write1"));
}

TEST_F(FizzBaseTest, TestDeleteInCallback) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() { testFizz_.reset(); }));
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2())
      .InSequence(s_)
      .WillOnce(Invoke([ptr = testFizz_.get()]() { ptr->waitForData(); }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestStopOnError) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(
          Invoke([this]() { testFizz_->state_.state_ = StateEnum::Error; }));
  EXPECT_FALSE(testFizz_->inErrorState());
  testFizz_->newTransportData();
  EXPECT_TRUE(testFizz_->inErrorState());
}

TEST_F(FizzBaseTest, TestPause) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a1())
      .InSequence(s_)
      .WillOnce(Invoke([this]() {
        testFizz_->appWrite(appWrite("write1"));
        testFizz_->pause();
        testFizz_->waitForData();
      }));
  testFizz_->newTransportData();

  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .InSequence(s_)
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  testFizz_->resume();
}

TEST_F(FizzBaseTest, TestAsyncAction) {
  Promise<Actions> p;
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .WillOnce(InvokeWithoutArgs([&p]() { return p.getFuture(); }));
  testFizz_->appWrite(appWrite("write1"));
  testFizz_->appWrite(appWrite("write2"));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  p.setValue(Actions{});
}

TEST_F(FizzBaseTest, TestAsyncActionDelete) {
  Promise<Actions> p;
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .WillOnce(InvokeWithoutArgs([&p]() { return p.getFuture(); }));
  testFizz_->appWrite(appWrite("write1"));
  testFizz_->appWrite(appWrite("write2"));
  testFizz_.reset();
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .WillOnce(InvokeWithoutArgs([]() { return Actions{}; }));
  p.setValue(Actions{});
}

TEST_F(FizzBaseTest, TestActionProcessing) {
  EXPECT_CALL(*TestStateMachine::instance, processAppClose(_))
      .WillOnce(InvokeWithoutArgs([this]() {
        EXPECT_TRUE(testFizz_->actionProcessing());
        return Actions{};
      }));
  EXPECT_FALSE(testFizz_->actionProcessing());
  testFizz_->appClose();
  EXPECT_FALSE(testFizz_->actionProcessing());
}

TEST_F(FizzBaseTest, TestActionProcessingAsync) {
  Promise<Actions> p;
  EXPECT_CALL(*TestStateMachine::instance, processAppClose(_))
      .WillOnce(InvokeWithoutArgs([this, &p]() {
        EXPECT_TRUE(testFizz_->actionProcessing());
        return p.getFuture();
      }));
  EXPECT_FALSE(testFizz_->actionProcessing());
  testFizz_->appClose();
  EXPECT_TRUE(testFizz_->actionProcessing());
  p.setValue(Actions{});
  EXPECT_FALSE(testFizz_->actionProcessing());
}

TEST_F(FizzBaseTest, TestErrorPendingEvents) {
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  auto w2 = appWrite("write2");
  EXPECT_CALL(testFizz_->visitor_, a1()).WillOnce(Invoke([&]() {
    testFizz_->appWrite(std::move(w2));
    EarlyAppWrite earlyWrite;
    earlyWrite.token = &earlyWriteCallback_;
    testFizz_->earlyAppWrite(std::move(earlyWrite));
    auto w3 = appWrite("write3");
    w3.token = &writeCallback_;
    testFizz_->appWrite(std::move(w3));
    testFizz_->appWrite(appWrite("write4"));
    testFizz_->appClose();
  }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .WillOnce(InvokeWithoutArgs([this]() {
        testFizz_->moveToErrorState([](void* token) {
          writeErrorCallback(
              AsyncSocketException{AsyncSocketException::UNKNOWN, "unit test"},
              token);
        });
        return Actions{};
      }));
  EXPECT_CALL(errCallback, cb(0, _, &earlyWriteCallback_));
  EXPECT_CALL(errCallback, cb(0, _, &writeCallback_));
  EXPECT_FALSE(testFizz_->inErrorState());
  EXPECT_FALSE(testFizz_->inTerminalState());
  testFizz_->appWrite(appWrite("write1"));
  EXPECT_FALSE(testFizz_->inErrorState());
  EXPECT_TRUE(testFizz_->inTerminalState());
}

TEST_F(FizzBaseTest, TestErrorWhileErroring) {
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write1")))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        return actions;
      }));
  StrictMock<MockWriteCallback> wcb3;
  StrictMock<MockWriteCallback> wcb4;
  EXPECT_CALL(testFizz_->visitor_, a1()).WillOnce(Invoke([&]() {
    testFizz_->appWrite(appWrite("write2"));
    auto w3 = appWrite("write3");
    w3.token = &wcb3;
    testFizz_->appWrite(std::move(w3));
    auto w4 = appWrite("write4");
    w4.token = &wcb4;
    testFizz_->appWrite(std::move(w4));
  }));
  EXPECT_CALL(
      *TestStateMachine::instance, processAppWrite_(_, WriteMatches("write2")))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_CALL(testFizz_->visitor_, a2()).WillOnce(Invoke([&]() {
    testFizz_->moveToErrorState([](void* token) {
      writeErrorCallback(
          AsyncSocketException{AsyncSocketException::UNKNOWN, "unit test"},
          token);
    });
  }));
  EXPECT_CALL(errCallback, cb(0, _, &wcb3)).WillOnce(InvokeWithoutArgs([&]() {
    testFizz_->moveToErrorState([](void* token) {
      writeErrorCallback(
          AsyncSocketException{AsyncSocketException::UNKNOWN, "unit test"},
          token);
    });

    EXPECT_CALL(errCallback, cb(0, _, &wcb4));
  }));
  EXPECT_FALSE(testFizz_->inErrorState());
  EXPECT_FALSE(testFizz_->inTerminalState());
  testFizz_->appWrite(appWrite("write1"));
  EXPECT_FALSE(testFizz_->inErrorState());
  EXPECT_TRUE(testFizz_->inTerminalState());
}

TEST_F(FizzBaseTest, EventAfterErrorState) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([this]() {
        testFizz_->moveToErrorState([](void* token) {
          writeErrorCallback(
              AsyncSocketException{AsyncSocketException::UNKNOWN, "unit test"},
              token);
        });
        return Actions{};
      }));
  EXPECT_FALSE(testFizz_->inErrorState());
  EXPECT_FALSE(testFizz_->inTerminalState());
  testFizz_->newTransportData();
  EXPECT_FALSE(testFizz_->inErrorState());
  EXPECT_TRUE(testFizz_->inTerminalState());
}

TEST_F(FizzBaseTest, TestManyActions) {
  size_t i = 0;
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .WillRepeatedly(InvokeWithoutArgs([this, &i]() {
        if (++i == 10000) {
          testFizz_->waitForData();
        }
        return Actions{};
      }));
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestMoveToErrorStateOnVisit) {
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        Actions actions;
        actions.push_back(A1());
        actions.push_back(A2());
        return actions;
      }));

  EXPECT_CALL(testFizz_->visitor_, a1()).WillOnce(Invoke([this]() {
    testFizz_->moveToErrorState([](void* token) {
      writeErrorCallback(
          folly::AsyncSocketException(
              folly::AsyncSocketException::NOT_OPEN, "Transport is not good"),
          token);
    });
  }));

  EXPECT_CALL(testFizz_->visitor_, a2());
  testFizz_->newTransportData();
}

TEST_F(FizzBaseTest, TestActionProcessedAfterError) {
  EXPECT_CALL(testFizz_->visitor_, a1());
  EXPECT_CALL(testFizz_->visitor_, a2());
  EXPECT_CALL(*TestStateMachine::instance, processSocketData(_, _, _))
      .WillOnce(InvokeWithoutArgs([&]() {
        testFizz_->state_.state_ = StateEnum::Error;
        Actions actions;
        actions.push_back(A1());
        actions.push_back(A2());
        return actions;
      }));
  EXPECT_FALSE(testFizz_->inErrorState());
  testFizz_->newTransportData();
  EXPECT_TRUE(testFizz_->inErrorState());
}
} // namespace test
} // namespace fizz
