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

#include "hphp/runtime/vm/jit/cprof-consume.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <folly/ScopeGuard.h>
#include <gtest/gtest.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/cprof-key.h"
#include "hphp/runtime/vm/jit/cprof-record.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/configs/repo.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/sha1.h"

namespace HPHP::jit::cprof {
namespace {

constexpr auto kUnitPath = "hphp/runtime/test/cont-prof-consume-test.php";
constexpr auto kHhas = R"HHAS(
.function N cont_prof_consume_alpha_52d9c47b(N $x) {
  .declvars $local;
  Null
  PopC
  Null
  RetC None
}
.function N cont_prof_consume_beta_52d9c47b() {
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

struct ContProfConsumeTest : testing::Test {
  void SetUp() override {
    auto const emitter = assemble_string(
      kHhas,
      kUnitPath,
      SHA1{"3333333333333333333333333333333333333333"},
      nullptr,
      RepoOptions::defaults().packageInfo(),
      UnitEmitterAttributes::defaults(),
      false
    );
    ASSERT_NE(nullptr, emitter);
    ASSERT_FALSE(emitter->m_fatalUnit);
    m_unit.reset(emitter->create().release());
    ASSERT_NE(nullptr, m_unit);
    ASSERT_EQ(2, m_unit->funcs().size());
    m_unit->merge();

    m_alphaFunc = m_unit->funcs()[0];
    m_betaFunc = m_unit->funcs()[1];
    auto const alphaKey = makeContProfFuncKey(*m_alphaFunc);
    auto const betaKey = makeContProfFuncKey(*m_betaFunc);
    ASSERT_TRUE(alphaKey);
    ASSERT_TRUE(betaKey);
    m_alpha.header = {*alphaKey, 1};
    m_alpha.translations = {{ContProfStartKind::FuncEntry, 1, 2, 7}};
    m_beta.header = {*betaKey, 1};
    m_beta.translations = {{ContProfStartKind::FuncEntry, 0, 2, 13}};
    m_units.emplace(alphaKey->resolutionUnitPath, m_unit.get());
    m_units.emplace(betaKey->resolutionUnitPath, m_unit.get());
  }

  std::unique_ptr<Unit, DestroyTestUnit> m_unit;
  Func* m_alphaFunc{nullptr};
  Func* m_betaFunc{nullptr};
  ContProfProfileRecord m_alpha;
  ContProfProfileRecord m_beta;
  std::map<std::string, Unit*> m_units;
};

TEST_F(ContProfConsumeTest, StaleHotterRecordDoesNotConsumeSlot) {
  auto stale = m_alpha;
  stale.header.funcKey.bytecodeUnitHash = SHA1{uint64_t{2}};
  stale.translations.front().executionCount = 100;

  auto const prepared = prepareContProfStartupCandidates(
    {stale, m_alpha}, m_units, 1
  );

  ASSERT_EQ(1, prepared.candidates.size());
  auto const& candidate = prepared.candidates.front();
  EXPECT_EQ(m_alphaFunc, candidate.func);
  ASSERT_EQ(1, candidate.translations.size());
  EXPECT_EQ(7, candidate.translations.front().executionCount);
}

TEST_F(ContProfConsumeTest, PreparesBytecodeTranslation) {
  auto const popOffset = instrLen(m_alphaFunc->at(0));
  auto const midOffset = popOffset + instrLen(m_alphaFunc->at(popOffset));
  auto const midStart = SrcKey{m_alphaFunc, midOffset, ResumeMode::None};
  m_alpha.translations.front().localTypeGuards = {{0, KindOfInt64}};
  m_alpha.translations.push_back({
    ContProfStartKind::Bytecode,
    static_cast<uint32_t>(midOffset),
    2,
    11,
    {{1, KindOfString}},
  });

  auto const prepared = prepareContProfStartupCandidates(
    {m_alpha}, m_units, 1
  );

  ASSERT_EQ(1, prepared.candidates.size());
  auto const& candidate = prepared.candidates.front();
  EXPECT_EQ(m_alphaFunc, candidate.func);
  ASSERT_EQ(2, candidate.translations.size());

  auto const& entry = candidate.translations[0];
  EXPECT_TRUE(entry.start.funcEntry());
  EXPECT_EQ(7, entry.executionCount);
  ASSERT_NE(nullptr, entry.region);
  ASSERT_EQ(1, entry.region->blocks().size());
  GuardedLocations const expectedEntryGuards{
    {Location::Local{0}, TInt, DataTypeSpecific},
  };
  EXPECT_EQ(expectedEntryGuards, entry.region->entry()->typePreConditions());

  auto const& translation = candidate.translations[1];
  EXPECT_EQ(midStart, translation.start);
  EXPECT_FALSE(translation.start.anyFuncEntry());
  EXPECT_EQ(11, translation.executionCount);
  ASSERT_NE(nullptr, translation.region);
  ASSERT_EQ(1, translation.region->blocks().size());
  EXPECT_EQ(midStart, translation.region->start());
  auto const block = translation.region->entry();
  EXPECT_EQ(2, block->length());
  EXPECT_EQ(SBInvOffset{0}, block->initialSpOffset());
  GuardedLocations const expectedBytecodeGuards{
    {Location::Local{1}, TStr, DataTypeSpecific},
  };
  EXPECT_EQ(expectedBytecodeGuards, block->typePreConditions());
}

TEST_F(ContProfConsumeTest, IncompatibleEntryDoesNotConsumeSlot) {
  m_beta.translations.front().startKind =
    ContProfStartKind::NamedParamsFuncEntry;

  auto const prepared = prepareContProfStartupCandidates(
    {m_beta, m_alpha}, m_units, 1
  );

  ASSERT_EQ(1, prepared.candidates.size());
  EXPECT_EQ(m_alphaFunc, prepared.candidates.front().func);
  EXPECT_EQ(2, prepared.recordsResolved);
}

TEST_F(ContProfConsumeTest, CapSelectsHottestCompatibleFunction) {
  auto const prepared = prepareContProfStartupCandidates(
    {m_alpha, m_beta}, m_units, 1
  );

  ASSERT_EQ(1, prepared.candidates.size());
  EXPECT_EQ(m_betaFunc, prepared.candidates.front().func);
  EXPECT_EQ(1, prepared.recordsResolved);
}

TEST_F(ContProfConsumeTest, EqualHotnessUsesKeyOrder) {
  m_beta.translations.front().executionCount = 7;

  auto const prepared = prepareContProfStartupCandidates(
    {m_beta, m_alpha}, m_units, 1
  );

  ASSERT_EQ(1, prepared.candidates.size());
  EXPECT_EQ(m_alphaFunc, prepared.candidates.front().func);
}

TEST_F(ContProfConsumeTest, ZeroCapSkipsPreparation) {
  auto const prepared = prepareContProfStartupCandidates(
    {m_alpha, m_beta}, m_units, 0
  );

  EXPECT_TRUE(prepared.candidates.empty());
  EXPECT_EQ(0, prepared.recordsResolved);
}

TEST(ContProfConsume, StartupActivation) {
  auto const oldServerMode = std::exchange(Cfg::Server::Mode, true);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldMaxFunctions =
    std::exchange(Cfg::Jit::ContProfStartupMaxFunctions, 1);
  auto const oldDirectory = std::exchange(
    Cfg::Jit::ContProfCheckpointDirectory,
    std::string{"/tmp/cont-prof-startup-activation"}
  );
  auto const oldJitEnabled = std::exchange(Cfg::Jit::Enabled, true);
  auto const oldPGO = std::exchange(Cfg::Jit::PGO, true);
  auto const oldRetranslateAllRequest =
    std::exchange(Cfg::Jit::RetranslateAllRequest, 0u);
  auto const oldRetranslateAllSeconds =
    std::exchange(Cfg::Jit::RetranslateAllSeconds, 1u);
  auto const oldSerdesMode =
    std::exchange(RuntimeOption::EvalJitSerdesMode, JitSerdesMode::Off);
  auto const oldDisabledByVSDebug =
    std::exchange(Cfg::Jit::DisabledByVSDebug, false);
  SCOPE_EXIT {
    Cfg::Server::Mode = oldServerMode;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Jit::ContProfStartupMaxFunctions = oldMaxFunctions;
    Cfg::Jit::ContProfCheckpointDirectory = oldDirectory;
    Cfg::Jit::Enabled = oldJitEnabled;
    Cfg::Jit::PGO = oldPGO;
    Cfg::Jit::RetranslateAllRequest = oldRetranslateAllRequest;
    Cfg::Jit::RetranslateAllSeconds = oldRetranslateAllSeconds;
    RuntimeOption::EvalJitSerdesMode = oldSerdesMode;
    Cfg::Jit::DisabledByVSDebug = oldDisabledByVSDebug;
  };

  EXPECT_TRUE(contProfStartupActive());

  Cfg::Server::Mode = false;
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Server::Mode = true;

  Cfg::Repo::Authoritative = false;
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Repo::Authoritative = true;

  Cfg::Jit::ContProfStartupMaxFunctions = 0;
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Jit::ContProfStartupMaxFunctions = 1;

  Cfg::Jit::ContProfCheckpointDirectory.clear();
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Jit::ContProfCheckpointDirectory = "/tmp/cont-prof-startup-activation";

  Cfg::Jit::Enabled = false;
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Jit::Enabled = true;

  Cfg::Jit::PGO = false;
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Jit::PGO = true;

  Cfg::Jit::RetranslateAllRequest = 1;
  EXPECT_FALSE(contProfStartupActive());
  Cfg::Jit::RetranslateAllRequest = 0;

  RuntimeOption::EvalJitSerdesMode = JitSerdesMode::Deserialize;
  EXPECT_FALSE(contProfStartupActive());
  RuntimeOption::EvalJitSerdesMode = JitSerdesMode::Off;

  EXPECT_TRUE(contProfStartupActive());
}

}
}
