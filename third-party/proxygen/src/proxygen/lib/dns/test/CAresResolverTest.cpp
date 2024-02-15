/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include "proxygen/lib/dns/CAresResolver.h"

using namespace folly;
using namespace proxygen;
using namespace testing;

using folly::DelayedDestruction;

class MockCAresResolver : public CAresResolver {
 public:
  using CAresResolver::CAresResolver;

  MOCK_METHOD(void, queryFinished, ());
  MOCK_METHOD(
      void,
      aresQuery,
      (const std::string&, CAresResolver::RecordType, ares_callback, void*),
      (const));
  MOCK_METHOD(void, recordPlaintext, (CAresResolver::Query*), (const));
  MOCK_METHOD(void, recordEncrypted, (CAresResolver::Query*), (const));

 private:
  ~MockCAresResolver() override {
  }
};

class MockQuery : public CAresResolver::Query {
 public:
  using CAresResolver::Query::fail;
  using CAresResolver::Query::Query;
  using CAresResolver::Query::timeoutExpired;
};

class CAresResolverTest : public testing::Test {
 public:
  void SetUp() override {
    resolver.reset(new MockCAresResolver());
  }

  std::unique_ptr<MockCAresResolver, DelayedDestruction::Destructor> resolver;
  const std::string name = "test.fb.com";
  TraceEventContext teContext;
};

class MockQueryWithCob : public MockQuery {
 public:
  MockQueryWithCob(CAresResolver* resolver,
                   CAresResolver::RecordType type,
                   const std::string& name,
                   bool recordStats,
                   TraceEvent dnsEvent,
                   const TimeUtil* timeUtil = nullptr,
                   TraceEventContext teContext = TraceEventContext(),
                   CAresResolver::ResolutionCallback* cb = nullptr)
      : MockQuery(
            resolver, type, name, recordStats, dnsEvent, timeUtil, teContext) {
    callback_ = cb;
  }
};

class MockResolutionCallback : public CAresResolver::ResolutionCallback {
 public:
  using CAresResolver::ResolutionCallback::ResolutionCallback;
  void resolutionSuccess(
      std::vector<DNSResolver::Answer> /*answers*/) noexcept override {
  }
  void resolutionError(
      const folly::exception_wrapper& /*ew*/) noexcept override {
  }
};

TEST_F(CAresResolverTest, ScheduleFallbackOnTimeout) {
  TraceEvent te(TraceEventType::DnsResolution);
  auto cb = std::make_unique<MockResolutionCallback>();
  auto query =
      std::make_unique<MockQueryWithCob>(resolver.get(),
                                         CAresResolver::RecordType::kTXT,
                                         name,
                                         true,
                                         std::move(te),
                                         nullptr,
                                         std::move(teContext),
                                         cb.get());
  query->timeoutExpired();
}

TEST_F(CAresResolverTest, DontScheduleFallbackOnTimeoutNoCob) {
  TraceEvent te(TraceEventType::DnsResolution);
  auto cb = std::make_unique<MockResolutionCallback>();
  auto query =
      std::make_unique<MockQueryWithCob>(resolver.get(),
                                         CAresResolver::RecordType::kTXT,
                                         name,
                                         true,
                                         std::move(te),
                                         nullptr,
                                         std::move(teContext),
                                         nullptr);
  query->timeoutExpired();
}

TEST_F(CAresResolverTest, ScheduleFallbackOnFailureDnscr) {
  TraceEvent te(TraceEventType::DnsResolution);
  TimeUtil tu;
  auto cb = std::make_unique<MockResolutionCallback>();
  auto query = new MockQueryWithCob(resolver.get(),
                                    CAresResolver::RecordType::kTXT,
                                    name,
                                    true,
                                    std::move(te),
                                    &tu,
                                    std::move(teContext),
                                    cb.get());
  query->setDnsCryptUsed(true, 1);
  query->fail(static_cast<DNSResolver::ResolutionStatus>(1), "error");
}

TEST_F(CAresResolverTest, ScheduleFallbackOnFailureDnscrNoCob) {
  TraceEvent te(TraceEventType::DnsResolution);
  auto query = new MockQueryWithCob(resolver.get(),
                                    CAresResolver::RecordType::kTXT,
                                    name,
                                    true,
                                    std::move(te),
                                    nullptr,
                                    std::move(teContext));
  query->setDnsCryptUsed(true, 1);
  query->fail(static_cast<DNSResolver::ResolutionStatus>(1), "error");
}

static unsigned char TWO_TXT_RESPONSE[] =
    "\x56\x14\x85\x80\x00\x01\x00\x02\x00\x00\x00\x01\x01\x32\x0d\x64"
    "\x6e\x73\x63\x72\x79\x70\x74\x2d\x63\x65\x72\x74\x08\x66\x61\x63"
    "\x65\x62\x6f\x6f\x6b\x03\x63\x6f\x6d\x00\x00\x10\x00\x01\xc0\x0c"
    "\x00\x10\x00\x01\x00\x01\x51\x80\x00\x7d\x7c\x44\x4e\x53\x43\x00"
    "\x01\x00\x00\x58\x44\x23\xfa\xdd\xef\xd5\x7a\x75\xd6\xd1\x6d\x7e"
    "\x5b\xa8\x8c\x0f\x25\xfc\x50\x40\x4e\xc6\x7e\x87\xf8\x53\x44\x0c"
    "\x8c\xa5\x51\x16\xa7\xd6\x82\x69\xfc\x7e\xbb\xfd\x80\x8c\x79\xc8"
    "\x43\xae\xa3\x4d\xe6\x8b\xec\x94\xfa\x77\xfe\x40\xda\x28\x3c\x7c"
    "\x5f\x38\x04\x44\xfd\x3a\xe3\x36\x85\xb5\xa4\xc7\xa1\x75\x22\x09"
    "\x08\x28\x0d\x3b\xbd\x15\x2f\x34\x69\xce\x04\x84\xe0\x57\xfe\x0c"
    "\x0d\x42\x6a\x44\xfd\x3a\xe3\x36\x85\xb5\xa4\x59\xdd\x7a\x18\x59"
    "\xdd\x7a\x18\x5b\xbe\xad\x98\xc0\x0c\x00\x10\x00\x01\x00\x01\x51"
    "\x80\x00\x7d\x7c\x44\x4e\x53\x43\x00\x01\x00\x00\x9c\x18\x01\xa3"
    "\x3d\x01\xab\x07\x2d\x94\x26\xa3\xd3\x0a\x52\x7b\xb4\x57\x08\x1b"
    "\xf3\x39\x36\x6a\x84\x51\xa0\x90\x5f\x38\xc3\xe3\xce\x52\x0e\xcc"
    "\x01\x3f\xfd\x4e\x25\xd7\x1a\xe2\x34\x57\xdd\x73\xab\xfd\x5b\x38"
    "\x09\x39\x93\x3c\x2e\x53\x67\x0b\x82\xc6\xd8\x0f\xbd\xc2\xde\x1b"
    "\x10\x7b\xbe\x4e\x83\x87\x7e\xc6\xd1\x60\xaa\x26\x26\x4a\xd7\x4f"
    "\xe4\x41\x8a\xc6\x57\x1d\xb3\xc0\xb4\x4c\xd1\x51\xbd\xc2\xde\x1b"
    "\x10\x7b\xbe\x4e\x59\xdd\x78\x66\x59\xdd\x78\x66\x5b\xbe\xab\xe6"
    "\x00\x00\x29\x10\x00\x00\x00\x00\x00\x00\x00";

static unsigned char ONE_TXT_RESPONSE[] =
    "\x15\xbe\x85\x80\x00\x01\x00\x01\x00\x00\x00\x01\x01\x32\x0d\x64"
    "\x6e\x73\x63\x72\x79\x70\x74\x2d\x63\x65\x72\x74\x08\x66\x61\x63"
    "\x65\x62\x6f\x6f\x6b\x03\x63\x6f\x6d\x00\x00\x10\x00\x01\xc0\x0c"
    "\x00\x10\x00\x01\x00\x01\x51\x80\x00\x7d\x7c\x44\x4e\x53\x43\x00"
    "\x01\x00\x00\xfd\x8f\xb6\xe8\xb2\x04\x63\xc7\x52\x90\x13\xbc\x36"
    "\xd6\xb1\x3d\x82\x77\xf8\x62\x7b\x58\x77\xf1\x24\xaf\x62\x79\x24"
    "\x10\x9c\xc0\xc7\x54\x72\x4e\x31\x7f\x7f\xc0\xd4\xb1\xd8\x97\xd9"
    "\x89\x37\xfe\x6a\x56\xd5\x7a\xde\x68\x0b\x27\x7a\x18\x3a\x7a\x20"
    "\xad\xf8\x09\x68\x71\xbe\xa1\x6c\xe1\xb1\xc4\x4c\xa9\x65\x98\x93"
    "\x07\xad\x90\xb2\xff\xae\xe0\x03\x07\x31\x4e\x0a\x6c\x09\xc3\x63"
    "\xf5\x83\x64\x68\x71\xbe\xa1\x6c\xe1\xb1\xc4\x58\x22\x4d\x2f\x58"
    "\x22\x4d\x2f\x5a\x03\x80\xaf\x00\x00\x29\x10\x00\x00\x00\x00\x00"
    "\x00\x00";

static unsigned char GARBAGE[] = "GARBAGE";

// This test is because of T22586769
TEST_F(CAresResolverTest, TestParseTxtRecords) {
  auto res = proxygen::detail::parseTxtRecords(&TWO_TXT_RESPONSE[0],
                                               sizeof(TWO_TXT_RESPONSE));
  ASSERT_TRUE(res.hasValue());

  auto answers = std::move(res).value();
  EXPECT_EQ(2, answers.size());

  res = proxygen::detail::parseTxtRecords(&ONE_TXT_RESPONSE[0],
                                          sizeof(ONE_TXT_RESPONSE));
  ASSERT_TRUE(res.hasValue());

  answers = std::move(res).value();
  EXPECT_EQ(1, answers.size());

  res = proxygen::detail::parseTxtRecords(&GARBAGE[0], sizeof(GARBAGE));
  EXPECT_TRUE(res.hasError());
}
