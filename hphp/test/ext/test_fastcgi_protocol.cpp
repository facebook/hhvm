/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/test/ext/test_fastcgi_protocol.h"
#include "hphp/test/ext/test_fastcgi.h"
#include "hphp/runtime/server/fastcgi/fastcgi-session.h"
#include "hphp/runtime/server/fastcgi/protocol-session-handler.h"
#include "hphp/runtime/ext/ext_json.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/shared-string.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/types.h"
#include "hphp/util/logger.h"
#include "folly/io/IOBuf.h"
#include "folly/io/IOBufQueue.h"
#include "folly/io/Cursor.h"

#define LOAD_JSON(mx, s)     if (!Count(LoadExchangeFromJson( \
                                        std::string("test/ext/fastcgi/") + s, \
                                        mx, __FILE__,__LINE__))) { \
    return false; \
  }

#define EXCHANGE(messages, reps) \
  if (!Count(VerifyExchange(messages, reps, __FILE__,__LINE__))) { \
    return false; \
  }

///////////////////////////////////////////////////////////////////////////////

using folly::IOBuf;
using folly::IOBufQueue;
using folly::io::Cursor;
using folly::io::Appender;

DECLARE_BOOST_TYPES(TestProtocolHandler);

class TestProtocolHandler : public ProtocolSessionHandler {
public:
  virtual ~TestProtocolHandler() {}

  virtual void onBody(std::unique_ptr<IOBuf> chain) override;
  virtual void onBodyComplete() override;
  virtual void onHeader(std::unique_ptr<IOBuf> key_chain,
                        std::unique_ptr<IOBuf> value_chain) override;
  virtual void onHeadersComplete() override;

  void onComplete() {
    m_callback->onComplete();
  }
  void onStdOut(std::unique_ptr<IOBuf> chain) {
    m_callback->onStdOut(std::move(chain));
  }
  void onStdErr(std::unique_ptr<IOBuf> chain) {
    m_callback->onStdErr(std::move(chain));
  }

  IOBufQueue m_body{IOBufQueue::cacheChainLength()};
  bool m_bodyComplete{false};
  HeaderMap m_headers;
  bool m_headersComplete{false};
};

void TestProtocolHandler::onBody(std::unique_ptr<IOBuf> chain) {
  m_body.append(std::move(chain));
}

void TestProtocolHandler::onBodyComplete() {
  m_bodyComplete = true;
}

void TestProtocolHandler::onHeader(std::unique_ptr<IOBuf> key_chain,
                                   std::unique_ptr<IOBuf> value_chain) {
  Cursor key_cursor(key_chain.get());
  std::string key = key_cursor.readFixedString(
                      key_chain->computeChainDataLength());
  Cursor value_cursor(value_chain.get());
  std::string value = value_cursor.readFixedString(
                        value_chain->computeChainDataLength());
  m_headers.insert(std::make_pair(key, std::vector<std::string>(1, value)));
}

void TestProtocolHandler::onHeadersComplete() {
  m_headersComplete = true;
}

///////////////////////////////////////////////////////////////////////////////

class TestSessionCallback : public ProtocolSession::Callback {
public:
  TestSessionCallback();
  virtual ~TestSessionCallback() {}

  virtual ProtocolSessionHandlerPtr newSessionHandler(int request_id) override;
  virtual void onSessionEgress(std::unique_ptr<IOBuf> chain) override {
    m_egress.append(std::move(chain));
  }
  virtual void onSessionError() override {
    m_sessionError = true;
  }
  virtual void onSessionClose() override {
    m_sessionClosed = true;
  }

  IOBufQueue m_egress;
  bool m_sessionError{false};
  bool m_sessionClosed{false};
  std::map<int, TestProtocolHandlerPtr> m_handlers;
};

TestSessionCallback::TestSessionCallback()
  : m_egress(IOBufQueue::cacheChainLength()) {}

ProtocolSessionHandlerPtr TestSessionCallback::newSessionHandler(
                                                 int request_id) {
  auto handler = std::make_shared<TestProtocolHandler>();
  m_handlers[request_id] = handler;
  return handler;
}

///////////////////////////////////////////////////////////////////////////////

class TestFastCGISession : public FastCGISession {
public:
  virtual ~TestFastCGISession() {}

  bool testTransactionExists(RequestId request_id) {
    return m_transactions.count(request_id);
  }
};

///////////////////////////////////////////////////////////////////////////////

TestFastCGIProtocol::TestFastCGIProtocol() {}

bool TestFastCGIProtocol::LoadExchangeFromJson(const std::string& path,
                                             TestMessageExchange& result,
                                             const char* file,
                                             int line) {
  std::ifstream f(path.c_str());
  if (!f) {
    printf("Unable to open %s for reading. Run this test from hphp/.\n",
           path.c_str());
    return false;
  }
  std::string json_str((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
  Variant json = f_json_decode(json_str, true);
  if (!result.fromJson(json)) {
    printf("Error loading message exchange file: %s\n", path.c_str());
    return false;
  }
  return true;
}

bool TestFastCGIProtocol::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestSanity);
  RUN_TEST(TestBeginRequest);
  RUN_TEST(TestAbortRequest);
  RUN_TEST(TestEndRequest);
  RUN_TEST(TestDoubleBeginRequest);
  RUN_TEST(TestParams);
  RUN_TEST(TestStdIn);
  RUN_TEST(TestStdOut);
  RUN_TEST(TestStdErr);
  RUN_TEST(TestData);
  RUN_TEST(TestGetValues);
  RUN_TEST(TestIgnoreNonActive);
  RUN_TEST(TestUnknownType);
  RUN_TEST(TestInvalidType);
  return ret;
}

bool TestFastCGIProtocol::VerifyExchange(const TestMessageExchange& mx,
                                         int reps,
                                         const char* file,
                                         int line) {
  for (int r = 0; r < reps; ++r) {
    printf("-------------------------------------\n");
    TestFastCGISession session;
    TestSessionCallback session_callback;
    session.setCallback(&session_callback);
    session.setMaxConns(50);
    session.setMaxRequests(1000);
    IOBufQueue send_buf(IOBufQueue::cacheChainLength());

    TestMessageExchange mx2 = mx;
    if (r > 0) {
      mx2 = mx2.aggregate().split();
    }

    for (auto it = mx2.m_messages.begin(); it != mx2.m_messages.end(); ++it) {
      int len = it->m_body.size();
      CHECK(it->m_body.size() == it->m_mask.size());

      switch (it->m_command) {
        case TestMessage::Command::SEND: {
          if (it->m_command == TestMessage::Command::SEND) {
            for (int i = 0; i < len; ++i) {
              uint8_t c = it->m_body[i] & it->m_mask[i];
              send_buf.append(&c, sizeof(uint8_t));
            }
            CHECK(send_buf.chainLength() > 0);
            int read = session.onIngress(send_buf.front());
            send_buf.split(read);
            printf("Successfully sent message, length=%d\n", len);
          }
          break;
        }
        case TestMessage::Command::RECV: {
          int actual_len = session_callback.m_egress.chainLength();
          if (actual_len < len) {
            printf("Too little data available in the buffer "
                   "got length=%d, expected length=%d\n",
                   actual_len, len);
            return false;
          }
          Cursor actual_cursor(session_callback.m_egress.front());
          bool result = true;
          for (int i = 0; i < len; ++i) {
            uint8_t c = actual_cursor.readBE<uint8_t>();
            if ((c & it->m_mask[i]) != (it->m_body[i] & it->m_mask[i])) {
              printf("Message mismatch at offset=%d "
                     "got value=%02x, expected value=%02x, "
                     "mask was=%02x\n",
                     i,
                     c & it->m_mask[i],
                     it->m_body[i] & it->m_mask[i],
                     it->m_mask[i]);
              result = false;
            }
          }
          session_callback.m_egress.split(len);
          printf("Successfully read message, length=%d\n", len);
          if (!result) {
            return false;
          }
          break;
        }
        case TestMessage::Command::CLOSE: {
          // No-op for now. We don't notify the session of connection
          // closure.
          break;
        }
        case TestMessage::Command::RECV_CLOSE: {
          if (!session_callback.m_sessionClosed) {
            printf("Session didn't close as expected\n");
            return false;
          }
          printf("Session was successfully closed\n");
          break;
        }
        case TestMessage::Command::CALL: {
          CHECK(it->m_args.count("name"));
          CHECK(it->m_args.at("name").isString());
          std::string name = it->m_args.at("name").toString().data();
          if (name == "transaction_exists") {
            CHECK(it->m_args.count("request_id"));
            CHECK(it->m_args.at("request_id").isInteger());
            CHECK(it->m_args.count("result"));
            CHECK(it->m_args.at("result").isBoolean());
            int request_id = it->m_args.at("request_id").toInt32();
            bool result = it->m_args.at("result").toBoolean();
            if (session.testTransactionExists(request_id) != result) {
              printf("Transaction %d existance test failure, "
                     "expected=%d\n", request_id, (int) result);
              return false;
            }
          } else if (name == "end_transaction") {
            CHECK(it->m_args.count("request_id"));
            CHECK(it->m_args.at("request_id").isInteger());
            int request_id = it->m_args.at("request_id").toInt32();
            auto handler = session_callback.m_handlers[request_id];
            handler->onComplete();
          } else if (name == "session_error") {
            CHECK(it->m_args.count("result"));
            CHECK(it->m_args.at("result").isBoolean());
            bool result = it->m_args.at("result").toBoolean();
            if (session_callback.m_sessionError != result) {
              printf("Session error test failure, "
                     "expected=%d\n", (int) result);
              return false;
            }
          } else if (name == "session_closed") {
            CHECK(it->m_args.count("result"));
            CHECK(it->m_args.at("result").isBoolean());
            bool result = it->m_args.at("result").toBoolean();
            if (session_callback.m_sessionClosed != result) {
              printf("Session closure test failure, "
                     "expected=%d\n", (int) result);
              return false;
            }
          } else if (name == "body") {
            CHECK(it->m_args.count("request_id"));
            CHECK(it->m_args.at("request_id").isInteger());
            int request_id = it->m_args.at("request_id").toInt32();
            auto handler = session_callback.m_handlers[request_id];
            if (it->m_args.count("length")) {
              CHECK(it->m_args.at("length").isInteger());
              int body_len = it->m_args.at("length").toInt32();
              if (handler->m_body.chainLength() != body_len) {
                printf("Body length didn't match, "
                       "expeted=%d, got=%d\n",
                       (int) handler->m_body.chainLength(),
                       body_len);
                return false;
              }
            }
            if (it->m_args.count("message")) {
              CHECK(it->m_args.at("message").isString());
              String message = it->m_args.at("message").toString();
              if (handler->m_body.chainLength() != message.size()) {
                printf("Body length didn't match, "
                       "expected=%d, got=%d\n",
                       (int) handler->m_body.chainLength(),
                       message.size());
                return false;
              }
              bool match = true;
              Cursor cursor(handler->m_body.front());
              for (int i = 0; i < message.size(); ++i) {
                uint8_t c = cursor.readBE<uint8_t>();
                if (c != (int8_t) message[i]) {
                  printf("Body contents mismatch at offset=%d, "
                         "expected=%02x ('%c'), got=%02x ('%c')\n",
                         i,
                         (int) c,
                         (char) c,
                         (int) message[i],
                         (char) message[i]);
                  match = false;
                }
              }
              if (!match) {
                return false;
              }
            }
          } else if (name == "stdout" || name == "stderr") {
            CHECK(it->m_args.count("request_id"));
            CHECK(it->m_args.at("request_id").isInteger());
            int request_id = it->m_args.at("request_id").toInt32();
            IOBufQueue queue;
            if (it->m_args.count("message")) {
              CHECK(it->m_args.at("message").isString());
              String message = it->m_args.at("message").toString();
              for (int i = 0; i < message.size(); ++i) {
                uint8_t c = message[i];
                queue.append(&c, sizeof(uint8_t));
              }
            } else {
              CHECK(it->m_args.count("length"));
              CHECK(it->m_args.at("length").isInteger());
              int len = it->m_args.at("length").toInt32();
              for (int i = 0; i < len; ++i) {
                uint8_t c = rand();
                queue.append(&c, sizeof(uint8_t));
              }
            }
            auto handler = session_callback.m_handlers[request_id];
            if (name == "stdout") {
              handler->onStdOut(queue.move());
            } else {
              CHECK(name == "stderr");
              handler->onStdErr(queue.move());
            }
          } else {
            printf("Invalid CALL: %s\n", name.c_str());
            return false;
          }
          printf("Parameter test successful\n");
          break;
        }
        default:
          printf("Invalid message type\n");
          return false;
      }
    }

    if (send_buf.chainLength() > 0) {
      printf("Send buffer not fully consumed, length=%d\n",
             (int) send_buf.chainLength());
      return false;
    }
    if (!session_callback.m_egress.empty()) {
      printf("Receive buffer not fully consumed, length=%d\n",
             (int) session_callback.m_egress.chainLength());
      return false;
    }
  }

  return true;
}

bool TestFastCGIProtocol::TestSanity() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "sanity.json");
  EXCHANGE(mx, 1);
  return Count(true);
}

bool TestFastCGIProtocol::TestBeginRequest() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "begin_request.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestDoubleBeginRequest() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "double_begin_request.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestAbortRequest() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "abort_request.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestEndRequest() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "end_request.json");
  EXCHANGE(mx, 100);
  return Count(true);
}

bool TestFastCGIProtocol::TestParams() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "params.json");
  EXCHANGE(mx, 1000);
  return Count(true);
}

bool TestFastCGIProtocol::TestStdIn() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "stdin.json");
  EXCHANGE(mx, 1000);
  return Count(true);
}

bool TestFastCGIProtocol::TestStdOut() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "stdout.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestStdErr() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "stderr.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestData() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "data.json");
  EXCHANGE(mx, 10);
  return Count(true);
}

bool TestFastCGIProtocol::TestGetValues() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "get_values.json");
  EXCHANGE(mx, 100);
  return Count(true);
}

bool TestFastCGIProtocol::TestUnknownType() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "unknown_type.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestIgnoreNonActive() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "ignore_non_active.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

bool TestFastCGIProtocol::TestInvalidType() {
  TestMessageExchange mx;
  LOAD_JSON(mx, "invalid_type.json");
  EXCHANGE(mx, 50);
  return Count(true);
}

