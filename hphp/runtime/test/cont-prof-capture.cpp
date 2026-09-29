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

#include "hphp/runtime/vm/jit/cont-prof-capture.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/cont-prof-controller.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/assertions.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit {
namespace {

constexpr auto kUnitPath = "hphp/runtime/test/cont-prof-capture-test.php";

constexpr auto kHhas = R"HHAS(
.function N cont_prof_capture_test_81d926f4() {
  Null
  RetC None
}
)HHAS";

struct DestroyTestUnit {
  void operator()(Unit* unit) const {
    if (!unit) return;

    for (auto const func : unit->funcs()) {
      auto const named = func->getNamedFunc();
      if (named->getCachedFunc() == func) {
        named->setCachedFunc(nullptr);
      }
    }
    unit->destroy();
  }
};

using TestUnit = std::unique_ptr<Unit, DestroyTestUnit>;

TestUnit makeTestUnit() {
  auto const emitter = assemble_string(
    kHhas,
    kUnitPath,
    SHA1{"2222222222222222222222222222222222222222"},
    nullptr,
    RepoOptions::defaults().packageInfo(),
    UnitEmitterAttributes::defaults(),
    false
  );
  if (!emitter || emitter->m_fatalUnit) return nullptr;
  return TestUnit{emitter->create().release()};
}

void addProfileTranslation(
  ProfData& profData,
  SrcKey start,
  int regionLength,
  int64_t executionCount,
  SBInvOffset spOffset = SBInvOffset{0}
) {
  auto region = std::make_shared<RegionDesc>();
  region->addBlock(start, regionLength, spOffset);

  auto const transId = profData.allocTransID();
  profData.addTransProfile(transId, region, PostConditions{}, 0);

  always_assert(executionCount >= 0);
  always_assert(executionCount <= profData.counterDefault());
  *profData.transCounterAddr(transId) =
    profData.counterDefault() - executionCount;
}

struct ContProfCaptureTest : testing::Test {
  static void SetUpTestSuite() {
    s_unit = makeTestUnit();
    ASSERT_NE(nullptr, s_unit);
    ASSERT_EQ(1, s_unit->funcs().size());

    s_func = s_unit->funcs()[0];
    s_unit->merge();
  }

  static void TearDownTestSuite() {
    s_func = nullptr;
    s_unit.reset();
  }

  static Func* func() { return s_func; }

private:
  static inline TestUnit s_unit;
  static inline Func* s_func{nullptr};
};

TEST_F(ContProfCaptureTest, CapturesNonzeroEntries) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  auto const named = SrcKey{func(), 0, true, SrcKey::FuncEntryTag{}};
  auto const mid = SrcKey{func(), 0, ResumeMode::None};

  addProfileTranslation(profData, named, 1, 11);
  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(profData, mid, 1, 13);
  addProfileTranslation(profData, main, 3, 0);

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  auto const expectedKey = makeContProfFuncKey(*func());
  ASSERT_TRUE(expectedKey);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 2, 7},
    {ContProfStartKind::NamedParamsFuncEntry, 0, 1, 11},
  };
  EXPECT_EQ(*expectedKey, record->header.funcKey);
  EXPECT_GT(record->header.capturedAtMs, 0);
  EXPECT_EQ(18, record->functionExecutions());
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, SelectsBestDuplicate) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 3, 5);
  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(profData, main, 1, 7);

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 1, 7},
  };
  EXPECT_EQ(7, record->functionExecutions());
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, SkipsInvalidEntryRegion) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 1, 5, SBInvOffset{1});

  EXPECT_FALSE(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, SkipsUnusableTranslations) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 1, 5, SBInvOffset{1});
  addProfileTranslation(profData, main, 2, 9);

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 2, 9},
  };
  EXPECT_EQ(9, record->functionExecutions());
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, CaptureContProfProfileStoresFirstRecordOnly) {
  auto const before = snapshotContProfProfileRecords();

  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 2, 7);
  EXPECT_TRUE(captureContProfProfile(profData, *func()));

  addProfileTranslation(profData, main, 1, 11);
  EXPECT_FALSE(captureContProfProfile(profData, *func()));

  auto const records = snapshotContProfProfileRecords();
  ASSERT_EQ(before.size() + 1, records.size());

  auto const expectedKey = makeContProfFuncKey(*func());
  ASSERT_TRUE(expectedKey);

  auto const stored = std::find_if(
      records.begin(), records.end(), [&](auto const &record) {
        return record.header.funcKey == *expectedKey;
      });
  ASSERT_NE(records.end(), stored);

  std::vector<ContProfProfileTranslation> const expected{
      {ContProfStartKind::FuncEntry, 0, 2, 7},
  };
  EXPECT_EQ(7, stored->functionExecutions());
  EXPECT_EQ(expected, stored->translations);
}

}
}
