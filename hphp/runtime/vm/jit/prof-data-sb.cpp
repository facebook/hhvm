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

#include "hphp/runtime/vm/jit/prof-data-sb.h"

#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP::jit {
namespace {
std::mutex s_sbProfMutex;
std::vector<std::unique_ptr<ProfDataSBSer>> s_sbProfSer;
std::vector<std::unique_ptr<ProfDataSBDeser>> s_sbProfDeser;
}

ProfDataSBSer::ProfDataSBSer(const Func* func, ProfDataSBKind k)
  : m_kind(k)
  , m_bcUnitPath(func->unit()->origFilepath())
  , m_bcUnitSha1(func->unit()->sha1())
  , m_funcName(func->name()) {
  if (func->isMethod()) {
    auto cls = func->implCls();
    auto const unit = cls->preClass()->unit();
    m_unitPath = unit->origFilepath();
    m_clsName = cls->preClass()->name();
    if (func->isClosureBody() && func->cls()) m_context = func->cls()->name();
  } else {
    auto const unit = func->unit();
    m_unitPath = unit->origFilepath();
  }
}

ProfDataSBRegionSer::ProfDataSBRegionSer(RegionContext& ctx)
  : ProfDataSBSer(ctx.sk.func(), ProfDataSBKind::Region)
  , m_offsetAndMode(ctx.sk.toAtomicInt() >> 32)
  , m_spOffset(ctx.spOffset)
{
  for (auto const& lt : ctx.liveTypes) {
    auto const& t = lt.type;
    SBProfType pt;
    pt.m_bits = t.rawBits();
    always_assert(t.ptrLocation() == PtrLocation::Bottom);
    always_assert(!t.hasConstVal());
    if (auto const& clsSpec = t.clsSpec()) {
      pt.m_spec = clsSpec.exact() ? SBProfTypeSpec::Exact : SBProfTypeSpec::Sub;
      pt.m_clsName = clsSpec.cls()->name();
    } else {
      pt.m_spec = SBProfTypeSpec::None;
    }
    m_liveProfTypes.emplace_back(lt.location, pt);
  }
}

ProfDataSBPrologueSer::ProfDataSBPrologueSer(const Func* func, int nPassed)
  : ProfDataSBSer(func, ProfDataSBKind::Prologue)
  , m_nPassed(nPassed)
{}

ProfDataSBDeser::ProfDataSBDeser(Unit* unit, const StringData* funcName,
                                 const StringData* clsName,
                                 const StringData* context, ProfDataSBKind k)
  : m_kind(k)
  , m_unit(unit)
  , m_funcName(funcName)
  , m_clsName(clsName)
  , m_context(context)
{}

ProfDataSBRegionDeser::
ProfDataSBRegionDeser(Unit* unit,
                      const StringData* funcName, const StringData* clsName,
                      const StringData* context, uint32_t offsetAndMode,
                      const std::vector<SBProfTypedLocation>& liveProfTypes,
                      SBInvOffset spOffset)
  : ProfDataSBDeser(unit, funcName, clsName, context, ProfDataSBKind::Region)
  , m_offsetAndMode(offsetAndMode)
  , m_liveProfTypes(liveProfTypes)
  , m_spOffset(spOffset)
{}

ProfDataSBPrologueDeser::
ProfDataSBPrologueDeser(Unit* unit,
                        const StringData* funcName, const StringData* clsName,
                        const StringData* context, int nPassed)
  : ProfDataSBDeser(unit, funcName, clsName, context, ProfDataSBKind::Prologue)
  , m_nPassed(nPassed)
{}

void addToSBProfile(RegionContext& ctx) {
  auto pd = std::make_unique<ProfDataSBRegionSer>(ctx);
  // TODO (T208863482): exclude systemlib from preloading, not from jumpstart
  if (FileUtil::isSystemName(pd->m_unitPath->slice()) ||
      FileUtil::isSystemName(pd->m_bcUnitPath->slice())) {
    return;
  }
  std::lock_guard<std::mutex> l{s_sbProfMutex};
  s_sbProfSer.emplace_back(std::move(pd));
}

void addToSBProfile(const Func* func, int nPassed) {
  auto pd = std::make_unique<ProfDataSBPrologueSer>(func, nPassed);
  // TODO (T208863482): exclude systemlib from preloading, not from jumpstart
  if (FileUtil::isSystemName(pd->m_unitPath->slice()) ||
      FileUtil::isSystemName(pd->m_bcUnitPath->slice())) {
    return;
  }
  std::lock_guard<std::mutex> l{s_sbProfMutex};
  s_sbProfSer.emplace_back(std::move(pd));
}

std::vector<ProfDataSBSer*> getSBSerProfDataCopy() {
  std::lock_guard<std::mutex> l{s_sbProfMutex};
  std::vector<ProfDataSBSer*> copy;
  copy.reserve(s_sbProfSer.size());
  std::for_each(s_sbProfSer.begin(), s_sbProfSer.end(),
                [&](std::unique_ptr<ProfDataSBSer>& p) {
                  copy.push_back(p.get());
                });
  return copy;
}

std::vector<std::unique_ptr<ProfDataSBDeser>>& getSBDeserProfData() {
  return s_sbProfDeser;
}
} // namespace HPHP::jit
