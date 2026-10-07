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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include <folly/ScopeGuard.h>
#include <gtest/gtest.h>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/prof-data-target-profile.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/target-profile.h"

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

}
}
