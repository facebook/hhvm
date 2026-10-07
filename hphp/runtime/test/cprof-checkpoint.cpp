/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/cprof-checkpoint.h"

#include <string>
#include <utility>
#include <vector>

#include <folly/ScopeGuard.h>
#include <folly/testing/TestUtil.h>
#include <gtest/gtest.h>

#include "hphp/util/configs/jit.h"
#include "hphp/util/configs/server.h"

namespace HPHP::jit::cprof {
namespace {

ContProfProfileRecord makeRecord(
  std::string name,
  uint8_t identity,
  uint64_t executionCount
) {
  ContProfProfileRecord record{};
  record.header.funcKey.resolutionUnitPath = "src/" + name + ".php";
  record.header.funcKey.bytecodeUnitHash =
    SHA1{static_cast<uint64_t>(identity)};
  record.header.funcKey.functionName = std::move(name);
  record.header.capturedAtMs = 1'700'000'000'000 + identity;
  record.translations = {
    {
      ContProfStartKind::FuncEntry,
      0,
      2,
      executionCount,
    },
  };
  return record;
}

TEST(ContProfCheckpoint, RoundTripsRecords) {
  std::vector<ContProfProfileRecord> const expected{
    makeRecord("alpha", 1, 7),
    makeRecord("beta", 2, 11),
  };

  auto const encoded = serializeContProfCheckpoint(expected);
  ASSERT_TRUE(encoded);

  auto const decoded = deserializeContProfCheckpoint(
    folly::ByteRange{encoded->data(), encoded->size()}
  );
  ASSERT_TRUE(decoded);
  EXPECT_EQ(expected, *decoded);
}

TEST(ContProfCheckpoint, SkipsInvalidRecords) {
  auto const first = makeRecord("alpha", 1, 7);
  auto invalid = makeRecord("beta", 2, 11);
  invalid.header.capturedAtMs = 0;
  auto const last = makeRecord("gamma", 3, 13);

  auto const encoded = serializeContProfCheckpoint({
    first,
    invalid,
    last,
  });
  ASSERT_TRUE(encoded);

  auto const decoded = deserializeContProfCheckpoint(
    folly::ByteRange{encoded->data(), encoded->size()}
  );
  ASSERT_TRUE(decoded);

  std::vector<ContProfProfileRecord> const expected{first, last};
  EXPECT_EQ(expected, *decoded);
}

TEST(ContProfCheckpoint, RejectsMalformedOrNoncanonicalInput) {
  auto const first = makeRecord("alpha", 1, 7);
  auto const second = makeRecord("beta", 2, 11);

  EXPECT_FALSE(serializeContProfCheckpoint({}));
  EXPECT_FALSE(serializeContProfCheckpoint({second, first}));
  EXPECT_FALSE(serializeContProfCheckpoint({first, first}));

  auto const encoded = serializeContProfCheckpoint({first, second});
  ASSERT_TRUE(encoded);

  for (size_t size = 0; size < encoded->size(); ++size) {
    EXPECT_FALSE(deserializeContProfCheckpoint(
      folly::ByteRange{encoded->data(), size}
    ));
  }

  auto wrongRecordCount = *encoded;
  wrongRecordCount[0] ^= 1;
  EXPECT_FALSE(deserializeContProfCheckpoint(
    folly::ByteRange{wrongRecordCount.data(), wrongRecordCount.size()}
  ));

  auto trailingByte = *encoded;
  trailingByte.push_back(0);
  EXPECT_FALSE(deserializeContProfCheckpoint(
    folly::ByteRange{trailingByte.data(), trailingByte.size()}
  ));
}

TEST(ContProfCheckpoint, AtomicallyReplacesFile) {
  folly::test::TemporaryDirectory temp{"cont-prof-checkpoint"};
  auto const path = temp.path().native() + "/checkpoint.cprof";

  std::vector<ContProfProfileRecord> const first{
    makeRecord("alpha", 1, 7),
  };
  ASSERT_TRUE(writeContProfCheckpointFile(path, first));
  EXPECT_EQ(first, readContProfCheckpointFile(path));

  EXPECT_FALSE(writeContProfCheckpointFile(path, {}));
  EXPECT_EQ(first, readContProfCheckpointFile(path));

  std::vector<ContProfProfileRecord> const replacement{
    makeRecord("alpha", 1, 7),
    makeRecord("beta", 2, 11),
  };
  ASSERT_TRUE(writeContProfCheckpointFile(path, replacement));
  EXPECT_EQ(replacement, readContProfCheckpointFile(path));
}

TEST(ContProfCheckpoint, ActivatesOnlyWhileWriterIsRunning) {
  folly::test::TemporaryDirectory temp{"cont-prof-checkpoint-activation"};

  auto const oldServerMode = std::exchange(Cfg::Server::Mode, false);
  auto const oldCaptureEnabled =
    std::exchange(Cfg::Jit::ContProfCaptureEnabled, true);
  auto const oldInterval = std::exchange(
    Cfg::Jit::ContProfCheckpointIntervalSeconds,
    uint32_t{60}
  );
  auto const oldDirectory = std::exchange(
    Cfg::Jit::ContProfCheckpointDirectory,
    temp.path().native()
  );
  SCOPE_EXIT {
    stopContProfCheckpointWriter();
    Cfg::Server::Mode = oldServerMode;
    Cfg::Jit::ContProfCaptureEnabled = oldCaptureEnabled;
    Cfg::Jit::ContProfCheckpointIntervalSeconds = oldInterval;
    Cfg::Jit::ContProfCheckpointDirectory = oldDirectory;
  };

  startContProfCheckpointWriter();
  EXPECT_FALSE(contProfActive());

  Cfg::Server::Mode = true;
  Cfg::Jit::ContProfCheckpointDirectory = temp.path().native() + "/missing";
  startContProfCheckpointWriter();
  EXPECT_FALSE(contProfActive());

  Cfg::Jit::ContProfCheckpointDirectory = temp.path().native();
  startContProfCheckpointWriter();
  EXPECT_TRUE(contProfActive());

  stopContProfCheckpointWriter();
  EXPECT_FALSE(contProfActive());
}

}
}
