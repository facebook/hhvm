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

#include "hphp/runtime/vm/jit/cprof-capture.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/cprof-controller.h"
#include "hphp/runtime/vm/jit/cprof-target-profile.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/assertions.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit::cprof {
namespace {

constexpr auto kUnitPath = "hphp/runtime/test/cont-prof-capture-test.php";

constexpr auto kHhas = R"HHAS(
.function N cont_prof_capture_test_81d926f4(named N $x = DV) {
  .declvars $y $z;
main:
  Null
  PopC
  Null
  RetC None
DV:
  Null
  PopL $x
  Enter main
}

.function N cont_prof_capture_required_test_81d926f4(N $x) {
main:
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

TransID addProfileTranslation(
  ProfData& profData,
  SrcKey start,
  int regionLength,
  int64_t executionCount,
  SBInvOffset spOffset = SBInvOffset{0},
  GuardedLocations preconditions = {},
  RegionDesc::BlockIdSet incoming = {},
  PostConditions postConditions = {}
) {
  auto region = std::make_shared<RegionDesc>();
  auto const block = region->addBlock(start, regionLength, spOffset);
  for (auto const& precondition : preconditions) {
    block->addPreCondition(precondition);
  }

  auto const transId = profData.allocTransID();
  region->incoming(std::move(incoming));
  profData.addTransProfile(transId, region, postConditions, 0);

  always_assert(executionCount >= 0);
  always_assert(executionCount <= profData.counterDefault());
  *profData.transCounterAddr(transId) =
    profData.counterDefault() - executionCount;
  return transId;
}

DecRefProfile makeDecRefProfileValue(
  uint32_t total,
  uint32_t refcounted,
  uint32_t released,
  uint32_t decremented,
  uint32_t arrayOfUncountedReleaseCount,
  DataType datatype
) {
  return {
    total,
    refcounted,
    released,
    decremented,
    arrayOfUncountedReleaseCount,
    datatype,
  };
}

struct ScopedDecRefProfile {
  ScopedDecRefProfile(
    TransID transId,
    Offset bytecodeOffset,
    const StringData* name,
    const DecRefProfile& value
  )
    : m_key{
        static_cast<DecRefProfile*>(nullptr),
        transId,
        bytecodeOffset,
        name,
      }
    , m_handle{
        rds::bind<DecRefProfile, rds::Mode::Local>(
          rds::Symbol{m_key}
        ).handle()
      } {
    set(value);
  }

  ~ScopedDecRefProfile() {
    rds::unbind(rds::Symbol{m_key}, m_handle);
  }

  ScopedDecRefProfile(const ScopedDecRefProfile&) = delete;
  ScopedDecRefProfile& operator=(const ScopedDecRefProfile&) = delete;

  void set(const DecRefProfile& value) {
    rds::handleToRef<DecRefProfile, rds::Mode::Local>(m_handle) = value;
  }

private:
  rds::Profile m_key;
  rds::Handle m_handle;
};

template<class T>
void appendTargetProfilePayload(std::vector<uint8_t>& payload, T value) {
  static_assert(std::is_unsigned_v<T>);

  auto const bytes = reinterpret_cast<const uint8_t*>(&value);
  payload.insert(payload.end(), bytes, bytes + sizeof(value));
}

ContProfTargetProfile makeContProfDecRefTargetProfile(
  Offset bytecodeOffset,
  int32_t profileId,
  const DecRefProfile& value
) {
  std::vector<uint8_t> payload;
  appendTargetProfilePayload(payload, static_cast<uint32_t>(value.total));
  appendTargetProfilePayload(
    payload,
    static_cast<uint32_t>(value.refcounted)
  );
  appendTargetProfilePayload(payload, static_cast<uint32_t>(value.released));
  appendTargetProfilePayload(
    payload,
    static_cast<uint32_t>(value.decremented)
  );
  appendTargetProfilePayload(
    payload,
    static_cast<uint32_t>(value.arrayOfUncountedReleaseCount)
  );
  appendTargetProfilePayload(payload, static_cast<uint8_t>(value.datatype));

  return {
    ContProfTargetProfileKind::DecRef,
    bytecodeOffset,
    decRefProfileKey(profileId)->toCppString(),
    std::move(payload),
  };
}

struct ContProfCaptureTest : testing::Test {
  static void SetUpTestSuite() {
    s_unit = makeTestUnit();
    ASSERT_NE(nullptr, s_unit);
    ASSERT_EQ(2, s_unit->funcs().size());

    s_func = s_unit->funcs()[0];
    s_requiredFunc = s_unit->funcs()[1];
    ASSERT_EQ(3, s_func->numLocals());
    ASSERT_EQ(1, s_func->numFuncEntryInputs());
    ASSERT_EQ(1, s_requiredFunc->numRequiredPositionalParams());
    s_midOffset = instrLen(s_func->at(0));
    ASSERT_GT(s_midOffset, 0);
    ASSERT_TRUE(s_func->contains(s_midOffset));
    ASSERT_FALSE(s_func->isEntry(s_midOffset));
    s_emptyMidOffset = s_midOffset + instrLen(s_func->at(s_midOffset));
    ASSERT_TRUE(s_func->contains(s_emptyMidOffset));
    ASSERT_FALSE(s_func->isEntry(s_emptyMidOffset));
    s_unit->merge();
  }

  static void TearDownTestSuite() {
    s_func = nullptr;
    s_requiredFunc = nullptr;
    s_unit.reset();
  }

  static Func* func() { return s_func; }
  static Func* requiredFunc() { return s_requiredFunc; }

  static SrcKey mid() { return SrcKey{func(), s_midOffset, ResumeMode::None}; }

  static SrcKey emptyMid() {
    return SrcKey{func(), s_emptyMidOffset, ResumeMode::None};
  }

private:
  static inline TestUnit s_unit;
  static inline Func* s_func{nullptr};
  static inline Func* s_requiredFunc{nullptr};
  static inline Offset s_midOffset{0};
  static inline Offset s_emptyMidOffset{0};
};

void expectEntryOnlyRecord(
  const std::optional<ContProfProfileRecord>& record
) {
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 2, 7},
  };
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, PreservesIncomingAcrossProfileRenumbering) {
  ProfData profData;
  profData.resetCounters(100);

  auto const transId = profData.allocTransID();
  auto const laterTransId = profData.allocTransID();
  auto region = std::make_shared<RegionDesc>();
  auto const oldId = region->addBlock(emptyMid(), 1, SBInvOffset{0})->id();
  region->incoming({transId, laterTransId});

  profData.addTransProfile(transId, region, PostConditions{}, 0);

  auto const record = profData.transRec(transId);
  ASSERT_NE(nullptr, record);
  auto const& published = *record->region();
  ASSERT_EQ(1, published.blocks().size());
  EXPECT_EQ(transId, published.entry()->id());
  EXPECT_TRUE(published.hasBlock(transId));
  EXPECT_FALSE(published.hasBlock(oldId));
  EXPECT_FALSE(published.hasBlock(laterTransId));
  RegionDesc::BlockIdSet const expectedIncoming{transId, laterTransId};
  ASSERT_NE(nullptr, published.incoming());
  EXPECT_EQ(expectedIncoming, *published.incoming());
  EXPECT_TRUE(published.preds(transId).empty());
  EXPECT_TRUE(published.succs(transId).empty());
}

TEST_F(ContProfCaptureTest, CapturesNonzeroTranslations) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  auto const named = SrcKey{func(), 0, true, SrcKey::FuncEntryTag{}};

  addProfileTranslation(profData, named, 1, 11);
  auto const mainTransId = addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    13,
    SBInvOffset{0},
    {},
    {mainTransId}
  );
  addProfileTranslation(profData, main, 3, 0);

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  auto const expectedKey = makeContProfFuncKey(*func());
  ASSERT_TRUE(expectedKey);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 2, 7},
    {ContProfStartKind::NamedParamsFuncEntry, 0, 1, 11},
    {
      ContProfStartKind::Bytecode,
      static_cast<uint32_t>(emptyMid().offset()),
      1,
      13,
      {},
      {0},
    },
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

TEST_F(ContProfCaptureTest, SelectsBestBytecodeDuplicate) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(profData, emptyMid(), 3, 5);
  addProfileTranslation(profData, emptyMid(), 2, 7);
  addProfileTranslation(profData, emptyMid(), 1, 7);

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 2, 7},
    {
      ContProfStartKind::Bytecode,
      static_cast<uint32_t>(emptyMid().offset()),
      1,
      7,
    },
  };
  EXPECT_EQ(7, record->functionExecutions());
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, RequiresEntryTranslation) {
  ProfData profData;
  profData.resetCounters(100);

  addProfileTranslation(profData, emptyMid(), 1, 7);

  EXPECT_FALSE(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, SkipsBytecodeWithNonzeroStackOffset) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(profData, mid(), 1, 5, SBInvOffset{1});
  addProfileTranslation(profData, mid(), 1, 11, SBInvOffset{-1});

  expectEntryOnlyRecord(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, SkipsBytecodeWithNonLocalTypeGuards) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    5,
    SBInvOffset{0},
    {{Location::Stack{SBInvOffset{0}}, TObj, DataTypeSpecific}}
  );
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    11,
    SBInvOffset{0},
    {{Location::MBase{0}, TObj, DataTypeSpecific}}
  );

  expectEntryOnlyRecord(snapshotContProfProfileRecord(profData, *func()));
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

TEST_F(ContProfCaptureTest, CapturesCanonicalLocalTypeGuards) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {Location::Local{1}, TStaticStr, DataTypeSpecific},
    {
      Location::Local{0},
      Type::cns(int64_t{42}),
      DataTypeSpecific,
    },
    {Location::Local{0}, TInt, DataTypeSpecific},
    {Location::Local{1}, TCell, DataTypeGeneric},
  };

  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    11,
    SBInvOffset{0},
    preconditions
  );

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 2, 7},
    {
      ContProfStartKind::Bytecode,
      static_cast<uint32_t>(emptyMid().offset()),
      1,
      11,
      {
        {0, KindOfInt64},
        {1, KindOfPersistentString},
      },
    },
  };
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, SkipsNonPrimitiveLocalTypeGuard) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {Location::Local{0}, TInt | TStr, DataTypeGeneric},
  };

  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    11,
    SBInvOffset{0},
    preconditions
  );

  expectEntryOnlyRecord(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, KeepsEntryWithNonPrimitiveLocalTypeGuard) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {Location::Local{0}, TInt | TStr, DataTypeGeneric},
  };

  addProfileTranslation(
    profData,
    main,
    2,
    7,
    SBInvOffset{0},
    preconditions
  );

  expectEntryOnlyRecord(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, CapturesEntryLocalTypeGuard) {
  ProfData profData;
  profData.resetCounters(100);

  auto const entry = SrcKey{
    func(),
    func()->numPositionalParams(),
    false,
    SrcKey::FuncEntryTag{},
  };
  GuardedLocations const preconditions{
    {Location::Local{0}, TInt, DataTypeSpecific},
  };

  addProfileTranslation(
    profData,
    entry,
    2,
    7,
    SBInvOffset{0},
    preconditions
  );

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {
      ContProfStartKind::FuncEntry,
      func()->numPositionalParams(),
      2,
      7,
      {{0, KindOfInt64}},
    },
  };
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, SkipsEntryGuardOutsideEntryInputs) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {Location::Local{func()->numFuncEntryInputs()}, TInt, DataTypeSpecific},
  };

  addProfileTranslation(
    profData,
    main,
    2,
    7,
    SBInvOffset{0},
    preconditions
  );

  EXPECT_FALSE(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, SkipsGuardedEntryBelowRequiredArguments) {
  ProfData profData;
  profData.resetCounters(100);

  auto const entry = SrcKey{requiredFunc(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {Location::Local{0}, TInt, DataTypeSpecific},
  };

  addProfileTranslation(profData, entry, 1, 5);
  addProfileTranslation(
    profData,
    entry,
    1,
    7,
    SBInvOffset{0},
    preconditions
  );

  auto const record = snapshotContProfProfileRecord(profData, *requiredFunc());
  ASSERT_TRUE(record);

  std::vector<ContProfProfileTranslation> const expected{
    {ContProfStartKind::FuncEntry, 0, 1, 5},
  };
  EXPECT_EQ(expected, record->translations);
}

TEST_F(ContProfCaptureTest, SkipsConflictingLocalTypeGuards) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {Location::Local{0}, TInt, DataTypeSpecific},
    {Location::Local{0}, TStr, DataTypeSpecific},
  };

  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    11,
    SBInvOffset{0},
    preconditions
  );

  expectEntryOnlyRecord(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, SkipsOutOfRangeLocalTypeGuard) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  GuardedLocations const preconditions{
    {
      Location::Local{
        static_cast<uint32_t>(func()->numLocals())
      },
      TInt,
      DataTypeSpecific,
    },
  };

  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    11,
    SBInvOffset{0},
    preconditions
  );

  expectEntryOnlyRecord(snapshotContProfProfileRecord(profData, *func()));
}

TEST_F(ContProfCaptureTest, CapturesLocalPostConditions) {
  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  PostConditions postConditions{};
  postConditions.changed = {
    {Location::Local{0}, TInt},
    {Location::Local{1}, TInt | TStr},
  };
  postConditions.refined = {
    {Location::Local{2}, TStaticStr},
  };

  addProfileTranslation(profData, main, 2, 7);
  addProfileTranslation(
    profData,
    emptyMid(),
    1,
    11,
    SBInvOffset{0},
    {},
    {},
    postConditions
  );

  auto const record = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(record);

  std::vector<ContProfLocalPostCondition> const expected{
    {0, true, KindOfInt64},
    {1, true, kInvalidDataType},
    {2, false, KindOfPersistentString},
  };
  ASSERT_EQ(2, record->translations.size());
  EXPECT_EQ(expected, record->translations.back().localPostConditions);
}

TEST_F(ContProfCaptureTest, StoresFirstRecordAndFinalizesTargetProfilesOnce) {
  auto const before = snapshotContProfProfileRecords();

  ProfData profData;
  profData.resetCounters(100);

  auto const main = SrcKey{func(), 0, false, SrcKey::FuncEntryTag{}};
  auto const discardedMainTransId = addProfileTranslation(profData, main, 3, 5);
  auto const selectedMainTransId = addProfileTranslation(profData, main, 2, 7);
  auto const midTransId = addProfileTranslation(
    profData,
    emptyMid(),
    1,
    13,
    SBInvOffset{0}
  );

  auto const selectedDefaultValue =
    makeDecRefProfileValue(9, 6, 2, 3, 1, KindOfString);
  ScopedDecRefProfile selectedDefault{
    selectedMainTransId,
    0,
    decRefProfileKey(-1),
    selectedDefaultValue,
  };

  auto const direct = snapshotContProfProfileRecord(profData, *func());
  ASSERT_TRUE(direct);
  ASSERT_EQ(2, direct->translations.size());
  EXPECT_TRUE(direct->translations[0].targetProfiles.empty());
  EXPECT_TRUE(direct->translations[1].targetProfiles.empty());

  EXPECT_TRUE(captureContProfProfile(profData, *func()));

  auto const laterMainTransId = addProfileTranslation(profData, main, 1, 11);
  EXPECT_FALSE(captureContProfProfile(profData, *func()));

  auto const selectedNamedValue =
    makeDecRefProfileValue(17, 12, 3, 5, 1, KindOfObject);
  ScopedDecRefProfile const selectedNamed{
    selectedMainTransId,
    0,
    decRefProfileKey(2'000'000),
    selectedNamedValue,
  };
  auto const selectedMidValue =
    makeDecRefProfileValue(11, 8, 2, 4, 0, kInvalidDataType);
  ScopedDecRefProfile const selectedMid{
    midTransId,
    emptyMid().offset(),
    decRefProfileKey(2'000'001),
    selectedMidValue,
  };
  ScopedDecRefProfile const discarded{
    discardedMainTransId,
    0,
    decRefProfileKey(2'000'002),
    makeDecRefProfileValue(23, 15, 4, 7, 1, KindOfVec),
  };
  ScopedDecRefProfile const later{
    laterMainTransId,
    0,
    decRefProfileKey(2'000'003),
    makeDecRefProfileValue(29, 19, 5, 8, 2, KindOfDict),
  };
  ScopedDecRefProfile const negativeOffset{
    selectedMainTransId,
    Offset{-1},
    decRefProfileKey(2'000'011),
    makeDecRefProfileValue(7, 4, 1, 2, 0, KindOfString),
  };
  ScopedDecRefProfile const outsideFunction{
    selectedMainTransId,
    func()->bclen(),
    decRefProfileKey(2'000'012),
    makeDecRefProfileValue(7, 4, 1, 2, 0, KindOfString),
  };
  ScopedDecRefProfile const zero{
    selectedMainTransId,
    0,
    decRefProfileKey(2'000'014),
    makeDecRefProfileValue(0, 0, 0, 0, 0, kExtraInvalidDataType),
  };

  auto const requiredTransId = addProfileTranslation(
    profData,
    SrcKey{requiredFunc(), 1, false, SrcKey::FuncEntryTag{}},
    2,
    5
  );
  EXPECT_TRUE(captureContProfProfile(profData, *requiredFunc()));

  auto const records = snapshotContProfProfileRecords();
  ASSERT_EQ(before.size() + 2, records.size());

  auto const expectedKey = makeContProfFuncKey(*func());
  ASSERT_TRUE(expectedKey);

  auto const stored = std::find_if(
    records.begin(),
    records.end(),
    [&](auto const& record) {
      return record.header.funcKey == *expectedKey;
    }
  );
  ASSERT_NE(records.end(), stored);

  EXPECT_EQ(7, stored->functionExecutions());
  ASSERT_EQ(2, stored->translations.size());

  auto const& entry = stored->translations[0];
  EXPECT_EQ(ContProfStartKind::FuncEntry, entry.startKind);
  EXPECT_EQ(2, entry.regionLength);
  EXPECT_EQ(7, entry.executionCount);
  ASSERT_EQ(2, entry.targetProfiles.size());
  EXPECT_EQ(
    makeContProfDecRefTargetProfile(0, -1, selectedDefaultValue),
    entry.targetProfiles[0]
  );
  EXPECT_EQ(
    makeContProfDecRefTargetProfile(0, 2'000'000, selectedNamedValue),
    entry.targetProfiles[1]
  );

  auto const& bytecode = stored->translations[1];
  ASSERT_EQ(ContProfStartKind::Bytecode, bytecode.startKind);
  EXPECT_EQ(emptyMid().offset(), bytecode.offset());
  EXPECT_EQ(13, bytecode.executionCount);
  ASSERT_EQ(1, bytecode.targetProfiles.size());
  EXPECT_EQ(
    makeContProfDecRefTargetProfile(
      emptyMid().offset(),
      2'000'001,
      selectedMidValue
    ),
    bytecode.targetProfiles[0]
  );

  selectedDefault.set(
    makeDecRefProfileValue(31, 21, 6, 9, 2, KindOfVec)
  );

  auto const requiredKey = makeContProfFuncKey(*requiredFunc());
  ASSERT_TRUE(requiredKey);
  auto const required = std::find_if(
    records.begin(),
    records.end(),
    [&](auto const& record) {
      return record.header.funcKey == *requiredKey;
    }
  );
  ASSERT_NE(records.end(), required);
  ASSERT_EQ(1, required->translations.size());
  EXPECT_TRUE(required->translations[0].targetProfiles.empty());

  // Even a sweep with no target profiles finalizes the record.
  ScopedDecRefProfile const afterFinalization{
    requiredTransId,
    0,
    decRefProfileKey(-1),
    selectedDefaultValue,
  };
  EXPECT_EQ(records, snapshotContProfProfileRecords());
}

}
}
