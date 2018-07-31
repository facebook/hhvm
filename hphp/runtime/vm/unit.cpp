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

#include <folly/Format.h>

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/util/functional.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include "hphp/util/smalllocks.h"

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/execution-context.h"
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
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hh-utils.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/reverse-data-map.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/server/source-root-info.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_stdin("STDIN");
const StaticString s_stdout("STDOUT");
const StaticString s_stderr("STDERR");

//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////

/*
 * We store 'detailed' line number information on a table on the side, because
 * in production modes for HHVM it's generally not useful (which keeps Unit
 * smaller in that case)---this stuff is only used for the debugger, where we
 * can afford the lookup here.  The normal Unit m_lineMap is capable of
 * producing enough line number information for things needed in production
 * modes (backtraces, warnings, etc).
 */

using LineToOffsetRangeVecMap = std::map<int,OffsetRangeVec>;

struct ExtendedLineInfo {
  SourceLocTable sourceLocTable;

  /*
   * Map from source lines to a collection of all the bytecode ranges the line
   * encompasses.
   *
   * The value type of the map is a list of offset ranges, so a single line
   * with several sub-statements may correspond to the bytecodes of all of the
   * sub-statements.
   *
   * May not be initialized.  Lookups need to check if it's empty() and if so
   * compute it from sourceLocTable.
   */
  LineToOffsetRangeVecMap lineToOffsetRange;
};

using ExtendedLineInfoCache = tbb::concurrent_hash_map<
  const Unit*,
  ExtendedLineInfo,
  pointer_hash<Unit>
>;
ExtendedLineInfoCache s_extendedLineInfo;

using LineTableStash = tbb::concurrent_hash_map<
  const Unit*,
  LineTable,
  pointer_hash<Unit>
>;
LineTableStash s_lineTables;

struct LineCacheEntry {
  LineCacheEntry(const Unit* unit, LineTable&& table)
    : unit{unit}
    , table{std::move(table)}
  {}
  const Unit* unit;
  LineTable table;
};
std::array<std::atomic<LineCacheEntry*>, 512> s_lineCache;

//////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////
// MergeInfo.

Unit::MergeInfo* Unit::MergeInfo::alloc(size_t size) {
  MergeInfo* mi = (MergeInfo*)malloc(
    sizeof(MergeInfo) + size * sizeof(void*));
  mi->m_firstHoistableFunc = 0;
  mi->m_firstHoistablePreClass = 0;
  mi->m_firstMergeablePreClass = 0;
  mi->m_mergeablesSize = size;
  return mi;
}


///////////////////////////////////////////////////////////////////////////////
// Construction and destruction.

Unit::Unit()
  : m_mergeOnly(false)
  , m_interpretOnly(false)
  , m_isHHFile(false)
  , m_extended(false)
  , m_serialized(false)
  , m_ICE(false)
  , m_mainReturn(make_tv<KindOfUninit>())
{}

Unit::~Unit() {
  if (RuntimeOption::EvalEnableReverseDataMap &&
      m_mergeState.load(std::memory_order_relaxed) != MergeState::Unmerged) {
    // Units are registered to data_map in Unit::initialMerge().
    data_map::deregister(this);
  }

  s_extendedLineInfo.erase(this);
  s_lineTables.erase(this);

  auto const hash = pointer_hash<Unit>{}(this) % s_lineCache.size();
  auto& entry = s_lineCache[hash];
  if (auto lce = entry.load(std::memory_order_acquire)) {
    if (lce->unit == this &&
        entry.compare_exchange_strong(lce, nullptr,
                                      std::memory_order_release)) {
      Treadmill::enqueue([lce] { delete lce; });
    }
  }

  if (!RuntimeOption::RepoAuthoritative) {
    if (debug) {
      // poison released bytecode
      memset(const_cast<unsigned char*>(m_bc), 0xff, m_bclen);
    }
    free(const_cast<unsigned char*>(m_bc));
    g_hhbc_size->addValue(-int64_t(m_bclen));
  }

  auto const mi = mergeInfo();
  if (mi) {
    for (auto const func : mi->mutableFuncs()) Func::destroy(func);
  }

  // ExecutionContext and the TC may retain references to Class'es, so
  // it is possible for Class'es to outlive their Unit.
  for (auto const& pcls : m_preClasses) {
    Class* cls = pcls->namedEntity()->clsList();
    while (cls) {
      Class* cur = cls;
      cls = cls->m_nextClass;
      if (cur->preClass() == pcls.get()) {
        cur->destroy();
      }
    }
  }

  free(mi);

  if (m_pseudoMainCache) {
    for (auto& kv : *m_pseudoMainCache) {
      Func::destroy(kv.second);
    }
    delete m_pseudoMainCache;
  }
}

void* Unit::operator new(size_t sz) {
  return low_malloc(sz);
}

void Unit::operator delete(void* p, size_t /*sz*/) {
  low_free(p);
}


///////////////////////////////////////////////////////////////////////////////
// Code locations.

static SourceLocTable loadSourceLocTable(const Unit* unit) {
  auto ret = SourceLocTable{};
  if (unit->repoID() == RepoIdInvalid) return ret;

  Lock lock(g_classesMutex);
  auto& urp = Repo::get().urp();
  urp.getSourceLocTab[unit->repoID()].get(unit->sn(), ret);
  return ret;
}

/*
 * Return the Unit's SourceLocTable, extracting it from the repo if
 * necessary.
 */
const SourceLocTable& getSourceLocTable(const Unit* unit) {
  {
    ExtendedLineInfoCache::const_accessor acc;
    if (s_extendedLineInfo.find(acc, unit)) {
      return acc->second.sourceLocTable;
    }
  }

  // Try to load it while we're not holding the lock.
  auto newTable = loadSourceLocTable(unit);
  ExtendedLineInfoCache::accessor acc;
  if (s_extendedLineInfo.insert(acc, unit)) {
    acc->second.sourceLocTable = std::move(newTable);
  }
  return acc->second.sourceLocTable;
}

/**
 * Generate line->vector<OffsetRange> reverse map from SourceLocTable.
 *
 * Algorithm:
 * We first generate the OffsetRange for each SourceLoc,
 * then sort the pair<SourceLoc, OffsetRange> in most nested to outward order
 * so that we can add vector<OffsetRange> for nested lines first.
 * After merging continuous duplicate line ranges into one we build the final
 * map by adding vector<OffsetRange> for each line in the LineRange only if
 * it hasn't got any vector<OffsetRange> from inner LineRange yet.
 * By doing this we ensure the outer LineRange's vector<OffsetRange> will not be
 * added for inner lines.
 */
static void generateLineToOffsetRangesMap(
  const Unit* unit,
  LineToOffsetRangeVecMap& map
) {
  // First generate an OffsetRange for each SourceLoc.
  auto const& srcLocTable = getSourceLocTable(unit);

  struct LineRange {
    LineRange(int start, int end)
    : line0(start), line1(end)
    {}
    int line0;
    int line1;

    bool operator!=(const LineRange& other) const {
      return this->line0 != other.line0 || this->line1 != other.line1;
    }
  };

  using LineRangeOffsetRangePair = std::pair<LineRange, OffsetRange>;
  std::vector<LineRangeOffsetRangePair> lineRangesTable;
  Offset baseOff = 0;
  for (const auto& sourceLoc: srcLocTable) {
    Offset pastOff = sourceLoc.pastOffset();
    OffsetRange offsetRange(baseOff, pastOff);
    LineRange lineRange(sourceLoc.val().line0, sourceLoc.val().line1);
    lineRangesTable.emplace_back(lineRange, offsetRange);
    baseOff = pastOff;
  }

  // Sort the line ranges in most nested to outward order:
  // First sort them in ascending order by line range end;
  // if range end ties, sort in descending order by line range start.
  std::sort(
    lineRangesTable.begin(),
    lineRangesTable.end(),
    [](const LineRangeOffsetRangePair& a, const LineRangeOffsetRangePair& b) {
      return a.first.line1 == b.first.line1 ?
        a.first.line0 > b.first.line0 :
        a.first.line1 < b.first.line1;
    }
  );

  // Merge continuous duplicate line ranges into one.
  using LineRangeToOffsetRangesTable =
    std::vector<std::pair<LineRange, std::vector<OffsetRange>>>;
  LineRangeToOffsetRangesTable lineRangeToOffsetRangesTable;
  for (auto i = 0; i < lineRangesTable.size(); ++i) {
    if (i == 0 || lineRangesTable[i].first != lineRangesTable[i-1].first) {
      // New line range starts.
      std::vector<OffsetRange> offsetRanges;
      offsetRanges.emplace_back(lineRangesTable[i].second);
      const auto& lineRange = lineRangesTable[i].first;
      lineRangeToOffsetRangesTable.emplace_back(lineRange, offsetRanges);
    } else {
      // Duplicate LineRange.
      assertx(lineRangeToOffsetRangesTable.size() > 0);
      auto& offsetRanges = lineRangeToOffsetRangesTable.back().second;
      offsetRanges.emplace_back(lineRangesTable[i].second);
    }
  }

  // Generate the final line to offset ranges map.
  for (auto& entry: lineRangeToOffsetRangesTable) {
    // Sort the offset ranges of each line range.
    std::sort(
      entry.second.begin(),
      entry.second.end(),
      [](const OffsetRange& a, const OffsetRange& b) {
        return a.base == b.base ? a.past < b.past : a.base < b.base;
      }
    );

    const auto& offsetRanges = entry.second;
    auto line0 = entry.first.line0;
    auto line1 = entry.first.line1;
    for (auto line = line0; line <= line1; ++line) {
      // Only add if not added by inner LineRange yet.
      if (map.find(line) == map.end()) {
        map[line] = offsetRanges;
      }
    }
  }
}

/*
 * Return a copy of the Unit's line to OffsetRangeVec table.
 */
static LineToOffsetRangeVecMap getLineToOffsetRangeVecMap(const Unit* unit) {
  {
    ExtendedLineInfoCache::const_accessor acc;
    if (s_extendedLineInfo.find(acc, unit)) {
      if (!acc->second.lineToOffsetRange.empty()) {
        return acc->second.lineToOffsetRange;
      }
    }
  }

  LineToOffsetRangeVecMap map;
  generateLineToOffsetRangesMap(unit, map);

  ExtendedLineInfoCache::accessor acc;
  if (!s_extendedLineInfo.find(acc, unit)) {
    always_assert_flog(0, "ExtendedLineInfoCache was not found when it should "
      "have been");
  }
  if (acc->second.lineToOffsetRange.empty()) {
    acc->second.lineToOffsetRange = std::move(map);
  }
  return acc->second.lineToOffsetRange;
}

static const LineTable& loadLineTable(const Unit* unit) {
  if (unit->repoID() == RepoIdInvalid) {
    LineTableStash::accessor acc;
    if (s_lineTables.find(acc, unit)) {
      return acc->second;
    }
    static LineTable empty;
    return empty;
  }

  auto const hash = pointer_hash<Unit>{}(unit) % s_lineCache.size();
  auto& entry = s_lineCache[hash];
  if (auto const p = entry.load(std::memory_order_acquire)) {
    if (p->unit == unit) return p->table;
  }

  // We already hold a lock on the unit in Unit::getLineNumber below,
  // so nobody else is going to be reading the line table while we are
  // (this is only an efficiency concern).
  auto& urp = Repo::get().urp();
  auto table = LineTable{};
  urp.getUnitLineTable(unit->repoID(), unit->sn(), table);
  auto const p = new LineCacheEntry(unit, std::move(table));
  if (auto const old = entry.exchange(p, std::memory_order_release)) {
    Treadmill::enqueue([old] { delete old; });
  }
  return p->table;
}

static LineInfo getLineInfo(const LineTable& table, Offset pc) {
  auto const it =
    std::upper_bound(begin(table), end(table), LineEntry{ pc, -1 });

  auto const e = end(table);
  if (it != e) {
    auto const line = it->val();
    if (line > 0) {
      auto const pastOff = it->pastOffset();
      auto const baseOff = it == begin(table) ?
        pc : std::prev(it)->pastOffset();
      assertx(baseOff <= pc && pc < pastOff);
      return { { baseOff, pastOff }, line };
    }
  }
  return LineInfo{ { pc, pc + 1 }, -1 };
}

int getLineNumber(const LineTable& table, Offset pc) {
  auto const key = LineEntry(pc, -1);
  auto it = std::upper_bound(begin(table), end(table), key);
  if (it != end(table)) {
    assertx(pc < it->pastOffset());
    return it->val();
  }
  return -1;
}

int Unit::getLineNumber(Offset pc) const {
  if (UNLIKELY(m_repoId == RepoIdInvalid)) {
    auto const lineTable = [&] () -> const LineTable* {
      LineTableStash::accessor acc;
      if (s_lineTables.find(acc, this)) {
        return &acc->second;
      }
      return nullptr;
    }();
    if (lineTable) return HPHP::getLineNumber(*lineTable, pc);
  }

  auto findLine = [&] {
    // lineMap is an atomically acquired bitwise copy of m_lineMap,
    // with no destructor
    auto lineMap(m_lineMap.get());
    if (lineMap->empty()) return INT_MIN;
    auto const it = std::upper_bound(
      lineMap->begin(), lineMap->end(),
      *lineMap->begin(), // Will be the first (ignored) param to our predicate
      [&] (const LineInfo&, const LineInfo& elm) {
        return pc < elm.first.past;
      }
    );
    if (it != lineMap->end() && it->first.base <= pc) return it->second;
    return INT_MIN;
  };

  auto line = findLine();
  if (line != INT_MIN) return line;

  m_lineMap.lock_for_update();
  try {
    line = findLine();
    if (line != INT_MIN) {
      m_lineMap.unlock();
      return line;
    }

    auto const info = HPHP::getLineInfo(loadLineTable(this), pc);
    auto copy = m_lineMap.copy();
    auto const it = std::upper_bound(
      copy.begin(), copy.end(),
      info,
      [&] (const LineInfo& a, const LineInfo& b) {
        return a.first.base < b.first.past;
      }
    );
    assertx(it == copy.end() || (it->first.past > pc && it->first.base > pc));
    copy.insert(it, info);
    auto old = m_lineMap.update_and_unlock(std::move(copy));
    Treadmill::enqueue([old = std::move(old)] () mutable { old.clear(); });
    return info.second;
  } catch (...) {
    m_lineMap.unlock();
    throw;
  }
}

bool getSourceLoc(const SourceLocTable& table, Offset pc, SourceLoc& sLoc) {
  SourceLocEntry key(pc, sLoc);
  auto it = std::upper_bound(table.begin(), table.end(), key);
  if (it != table.end()) {
    assertx(pc < it->pastOffset());
    sLoc = it->val();
    return true;
  }
  return false;
}

bool Unit::getSourceLoc(Offset pc, SourceLoc& sLoc) const {
  auto const& sourceLocTable = getSourceLocTable(this);
  return HPHP::getSourceLoc(sourceLocTable, pc, sLoc);
}

bool Unit::getOffsetRange(Offset pc, OffsetRange& range) const {
  OffsetRangeVec offsets;
  auto line = getLineNumber(pc);
  getOffsetRanges(line, offsets);

  for (auto offset: offsets) {
    if (pc >= offset.base && pc < offset.past) {
      range = offset;
      return true;
    }
  }
  return false;
}

bool Unit::getOffsetRanges(int line, OffsetRangeVec& offsets) const {
  assertx(offsets.size() == 0);
  auto map = getLineToOffsetRangeVecMap(this);
  auto it = map.find(line);
  if (it == map.end()) return false;
  offsets = it->second;
  return true;
}

int Unit::getNearestLineWithCode(int line) const {
  auto map = getLineToOffsetRangeVecMap(this);
  auto it = map.lower_bound(line);
  return it == map.end() ? -1 : it->first;
}

const Func* Unit::getFunc(Offset pc) const {
  auto& table = getExtended()->m_funcTable;
  auto it = std::upper_bound(table.begin(), table.end(), nullptr,
                             [&] (const Func* a, const Func* b) {
                               assertx(a == nullptr);
                               return pc < b->past();
                             });
  if (it != table.end()) {
    assertx(pc < (*it)->past());
    return *it;
  }
  return nullptr;
}

void stashLineTable(const Unit* unit, LineTable table) {
  LineTableStash::accessor acc;
  if (s_lineTables.insert(acc, unit)) {
    acc->second = std::move(table);
  }
}

void stashExtendedLineTable(const Unit* unit, SourceLocTable table) {
  ExtendedLineInfoCache::accessor acc;
  if (s_extendedLineInfo.insert(acc, unit)) {
    acc->second.sourceLocTable = std::move(table);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Funcs and PreClasses.

Func* Unit::getMain(Class* cls /* = nullptr */) const {
  auto const mi = mergeInfo();
  if (!cls) return *mi->funcBegin();
  Lock lock(g_classesMutex);
  if (!m_pseudoMainCache) {
    m_pseudoMainCache = new PseudoMainCacheMap;
  }
  auto it = m_pseudoMainCache->find(cls);
  if (it != m_pseudoMainCache->end()) {
    return it->second;
  }
  Func* f = (*mi->funcBegin())->clone(cls);
  f->setNewFuncId();
  f->setBaseCls(cls);
  (*m_pseudoMainCache)[cls] = f;
  return f;
}

Func* Unit::getCachedEntryPoint() const {
  return m_cachedEntryPoint;
}

void Unit::renameFunc(const StringData* oldName, const StringData* newName) {
  // We do a linear scan over all the functions in the unit searching for the
  // func with a given name; in practice this is okay because the units created
  // by create_function() will always have the function being renamed at the
  // beginning
  assertx(oldName && oldName->isStatic());
  assertx(newName && newName->isStatic());

  for (auto& func : mergeInfo()->hoistableFuncs()) {
    auto const name = func->name();
    assertx(name);
    if (name->same(oldName)) {
      func->rename(newName);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Func lookup.

void Unit::defFunc(Func* func, bool debugger) {
  assertx(!func->isMethod());
  auto const handle = func->funcHandle();

  if (rds::isPersistentHandle(handle)) {
    auto& funcAddr = rds::handleToRef<LowPtr<Func>,
                                      rds::Mode::Persistent>(handle);
    auto const oldFunc = funcAddr.get();
    if (oldFunc == func) return;
    if (UNLIKELY(oldFunc != nullptr)) {
      assertx(oldFunc->isBuiltin() && !func->isBuiltin());
      raise_error(Strings::REDECLARE_BUILTIN, func->name()->data());
    }
    funcAddr = func;
  } else {
    assertx(rds::isNormalHandle(handle));
    auto& funcAddr = rds::handleToRef<LowPtr<Func>, rds::Mode::Normal>(handle);
    if (!rds::isHandleInit(handle, rds::NormalTag{})) {
      rds::initHandle(handle);
    } else {
      if (funcAddr.get() == func) return;
      raise_error(Strings::FUNCTION_ALREADY_DEFINED, func->name()->data());
    }
    funcAddr = func;
  }

  if (func->isUnique()) func->getNamedEntity()->setUniqueFunc(func);

  if (UNLIKELY(debugger)) phpDebuggerDefFuncHook(func);
}

Func* Unit::lookupFunc(const NamedEntity* ne) {
  return ne->getCachedFunc();
}

Func* Unit::lookupFunc(const StringData* name) {
  const NamedEntity* ne = NamedEntity::get(name);
  return ne->getCachedFunc();
}

Func* Unit::lookupBuiltin(const StringData* name) {
  // Builtins are either persistent (the normal case), or defined at the
  // beginning of every request (if JitEnableRenameFunction or interception is
  // enabled). In either case, they're unique, so they should be present in the
  // NamedEntity.
  auto const ne = NamedEntity::get(name);
  auto const f = ne->uniqueFunc();
  return (f && f->isBuiltin()) ? f : nullptr;
}

Func* Unit::loadFunc(const NamedEntity* ne, const StringData* name) {
  Func* func = ne->getCachedFunc();
  if (LIKELY(func != nullptr)) return func;
  if (AutoloadHandler::s_instance->autoloadFunc(
        const_cast<StringData*>(name))) {
    func = ne->getCachedFunc();
  }
  return func;
}

Func* Unit::loadFunc(const StringData* name) {
  String normStr;
  auto ne = NamedEntity::get(name, true, &normStr);
  if (normStr) {
    name = normStr.get();
  }
  return loadFunc(ne, name);
}

void Unit::bindFunc(Func *func) {
  assertx(!func->isMethod());
  auto const ne = func->getNamedEntity();
  ne->m_cachedFunc.bind(
    [&] {
      auto const isPersistent =
        (RuntimeOption::RepoAuthoritative || !SystemLib::s_inited) &&
        (func->attrs() & AttrPersistent);

      auto const handle = isPersistent ?
        rds::alloc<LowPtr<const Func>, rds::Mode::Persistent>().handle() :
        rds::alloc<LowPtr<const Func>, rds::Mode::Normal>().handle();

      if (func->isUnique()) ne->setUniqueFunc(func);
      if (RuntimeOption::EvalPerfDataMap) {
        rds::recordRds(
          handle,
          sizeof(LowPtr<const Func>),
          "Func",
          func->name()->slice()
        );
      }
      return handle;
    },
    func
  );
  func->setFuncHandle(ne->m_cachedFunc);
}

///////////////////////////////////////////////////////////////////////////////

struct FrameRestore {
  explicit FrameRestore(const PreClass* preClass) :
      FrameRestore(preClass->unit(), preClass->getOffset()) {}
  explicit FrameRestore(const Unit* unit, Op op, Id id) :
      FrameRestore(unit, (static_cast<size_t>(op) << 32) | id) {}
  explicit NEVER_INLINE FrameRestore(const Unit* unit, size_t offsetOrOp) {
    ActRec* fp = vmfp();

    if (vmsp() && (!fp || fp->m_func->unit() != unit)) {
      m_top = vmsp();
      m_fp = fp;
      m_pc = vmpc();

      /*
        we can be called from Unit::merge, which hasnt yet setup
        the frame (because often it doesnt need to).
        Set up a fake frame here, in case of errors.
        But note that mergeUnit is called for systemlib etc before the
        stack has been setup. So dont do anything if m_stack.top()
        is NULL
      */
      ActRec &tmp = *vmStack().allocA();
      tmp.m_sfp = fp;
      tmp.m_savedRip = 0;
      tmp.m_func = unit->getMain(nullptr);
      tmp.m_soff = !fp
        ? 0
        : fp->m_func->unit()->offsetOf(m_pc) - fp->m_func->base();
      tmp.trashThis();
      tmp.m_varEnv = 0;
      tmp.initNumArgs(0);
      vmfp() = &tmp;
      auto const offset = [&] {
        if (offsetOrOp < kInvalidOffset) return static_cast<Offset>(offsetOrOp);
        auto const op = Op(offsetOrOp >> 32);
        auto const id = Id(offsetOrOp & 0xffffffff);
        auto pc = unit->at(tmp.m_func->base());
        auto const past = unit->at(tmp.m_func->past());
        while (pc < past) {
          if (peek_op(pc) == op) {
            auto tpc = pc;
            decode_op(tpc);
            if (decode_iva(tpc) == id) return unit->offsetOf(pc);
          }
          pc += instrLen(pc);
        }
        return tmp.m_func->base();
      }();
      vmpc() = unit->at(offset);
      pushFrameSlots(tmp.m_func);
    } else {
      m_top = nullptr;
      m_fp = nullptr;
      m_pc = nullptr;
    }
  }
  ~FrameRestore() {
    if (m_top) {
      vmsp() = m_top;
      vmfp() = m_fp;
      vmpc() = m_pc;
    }
  }
 private:
  Cell*   m_top;
  ActRec* m_fp;
  PC      m_pc;
};

///////////////////////////////////////////////////////////////////////////////
// Class lookup.

namespace {
void setupClass(Class* newClass, NamedEntity* nameList) {
  bool const isPersistent =
    (!SystemLib::s_inited || RuntimeOption::RepoAuthoritative) &&
    newClass->verifyPersistent();
  nameList->m_cachedClass.bind(
    isPersistent ? rds::Mode::Persistent : rds::Mode::Normal);

  newClass->setClassHandle(nameList->m_cachedClass);
  newClass->incAtomicCount();

  InstanceBits::ifInitElse(
    [&] { newClass->setInstanceBits();
          nameList->pushClass(newClass); },
    [&] { nameList->pushClass(newClass); }
  );

  if (RuntimeOption::EvalEnableReverseDataMap) {
    // The corresponding deregister is in NamedEntity::removeClass().
    data_map::register_start(newClass);
  }
}
}

Class* Unit::defClass(const PreClass* preClass,
                      bool failIsFatal /* = true */) {
  NamedEntity* const nameList = preClass->namedEntity();
  Class* top = nameList->clsList();

  /*
   * Check if there is already a name defined in this request for this
   * NamedEntity.
   *
   * Raise a fatal unless the existing class definition is identical to the
   * one this invocation would create.
   */
  if (auto current = nameList->getCachedTypeAlias()) {
    FrameRestore fr(preClass);
    raise_error("Cannot declare class with the same name (%s) as an "
                "existing type", current->name->data());
    return nullptr;
  }

  // If there was already a class declared with DefClass, check if it's
  // compatible.
  if (Class* cls = nameList->getCachedClass()) {
    if (cls->preClass() != preClass) {
      if (failIsFatal) {
        FrameRestore fr(preClass);
        raise_error("Class already declared: %s", preClass->name()->data());
      }
      return nullptr;
    }
    return cls;
  }

  // Get a compatible Class, and add it to the list of defined classes.
  Class* parent = nullptr;
  for (;;) {
    // Search for a compatible extant class.  Searching from most to least
    // recently created may have better locality than alternative search orders.
    // In addition, its the only simple way to make this work lock free...
    for (Class* class_ = top; class_ != nullptr; ) {
      Class* cur = class_;
      class_ = class_->m_nextClass;
      if (cur->preClass() != preClass) continue;
      Class::Avail avail = cur->avail(parent, failIsFatal /*tryAutoload*/);
      if (LIKELY(avail == Class::Avail::True)) {
        cur->setCached();
        DEBUGGER_ATTACHED_ONLY(phpDebuggerDefClassHook(cur));
        return cur;
      }
      if (avail == Class::Avail::Fail) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", parent->name()->data());
        }
        return nullptr;
      }
      assertx(avail == Class::Avail::False);
    }

    // Create a new class.
    if (!parent && preClass->parent()->size() != 0) {
      parent = Unit::getClass(preClass->parent(), failIsFatal);
      if (parent == nullptr) {
        if (failIsFatal) {
          FrameRestore fr(preClass);
          raise_error("unknown class %s", preClass->parent()->data());
        }
        return nullptr;
      }
    }

    ClassPtr newClass;
    {
      FrameRestore fr(preClass);
      newClass = Class::newClass(const_cast<PreClass*>(preClass), parent);
    }
    Lock l(g_classesMutex);

    if (UNLIKELY(top != nameList->clsList())) {
      top = nameList->clsList();
      continue;
    }

    setupClass(newClass.get(), nameList);

    /*
     * call setCached after adding to the class list, otherwise the
     * target-cache short circuit at the top could return a class
     * which is not yet on the clsList().
     */
    newClass.get()->setCached();
    DEBUGGER_ATTACHED_ONLY(phpDebuggerDefClassHook(newClass.get()));
    return newClass.get();
  }
}

Class* Unit::defClosure(const PreClass* preClass) {
  auto const nameList = preClass->namedEntity();

  if (nameList->clsList()) return nameList->clsList();

  auto const parent = c_Closure::classof();

  assertx(preClass->parent() == parent->name());
  // Create a new class.

  ClassPtr newClass {
    Class::newClass(const_cast<PreClass*>(preClass), parent)
  };

  Lock l(g_classesMutex);

  if (UNLIKELY(nameList->clsList() != nullptr)) return nameList->clsList();

  setupClass(newClass.get(), nameList);

  if (classHasPersistentRDS(newClass.get())) newClass.get()->setCached();
  return newClass.get();
}

namespace {
bool isPHP7ReservedType(const StringData* alias) {
  return
    !strcmp("int",    alias->data()) ||
    !strcmp("bool",   alias->data()) ||
    !strcmp("float",  alias->data()) ||
    !strcmp("string", alias->data());
}
}

bool Unit::aliasClass(const StringData* original, const StringData* alias,
                      bool autoload) {
  if (RuntimeOption::PHP7_ScalarTypes && isPHP7ReservedType(alias)) {
    raise_error("Fatal error: Cannot use '%s' as class name as it is reserved",
                alias->data());
  }
  auto const origClass =
    autoload ? Unit::loadClass(original)
             : Unit::lookupClass(original);
  if (!origClass) {
    raise_warning("Class %s not found", original->data());
    return false;
  }
  if (origClass->isBuiltin()) {
    raise_warning("First argument of class_alias() must be "
                  "the name of a user defined class");
    return false;
  }

  auto const aliasNe = NamedEntity::get(alias);
  aliasNe->m_cachedClass.bind(rds::Mode::Normal);

  auto const aliasClass = aliasNe->getCachedClass();
  if (aliasClass) {
    raise_warning("Cannot redeclare class %s", alias->data());
    return false;
  }
  aliasNe->setCachedClass(origClass);
  return true;
}

Class* Unit::loadClass(const NamedEntity* ne,
                       const StringData* name) {
  Class* cls;
  if (LIKELY((cls = ne->getCachedClass()) != nullptr)) {
    return cls;
  }
  return loadMissingClass(ne, name);
}

Class* Unit::loadMissingClass(const NamedEntity* ne,
                              const StringData* name) {
  VMRegAnchor _;
  AutoloadHandler::s_instance->autoloadClass(
    StrNR(const_cast<StringData*>(name)));
  return Unit::lookupClass(ne);
}

Class* Unit::getClass(const NamedEntity* ne,
                      const StringData *name, bool tryAutoload) {
  Class *cls = lookupClass(ne);
  if (UNLIKELY(!cls)) {
    if (tryAutoload) {
      return loadMissingClass(ne, name);
    }
  }
  return cls;
}

bool Unit::classExists(const StringData* name, bool autoload, ClassKind kind) {
  Class* cls = Unit::getClass(name, autoload);
  return cls &&
    (cls->attrs() & (AttrInterface | AttrTrait)) == classKindAsAttr(kind);
}


///////////////////////////////////////////////////////////////////////////////
// Constant lookup.

tv_rval Unit::lookupCns(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);

  if (LIKELY(rds::isHandleBound(handle) &&
             rds::isHandleInit(handle))) {
    auto const& tv = rds::handleToRef<TypedValue, rds::Mode::NonLocal>(handle);

    if (LIKELY(tv.m_type != KindOfUninit)) {
      assertx(cellIsPlausible(tv));
      return &tv;
    }

    assertx(tv.m_data.pref != nullptr);
    auto const callback =
      reinterpret_cast<Native::ConstantCallback>(tv.m_data.pref);
    const Cell* tvRet = callback().asTypedValue();
    assertx(cellIsPlausible(*tvRet));
    if (LIKELY(tvRet->m_type != KindOfUninit)) {
      return tvRet;
    }
    return nullptr;
  }
  if (UNLIKELY(rds::s_constants().get() != nullptr)) {
    return rds::s_constants()->rval(cnsName);
  }
  return nullptr;
}

const Cell* Unit::lookupPersistentCns(const StringData* cnsName) {
  auto const handle = lookupCnsHandle(cnsName);
  if (!rds::isHandleBound(handle) || !rds::isPersistentHandle(handle)) {
    return nullptr;
  }
  auto const ret = rds::handleToPtr<Cell, rds::Mode::Persistent>(handle);
  assertx(cellIsPlausible(*ret));
  return ret;
}

tv_rval Unit::loadCns(const StringData* cnsName) {
  auto const tv = lookupCns(cnsName);
  if (LIKELY(tv != nullptr)) return tv;

  if (needsNSNormalization(cnsName)) {
    return loadCns(normalizeNS(cnsName));
  }

  if (!AutoloadHandler::s_instance->autoloadConstant(
        const_cast<StringData*>(cnsName))) {
    return nullptr;
  }
  return lookupCns(cnsName);
}

static bool defCnsHelper(rds::Handle ch,
                         const TypedValue *value,
                         const StringData *cnsName) {
  auto cns = rds::handleToPtr<TypedValue, rds::Mode::NonLocal>(ch);

  if (!rds::isHandleInit(ch)) {
    cns->m_type = KindOfUninit;
    cns->m_data.pref = nullptr;
  }

  if (UNLIKELY(cns->m_type != KindOfUninit ||
               cns->m_data.pref != nullptr)) {
    raise_notice(Strings::CONSTANT_ALREADY_DEFINED, cnsName->data());
    return false;
  }

  if (UNLIKELY(!tvAsCVarRef(value).isAllowedAsConstantValue())) {
    raise_warning(Strings::CONSTANTS_MUST_BE_SCALAR);
    return false;
  }

  assertx(rds::isNormalHandle(ch));
  cellDup(*value, *cns);
  rds::initHandle(ch);
  return true;
}

bool Unit::defCns(const StringData* cnsName, const TypedValue* value) {
  auto const handle = makeCnsHandle(cnsName);

  if (UNLIKELY(!rds::isHandleBound(handle))) {
    if (UNLIKELY(!rds::s_constants().get())) {
      /*
       * This only happens when we call define on a non
       * static string. Not worth presizing or otherwise
       * optimizing for.
       */
      rds::s_constants() =
        Array::attach(PackedArray::MakeReserve(PackedArray::SmallSize));
    }
    auto const existed = !!rds::s_constants()->rval(cnsName);
    if (!existed) {
      rds::s_constants().set(StrNR(cnsName),
        tvAsCVarRef(value), true /* isKey */);
      return true;
    }
    raise_notice(Strings::CONSTANT_ALREADY_DEFINED, cnsName->data());
    return false;
  }
  return defCnsHelper(handle, value, cnsName);
}

bool Unit::defNativeConstantCallback(const StringData* cnsName,
                                     Cell value) {
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
// Type aliases.

namespace {

TypeAliasReq typeAliasFromClass(Unit* unit, const TypeAlias* thisType,
                                Class *klass) {
  assertx(unit);
  TypeAliasReq req;
  req.unit = unit;
  req.name = thisType->name;
  req.nullable = thisType->nullable;
  if (isEnum(klass)) {
    // If the class is an enum, pull out the actual base type.
    if (auto const enumType = klass->enumBaseTy()) {
      req.type = dataTypeToAnnotType(*enumType);
    } else {
      req.type = AnnotType::ArrayKey;
    }
  } else {
    req.type = AnnotType::Object;
    req.klass = klass;
  }
  req.userAttrs = thisType->userAttrs;
  assertx(thisType->typeStructure.isDictOrDArray());
  req.typeStructure = thisType->typeStructure;
  return req;
}

TypeAliasReq resolveTypeAlias(Unit* unit, const TypeAlias* thisType) {
  /*
   * If this type alias is a KindOfObject and the name on the right
   * hand side was another type alias, we will bind the name to the
   * other side for this request (i.e. resolve that type alias now).
   *
   * We need to inspect the right hand side and figure out what it was
   * first.
   *
   * If the right hand side was a class, we need to autoload and
   * ensure it exists at this point.
   */
  if (thisType->type != AnnotType::Object) {
    return TypeAliasReq::From(unit, *thisType);
  }

  /*
   * If the right hand side is already defined, don't invoke the
   * autoloader at all, this means we have to check for both a type
   * alias and a class before attempting to load them via the
   * autoloader.
   *
   * While normal autoloaders are fine, the "failure" entry in the
   * map passed to `HH\set_autoload_paths` is called for all failed
   * lookups. The failure function can do anything, including something
   * like like raising an error or rebuilding the map. We don't want to
   * send speculative (or worse, repeat) requests to the autoloader, so
   * do our due diligence here.
   */

  const StringData* typeName = thisType->value;
  auto targetNE = NamedEntity::get(typeName);

  if (auto klass = Unit::lookupClass(targetNE)) {
    return typeAliasFromClass(unit, thisType, klass);
  }

  if (auto targetTd = targetNE->getCachedTypeAlias()) {
    return TypeAliasReq::From(unit, *targetTd, *thisType);
  }

  if (AutoloadHandler::s_instance->autoloadClassOrType(
        StrNR(const_cast<StringData*>(typeName))
      )) {
    if (auto klass = Unit::lookupClass(targetNE)) {
      return typeAliasFromClass(unit, thisType, klass);
    }
    if (auto targetTd = targetNE->getCachedTypeAlias()) {
      return TypeAliasReq::From(unit, *targetTd, *thisType);
    }
  }

  return TypeAliasReq::Invalid(unit);
}

///////////////////////////////////////////////////////////////////////////////
}

const TypeAliasReq* Unit::lookupTypeAlias(const StringData* name,
                                          bool* persistent) {
  auto ne = NamedEntity::get(name);
  auto target = ne->getCachedTypeAlias();
  if (persistent) *persistent = ne->isPersistentTypeAlias();
  return target;
}

const TypeAliasReq* Unit::loadTypeAlias(const StringData* name,
                                        bool* persistent) {
  auto ne = NamedEntity::get(name);
  auto target = ne->getCachedTypeAlias();
  if (!target) {
    if (AutoloadHandler::s_instance->autoloadClassOrType(
          StrNR(const_cast<StringData*>(name))
        )) {
      target = ne->getCachedTypeAlias();
    } else {
      return nullptr;
    }
  }

  if (persistent) *persistent = ne->isPersistentTypeAlias();
  return target;
}

bool Unit::defTypeAlias(Id id) {
  assertx(id < m_typeAliases.size());
  auto thisType = &m_typeAliases[id];
  auto nameList = NamedEntity::get(thisType->name);
  const StringData* typeName = thisType->value;

  /*
   * Check if this name already was defined as a type alias, and if so
   * make sure it is compatible.
   */
  if (auto current = nameList->getCachedTypeAlias()) {
    auto raiseIncompatible = [&] {
      FrameRestore _(this, Op::DefTypeAlias, id);
      raise_error("The type %s is already defined to an incompatible type",
                  thisType->name->data());
    };
    if (nameList->isPersistentTypeAlias()) {
      // We may have cached the fully resolved type in a previous request.
      if (resolveTypeAlias(this, thisType) != *current) {
        raiseIncompatible();
      }
      return true;
    }
    if (!current->compat(*thisType)) {
      raiseIncompatible();
    }
    return false;
  }

  // There might also be a class with this name already.
  if (nameList->getCachedClass()) {
    FrameRestore _(this, Op::DefTypeAlias, id);
    raise_error("The name %s is already defined as a class",
                thisType->name->data());
    not_reached();
  }

  auto resolved = resolveTypeAlias(this, thisType);
  if (resolved.invalid) {
    FrameRestore _(this, Op::DefTypeAlias, id);
    raise_error("Unknown type or class %s", typeName->data());
    not_reached();
  }

  nameList->m_cachedTypeAlias.bind(
    [thisType, &resolved] {
      auto const persistent = (thisType->attrs & AttrPersistent) &&
        (!resolved.klass || classHasPersistentRDS(resolved.klass));

      auto const handle = persistent ?
        rds::alloc<TypeAliasReq, rds::Mode::Persistent>().handle() :
        rds::alloc<TypeAliasReq, rds::Mode::Normal>().handle();

      rds::recordRds(handle,
                     sizeof(TypeAliasReq),
                     "TypeAlias", thisType->value->slice());
      return handle;
    },
    resolved
  );
  if (nameList->m_cachedTypeAlias.isPersistent()) return true;

  nameList->setCachedTypeAlias(resolved);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Merge.

namespace {
///////////////////////////////////////////////////////////////////////////////

SimpleMutex unitInitLock(false /* reentrant */, RankUnitInit);

void setGlobal(StringData* name, TypedValue *value) {
  g_context->m_globalVarEnv->set(name, value);
}

/*
 * count the number of the EntryPoint in a file, and return a iterator
 * 1) there is not EntryPoint, return the begin()
 * 2) there are multiple EntryPoints, return the end()
 * 3) there is exact one EntryPoint, return that iterator points to the location
 */
Func* findEntryPoint(const Unit* unit) {
  auto it = unit->funcs().begin();
  auto retIt = it;
  bool found = false;
  auto EntryPointTag = makeStaticString("__EntryPoint");
  for (; it != unit->funcs().end(); it++) {
    if ((*it)->userAttributes().count(EntryPointTag)) {
      if (found) {
        raise_fatal_error(
          folly::sformat("There are multiple entryPoint in {}",
                         unit->filepath()->data()).c_str()
        );
      }
      found = true;
      retIt = it;
    }
  }
  if (found)  return *retIt;

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}

void Unit::initialMerge() {
  unitInitLock.assertOwnedBySelf();
  if (m_mergeState.load(std::memory_order_relaxed) != MergeState::Unmerged) {
    return;
  }

  this->m_cachedEntryPoint = findEntryPoint(this);

  if (RuntimeOption::EvalEnableReverseDataMap) {
    data_map::register_start(this);
  }

  int state = 0;
  bool needsCompact = false;
  m_mergeState.store(MergeState::Merging, std::memory_order_relaxed);

  auto const mi = m_mergeInfo.load(std::memory_order_relaxed);
  bool allFuncsUnique = RuntimeOption::RepoAuthoritative;
  for (auto& func : mi->nonMainFuncs()) {
    if (allFuncsUnique) {
      allFuncsUnique = (func->attrs() & AttrUnique);
    }
    bindFunc(func);
    if (rds::isPersistentHandle(func->funcHandle())) {
      needsCompact = true;
    }
  }
  if (allFuncsUnique) state |= MergeState::UniqueFuncs;

  if (RuntimeOption::RepoAuthoritative || !SystemLib::s_inited) {
    /*
     * The mergeables array begins with the hoistable Func*s,
     * followed by the (potentially) hoistable Class*s.
     *
     * If the Unit is merge only, it then contains enough information
     * to simulate executing the pseudomain. Normally, this is just
     * the Class*s that might not be hoistable. In RepoAuthoritative
     * mode it also includes assignments of the form:
     *  $GLOBALS[string-literal] = scalar;
     * defines of the form:
     *  define(string-literal, scalar);
     * and requires.
     *
     * These cases are differentiated using the bottom 3 bits
     * of the pointer. In the case of a define or a global,
     * the pointer will be followed by a TypedValue representing
     * the value being defined/assigned.
     */
    int ix = mi->m_firstHoistablePreClass;
    int end = mi->m_firstMergeablePreClass;
    while (ix < end) {
      PreClass* pre = (PreClass*)mi->mergeableObj(ix++);
      if (pre->attrs() & AttrUnique) {
        needsCompact = true;
      }
    }

    if (isMergeOnly()) {
      ix = mi->m_firstMergeablePreClass;
      end = mi->m_mergeablesSize;
      while (ix < end) {
        void *obj = mi->mergeableObj(ix);
        auto k = MergeKind(uintptr_t(obj) & 7);
        switch (k) {
          case MergeKind::UniqueDefinedClass:
          case MergeKind::Done:
            not_reached();
          case MergeKind::TypeAlias: {
            auto const aliasId = static_cast<Id>(intptr_t(obj)) >> 3;
            if (m_typeAliases[aliasId].attrs & AttrPersistent) {
              needsCompact = true;
            }
            break;
          }
          case MergeKind::Class:
            if (static_cast<PreClass*>(obj)->attrs() & AttrUnique) {
              needsCompact = true;
            }
            break;
          case MergeKind::ReqDoc: {
            StringData* s = (StringData*)((char*)obj - (int)k);
            auto const unit = lookupUnit(
              SourceRootInfo::RelativeToPhpRoot(StrNR(s)).get(),
              "",
              nullptr /* initial_opt */
            );
            unit->initialMerge();
            mi->mergeableObj(ix) = (void*)((char*)unit + (int)k);
            break;
          }
          case MergeKind::PersistentDefine:
            needsCompact = true;
          case MergeKind::Define: {
            auto const s = (StringData*)((char*)obj - (int)k);
            auto const v = (TypedValueAux*)mi->mergeableData(ix + 1);
            if (k == MergeKind::PersistentDefine && bindPersistentCns(s, *v)) {
              Stats::inc(Stats::UnitMerge_mergeable);
              Stats::inc(Stats::UnitMerge_mergeable_persistent_define);
            }
            ix += sizeof(*v) / sizeof(void*);
            v->rdsHandle() = makeCnsHandle(s);
            break;
          }
          case MergeKind::Global:
            // Skip over the value of the global, embedded in mergeableData
            ix += sizeof(TypedValueAux) / sizeof(void*);
            break;
        }
        ix++;
      }
    }
    if (needsCompact) state |= MergeState::NeedsCompact;
  }

  m_mergeState.store(MergeState::Merged | state, std::memory_order_relaxed);
}

void Unit::merge() {
  if (UNLIKELY(!(m_mergeState.load(std::memory_order_relaxed) &
                 MergeState::Merged))) {
    SimpleLock lock(unitInitLock);
    initialMerge();
  }

  if (UNLIKELY(isDebuggerAttached())) {
    mergeImpl<true>(mergeInfo());
  } else {
    mergeImpl<false>(mergeInfo());
  }
}

void* Unit::replaceUnit() const {
  if (isEmpty()) return nullptr;
  if (!isMergeOnly()) return const_cast<Unit*>(this);
  auto const mi = mergeInfo();
  if (mi->m_mergeablesSize == mi->m_firstHoistableFunc + 1) {
    void* obj =
      mi->mergeableObj(mi->m_firstHoistableFunc);
    if (mi->m_firstMergeablePreClass ==
        mi->m_firstHoistableFunc) {
      auto k = MergeKind(uintptr_t(obj) & 7);
      if (k != MergeKind::Class) return obj;
    } else if (mi->m_firstHoistablePreClass ==
               mi->m_firstHoistableFunc) {
      if (uintptr_t(obj) & 1) {
        return (char*)obj - 1 + (int)MergeKind::UniqueDefinedClass;
      }
    }
  }
  return const_cast<Unit*>(this);
}

static size_t compactMergeInfo(Unit::MergeInfo* in, Unit::MergeInfo* out,
                               const FixedVector<TypeAlias>& aliasInfo) {
  using MergeKind = Unit::MergeKind;

  Func** it = in->funcHoistableBegin();
  Func** fend = in->funcEnd();
  Func** iout = 0;
  unsigned ix, end, oix = 0;

  if (out) {
    if (in != out) memcpy(out, in, uintptr_t(it) - uintptr_t(in));
    iout = out->funcHoistableBegin();
  }

  size_t delta = 0;
  while (it != fend) {
    Func* func = *it++;
    if (rds::isPersistentHandle(func->funcHandle())) {
      delta++;
    } else if (iout) {
      *iout++ = func;
    }
  }

  if (out) {
    oix = out->m_firstHoistablePreClass -= delta;
  }

  ix = in->m_firstHoistablePreClass;
  end = in->m_firstMergeablePreClass;
  for (; ix < end; ++ix) {
    void* obj = in->mergeableObj(ix);
    assertx((uintptr_t(obj) & 1) == 0);
    PreClass* pre = (PreClass*)obj;
    if (pre->attrs() & AttrUnique) {
      Class* cls = pre->namedEntity()->clsList();
      assertx(cls && !cls->m_nextClass);
      assertx(cls->preClass() == pre);
      if (rds::isPersistentHandle(cls->classHandle())) {
        delta++;
      } else if (out) {
        out->mergeableObj(oix++) = (void*)(uintptr_t(cls) | 1);
      }
    } else if (out) {
      out->mergeableObj(oix++) = obj;
    }
  }

  if (out) {
    out->m_firstMergeablePreClass = oix;
  }

  end = in->m_mergeablesSize;
  while (ix < end) {
    void* obj = in->mergeableObj(ix++);
    auto k = MergeKind(uintptr_t(obj) & 7);
    switch (k) {
      case MergeKind::Class: {
        PreClass* pre = (PreClass*)obj;
        if (pre->attrs() & AttrUnique) {
          Class* cls = pre->namedEntity()->clsList();
          assertx(cls && !cls->m_nextClass);
          assertx(cls->preClass() == pre);
          if (rds::isPersistentHandle(cls->classHandle())) {
            delta++;
          } else if (out) {
            out->mergeableObj(oix++) = (void*)
              (uintptr_t(cls) | uintptr_t(MergeKind::UniqueDefinedClass));
          }
        } else if (out) {
          out->mergeableObj(oix++) = obj;
        }
        break;
      }
      case MergeKind::TypeAlias: {
        auto const aliasId = static_cast<Id>(intptr_t(obj)) >> 3;
        if (aliasInfo[aliasId].attrs & AttrPersistent) {
          delta++;
        } else if (out) {
          out->mergeableObj(oix++) = obj;
        }
        break;
      }
      case MergeKind::UniqueDefinedClass:
        not_reached();

      case MergeKind::PersistentDefine: {
        auto const v = (TypedValueAux*)in->mergeableData(ix);
        if (rds::isPersistentHandle(v->rdsHandle())) {
          delta += 1 + sizeof(TypedValueAux) / sizeof(void*);
          ix += sizeof(TypedValueAux) / sizeof(void*);
          break;
        }
        obj = (char*)obj - (int)MergeKind::PersistentDefine +
          (int)MergeKind::Define;
        // fall through
      }
      case MergeKind::Define:
      case MergeKind::Global:
        if (out) {
          out->mergeableObj(oix++) = obj;
          *(TypedValueAux*)out->mergeableData(oix) =
            *(TypedValueAux*)in->mergeableData(ix);
          oix += sizeof(TypedValueAux) / sizeof(void*);
        }
        ix += sizeof(TypedValueAux) / sizeof(void*);
        break;

      case MergeKind::ReqDoc: {
        Unit *unit = (Unit*)((char*)obj - (int)k);
        void *rep = unit->replaceUnit();
        if (!rep) {
          delta++;
        } else if (out) {
          if (rep == unit) {
            out->mergeableObj(oix++) = obj;
          } else {
            out->mergeableObj(oix++) = rep;
          }
        }
        break;
      }
      case MergeKind::Done:
        not_reached();
    }
  }
  if (out) {
    // copy the MergeKind::Done marker
    out->mergeableObj(oix) = in->mergeableObj(ix);
    out->m_mergeablesSize = oix;
  }
  return delta;
}

template <bool debugger>
void Unit::mergeImpl(MergeInfo* mi) {
  assertx(m_mergeState.load(std::memory_order_relaxed) & MergeState::Merged);
  autoTypecheck(this);

  Func** it = mi->funcHoistableBegin();
  Func** fend = mi->funcEnd();
  if (it != fend) {
    if (LIKELY((m_mergeState.load(std::memory_order_relaxed) &
                MergeState::UniqueFuncs) != 0)) {
      do {
        Func* func = *it;
        assertx(func->top());
        assertx(func->isUnique());
        auto const handle = func->funcHandle();
        if (rds::isNormalHandle(handle)) {
          rds::handleToRef<LowPtr<Func>, rds::Mode::Normal>(handle) = func;
          rds::initHandle(handle);
        } else {
          assertx(rds::isPersistentHandle(handle));
          rds::handleToRef<LowPtr<Func>, rds::Mode::Persistent>(handle) = func;
        }
        func->getNamedEntity()->setUniqueFunc(func);
        if (debugger) phpDebuggerDefFuncHook(func);
      } while (++it != fend);
    } else {
      do {
        Func* func = *it;
        assertx(func->top());
        defFunc(func, debugger);
      } while (++it != fend);
    }
  }

  bool redoHoistable = false;
  int ix = mi->m_firstHoistablePreClass;
  int end = mi->m_firstMergeablePreClass;
  // iterate over all the potentially hoistable classes
  // with no fatals on failure
  if (ix < end) {
    do {
      // The first time this unit is merged, if the classes turn out to be all
      // unique and defined, we replace the PreClass*'s with the corresponding
      // Class*'s, with the low-order bit marked.
      PreClass* pre = (PreClass*)mi->mergeableObj(ix);
      if (LIKELY(uintptr_t(pre) & 1)) {
        Stats::inc(Stats::UnitMerge_hoistable);
        Class* cls = (Class*)(uintptr_t(pre) & ~1);
        auto const handle = cls->classHandle();
        auto const handle_persistent = rds::isPersistentHandle(handle);
        if (cls->isPersistent()) {
          Stats::inc(Stats::UnitMerge_hoistable_persistent);
        }
        if (Stats::enabled() && handle_persistent) {
          Stats::inc(Stats::UnitMerge_hoistable_persistent_cache);
        }
        if (Class* parent = cls->parent()) {
          auto const parent_handle = parent->classHandle();
          auto const parent_handle_persistent =
            rds::isPersistentHandle(parent_handle);
          if (parent->isPersistent()) {
            Stats::inc(Stats::UnitMerge_hoistable_persistent_parent);
          }
          if (Stats::enabled() && parent_handle_persistent) {
            Stats::inc(Stats::UnitMerge_hoistable_persistent_parent_cache);
          }
          auto const parent_cls_present =
            rds::isHandleInit(parent_handle) &&
            rds::handleToRef<LowPtr<Class>, rds::Mode::NonLocal>(parent_handle);
          if (UNLIKELY(!parent_cls_present)) {
            redoHoistable = true;
            continue;
          }
        }
        if (handle_persistent) {
          rds::handleToRef<LowPtr<Class>, rds::Mode::Persistent>(handle) = cls;
        } else {
          assertx(rds::isNormalHandle(handle));
          rds::handleToRef<LowPtr<Class>, rds::Mode::Normal>(handle) = cls;
          rds::initHandle(handle);
        }
        if (debugger) phpDebuggerDefClassHook(cls);
      } else {
        if (UNLIKELY(!defClass(pre, false))) {
          redoHoistable = true;
        }
      }
    } while (++ix < end);

    if (UNLIKELY(redoHoistable)) {
      // if this unit isnt mergeOnly, we're done
      if (!isMergeOnly()) return;

      // As a special case, if all the classes are potentially hoistable, we
      // don't list them twice, but instead iterate over them again.
      //
      // At first glance, it may seem like we could leave the maybe-hoistable
      // classes out of the second list and then always reset ix to 0; but that
      // gets this case wrong if there's an autoloader for C, and C extends B:
      //
      // class A {}
      // class B implements I {}
      // class D extends C {}
      //
      // because now A and D go on the maybe-hoistable list B goes on the never
      // hoistable list, and we fatal trying to instantiate D before B
      Stats::inc(Stats::UnitMerge_redo_hoistable);
      if (end == (int)mi->m_mergeablesSize) {
        ix = mi->m_firstHoistablePreClass;
        do {
          void* obj = mi->mergeableObj(ix);
          if (UNLIKELY(uintptr_t(obj) & 1)) {
            Class* cls = (Class*)(uintptr_t(obj) & ~1);
            defClass(cls->preClass(), true);
          } else {
            defClass((PreClass*)obj, true);
          }
        } while (++ix < end);
        return;
      }
    }
  }

  // iterate over all but the guaranteed hoistable classes
  // fataling if we fail.
  void* obj = mi->mergeableObj(ix);
  auto k = MergeKind(uintptr_t(obj) & 7);
  do {
    switch (k) {
      case MergeKind::Class:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_class);
          defClass((PreClass*)obj, true);
          obj = mi->mergeableObj(++ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::Class);
        continue;

      case MergeKind::UniqueDefinedClass:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_unique);
          Class* other = nullptr;
          Class* cls = (Class*)((char*)obj - (int)k);
          auto const handle = cls->classHandle();
          auto const handle_persistent = rds::isPersistentHandle(handle);
          if (cls->isPersistent()) {
            Stats::inc(Stats::UnitMerge_mergeable_unique_persistent);
          }
          if (Stats::enabled() && handle_persistent) {
            Stats::inc(Stats::UnitMerge_mergeable_unique_persistent_cache);
          }
          Class::Avail avail = cls->avail(other, true);
          if (UNLIKELY(avail == Class::Avail::Fail)) {
            raise_error("unknown class %s", other->name()->data());
          }
          assertx(avail == Class::Avail::True);
          if (handle_persistent) {
            rds::handleToRef<LowPtr<Class>,
                             rds::Mode::Persistent>(handle) = cls;
          } else {
            assertx(rds::isNormalHandle(handle));
            rds::handleToRef<LowPtr<Class>,
                             rds::Mode::Normal>(handle) = cls;
            rds::initHandle(handle);
          }
          if (debugger) phpDebuggerDefClassHook(cls);
          obj = mi->mergeableObj(++ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::UniqueDefinedClass);
        continue;

      case MergeKind::PersistentDefine:
        // will be removed by compactMergeInfo but will be hit at
        // least once before that happens.
        do {
          auto const v = (TypedValueAux*)mi->mergeableData(ix + 1);
          if (!rds::isPersistentHandle(v->rdsHandle())) {
            obj = (char*)obj - (int)MergeKind::PersistentDefine +
              (int)MergeKind::Define;
            k = MergeKind::Define;
            break;
          }
          ix += 1 + sizeof(TypedValueAux) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::PersistentDefine);
        continue;

      case MergeKind::Define:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_define);

          auto const name = (StringData*)((char*)obj - (int)k);
          auto const v = (TypedValueAux*)mi->mergeableData(ix + 1);
          assertx(v->m_type != KindOfUninit);

          auto const handle = v->rdsHandle();
          assertx(rds::isNormalHandle(handle));
          if (UNLIKELY(rds::isHandleInit(handle, rds::NormalTag{}))) {
            raise_notice(Strings::CONSTANT_ALREADY_DEFINED, name->data());
          } else {
            rds::handleToRef<TypedValue, rds::Mode::Normal>(handle) = *v;
            rds::initHandle(handle);
          }
          ix += 1 + sizeof(*v) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::Define);
        continue;

      case MergeKind::Global:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_global);
          StringData* name = (StringData*)((char*)obj - (int)k);
          auto* v = (TypedValueAux*)mi->mergeableData(ix + 1);
          setGlobal(name, v);
          ix += 1 + sizeof(*v) / sizeof(void*);
          obj = mi->mergeableObj(ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::Global);
        continue;

      case MergeKind::ReqDoc:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_require);
          Unit *unit = (Unit*)((char*)obj - (int)k);

          unit->mergeImpl<debugger>(unit->mergeInfo());
          if (UNLIKELY(!unit->isMergeOnly())) {
            Stats::inc(Stats::PseudoMain_Reentered);
            VarEnv* ve = nullptr;
            ActRec* fp = vmfp();
            if (!fp) {
              ve = g_context->m_globalVarEnv;
            } else {
              if ((fp->func()->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
                ve = fp->m_varEnv;
              } else {
                // Nothing to do. If there is no varEnv, the enclosing
                // file was called by fb_autoload_map, which wants a
                // local scope.
              }
            }
            tvDecRefGen(
              g_context->invokeFunc(unit->getMain(nullptr),
                                    init_null_variant,
                                    nullptr, nullptr, ve)
            );
          } else {
            Stats::inc(Stats::PseudoMain_SkipDeep);
          }

          obj = mi->mergeableObj(++ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::ReqDoc);
        continue;
      case MergeKind::TypeAlias:
        do {
          Stats::inc(Stats::UnitMerge_mergeable);
          Stats::inc(Stats::UnitMerge_mergeable_typealias);
          auto const aliasId = static_cast<Id>(intptr_t(obj)) >> 3;
          if (!defTypeAlias(aliasId)) {
            auto& attrs = m_typeAliases[aliasId].attrs;
            if (attrs & AttrPersistent) {
              attrs = static_cast<Attr>(attrs & ~AttrPersistent);
            }
          }
          obj = mi->mergeableObj(++ix);
          k = MergeKind(uintptr_t(obj) & 7);
        } while (k == MergeKind::TypeAlias);
        continue;
      case MergeKind::Done:
        assertx((unsigned)ix == mi->m_mergeablesSize);
        if (UNLIKELY(m_mergeState.load(std::memory_order_relaxed) &
                     MergeState::NeedsCompact)) {
          SimpleLock lock(unitInitLock);
          if (!(m_mergeState.load(std::memory_order_relaxed) &
                MergeState::NeedsCompact)) {
            return;
          }
          if (!redoHoistable) {
            /*
             * All the classes are known to be unique, and we just got
             * here, so all were successfully defined. We can now go
             * back and convert all MergeKind::Class entries to
             * MergeKind::UniqueDefinedClass, and all hoistable
             * classes to their Class*'s instead of PreClass*'s.
             *
             * We can also remove any Persistent Class/Func*'s,
             * and any requires of modules that are (now) empty
             */
            size_t delta = compactMergeInfo(mi, nullptr, m_typeAliases);
            MergeInfo* newMi = mi;
            if (delta) {
              newMi = MergeInfo::alloc(mi->m_mergeablesSize - delta);
            }
            /*
             * In the case where mi == newMi, there's an apparent
             * race here. Although we have a lock, so we're the only
             * ones modifying this, there could be any number of
             * readers. But thats ok, because it doesnt matter
             * whether they see the old contents or the new.
             */
            compactMergeInfo(mi, newMi, m_typeAliases);
            if (newMi != mi) {
              this->m_mergeInfo.store(newMi, std::memory_order_release);
              Treadmill::deferredFree(mi);
              if (isMergeOnly() &&
                  newMi->m_firstHoistableFunc == newMi->m_mergeablesSize) {
                m_mergeState.fetch_or(MergeState::Empty,
                                      std::memory_order_relaxed);
              }
            }
            assertx(newMi->m_firstMergeablePreClass
                      == newMi->m_mergeablesSize ||
                   isMergeOnly());
          }
          m_mergeState.fetch_and(~MergeState::NeedsCompact,
                                 std::memory_order_relaxed);
        }
        return;
    }
    // Normal cases should continue, KindDone returns
    not_reached();
  } while (true);
}

///////////////////////////////////////////////////////////////////////////////
// Info arrays.

namespace {

Array getClassesWithAttrInfo(Attr attrs, bool inverse = false) {
  Array a = Array::Create();
  NamedEntity::foreach_cached_class([&](Class* c) {
    if ((c->attrs() & attrs) ? !inverse : inverse) {
      if (c->isBuiltin()) {
        a.prepend(make_tv<KindOfPersistentString>(c->name()));
      } else {
        a.append(make_tv<KindOfPersistentString>(c->name()));
      }
    }
  });
  return a;
}

template<bool system>
Array getFunctions() {
  // Return an array of all defined functions.  This method is used
  // to support get_defined_functions().
  Array a = Array::Create();
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

void Unit::prettyPrint(std::ostream& out, PrintOpts opts) const {
  auto startOffset = opts.startOffset != kInvalidOffset
    ? opts.startOffset : 0;
  auto stopOffset = opts.stopOffset != kInvalidOffset
    ? opts.stopOffset : m_bclen;

  std::map<Offset,const Func*> funcMap;
  for (auto& func : funcs()) {
    funcMap[func->base()] = func;
  }
  for (auto it = m_preClasses.begin();
      it != m_preClasses.end(); ++it) {
    Func* const* methods = (*it)->methods();
    size_t const numMethods = (*it)->numMethods();
    for (size_t i = 0; i < numMethods; ++i) {
      funcMap[methods[i]->base()] = methods[i];
    }
  }

  auto funcIt = funcMap.lower_bound(startOffset);

  const auto* it = &m_bc[startOffset];
  int prevLineNum = -1;
  while (it < &m_bc[stopOffset]) {
    if (opts.showFuncs) {
      assertx(funcIt == funcMap.end() || funcIt->first >= offsetOf(it));
      if (funcIt != funcMap.end() &&
          funcIt->first == offsetOf(it)) {
        out.put('\n');
        funcIt->second->prettyPrint(out);
        ++funcIt;
        prevLineNum = -1;
      }
    }

    if (opts.showLines) {
      int lineNum = getLineNumber(offsetOf(it));
      if (lineNum != prevLineNum) {
        out << "  // line " << lineNum << std::endl;
        prevLineNum = lineNum;
      }
    }

    out << std::string(opts.indentSize, ' ')
        << std::setw(4) << (it - m_bc) << ": "
        << instrToString(it, this)
        << std::endl;
    it += instrLen(it);
  }
}

std::string Unit::toString() const {
  std::ostringstream ss;
  prettyPrint(ss);
  for (auto& pc : m_preClasses) {
    pc->prettyPrint(ss);
  }
  for (auto& func : funcs()) {
    func->prettyPrint(ss);
  }
  return ss.str();
}


///////////////////////////////////////////////////////////////////////////////
// Other methods.

bool Unit::compileTimeFatal(const StringData*& msg, int& line) const {
  auto entry = getMain(nullptr)->getEntry();
  auto pc = entry;
  // String <id>; Fatal;
  // ^^^^^^
  if (decode_op(pc) != Op::String) {
    return false;
  }
  // String <id>; Fatal;
  //        ^^^^
  Id id = *(Id*)pc;
  pc += sizeof(Id);
  // String <id>; Fatal;
  //              ^^^^^
  if (decode_op(pc) != Op::Fatal) {
    return false;
  }
  msg = lookupLitstrId(id);
  line = getLineNumber(Offset(pc - entry));
  return true;
}

bool Unit::parseFatal(const StringData*& msg, int& line) const {
  if (!compileTimeFatal(msg, line)) {
    return false;
  }

  auto pc = getMain(nullptr)->getEntry();

  // String <id>
  decode_op(pc);
  pc += sizeof(Id);

  // Fatal <kind>
  decode_op(pc);
  auto kind_char = *pc;
  return kind_char == static_cast<uint8_t>(FatalOp::Parse);
}
}
