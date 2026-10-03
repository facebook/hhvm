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

#include "hphp/runtime/vm/jit/cprof-reader.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <folly/testing/TestUtil.h>
#include <gtest/gtest.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/cprof-checkpoint.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit::cprof {
namespace {

constexpr auto kUnitPath = "hphp/runtime/test/cont-prof-reader-test.php";

constexpr auto kHhas = R"HHAS(
.function N cont_prof_reader_test_962aeb3f() {
  Null
  RetC None
}
)HHAS";

ContProfProfileRecord makeRecord(
  std::string name,
  uint8_t identity,
  uint64_t capturedAtMs,
  uint64_t executionCount,
  uint32_t regionLength = 1
) {
  ContProfProfileRecord record{};
  record.header.funcKey.resolutionUnitPath = "src/" + name + ".php";
  record.header.funcKey.bytecodeUnitHash =
    SHA1{static_cast<uint64_t>(identity)};
  record.header.funcKey.functionName = std::move(name);
  record.header.capturedAtMs = capturedAtMs;
  record.translations = {
    {
      ContProfStartKind::FuncEntry,
      0,
      regionLength,
      executionCount,
    },
  };
  return record;
}

std::unique_ptr<Unit> makeTestUnit() {
  auto const emitter = assemble_string(
    kHhas,
    kUnitPath,
    SHA1{"3333333333333333333333333333333333333333"},
    nullptr,
    RepoOptions::defaults().packageInfo(),
    UnitEmitterAttributes::defaults(),
    false
  );
  if (!emitter || emitter->m_fatalUnit) return nullptr;
  return emitter->create();
}

bool writeTextFile(const std::string& path, const std::string& contents) {
  std::ofstream output{path, std::ios::binary};
  output.write(contents.data(), contents.size());
  return output.good();
}

TEST(ContProfReader, ReadsEmptyDirectory) {
  folly::test::TemporaryDirectory directory{"hhvm_cont_prof_reader"};

  auto const result =
    readContProfCheckpointDirectory(directory.path().native());
  ASSERT_TRUE(result);
  EXPECT_EQ(0, result->filesRead);
  EXPECT_EQ(0, result->recordsDecoded);
  EXPECT_TRUE(result->records.empty());
}

TEST(ContProfReader, RejectsInvalidDirectory) {
  folly::test::TemporaryDirectory directory{"hhvm_cont_prof_reader"};
  auto const path = (directory.path() / "missing").native();

  EXPECT_FALSE(readContProfCheckpointDirectory(path));
}

TEST(ContProfReader, IgnoresSymlinkedCheckpoints) {
  folly::test::TemporaryDirectory directory{"hhvm_cont_prof_reader"};
  auto const target = (directory.path() / "checkpoint-data").native();
  auto const link =
    (directory.path() / "cont-prof-1-100.cprof").native();

  ASSERT_TRUE(writeContProfCheckpointFile(
    target,
    {makeRecord("alpha", 1, 100, 7)}
  ));

  std::error_code error;
  std::filesystem::create_symlink(target, link, error);
  ASSERT_FALSE(error) << error.message();

  auto const result =
    readContProfCheckpointDirectory(directory.path().native());
  ASSERT_TRUE(result);
  EXPECT_EQ(0, result->filesRead);
  EXPECT_EQ(0, result->recordsDecoded);
  EXPECT_TRUE(result->records.empty());
}

TEST(ContProfReader, SelectsBestCanonicalRecords) {
  folly::test::TemporaryDirectory directory{"hhvm_cont_prof_reader"};
  auto const path = [&](const char* filename) {
    return (directory.path() / filename).native();
  };

  auto const olderAlpha = makeRecord("alpha", 1, 100, 7, 1);
  auto const lowerBeta = makeRecord("beta", 2, 100, 7, 1);
  auto const olderGamma = makeRecord("gamma", 3, 100, 11, 2);

  auto const newerAlpha = makeRecord("alpha", 1, 200, 5, 2);
  auto const higherBeta = makeRecord("beta", 2, 100, 11, 2);
  auto const laterGamma = makeRecord("gamma", 3, 100, 11, 1);

  ASSERT_TRUE(writeContProfCheckpointFile(
    path("cont-prof-1-100.cprof"),
    {olderAlpha, lowerBeta, olderGamma}
  ));
  ASSERT_TRUE(writeContProfCheckpointFile(
    path("cont-prof-2-200.cprof"),
    {newerAlpha, higherBeta, laterGamma}
  ));

  ASSERT_TRUE(writeTextFile(path("cont-prof-4-400.cprof"), "not a checkpoint"));
  ASSERT_TRUE(writeTextFile(path("notes.txt"), "ignored"));

  auto const result =
    readContProfCheckpointDirectory(directory.path().native());
  ASSERT_TRUE(result);

  std::vector<ContProfProfileRecord> const expected{
    newerAlpha,
    higherBeta,
    laterGamma,
  };
  EXPECT_EQ(2, result->filesRead);
  EXPECT_EQ(6, result->recordsDecoded);
  EXPECT_EQ(expected, result->records);
}

TEST(ContProfReader, ChecksFunctionKey) {
  auto const unit = makeTestUnit();
  ASSERT_NE(nullptr, unit);
  ASSERT_EQ(1, unit->funcs().size());

  auto const func = unit->funcs()[0];
  auto const key = makeContProfFuncKey(*func);
  ASSERT_TRUE(key);

  ContProfProfileRecord record{};
  record.header.funcKey = *key;
  record.header.capturedAtMs = 100;
  record.translations = {
    {
      ContProfStartKind::FuncEntry,
      0,
      3,
      7,
    },
  };

  EXPECT_TRUE(isContProfProfileRecordCompatible(record, *func));

  record.header.funcKey.bytecodeUnitHash =
    SHA1{"4444444444444444444444444444444444444444"};
  EXPECT_FALSE(isContProfProfileRecordCompatible(record, *func));
}

TEST(ContProfReader, RejectsEntriesWhichDoNotExist) {
  auto const unit = makeTestUnit();
  ASSERT_NE(nullptr, unit);
  ASSERT_EQ(1, unit->funcs().size());

  auto const func = unit->funcs()[0];
  auto const key = makeContProfFuncKey(*func);
  ASSERT_TRUE(key);

  ContProfProfileRecord record{};
  record.header.funcKey = *key;
  record.header.capturedAtMs = 100;
  record.translations = {
    {
      ContProfStartKind::FuncEntry,
      1,
      1,
      7,
    },
  };
  EXPECT_FALSE(isContProfProfileRecordCompatible(record, *func));

  record.translations[0] = {
    ContProfStartKind::NamedParamsFuncEntry,
    0,
    1,
    7,
  };
  EXPECT_FALSE(isContProfProfileRecordCompatible(record, *func));
}

TEST(ContProfReader, RejectsUnsupportedBytecodeStarts) {
  auto const unit = makeTestUnit();
  ASSERT_NE(nullptr, unit);
  ASSERT_EQ(1, unit->funcs().size());

  auto const func = unit->funcs()[0];
  auto const key = makeContProfFuncKey(*func);
  ASSERT_TRUE(key);

  ContProfProfileRecord record{};
  record.header.funcKey = *key;
  record.header.capturedAtMs = 100;
  record.translations = {
    {
      ContProfStartKind::FuncEntry,
      0,
      3,
      7,
    },
    {
      ContProfStartKind::Bytecode,
      1,
      1,
      11,
    },
  };

  ASSERT_TRUE(isValidContProfProfileRecord(record));
  EXPECT_FALSE(isContProfProfileRecordCompatible(record, *func));
}

}
}
