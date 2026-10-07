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

#include "hphp/runtime/vm/jit/cprof-target-profile.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <folly/ScopeGuard.h>
#include <gtest/gtest.h>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/coeffect-fun-param-profile.h"
#include "hphp/runtime/vm/jit/cow-profile.h"
#include "hphp/runtime/vm/jit/cprof-serde.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/incref-profile.h"
#include "hphp/runtime/vm/jit/is-type-struct-profile.h"
#include "hphp/runtime/vm/jit/prof-data-target-profile.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-profile.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit::cprof {
namespace {

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

template<class T>
struct ScopedTargetProfile {
  ScopedTargetProfile(
    TransID transId,
    Offset bytecodeOffset,
    const StringData* name,
    const T& value,
    size_t extraSize = 0
  )
    : m_key{
        static_cast<T*>(nullptr),
        transId,
        bytecodeOffset,
        name,
      }
    , m_handle{
        rds::bind<T, rds::Mode::Local>(
          rds::Symbol{m_key},
          extraSize
        ).handle()
      }
    , m_allocationSize{
        static_cast<uint32_t>(sizeof(T) + extraSize)
      } {
    rds::handleToRef<T, rds::Mode::Local>(m_handle) = value;
  }

  ~ScopedTargetProfile() {
    rds::unbind(rds::Symbol{m_key}, m_handle);
  }

  ScopedTargetProfile(const ScopedTargetProfile&) = delete;
  ScopedTargetProfile& operator=(const ScopedTargetProfile&) = delete;

  std::optional<ContProfTargetProfile> snapshot() const {
    return snapshotContProfTargetProfile(
      m_key,
      m_handle,
      m_allocationSize
    );
  }

private:
  rds::Profile m_key;
  rds::Handle m_handle;
  uint32_t m_allocationSize;
};

void expectDecRefProfile(
  const DecRefProfile& expected,
  const DecRefProfile& actual
) {
  EXPECT_EQ(expected.total, actual.total);
  EXPECT_EQ(expected.refcounted, actual.refcounted);
  EXPECT_EQ(expected.released, actual.released);
  EXPECT_EQ(expected.decremented, actual.decremented);
  EXPECT_EQ(
    expected.arrayOfUncountedReleaseCount,
    actual.arrayOfUncountedReleaseCount
  );
  EXPECT_EQ(expected.datatype, actual.datatype);
}

TEST(ContProfTargetProfile, FallsBackToLocalRdsAndPrefersImportedData) {
  auto const previousProfData = jit::profData();

  ProfData isolatedProfData;
  *rl_profData = &isolatedProfData;
  SCOPE_EXIT { *rl_profData = previousProfData; };

  isolatedProfData.setTargetProfile(
    std::make_unique<ProfDataTargetProfile>(/*liveFallback=*/true)
  );
  auto const importedProfiles = isolatedProfData.targetProfile();
  ASSERT_NE(nullptr, importedProfiles);

  constexpr auto transId = TransID{2'100'000};
  constexpr auto bytecodeOffset = Offset{123};
  constexpr auto profileId = int32_t{2'100'001};
  auto const name = decRefProfileKey(profileId);

  auto const localValue = makeDecRefProfileValue(11, 8, 2, 4, 1, KindOfString);
  ScopedTargetProfile<DecRefProfile> const local{
    transId,
    bytecodeOffset,
    name,
    localValue,
  };

  TransIDSet transIds;
  transIds.insert(transId);
  auto const profile = TargetProfile<DecRefProfile>::deserialize(
    transIds,
    TransKind::Optimize,
    bytecodeOffset,
    name,
    0
  );

  ASSERT_TRUE(profile.optimizing());
  expectDecRefProfile(localValue, profile.data());

  auto const importedValue =
    makeDecRefProfileValue(17, 12, 3, 5, 0, KindOfObject);
  ScopedTargetProfile<DecRefProfile> const importedSource{
    TransID{2'100'004},
    bytecodeOffset,
    name,
    importedValue,
  };
  auto const importedSnapshot = importedSource.snapshot();
  ASSERT_TRUE(importedSnapshot);
  auto const prepared = prepareContProfTargetProfile(*importedSnapshot);
  ASSERT_TRUE(prepared);
  ASSERT_TRUE(installContProfTargetProfile(
    *prepared,
    transId,
    *importedProfiles
  ));
  expectDecRefProfile(importedValue, profile.data());

  constexpr auto importedOnlyTransId = TransID{2'100'002};
  constexpr auto importedOnlyProfileId = int32_t{2'100'003};
  auto const importedOnlyName = decRefProfileKey(importedOnlyProfileId);
  auto const importedOnlyValue =
    makeDecRefProfileValue(23, 15, 4, 7, 1, KindOfVec);
  ScopedTargetProfile<DecRefProfile> const importedOnlySource{
    TransID{2'100'005},
    bytecodeOffset,
    importedOnlyName,
    importedOnlyValue,
  };
  auto const importedOnlySnapshot = importedOnlySource.snapshot();
  ASSERT_TRUE(importedOnlySnapshot);
  auto const importedOnlyPrepared =
    prepareContProfTargetProfile(*importedOnlySnapshot);
  ASSERT_TRUE(importedOnlyPrepared);
  ASSERT_TRUE(installContProfTargetProfile(
    *importedOnlyPrepared,
    importedOnlyTransId,
    *importedProfiles
  ));

  TransIDSet importedOnlyTransIds;
  importedOnlyTransIds.insert(importedOnlyTransId);
  auto const importedOnly = TargetProfile<DecRefProfile>::deserialize(
    importedOnlyTransIds,
    TransKind::Optimize,
    bytecodeOffset,
    importedOnlyName,
    0
  );

  ASSERT_TRUE(importedOnly.optimizing());
  expectDecRefProfile(importedOnlyValue, importedOnly.data());

  // An exhaustive import suppresses the still-populated live RDS slot.
  isolatedProfData.setTargetProfile(std::make_unique<ProfDataTargetProfile>());
  EXPECT_FALSE(profile.optimizing());
}

TEST(ContProfTargetProfile, PreservesRecordedNameDuringPreparation) {
  ScopedTargetProfile<DecRefProfile> const source{
    TransID{2'200'000},
    Offset{123},
    decRefProfileKey(2'200'001),
    makeDecRefProfileValue(11, 8, 2, 4, 1, KindOfString),
  };
  auto snapshot = source.snapshot();
  ASSERT_TRUE(snapshot);
  snapshot->name = "DecRefProfile-02200001";

  auto const prepared = prepareContProfTargetProfile(*snapshot);
  ASSERT_TRUE(prepared);
  ASSERT_NE(nullptr, prepared->name);
  EXPECT_EQ(snapshot->name, prepared->name->toCppString());
}

std::optional<ContProfProfileRecord> roundTripTargetProfiles(
  std::vector<ContProfTargetProfile> profiles
) {
  ContProfProfileRecord record{};
  record.header.funcKey.resolutionUnitPath =
    "hphp/runtime/test/cont-prof-target-profile.php";
  record.header.funcKey.bytecodeUnitHash =
    SHA1{"3333333333333333333333333333333333333333"};
  record.header.funcKey.functionName = "cont_prof_target_profile_test";
  record.header.capturedAtMs = 1'700'000'000'000;

  ContProfProfileTranslation translation{};
  translation.regionLength = 1;
  translation.executionCount = 7;
  translation.targetProfiles = std::move(profiles);
  record.translations.push_back(std::move(translation));

  auto const serialized = serializeContProfProfileRecord(record);
  if (!serialized) return std::nullopt;
  return deserializeContProfProfileRecord(
    folly::ByteRange{serialized->data(), serialized->size()}
  );
}

template<class T>
void expectContProfFixedTargetProfile(
  const ContProfTargetProfile& profile,
  ContProfTargetProfileKind kind,
  Offset bytecodeOffset,
  const StringData* name,
  const T& expected,
  TransID transId,
  ProfDataTargetProfile& imported
) {
  EXPECT_EQ(kind, profile.kind);
  EXPECT_EQ(bytecodeOffset, profile.bytecodeOffset);
  EXPECT_EQ(name->toCppString(), profile.name);
  auto const bytes = reinterpret_cast<const uint8_t*>(&expected);
  EXPECT_EQ(
    std::vector<uint8_t>(bytes, bytes + sizeof(T)),
    profile.payload
  );

  auto const prepared = prepareContProfTargetProfile(profile);
  ASSERT_TRUE(prepared);
  ASSERT_TRUE(installContProfTargetProfile(
    *prepared,
    transId,
    imported
  ));

  auto const key = rds::Profile{
    static_cast<T*>(nullptr),
    transId,
    bytecodeOffset,
    name,
  };
  auto const actual = imported.get<T>(key);
  ASSERT_NE(nullptr, actual);
  EXPECT_EQ(expected.toDynamic(), actual->toDynamic());
}

TEST(ContProfTargetProfile, RoundTripsFixedTargetProfileKinds) {
  constexpr auto transId = TransID{2'300'000};

  auto const cowName = makeStaticString("ArrayCOW");
  auto const coeffectName = makeStaticString("CoeffectFunParam");
  // Raw profile names are preserved without a codec-specific allowlist.
  auto const incRefName = makeStaticString("CustomIncRefProfile");
  auto const isTypeStructName = makeStaticString("IsTypeStruct");
  auto const arrayAccessName = makeStaticString("DictAccess");
  auto const clsCnsName = makeStaticString("ClsCnsProfile");

  COWProfile const cow{};
  CoeffectFunParamProfile const coeffect{};
  IncRefProfile const incRef{17, 12, 5};
  IsTypeStructProfile const isTypeStruct{};
  ArrayAccessProfile const arrayAccess{};
  ClsCnsProfile const clsCns{};

  ScopedTargetProfile<COWProfile> const cowSite{
    transId, 0, cowName, cow
  };
  ScopedTargetProfile<CoeffectFunParamProfile> const coeffectSite{
    transId, 0, coeffectName, coeffect
  };
  ScopedTargetProfile<IncRefProfile> const incRefSite{
    transId, 0, incRefName, incRef
  };
  ScopedTargetProfile<IsTypeStructProfile> const isTypeStructSite{
    transId, 0, isTypeStructName, isTypeStruct
  };
  ScopedTargetProfile<ArrayAccessProfile> const arrayAccessSite{
    transId, 0, arrayAccessName, arrayAccess
  };
  ScopedTargetProfile<ClsCnsProfile> const clsCnsSite{
    transId, 0, clsCnsName, clsCns
  };

  auto cowSnapshot = cowSite.snapshot();
  auto coeffectSnapshot = coeffectSite.snapshot();
  auto incRefSnapshot = incRefSite.snapshot();
  auto isTypeStructSnapshot = isTypeStructSite.snapshot();
  auto arrayAccessSnapshot = arrayAccessSite.snapshot();
  auto clsCnsSnapshot = clsCnsSite.snapshot();
  ASSERT_TRUE(cowSnapshot);
  ASSERT_TRUE(coeffectSnapshot);
  ASSERT_TRUE(incRefSnapshot);
  ASSERT_TRUE(isTypeStructSnapshot);
  ASSERT_TRUE(arrayAccessSnapshot);
  ASSERT_TRUE(clsCnsSnapshot);

  std::vector<ContProfTargetProfile> capturedProfiles{
    std::move(*cowSnapshot),
    std::move(*coeffectSnapshot),
    std::move(*incRefSnapshot),
    std::move(*isTypeStructSnapshot),
    std::move(*arrayAccessSnapshot),
    std::move(*clsCnsSnapshot),
  };
  std::sort(
    capturedProfiles.begin(),
    capturedProfiles.end(),
    contProfTargetProfileKeyLess
  );

  auto const decoded = roundTripTargetProfiles(std::move(capturedProfiles));
  ASSERT_TRUE(decoded);
  ASSERT_EQ(1, decoded->translations.size());

  auto const& profiles = decoded->translations[0].targetProfiles;
  ASSERT_EQ(6, profiles.size());

  ProfDataTargetProfile imported;
  expectContProfFixedTargetProfile(
    profiles[0],
    ContProfTargetProfileKind::COW,
    0,
    cowName,
    cow,
    transId,
    imported
  );
  expectContProfFixedTargetProfile(
    profiles[1],
    ContProfTargetProfileKind::CoeffectFunParam,
    0,
    coeffectName,
    coeffect,
    transId,
    imported
  );
  expectContProfFixedTargetProfile(
    profiles[2],
    ContProfTargetProfileKind::IncRef,
    0,
    incRefName,
    incRef,
    transId,
    imported
  );
  expectContProfFixedTargetProfile(
    profiles[3],
    ContProfTargetProfileKind::IsTypeStruct,
    0,
    isTypeStructName,
    isTypeStruct,
    transId,
    imported
  );
  expectContProfFixedTargetProfile(
    profiles[4],
    ContProfTargetProfileKind::ArrayAccess,
    0,
    arrayAccessName,
    arrayAccess,
    transId,
    imported
  );
  expectContProfFixedTargetProfile(
    profiles[5],
    ContProfTargetProfileKind::ClsCns,
    0,
    clsCnsName,
    clsCns,
    transId,
    imported
  );
}

TEST(ContProfTargetProfile, RoundTripsTypeBearingTargetProfileKinds) {
  constexpr auto transId = TransID{2'500'000};
  auto const typeName = makeStaticString("CustomTypeProfile");
  auto const arrayIterName = makeStaticString("CustomArrayIterProfile");

  TypeProfile typeProfile{};
  typeProfile.type = Type::cns(makeStaticString("cprof value"));
  typeProfile.count = 17;

  auto const keyTypes = ArrayKeyTypes::Ints() | ArrayKeyTypes::StaticStrs();
  auto const arrayIterProfile = ArrayIterProfile{
    .m_key_types = keyTypes,
    .m_value_type = Type::cns(int64_t{7}),
  };

  ScopedTargetProfile<TypeProfile> const typeSite{
    transId, 0, typeName, typeProfile
  };
  ScopedTargetProfile<ArrayIterProfile> const arrayIterSite{
    transId, 0, arrayIterName, arrayIterProfile
  };

  auto typeSnapshot = typeSite.snapshot();
  auto arrayIterSnapshot = arrayIterSite.snapshot();
  ASSERT_TRUE(typeSnapshot);
  ASSERT_TRUE(arrayIterSnapshot);

  std::vector<ContProfTargetProfile> capturedProfiles{
    std::move(*typeSnapshot),
    std::move(*arrayIterSnapshot),
  };
  std::sort(
    capturedProfiles.begin(),
    capturedProfiles.end(),
    contProfTargetProfileKeyLess
  );

  auto const decoded = roundTripTargetProfiles(std::move(capturedProfiles));
  ASSERT_TRUE(decoded);
  ASSERT_EQ(1, decoded->translations.size());

  auto const& profiles = decoded->translations[0].targetProfiles;
  ASSERT_EQ(2, profiles.size());
  EXPECT_EQ(ContProfTargetProfileKind::Type, profiles[0].kind);
  EXPECT_EQ(typeName->toCppString(), profiles[0].name);
  EXPECT_EQ(ContProfTargetProfileKind::ArrayIter, profiles[1].kind);
  EXPECT_EQ(arrayIterName->toCppString(), profiles[1].name);

  ProfDataTargetProfile imported;
  for (auto const& profile : profiles) {
    auto const prepared = prepareContProfTargetProfile(profile);
    ASSERT_TRUE(prepared);
    ASSERT_TRUE(installContProfTargetProfile(*prepared, transId, imported));
  }

  auto const importedType = imported.get<TypeProfile>(rds::Profile{
    static_cast<TypeProfile*>(nullptr),
    transId,
    0,
    typeName,
  });
  ASSERT_NE(nullptr, importedType);
  EXPECT_EQ(17, importedType->count);
  EXPECT_TRUE(importedType->type == TStaticStr);

  auto const importedArrayIter =
    imported.get<ArrayIterProfile>(rds::Profile{
      static_cast<ArrayIterProfile*>(nullptr),
      transId,
      0,
      arrayIterName,
    });
  ASSERT_NE(nullptr, importedArrayIter);
  auto const importedResult = importedArrayIter->result();
  EXPECT_TRUE(importedResult.key_types == keyTypes);
  EXPECT_TRUE(importedResult.value_type == TInt);
}

TEST(ContProfTargetProfile, RejectsInvalidTypeBearingTargetProfiles) {
  constexpr auto transId = TransID{2'500'000};
  auto const typeName = makeStaticString("TypeProfile");
  auto const arrayIterName = makeStaticString("ArrayIterProfile");
  ScopedTargetProfile<TypeProfile> const typeSite{
    transId, 0, typeName, TypeProfile{}
  };
  ScopedTargetProfile<ArrayIterProfile> const arrayIterSite{
    transId, 0, arrayIterName, ArrayIterProfile{}
  };
  auto const typeSnapshot = typeSite.snapshot();
  auto const arrayIterSnapshot = arrayIterSite.snapshot();
  ASSERT_TRUE(typeSnapshot);
  ASSERT_TRUE(arrayIterSnapshot);

  for (auto const& profile : {*typeSnapshot, *arrayIterSnapshot}) {
    ASSERT_TRUE(prepareContProfTargetProfile(profile));

    auto truncated = profile;
    truncated.payload.pop_back();
    EXPECT_FALSE(prepareContProfTargetProfile(truncated));

    auto trailing = profile;
    trailing.payload.push_back(0);
    EXPECT_FALSE(prepareContProfTargetProfile(trailing));
  }

  auto invalidType = *typeSnapshot;
  std::fill_n(
    invalidType.payload.begin(), sizeof(Type::bits_t), uint8_t{0xff}
  );
  EXPECT_FALSE(prepareContProfTargetProfile(invalidType));

  auto invalidArrayIter = *arrayIterSnapshot;
  invalidArrayIter.payload[0] = 0x10;
  EXPECT_FALSE(prepareContProfTargetProfile(invalidArrayIter));

  TypeProfile nonCellTypeProfile{};
  nonCellTypeProfile.type = Type{Type::kMem, PtrLocation::Frame};
  ScopedTargetProfile<TypeProfile> const nonCellTypeSite{
    TransID{2'500'001}, 0, typeName, nonCellTypeProfile
  };
  auto nonCellType = nonCellTypeSite.snapshot();
  ASSERT_TRUE(nonCellType);
  EXPECT_FALSE(prepareContProfTargetProfile(*nonCellType));

  auto const uninitArrayIterProfile = ArrayIterProfile{
    .m_key_types = ArrayKeyTypes::Ints(),
    .m_value_type = TUninit,
  };
  ScopedTargetProfile<ArrayIterProfile> const uninitArrayIterSite{
    TransID{2'500'002}, 0, arrayIterName, uninitArrayIterProfile
  };
  auto uninitArrayIter = uninitArrayIterSite.snapshot();
  ASSERT_TRUE(uninitArrayIter);
  EXPECT_FALSE(prepareContProfTargetProfile(*uninitArrayIter));
}

TEST(ContProfTargetProfile, RoundTripsSwitchTargetProfile) {
  constexpr auto transId = TransID{2'400'000};
  for (auto const& counts : {
    std::vector<uint32_t>{17},
    std::vector<uint32_t>{17, 9, 3},
  }) {
    SCOPED_TRACE(counts.size());
    auto const key = rds::Profile{
      static_cast<SwitchProfile*>(nullptr),
      transId,
      0,
      makeStaticString("SwitchProfile"),
    };
    auto const extraSize = SwitchProfile::extraSize(
      static_cast<int>(counts.size())
    );
    auto const allocationSize =
      static_cast<uint32_t>(sizeof(SwitchProfile) + extraSize);
    auto const handle = rds::bind<SwitchProfile, rds::Mode::Local>(
      rds::Symbol{key}, extraSize
    ).handle();
    SCOPE_EXIT { rds::unbind(rds::Symbol{key}, handle); };
    auto& local = rds::handleToRef<SwitchProfile, rds::Mode::Local>(handle);
    std::copy(counts.begin(), counts.end(), local.cases());

    auto snapshot = snapshotContProfTargetProfile(key, handle, allocationSize);
    ASSERT_TRUE(snapshot);
    auto const decoded = roundTripTargetProfiles({std::move(*snapshot)});
    ASSERT_TRUE(decoded);
    ASSERT_EQ(1, decoded->translations.size());
    ASSERT_EQ(1, decoded->translations[0].targetProfiles.size());

    auto const& profile = decoded->translations[0].targetProfiles[0];
    EXPECT_EQ(ContProfTargetProfileKind::Switch, profile.kind);
    EXPECT_EQ(0, profile.bytecodeOffset);
    EXPECT_EQ("SwitchProfile", profile.name);
    ASSERT_EQ(allocationSize, profile.payload.size());

    auto const prepared = prepareContProfTargetProfile(profile);
    ASSERT_TRUE(prepared);
    ProfDataTargetProfile imported;
    ASSERT_TRUE(installContProfTargetProfile(*prepared, transId, imported));
    auto const actual = imported.get<SwitchProfile>(key);
    ASSERT_NE(nullptr, actual);
    auto const restored = std::vector<uint32_t>{
      actual->cases(), actual->cases() + counts.size()
    };
    EXPECT_EQ(counts, restored);
  }
}

TEST(ContProfTargetProfile, RejectsInvalidSwitchPayloadSizes) {
  ContProfTargetProfile profile{
    ContProfTargetProfileKind::Switch,
    0,
    "SwitchProfile",
    {},
  };
  EXPECT_FALSE(prepareContProfTargetProfile(profile));

  profile.payload.resize(sizeof(uint32_t) + 1);
  EXPECT_FALSE(prepareContProfTargetProfile(profile));
}

TEST(ContProfTargetProfile, RejectsInvalidRawTargetProfiles) {
  constexpr auto transId = TransID{2'500'000};

  ScopedTargetProfile<COWProfile> const source{
    transId,
    0,
    makeStaticString("ArrayCOW"),
    COWProfile{},
  };
  ScopedTargetProfile<COWProfile> const negativeOffset{
    transId,
    Offset{-1},
    makeStaticString("ArrayCOW"),
    COWProfile{},
  };
  EXPECT_FALSE(negativeOffset.snapshot());

  auto const snapshot = source.snapshot();
  ASSERT_TRUE(snapshot);

  auto invalid = *snapshot;
  invalid.bytecodeOffset = -1;
  EXPECT_FALSE(prepareContProfTargetProfile(invalid));

  invalid = *snapshot;
  invalid.payload.pop_back();
  EXPECT_FALSE(prepareContProfTargetProfile(invalid));
}

}
}
