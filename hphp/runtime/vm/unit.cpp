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
#include <filesystem>
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
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/debugger/debugger.h"

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
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/server/ext_server.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/ext_vsdebug.h"

#include "hphp/system/systemlib.h"
#include "hphp/runtime/base/program-functions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

//////////////////////////////////////////////////////////////////////

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
    Class* cls = pcls->namedType()->clsList();
    while (cls) {
      Class* cur = cls;
      cls = cls->m_next;
      if (cur->preClass() == pcls.get()) {
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
// Literal strings.

StringData* Unit::lookupLitstrId(Id id) const {
  assertx(id >= 0 && id < m_litstrs.size());

  auto& elem = m_litstrs[id];
  auto wrapper = elem.copy();
  if (wrapper.isPtr()) {
    assertx(!wrapper.ptr() || wrapper.ptr()->isStatic());
    return const_cast<StringData*>(wrapper.ptr());
  }
  auto lock = elem.lock_for_update();
  wrapper = elem.copy();
  if (wrapper.isPtr()) {
    assertx(!wrapper.ptr() || wrapper.ptr()->isStatic());
    return const_cast<StringData*>(wrapper.ptr());
  }
  auto const str = UnitEmitter::loadLitstrFromRepo(m_sn, wrapper.token(), true);
  assertx(!str || str->isStatic());
  lock.update(StringOrToken::FromPtr(str));
  return const_cast<StringData*>(str);
}

///////////////////////////////////////////////////////////////////////////////
// Literal arrays.

const ArrayData* Unit::lookupArrayId(Id id) const {
  assertx(id >= 0 && id < m_arrays.size());

  auto& elem = m_arrays[id];
  auto wrapper = elem.copy();
  if (wrapper.isPtr()) {
    assertx(wrapper.ptr());
    assertx(wrapper.ptr()->isStatic());
    return wrapper.ptr();
  }
  auto lock = elem.lock_for_update();
  wrapper = elem.copy();
  if (wrapper.isPtr()) {
    assertx(wrapper.ptr());
    assertx(wrapper.ptr()->isStatic());
    return wrapper.ptr();
  }

  auto const oldStrUnit = BlobEncoderHelper<const StringData*>::tl_unit;
  auto const oldArrUnit = BlobEncoderHelper<const ArrayData*>::tl_unit;

  BlobEncoderHelper<const StringData*>::tl_unit = const_cast<Unit*>(this);
  BlobEncoderHelper<const ArrayData*>::tl_unit = const_cast<Unit*>(this);
  SCOPE_EXIT {
    assertx(BlobEncoderHelper<const StringData*>::tl_unit == this);
    assertx(BlobEncoderHelper<const ArrayData*>::tl_unit == this);
    BlobEncoderHelper<const StringData*>::tl_unit = oldStrUnit;
    BlobEncoderHelper<const ArrayData*>::tl_unit = oldArrUnit;
  };

  auto const array =
    UnitEmitter::loadLitarrayFromRepo(m_sn, wrapper.token(), true);
  assertx(array);
  assertx(array->isStatic());
  lock.update(ArrayOrToken::FromPtr(array));
  return array;
}

///////////////////////////////////////////////////////////////////////////////
// RAT arrays.

const RepoAuthType::Array* Unit::lookupRATArray(Id id) const {
  assertx(id >= 0 && id < m_rats.size());
  auto& elem = m_rats[id];
  auto wrapper = elem.copy();
  if (wrapper.isPtr()) {
    assertx(wrapper.ptr());
    return wrapper.ptr();
  }
  auto lock = elem.lock_for_update();
  wrapper = elem.copy();
  if (wrapper.isPtr()) {
    assertx(wrapper.ptr());
    return wrapper.ptr();
  }

  assertx(!BlobEncoderHelper<const StringData*>::tl_unit);
  BlobEncoderHelper<const StringData*>::tl_unit = const_cast<Unit*>(this);
  SCOPE_EXIT {
    assertx(BlobEncoderHelper<const StringData*>::tl_unit == this);
    BlobEncoderHelper<const StringData*>::tl_unit = nullptr;
  };

  auto const array = UnitEmitter::loadRATArrayFromRepo(m_sn, wrapper.token());
  assertx(array);
  lock.update(RATArrayOrToken::FromPtr(array));
  return array;
}

///////////////////////////////////////////////////////////////////////////////
// Merge.

namespace {
///////////////////////////////////////////////////////////////////////////////

SimpleMutex unitInitLock(false /* reentrant */, RankUnitInit);
std::atomic<uint64_t> s_loadedUnits{0};

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
      if (RuntimeOption::ServerExecutionMode()) {
        ent.setInt("uptime", HHVM_FN(server_uptime)());
      }
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

  if (RO::EvalLogDeclDeps) {
    logDeclInfo();

    auto const thisPath = filepath();
    auto const& options = RepoOptions::forFile(thisPath->data());
    auto const dir = options.dir();
    auto const relPath = std::filesystem::relative(
      std::filesystem::path(thisPath->data()),
      dir
    );

    for (auto& [path, sha] : m_deps) {
      auto hash = SHA1{mangleUnitSha1(sha.toString(), path, options.flags())};
      auto fpath = makeStaticString(dir / path);
      g_context->m_loadedRdepMap[fpath].emplace_back(relPath, hash);
    }
  }

  if (RuntimeOption::RepoAuthoritative || (!SystemLib::s_inited && !RuntimeOption::EvalJitEnableRenameFunction)) {
    m_mergeState.store(MergeState::Merged, std::memory_order_relaxed);
  }
}

void Unit::logTearing(int64_t nsecs) {
  assertx(!RO::RepoAuthoritative);
  assertx(RO::EvalSampleRequestTearing);

  auto const repoOptions = g_context->getRepoOptionsForRequest();
  auto repoRoot = std::filesystem::path(repoOptions->path()).parent_path();

  assertx(!isSystemLib());
  StructuredLogEntry ent;

  auto const tpath = [&] () -> std::string {
    auto const orig = std::filesystem::path{origFilepath()->data()};
    auto const origNative = orig.native();
    if (repoRoot.native().size() > origNative.size() || repoRoot.empty()) {
      return origNative;
    }
    if (!std::equal(repoRoot.begin(), repoRoot.end(), orig.begin())) {
      return origNative;
    }
    return orig.lexically_relative(repoRoot).native();
  }();

  auto const debuggerCount = [&] {
    if (RuntimeOption::EnableHphpdDebugger) {
      return Eval::Debugger::CountConnectedProxy();
    }

    HPHP::VSDEBUG::Debugger* vspDebugger =
      HPHP::VSDEBUG::VSDebugExtension::getDebugger();
    if (vspDebugger != nullptr && vspDebugger->clientConnected()) {
      return 1;
    }

    return 0;
  }();

  auto const hash = sha1().toString();
  auto const bchash = bcSha1().toString();
  ent.setStr("filepath", tpath);
  ent.setStr("sha1", hash);
  ent.setStr("bc_sha1", bchash);
  ent.setInt("time_diff_ns", nsecs);
  ent.setInt("debuggers", debuggerCount);

  // always generate logs
  ent.force_init = RO::EvalSampleRequestTearingForce;

  FTRACE(2, "Tearing in {} ({} ns)\n", tpath.data(), nsecs);

  StructuredLog::log("hhvm_sandbox_file_tearing", ent);
}

void Unit::logDeclInfo() const {
  if (!RO::EvalLogAllDeclTearing &&
      !StructuredLog::coinflip(RO::EvalLogDeclDeps)) return;

  auto const thisPath = filepath();
  auto const& options = RepoOptions::forFile(thisPath->data());
  auto const dir = options.dir();
  auto const& map = g_context->m_evaledFiles;
  std::vector<std::string> rev_tears;
  std::vector<std::string> not_loaded;
  std::vector<std::string> loaded;

  std::vector<std::tuple<std::string, std::string, SHA1>> deps;
  for (auto const& [path, sha] : m_deps) {
    auto hash = SHA1{mangleUnitSha1(sha.toString(), path, options.flags())};
    deps.emplace_back(path, dir / path, std::move(hash));
  }

  for (auto const& [path, fullpath, sha] : deps) {
    auto const full = makeStaticString(fullpath);
    if (auto const info = folly::get_ptr(map, full)) {
      if (info->unit->sha1() != sha) rev_tears.emplace_back(path);
      else loaded.emplace_back(path);
    } else {
      not_loaded.emplace_back(path);
    }
  }

  StructuredLogEntry ent;

  ent.setInt("num_loaded_deps", loaded.size() + rev_tears.size());
  ent.setInt("num_torn_deps", rev_tears.size());
  ent.setInt("num_not_loaded_deps", not_loaded.size());
  ent.setInt("num_deps", deps.size());

  auto const logVec = [&] (auto name, auto& vec) {
    std::vector<folly::StringPiece> v;
    v.reserve(vec.size());
    for (auto& s : vec) v.emplace_back(s);
    ent.setVec(name, v);
  };

  logVec("loaded_deps", loaded);
  logVec("torn_deps", rev_tears);
  logVec("not_loaded_deps", not_loaded);

  std::vector<std::string> tears;
  std::vector<std::string> non_tears;
  for (auto const& [path, sha] : g_context->m_loadedRdepMap[thisPath]) {
    if (sha != m_sha1) tears.emplace_back(path);
    else non_tears.emplace_back(path);
  }

  ent.setInt("num_loaded_rdeps", tears.size() + non_tears.size());
  ent.setInt("num_torn_rdeps", tears.size());

  logVec("torn_rdeps", tears);
  logVec("loaded_rdeps", non_tears);

  if ((RO::EvalLogAllDeclTearing && (!rev_tears.empty() || !tears.empty()))) {
    ent.setInt("sample_rate", 1);
    StructuredLog::log("hhvm_decl_logging", ent);
  } else if(!RO::EvalLogAllDeclTearing ||
            StructuredLog::coinflip(RO::EvalLogDeclDeps)) {
    ent.setInt("sample_rate", RO::EvalLogDeclDeps);
    StructuredLog::log("hhvm_decl_logging", ent);
  }
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

  FTRACE(1, "Merging unit {} ({} funcs, {} constants, {} typealiases, {} classes, {} modules)\n",
         this->m_origFilepath->data(), m_funcs.size(), m_constants.size(), m_typeAliases.size(),
         m_preClasses.size(), m_modules.size());

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
      Constant::def(&constant);
    }

    for (auto& module : m_modules) {
      Stats::inc(Stats::UnitMerge_mergeable);
      Stats::inc(Stats::UnitMerge_mergeable_define);

      assertx(IMPLIES(RO::RepoAuthoritative, module.attrs & AttrPersistent));
      Module::def(&module);
    }

    boost::dynamic_bitset<> preClasses(m_preClasses.size());
    preClasses.set();
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

      // We do type alias last because they may depend on classes that needs to be define first
      madeProgress = defineSymbols(m_typeAliases, typeAliases, failIsFatal,
                                   Stats::UnitMerge_mergeable_typealias,
                                   [&](const PreTypeAlias& typeAlias) {
                                     assertx(typeAlias.isPersistent() == (this->isSystemLib() || RuntimeOption::RepoAuthoritative));
                                     return TypeAlias::def(&typeAlias, failIsFatal);
                                   }) || madeProgress;
      if (!madeProgress) failIsFatal = true;
    } while (typeAliases.any() || preClasses.any());
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
  NamedType::foreach_cached_class([&](Class* c) {
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
  NamedFunc::foreach_cached_func([&](Func* func) {
    if ((system ^ func->isBuiltin()) || func->isGenerated()) return; //continue
    a.append(Variant(func->nameStr()));
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
  for (auto& m : modules()) {
    m.prettyPrint(ss);
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
