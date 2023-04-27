/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/network/ServerMcParser.h"
#include "mcrouter/lib/network/test/TestMcAsciiParserUtil.h"

using namespace facebook::memcache;

namespace facebook {
namespace memcache {
struct CaretMessageInfo;
}
} // namespace facebook

struct DummyMultiOpEnd {};

namespace {

bool compareRequests(const DummyMultiOpEnd&, const DummyMultiOpEnd&) {
  return true;
}

template <class Request>
bool compareRequests(const Request& expected, const Request& actual) {
  const auto expectedSp =
      carbon::valueRangeSlow(const_cast<Request&>(expected));
  const auto actualSp = carbon::valueRangeSlow(const_cast<Request&>(actual));
  return expectedSp == actualSp;
}

class TestRunner {
 public:
  TestRunner() {}

  TestRunner(const TestRunner&) = delete;
  TestRunner operator=(const TestRunner&) = delete;

  template <class Request>
  TestRunner& expectNext(Request req, bool noreply = false) {
    callbacks_.emplace_back(std::make_unique<ExpectedRequestCallback<Request>>(
        std::move(req), noreply));
    return *this;
  }

  TestRunner& expectMultiOpEnd() {
    callbacks_.emplace_back(std::make_unique<ExpectedMultiOpEndCallback>());
    return *this;
  }

  TestRunner& expectError() {
    isError_ = true;
    return *this;
  }

  /**
   * Generates multiple possible splits of data and then runs test for each of
   * them.
   */
  TestRunner& run(folly::IOBuf data) {
    data.coalesce();

    // Limit max piece size, since it may produce huge amount of combinations.
    size_t pieceSize = 0;
    size_t cnt = 0;
    while (pieceSize <= data.length() && cnt < 20000) {
      cnt = chunkedDataSetsCnt(data.length(), ++pieceSize);
    }
    auto splits = genChunkedDataSets(data.length(), pieceSize - 1);
    splits.push_back({data.length()});
    for (const auto& split : splits) {
      auto tmp = chunkData(data, split);
      // If we failed a test already, just break.
      if (!runImpl(std::move(*tmp))) {
        return *this;
      }
    }

    return *this;
  }

  template <typename... Args>
  TestRunner& run(folly::StringPiece format, Args... args) {
    return run(folly::sformat(format, std::forward<Args>(args)...));
  }

  TestRunner& run(folly::StringPiece data) {
    run(folly::IOBuf(folly::IOBuf::COPY_BUFFER, data.begin(), data.size()));
    return *this;
  }

 private:
  template <class Request>
  class ExpectedRequestCallback;

  class ExpectedCallbackBase {
   public:
    template <class Request>
    bool validate(const Request& req, bool noreply) const {
      EXPECT_TRUE(reqType_ == typeid(Request))
          << "Parsed wrong type of request!";
      EXPECT_EQ(noreply_, noreply);

      if (reqType_ == typeid(Request) && noreply == noreply_) {
        auto& message =
            *reinterpret_cast<const ExpectedRequestCallback<Request>*>(this);
        return compareRequests(message.req_, req);
      } else {
        return false;
      }

      return true;
    }

    virtual ~ExpectedCallbackBase() = default;

   protected:
    template <class Request>
    ExpectedCallbackBase(const Request&, bool noreply)
        : reqType_(typeid(Request)), noreply_(noreply) {}

   private:
    std::type_index reqType_;
    bool noreply_{false};
  };

  template <class Request>
  class ExpectedRequestCallback : public ExpectedCallbackBase {
   public:
    explicit ExpectedRequestCallback(Request req, bool noreply = false)
        : ExpectedCallbackBase(req, noreply), req_(std::move(req)) {}
    ~ExpectedRequestCallback() override = default;

   private:
    Request req_;

    friend class ExpectedCallbackBase;
  };

  class ExpectedMultiOpEndCallback
      : public ExpectedRequestCallback<DummyMultiOpEnd> {
   public:
    ExpectedMultiOpEndCallback() : ExpectedRequestCallback(DummyMultiOpEnd()) {}
  };

  class ParserOnRequest {
   public:
    ParserOnRequest(
        std::vector<std::unique_ptr<ExpectedCallbackBase>>& cbs,
        bool isError)
        : callbacks_(cbs), isError_(isError) {}

    bool isFinished() const {
      return finished_;
    }

    bool isFailed() const {
      return failed_;
    }

    void setParser(ServerMcParser<ParserOnRequest>* parser) {
      parser_ = parser;
    }

   private:
    std::vector<std::unique_ptr<ExpectedCallbackBase>>& callbacks_;
    ServerMcParser<ParserOnRequest>* parser_{nullptr};
    bool isError_;
    size_t id_{0};
    bool finished_{false};
    bool failed_{false};

    // ServerMcParser callbacks.
    void caretRequestReady(const CaretMessageInfo&, const folly::IOBuf&) {
      FAIL() << "caretRequestReady should never be called for ASCII";
    }

    void parseError(carbon::Result, folly::StringPiece reason) {
      ASSERT_NE(nullptr, parser_)
          << "Test framework bug, didn't provide parser to callback!";
      EXPECT_TRUE(isError_) << "Unexpected parsing error: " << reason
                            << ". Ascii parser message: "
                            << parser_->getUnderlyingAsciiParserError();
      EXPECT_EQ(callbacks_.size(), id_)
          << "Didn't consume all requests, but already failed to parse!";
      finished_ = true;
      failed_ = !isError_ || callbacks_.size() != id_;
    }

    template <class Request>
    void checkNext(Request&& req, bool noreply) {
      EXPECT_LT(id_, callbacks_.size()) << "Unexpected callback!";
      if (id_ < callbacks_.size()) {
        bool validationRes = callbacks_[id_]->validate(req, noreply);
        EXPECT_TRUE(validationRes)
            << "Wrong callback was called or parsed incorrect request!";
        if (!validationRes) {
          finished_ = true;
          failed_ = true;
          return;
        }

        ++id_;
        if (id_ == callbacks_.size()) {
          // Mark test as finished if we don't expect an error, otherwise we
          // still need to see more data.
          finished_ = !isError_;
        }
      } else {
        finished_ = true;
        failed_ = true;
      }
    }

    template <class Request>
    void onRequest(Request&& req, bool noreply) {
      checkNext(std::move(req), noreply);
    }

    void multiOpEnd() {
      checkNext(DummyMultiOpEnd(), false);
    }

    friend class ServerMcParser<ParserOnRequest>;
  };

  std::vector<std::unique_ptr<ExpectedCallbackBase>> callbacks_;
  bool isError_{false};

  bool runImpl(folly::IOBuf data) {
    ParserOnRequest onRequest(callbacks_, isError_);
    ServerMcParser<ParserOnRequest> parser(
        onRequest, 4096 /* min buffer size */, 4096 /* max buffer size */);
    onRequest.setParser(&parser);

    for (auto range : data) {
      while (range.size() > 0 && !onRequest.isFinished()) {
        auto buffer = parser.getReadBuffer();
        auto readLen = std::min(buffer.second, range.size());
        memcpy(buffer.first, range.begin(), readLen);
        parser.readDataAvailable(readLen);
        range.advance(readLen);
      }
    }

    EXPECT_TRUE(onRequest.isFinished()) << "Not all of the callbacks were "
                                           "called or we didn't encounter "
                                           "error if it was expected!";
    if (!onRequest.isFinished() || onRequest.isFailed()) {
      std::string info = "";
      for (auto range : data) {
        if (!info.empty()) {
          info += ", ";
        }
        info += folly::sformat(
            "\"{}\"", folly::cEscape<std::string>(folly::StringPiece(range)));
      }
      LOG(INFO) << "Test data for failed test: " << info;
      return false;
    }

    return true;
  }
};

std::string createBigValue() {
  const size_t kSize = 16384;
  char bigValue[kSize];
  for (size_t i = 0; i < kSize; ++i) {
    bigValue[i] = (unsigned char)(i % 256);
  }
  return std::string(bigValue, bigValue + kSize);
}

template <class Request>
Request createUpdateLike(
    folly::StringPiece key,
    folly::StringPiece value,
    uint64_t flags,
    int32_t exptime) {
  // Test regular request
  Request r(key);
  r.value_ref() =
      folly::IOBuf(folly::IOBuf::COPY_BUFFER, value.begin(), value.size());
  r.flags_ref() = flags;
  r.exptime_ref() = exptime;
  return r;
}

template <class Request>
Request createArithmeticLike(folly::StringPiece key, double delta) {
  Request r(key);
  r.delta_ref() = delta;
  return r;
}

// Test body for get, gets, lease-get, metaget.
template <class Request>
void getLikeTest(std::string opCmd) {
  // Test single key requests.
  TestRunner()
      .expectNext(Request("test:stepan:1"))
      .expectMultiOpEnd()
      .run(opCmd + " test:stepan:1\r\n")
      .run(opCmd + "   test:stepan:1\r\n")
      .run(opCmd + "   test:stepan:1\n")
      .run(opCmd + " test:stepan:1  \r\n")
      .run(opCmd + " test:stepan:1  \n")
      .expectNext(Request("test:stepan:2"))
      .expectMultiOpEnd()
      .run(opCmd + " test:stepan:1\r\n" + opCmd + " test:stepan:2\r\n");

  // Test multi key.
  TestRunner()
      .expectNext(Request("test:stepan:1"))
      .expectNext(Request("test:stepan:2"))
      .expectMultiOpEnd()
      .run(opCmd + " test:stepan:1 test:stepan:2\r\n")
      .run(opCmd + " test:stepan:1   test:stepan:2\r\n")
      .run(opCmd + " test:stepan:1 test:stepan:2 \r\n")
      .run(opCmd + " test:stepan:1 test:stepan:2 \n")
      .run(opCmd + " test:stepan:1 test:stepan:2\n")
      .expectNext(Request("test:stepan:3"))
      .expectMultiOpEnd()
      .run(
          opCmd + " test:stepan:1 test:stepan:2\r\n" + opCmd +
          " test:stepan:3\r\n");

  TestRunner().expectError().run(opCmd + "no:space:before:key\r\n");

  // Missing key.
  TestRunner().expectError().run(opCmd + "\r\n").run(opCmd + "   \r\n");
}

const char* kTestValue = "someSmallTestValue";

// Test body for set, add, replace, append, prepend.
template <class Request>
void setLikeTest(std::string opCmd) {
  TestRunner()
      .expectNext(
          createUpdateLike<Request>("test:stepan:1", kTestValue, 123, 651342))
      .run(
          "{} test:stepan:1 123 651342 {}\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{}   test:stepan:1 123 651342 {}\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1   123 651342 {}\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123   651342 {}\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123 651342 {}  \r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123 651342 {}  \n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123 651342 {}  \r\n{}\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123 651342 {}  \n{}\n",
          opCmd,
          strlen(kTestValue),
          kTestValue);

  // Test negative exptime.
  TestRunner()
      .expectNext(createUpdateLike<Request>(
          "test:stepan:1", kTestValue, 123, -12341232))
      .run(
          "{} test:stepan:1 123 -12341232 {}\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue);

  // Test bad set command format.
  TestRunner().expectError().run(
      "{} test:stepan:1 12as3 -12341232 {}\r\n{}\r\n",
      opCmd,
      strlen(kTestValue),
      kTestValue);

  // Test noreply.
  TestRunner()
      .expectNext(
          createUpdateLike<Request>("test:stepan:1", kTestValue, 123, 651342),
          true)
      .run(
          "{} test:stepan:1 123 651342 {} noreply\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123 651342 {}   noreply\r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue)
      .run(
          "{} test:stepan:1 123 651342 {} noreply  \r\n{}\r\n",
          opCmd,
          strlen(kTestValue),
          kTestValue);

  // Test big value.
  std::string bigValue = createBigValue();
  TestRunner()
      .expectNext(createUpdateLike<Request>("test:stepan:1", bigValue, 345, -2))
      .run(
          "{} test:stepan:1 345 -2 {}\r\n{}\r\n",
          opCmd,
          bigValue.size(),
          bigValue);
}

template <class Request>
void multiTokenOpTest(std::string opCmd) {
  // With parameter.
  TestRunner()
      .expectNext(Request("token1"))
      .run(opCmd + " token1\r\n")
      .run(opCmd + "  token1  \r\n");

  // With multiple parameters.
  TestRunner()
      .expectNext(Request("token1 token2"))
      .run(opCmd + " token1 token2\r\n")
      .run(opCmd + "  token1 token2  \r\n");

  TestRunner()
      .expectNext(Request("token1   token2"))
      .run(opCmd + " token1   token2\r\n")
      .run(opCmd + "  token1   token2  \r\n");
}

template <class Request>
void arithmeticTest(std::string opCmd) {
  TestRunner()
      .expectNext(createArithmeticLike<Request>("test:stepan:1", 1324123))
      .run(opCmd + " test:stepan:1 1324123\r\n")
      .run(opCmd + "     test:stepan:1    1324123   \r\n")
      .run(opCmd + " test:stepan:1  1324123 \r\n");

  TestRunner()
      .expectNext(createArithmeticLike<Request>("test:stepan:1", 1324), true)
      .run(opCmd + " test:stepan:1 1324 noreply\r\n")
      .run(opCmd + "     test:stepan:1    1324   noreply  \r\n")
      .run(opCmd + " test:stepan:1  1324 noreply \r\n");

  // No delta.
  TestRunner()
      .expectError()
      .run(opCmd + " test:stepan:1 noreply\r\n")
      .run(opCmd + "     test:stepan:1       noreply  \r\n")
      .run(opCmd + " test:stepan:1   noreply \r\n");
}

} // namespace

TEST(McServerAsciiParserHarness, get) {
  getLikeTest<McGetRequest>("get");
}

TEST(McServerAsciiParserHarness, gets) {
  getLikeTest<McGetsRequest>("gets");
}

TEST(McServerAsciiParserHarness, lease_get) {
  getLikeTest<McLeaseGetRequest>("lease-get");
}

TEST(McServerAsciiParserHarness, metaget) {
  getLikeTest<McMetagetRequest>("metaget");
}

TEST(McServerAsciiParserHarness, set) {
  setLikeTest<McSetRequest>("set");
}

TEST(McServerAsciiParserHarness, add) {
  setLikeTest<McAddRequest>("add");
}

TEST(McServerAsciiParserHarness, replace) {
  setLikeTest<McReplaceRequest>("replace");
}

TEST(McServerAsciiParserHarness, append) {
  setLikeTest<McAppendRequest>("append");
}

TEST(McServerAsciiParserHarness, prepend) {
  setLikeTest<McPrependRequest>("prepend");
}

TEST(McServerAsciiParserHarness, quit) {
  TestRunner()
      .expectNext(McQuitRequest(), true /* quit is always noreply */)
      .run("quit\r\n")
      .run("quit    \r\n");
}

TEST(McServerAsciiParserHarness, version) {
  TestRunner()
      .expectNext(McVersionRequest())
      .run("version\r\n")
      .run("version    \r\n");
}

TEST(McServerAsciiParserHarness, quitWithVersion) {
  TestRunner()
      .expectNext(McQuitRequest(), true)
      .expectNext(McVersionRequest())
      .run(
          "quit\r\n"
          "version\r\n");
}

TEST(McServerAsciiParserHarness, shutdown) {
  TestRunner()
      .expectNext(McShutdownRequest())
      .run("shutdown\r\n")
      .run("shutdown    \r\n");
}

TEST(McServerAsciiParserHarness, stats) {
  TestRunner()
      .expectNext(McStatsRequest())
      .run("stats\r\n")
      .run("stats    \r\n");

  multiTokenOpTest<McStatsRequest>("stats");
}

TEST(McServerAsciiParserHarness, exec) {
  multiTokenOpTest<McExecRequest>("exec");
  multiTokenOpTest<McExecRequest>("admin");
}

TEST(McServerAsciiParserHarness, delete) {
  TestRunner()
      .expectNext(McDeleteRequest("test:stepan:1"))
      .run("delete test:stepan:1\r\n")
      .run("delete  test:stepan:1  \r\n");
  TestRunner()
      .expectNext(McDeleteRequest("test:stepan:1"), true)
      .run("delete test:stepan:1 noreply\r\n")
      .run("delete  test:stepan:1  noreply   \r\n");

  McDeleteRequest r("test:stepan:1");
  r.exptime_ref() = -10;
  TestRunner()
      .expectNext(r)
      .run("delete test:stepan:1 -10\r\n")
      .run("delete  test:stepan:1  -10  \r\n");

  r.exptime_ref() = 1234123;
  TestRunner()
      .expectNext(r)
      .run("delete test:stepan:1 1234123\r\n")
      .run("delete  test:stepan:1  1234123  \r\n");
  TestRunner()
      .expectNext(r, true)
      .run("delete test:stepan:1 1234123 noreply\r\n")
      .run("delete  test:stepan:1  1234123  noreply  \r\n");
}

TEST(McServerAsciiParserHarness, touch) {
  McTouchRequest r("test:key:1");
  r.exptime_ref() = -10;
  TestRunner()
      .expectNext(r)
      .run("touch test:key:1 -10\r\n")
      .run("touch  test:key:1  -10  \r\n");
  TestRunner()
      .expectNext(r, true)
      .run("touch test:key:1 -10 noreply\r\n")
      .run("touch  test:key:1  -10  noreply   \r\n");

  r.exptime_ref() = 1234567;
  TestRunner()
      .expectNext(r)
      .run("touch test:key:1 1234567\r\n")
      .run("touch  test:key:1  1234567  \r\n");
  TestRunner()
      .expectNext(r, true)
      .run("touch test:key:1 1234567 noreply\r\n")
      .run("touch  test:key:1  1234567  noreply  \r\n");
}

TEST(McServerAsciiParserHarness, incr) {
  arithmeticTest<McIncrRequest>("incr");
}

TEST(McServerAsciiParserHarness, decr) {
  arithmeticTest<McDecrRequest>("decr");
}

TEST(McServerAsciiParserHarness, flush_all) {
  TestRunner()
      .expectNext(McFlushAllRequest())
      .run("flush_all\r\n")
      .run("flush_all     \r\n");

  McFlushAllRequest r;
  r.delay_ref() = 123456789;
  TestRunner()
      .expectNext(std::move(r))
      .run("flush_all 123456789\r\n")
      .run("flush_all    123456789\r\n")
      .run("flush_all    123456789   \r\n");
}

TEST(McServerAsciiParserHarness, flush_regex) {
  // Flush_regex expects a key.
  TestRunner().expectError().run("flush_regex\r\n").run("flush_regex     \r\n");

  TestRunner()
      .expectNext(McFlushReRequest("test:stepan:1"))
      .run("flush_regex test:stepan:1\r\n")
      .run("flush_regex    test:stepan:1\r\n")
      .run("flush_regex   test:stepan:1   \r\n");
}

TEST(McServerAsciiParserHarness, lease_set) {
  auto r =
      createUpdateLike<McLeaseSetRequest>("test:stepan:1", kTestValue, 1, 65);
  r.leaseToken_ref() = 123;

  TestRunner()
      .expectNext(r)
      .run("lease-set test:stepan:1 123 1 65 18\r\nsomeSmallTestValue\r\n")
      .run(
          "lease-set   test:stepan:1   123   1   65   18  \r\n"
          "someSmallTestValue\r\n");

  TestRunner()
      .expectNext(r, true)
      .run(
          "lease-set test:stepan:1 123 1 65 18 noreply\r\n"
          "someSmallTestValue\r\n")
      .run(
          "lease-set   test:stepan:1   123   1   65   18  noreply  \r\n"
          "someSmallTestValue\r\n");
}

TEST(McServerAsciiParserHarness, cas) {
  auto r = createUpdateLike<McCasRequest>("test:stepan:1", kTestValue, 1, 65);
  r.casToken_ref() = 123;

  TestRunner()
      .expectNext(r)
      .run("cas test:stepan:1 1 65 18 123\r\nsomeSmallTestValue\r\n")
      .run(
          "cas   test:stepan:1   1   65   18  123  \r\n"
          "someSmallTestValue\r\n");

  TestRunner()
      .expectNext(r, true)
      .run(
          "cas test:stepan:1 1 65 18 123 noreply\r\n"
          "someSmallTestValue\r\n")
      .run(
          "cas   test:stepan:1   1   65   18   123   noreply  \r\n"
          "someSmallTestValue\r\n");
}

TEST(McServerAsciiParserHarness, allOps) {
  auto casRequest =
      createUpdateLike<McCasRequest>("test:stepan:11", "Facebook", 765, -1);
  casRequest.casToken_ref() = 893;

  auto leaseSetRequest =
      createUpdateLike<McLeaseSetRequest>("test:stepan:12", "hAcK", 294, 563);
  leaseSetRequest.leaseToken_ref() = 846;

  McDeleteRequest deleteRequest("test:stepan:13");
  deleteRequest.exptime_ref() = 2345234;

  TestRunner()
      .expectNext(McGetRequest("test:stepan:1"))
      .expectMultiOpEnd()
      .expectNext(McGetsRequest("test:stepan:2"))
      .expectNext(McGetsRequest("test:stepan:10"))
      .expectMultiOpEnd()
      .expectNext(McLeaseGetRequest("test:stepan:3"))
      .expectMultiOpEnd()
      .expectNext(McMetagetRequest("test:stepan:4"))
      .expectMultiOpEnd()
      .expectNext(createUpdateLike<McSetRequest>("test:stepan:5", "Abc", 1, 2))
      .expectNext(createUpdateLike<McAddRequest>(
          "test:stepan:6", "abcdefgHiJklMNo", 3, 4))
      .expectNext(
          createUpdateLike<McReplaceRequest>("test:stepan:7", "A", 6, 7))
      .expectNext(createUpdateLike<McAppendRequest>("test:stepan:8", "", 8, 9))
      .expectNext(
          createUpdateLike<McPrependRequest>("test:stepan:9", "xYZ", 10, 11))
      .expectNext(casRequest)
      .expectNext(leaseSetRequest)
      .expectNext(deleteRequest)
      .expectNext(McStatsRequest("test stats"))
      .expectNext(McExecRequest("reboot server"))
      .expectNext(McQuitRequest(), true)
      .expectNext(McVersionRequest())
      .expectNext(McShutdownRequest())
      .expectNext(createArithmeticLike<McIncrRequest>("arithm!", 90))
      .expectNext(createArithmeticLike<McDecrRequest>("ArItHm!", 87))
      .expectNext(McFlushAllRequest())
      .expectNext(McFlushReRequest("^reGex$"))
      .run(
          "get test:stepan:1\r\n"
          "gets test:stepan:2 test:stepan:10\r\n"
          "lease-get test:stepan:3\r\n"
          "metaget test:stepan:4\r\n"
          "set test:stepan:5 1 2 3\r\nAbc\r\n"
          "add test:stepan:6 3 4 15\r\nabcdefgHiJklMNo\r\n"
          "replace test:stepan:7 6 7 1\r\nA\r\n"
          "append test:stepan:8 8 9 0\r\n\r\n"
          "prepend test:stepan:9 10 11 3\r\nxYZ\r\n"
          "cas test:stepan:11 765 -1 8 893\r\nFacebook\r\n"
          "lease-set test:stepan:12 846 294 563 4\r\nhAcK\r\n"
          "delete test:stepan:13 2345234\r\n"
          "stats test stats\r\n"
          "exec reboot server\r\n"
          "quit\r\n"
          "version\r\n"
          "shutdown\n"
          "incr arithm! 90\r\n"
          "decr ArItHm! 87\r\n"
          "flush_all\r\n"
          "flush_regex ^reGex$\r\n");
}
