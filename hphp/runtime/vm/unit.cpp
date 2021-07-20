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

#include "hphp/runtime/vm/unit.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <map>
#include <ostream>
#include <sstream>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <folly/Format.h>

#include <tbb/concurrent_hash_map.h>

#include "hphp/system/systemlib.h"
#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/functional.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include "hphp/util/struct-log.h"

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/frame-restore.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/program-functions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
TRACE_SET_MOD(hhbc);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_stdin("STDIN");
const StaticString s_stdout("STDOUT");
const StaticString s_stderr("STDERR");

//////////////////////////////////////////////////////////////////////

}

std::atomic<size_t> Unit::s_createdUnits{0};
std::atomic<size_t> Unit::s_liveUnits{0};

///////////////////////////////////////////////////////////////////////////////
// Construction and destruction.

Unit::Unit()
  : m_interpretOnly(false)
  , m_extended(false)
  , m_ICE(false)
{
  ++s_createdUnits;
  ++s_liveUnits;
}

Unit::~Unit() {
  if (RuntimeOption::EvalEnableReverseDataMap &&
      m_mergeState.load(std::memory_order_relaxed) != MergeState::Unmerged) {
    // Units are registered to data_map in Unit::initialMerge().
    data_map::deregister(this);
  }

  for (auto const func : funcs()) Func::destroy(func);

  // ExecutionContext and the TC may retain references to Class'es, so
  // it is possible for Class'es to outlive their Unit.
  for (auto const& pcls : m_preClasses) {
    Class* cls = pcls->namedEntity()->clsList();
    while (cls) {
      Class* cur = cls;
      cls = cls->m_next;
      if (cur->preClass() == pcls.get()) {
        cur->destroy();
      }
    }
  }

  for (auto const& rec : m_preRecords) {
    RecordDesc* recList = rec->namedEntity()->recordList();
    while (recList) {
      RecordDesc* cur = recList;
      recList = recList->m_next;
      if (cur->preRecordDesc() == rec.get()) {
        cur->destroy();
      }
    }
  }

  --s_liveUnits;
}

void* Unit::operator new(size_t sz) {
  MemoryStats::LogAlloc(AllocKind::Unit, sz);
  return low_malloc(sz);
}

void Unit::operator delete(void* p, size_t /*sz*/) {
  low_free(p);
}

///////////////////////////////////////////////////////////////////////////////
// Code locations.

bool Unit::getOffsetRanges(int line, OffsetFuncRangeVec& offsets) const {
  assertx(offsets.size() == 0);
  forEachFunc([&](const Func* func) {
    // Quickly ignore functions that can't have that line in them
    if (line < func->line1() || line > func->line2()) return false;

    auto map = func->getLineToOffsetRangeVecMap();
    auto it = map.find(line);
    if (it != map.end()) {
      offsets.push_back(std::pair(func, it->second));
    }
    return false;
  });
  return offsets.size() != 0;
}

bool Unit::isCoverageEnabled() const {
  return m_coverage.bound() && m_coverage.isInit();
}
void Unit::enableCoverage() {
  if (!m_coverage.bound()) {
    assertx(!RO::RepoAuthoritative && RO::EvalEnablePerFileCoverage);
    m_coverage.bind(
      rds::Mode::Normal,
      rds::LinkName{"UnitCoverage", origFilepath()}
    );
  }
  if (m_coverage.isInit()) return;
  new (m_coverage.get()) req::dynamic_bitset{};
  m_coverage.markInit();
}
void Unit::disableCoverage() {
  if (!isCoverageEnabled()) return;

  m_coverage.markUninit();
  m_coverage->req::dynamic_bitset::~dynamic_bitset();
}
void Unit::clearCoverage() {
  assertx(isCoverageEnabled());
  m_coverage->reset();
}
void Unit::recordCoverage(int line) {
  assertx(isCoverageEnabled());

  if (line == -1) return;

  if (m_coverage->size() <= line) m_coverage->resize(line + 1);
  m_coverage->set(line);
}
Array Unit::reportCoverage() const {
  assertx(isCoverageEnabled());

  auto const& c = *m_coverage;
  auto const end = req::dynamic_bitset::npos;
  VecInit init{m_coverage->count()};
  for (auto i = c.find_first(); i != end; i = c.find_next(i)) {
    init.append(i);
  }

  return init.toArray();
}
rds::Handle Unit::coverageDataHandle() const {
  assertx(m_coverage.bound());
  return m_coverage.handle();
}

///////////////////////////////////////////////////////////////////////////////
// Funcs and PreClasses.

Func* Unit::getEntryPoint() const {
  if (m_entryPointId == kInvalidId) {
    return nullptr;
  }
  return lookupFuncId(m_entryPointId);
}

///////////////////////////////////////////////////////////////////////////////
// Constant lookup.

TypedValue Unit::lookupCns(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);

  if (LIKELY(rds::isHandleBound(handle) &&
             rds::isHandleInit(handle))) {
    auto const& tv = rds::handleToRef<TypedValue, rds::Mode::NonLocal>(handle);

    if (LIKELY(type(tv) != KindOfUninit)) {
      assertx(tvIsPlausible(tv));
      tvIncRefGen(tv);
      return tv;
    }

    assertx(tv.m_data.pcnt != nullptr);
    auto const callback =
      reinterpret_cast<Native::ConstantCallback>(tv.m_data.pcnt);
    Variant v = callback(cnsName);
    const TypedValue tvRet = v.detach();
    assertx(tvIsPlausible(tvRet));
    assertx(tvAsCVarRef(&tvRet).isAllowedAsConstantValue() ==
            Variant::AllowedAsConstantValue::Allowed);

    if (rds::isNormalHandle(handle) && type(tvRet) != KindOfResource) {
      tvIncRefGen(tvRet);
      rds::handleToRef<TypedValue, rds::Mode::Normal>(handle) = tvRet;
    }
    return tvRet;
  }
  return make_tv<KindOfUninit>();
}

const TypedValue* Unit::lookupPersistentCns(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);
  if (!rds::isHandleBound(handle) || !rds::isPersistentHandle(handle)) {
    return nullptr;
  }
  auto const ret = rds::handleToPtr<TypedValue, rds::Mode::Persistent>(handle);
  assertx(tvIsPlausible(*ret));
  return ret;
}

TypedValue Unit::loadCns(const StringData* cnsName) {
  auto const tv = lookupCns(cnsName);
  if (LIKELY(type(tv) != KindOfUninit)) return tv;

  if (needsNSNormalization(cnsName)) {
    return loadCns(normalizeNS(cnsName));
  }

  if (!AutoloadHandler::s_instance->autoloadConstant(
        const_cast<StringData*>(cnsName))) {
    return make_tv<KindOfUninit>();
  }
  return lookupCns(cnsName);
}

Variant Unit::getCns(const StringData* name) {
  const StringData* func_name = Constant::funcNameFromName(name);
  Func* func = Func::lookup(func_name);
  assertx(
    func &&
    "The function should have been autoloaded when we loaded the constant");
  return Variant::attach(
    g_context->invokeFuncFew(func, nullptr, 0, nullptr,
                             RuntimeCoeffects::fixme(), false, false)
  );
}

void Unit::defCns(const Constant* constant) {
  auto const cnsName = constant->name;
  FTRACE(3, "  Defining def {}\n", cnsName->data());
  auto const cnsVal = constant->val;

  if (constant->attrs & Attr::AttrPersistent) {
    DEBUG_ONLY auto res = bindPersistentCns(cnsName, cnsVal);
    assertx(res);
    return;
  }

  auto const ch = makeCnsHandle(cnsName);
  assertx(rds::isHandleBound(ch));
  auto cns = rds::handleToPtr<TypedValue, rds::Mode::NonLocal>(ch);

  if (!rds::isHandleInit(ch)) {
    cns->m_type = KindOfUninit;
    cns->m_data.pcnt = nullptr;
  }

  if (UNLIKELY(cns->m_type != KindOfUninit ||
               cns->m_data.pcnt != nullptr)) {
    raise_error(Strings::CONSTANT_ALREADY_DEFINED, cnsName->data());
  }

  assertx(tvAsCVarRef(&cnsVal).isAllowedAsConstantValue() ==
           Variant::AllowedAsConstantValue::Allowed ||
          (cnsVal.m_type == KindOfUninit &&
           cnsVal.m_data.pcnt != nullptr));

  assertx(rds::isNormalHandle(ch));
  tvDup(cnsVal, *cns);
  rds::initHandle(ch);
}

bool Unit::defNativeConstantCallback(const StringData* cnsName,
                                     TypedValue value) {
  static const bool kServer = RuntimeOption::ServerExecutionMode();
  // Zend doesn't define the STD* streams in server mode so we don't either
  if (UNLIKELY(kServer &&
       (s_stdin.equal(cnsName) ||
        s_stdout.equal(cnsName) ||
        s_stderr.equal(cnsName)))) {
    return false;
  }
  bindPersistentCns(cnsName, value);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Merge.

namespace {
///////////////////////////////////////////////////////////////////////////////

SimpleMutex unitInitLock(false /* reentrant */, RankUnitInit);
std::atomic<uint64_t> s_loadedUnits{0};

void setGlobal(StringData* name, TypedValue *value) {
  g_context->m_globalNVTable->set(name, value);
}

bool isEvalName(const StringData* name) {
  return name->empty() || boost::ends_with(name->slice(), EVAL_FILENAME_SUFFIX);
}

///////////////////////////////////////////////////////////////////////////////
}

void Unit::initialMerge() {
  unitInitLock.assertOwnedBySelf();
  if (m_mergeState.load(std::memory_order_relaxed) != MergeState::Unmerged) {
    return;
  }

  auto const nrecord = RuntimeOption::EvalRecordFirstUnits;
  if (s_loadedUnits.load(std::memory_order_relaxed) < nrecord) {
    auto const index = s_loadedUnits.fetch_add(1, std::memory_order_relaxed);
    if (index < nrecord) {
      StructuredLogEntry ent;
      ent.setStr("path", m_origFilepath->data());
      ent.setInt("index", index);
      StructuredLog::log("hhvm_first_units", ent);
    }
  }

  if (RuntimeOption::EvalEnableReverseDataMap) {
    data_map::register_start(this);
  }

  if (!RO::RepoAuthoritative && RO::EvalEnablePerFileCoverage) {
    m_coverage.bind(
      rds::Mode::Normal,
      rds::LinkName{"UnitCoverage", origFilepath()}
    );
  }

  m_mergeState.store(MergeState::InitialMerged, std::memory_order_relaxed);
}

void Unit::merge() {
  if (LIKELY(m_mergeState.load(std::memory_order_relaxed) == MergeState::Merged)) {
    return;
  }

  if (m_fatalInfo) {
    raise_parse_error(filepath(),
                      m_fatalInfo->m_fatalMsg.c_str(),
                      m_fatalInfo->m_fatalLoc);
  }

  auto mergeTypes = MergeTypes::Everything;
  if (UNLIKELY((m_mergeState.load(std::memory_order_relaxed) == MergeState::Unmerged))) {
    SimpleLock lock(unitInitLock);
    initialMerge();
  } else if (!RuntimeOption::RepoAuthoritative && !SystemLib::s_inited && RuntimeOption::EvalJitEnableRenameFunction) {
    mergeTypes = MergeTypes::Function;
  }

  mergeImpl(mergeTypes);

  if (RuntimeOption::RepoAuthoritative || (!SystemLib::s_inited && !RuntimeOption::EvalJitEnableRenameFunction)) {
    m_mergeState.store(MergeState::Merged, std::memory_order_relaxed);
  }
}

namespace {
RDS_LOCAL(hphp_fast_set<const Unit*>, rl_mergedUnits);
}

void Unit::logTearing() {
  assertx(!RO::RepoAuthoritative);
  assertx(RO::EvalSampleRequestTearing);
  assertx(g_context);

  auto const repoOptions = g_context->getRepoOptionsForRequest();
  auto repoRoot = folly::fs::path(repoOptions->path()).parent_path();

  for (auto const u : *rl_mergedUnits) {
    assertx(!u->isSystemLib());
    auto const loaded = lookupUnit(const_cast<StringData*>(u->origFilepath()),
                                   "", nullptr, Native::s_noNativeFuncs, true);

    if (loaded != u && (!loaded || loaded->sha1() != u->sha1())) {
      StructuredLogEntry ent;

      auto const tpath = [&] () -> std::string {
        auto const orig = folly::fs::path(u->origFilepath()->data());
        if (repoRoot.size() > orig.size()) return orig.native();
        if (!std::equal(repoRoot.begin(), repoRoot.end(), orig.begin())) {
          return orig.native();
        }
        return orig.lexically_relative(repoRoot).native();
      }();
      ent.setStr("filepath", tpath);
      ent.setStr("same_bc",
                 loaded && loaded->bcSha1() == u->bcSha1() ? "true" : "false");
      ent.setStr("removed", loaded ? "false" : "true");

      StructuredLog::log("hhvm_sandbox_file_tearing", ent);
    }
  }

  rl_mergedUnits->clear();
}

template <typename T, typename I>
static bool defineSymbols(const T& symbols, boost::dynamic_bitset<>& define, bool failIsFatal, Stats::StatCounter counter, I lambda) {
  auto i = define.find_first();
  if (failIsFatal) {
    if (i == define.npos) i = define.find_first();
  }

  bool madeProgress = false;
  for (; i != define.npos; i = define.find_next(i)) {
    auto& symbol = symbols[i];
    Stats::inc(Stats::UnitMerge_mergeable);
    Stats::inc(counter);
    auto res = lambda(symbol);
    if (res) {
      define.reset(i);
      madeProgress = true;
    }
  }
  return madeProgress;
}

void Unit::mergeImpl(MergeTypes mergeTypes) {
  assertx(m_mergeState.load(std::memory_order_relaxed) >= MergeState::InitialMerged);
  autoTypecheck(this);

  if (!RO::RepoAuthoritative &&
      !isSystemLib() &&
      !origFilepath()->empty() &&
      g_context && g_context->m_shouldSampleUnitTearing) {
    rl_mergedUnits->emplace(this);
  }

  FTRACE(1, "Merging unit {} ({} funcs, {} constants, {} typealiases, {} classes, {} records)\n",
         this->m_origFilepath->data(), m_funcs.size(), m_constants.size(), m_typeAliases.size(),
         m_preClasses.size(), m_preRecords.size());

  if (mergeTypes & MergeTypes::Function) {
    for (auto func : funcs()) {
      Func::def(func);
    }
  }

  if (mergeTypes & MergeTypes::NotFunction) {
    for (auto& constant : m_constants) {
      Stats::inc(Stats::UnitMerge_mergeable);
      Stats::inc(Stats::UnitMerge_mergeable_define);

      assertx((!!(constant.attrs & AttrPersistent)) == (this->isSystemLib() || RuntimeOption::RepoAuthoritative));
      defCns(&constant);
    }

    boost::dynamic_bitset<> preClasses(m_preClasses.size());
    preClasses.set();
    boost::dynamic_bitset<> preRecords(m_preRecords.size());
    preRecords.set();
    boost::dynamic_bitset<> typeAliases(m_typeAliases.size());
    typeAliases.set();

    bool failIsFatal = false;
    do {
      bool madeProgress = false;
      madeProgress = defineSymbols(m_preClasses, preClasses, failIsFatal,
                                   Stats::UnitMerge_mergeable_class,
                                   [&](const PreClassPtr& preClass) {
                                     // Anonymous classes doesn't need to be defined because they will be defined when used
                                     if (PreClassEmitter::IsAnonymousClassName(preClass->name()->toCppString())) {
                                       return true;
                                     }
                                     assertx(preClass->isPersistent() == (this->isSystemLib() || RuntimeOption::RepoAuthoritative));
                                     return Class::def(preClass.get(), failIsFatal) != nullptr;
                                   }) || madeProgress;

      madeProgress = defineSymbols(m_preRecords, preRecords, failIsFatal,
                                   Stats::UnitMerge_mergeable_record,
                                   [&](const PreRecordDescPtr& preRecord) {
                                     assertx(preRecord->isPersistent() == (this->isSystemLib() || RuntimeOption::RepoAuthoritative));
                                     return RecordDesc::def(preRecord.get(), failIsFatal) != nullptr;
                                   }) || madeProgress;

      // We do type alias last because they may depend on classes or records that needs to be define first
      madeProgress = defineSymbols(m_typeAliases, typeAliases, failIsFatal,
                                   Stats::UnitMerge_mergeable_typealias,
                                   [&](const PreTypeAlias& typeAlias) {
                                     assertx(typeAlias.isPersistent() == (this->isSystemLib() || RuntimeOption::RepoAuthoritative));
                                     return TypeAlias::def(&typeAlias, failIsFatal);
                                   }) || madeProgress;
      if (!madeProgress) failIsFatal = true;
    } while (typeAliases.any() || preClasses.any() || preRecords.any());
  }
}

bool Unit::isSystemLib() const {
  return FileUtil::isSystemName(m_origFilepath->slice());
}

///////////////////////////////////////////////////////////////////////////////
// Info arrays.

namespace {

Array getClassesWithAttrInfo(Attr attrs, bool inverse = false) {
  auto builtins = Array::CreateVec();
  auto non_builtins = Array::CreateVec();
  NamedEntity::foreach_cached_class([&](Class* c) {
    if ((c->attrs() & attrs) ? !inverse : inverse) {
      if (c->isBuiltin()) {
        builtins.append(make_tv<KindOfPersistentString>(c->name()));
      } else {
        non_builtins.append(make_tv<KindOfPersistentString>(c->name()));
      }
    }
  });
  if (builtins.empty()) return non_builtins;
  auto all = VecInit(builtins.size() + non_builtins.size());
  for (auto i = builtins.size(); i > 0; i--) {
    all.append(builtins.lookup(safe_cast<int64_t>(i - 1)));
  }
  IterateV(non_builtins.get(), [&](auto name) { all.append(name); });
  return all.toArray();
}

template<bool system>
Array getFunctions() {
  // Return an array of all defined functions.  This method is used
  // to support get_defined_functions().
  Array a = Array::CreateVec();
  NamedEntity::foreach_cached_func([&](Func* func) {
    if ((system ^ func->isBuiltin()) || func->isGenerated()) return; //continue
    a.append(HHVM_FN(strtolower)(func->nameStr()));
  });
  return a;
}

///////////////////////////////////////////////////////////////////////////////
}

Array Unit::getClassesInfo() {
  return getClassesWithAttrInfo(AttrInterface | AttrTrait,
                                 /* inverse = */ true);
}

Array Unit::getInterfacesInfo() {
  return getClassesWithAttrInfo(AttrInterface);
}

Array Unit::getTraitsInfo() {
  return getClassesWithAttrInfo(AttrTrait);
}

Array Unit::getUserFunctions() {
  return getFunctions<false>();
}

Array Unit::getSystemFunctions() {
  return getFunctions<true>();
}


///////////////////////////////////////////////////////////////////////////////
// Pretty printer.

std::string Unit::toString() const {
  std::ostringstream ss;
  for (auto& pc : m_preClasses) {
    pc->prettyPrint(ss);
  }
  for (auto& func : funcs()) {
    func->prettyPrint(ss);
  }
  for (auto& cns : constants()) {
    cns.prettyPrint(ss);
  }
  return ss.str();
}


///////////////////////////////////////////////////////////////////////////////
// Other methods.

std::string mangleReifiedGenericsName(const ArrayData* tsList) {
  std::vector<std::string> l;
  IterateV(
    tsList,
    [&](TypedValue v) {
      assertx(tvIsDict(v));
      auto str =
        TypeStructure::toString(ArrNR(v.m_data.parr),
          TypeStructure::TSDisplayType::TSDisplayTypeInternal).toCppString();
      str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
      l.emplace_back(str);
    }
  );
  return folly::sformat("<{}>", folly::join(",", l));
}

}
