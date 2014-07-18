/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/compiler/option.h"
#include "hphp/parser/location.h"
#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/ext/std/ext_std_variable.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/litstr-table.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/verifier/check.h"

#include "hphp/util/md5.h"
#include "hphp/util/read-only-arena.h"
#include "hphp/util/trace.h"

#include <boost/algorithm/string/predicate.hpp>

#include <folly/Memory.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

using MergeKind = Unit::MergeKind;

///////////////////////////////////////////////////////////////////////////////

static ReadOnlyArena& get_readonly_arena() {
  static ReadOnlyArena arena(RuntimeOption::EvalHHBCArenaChunkSize);
  return arena;
}

/*
 * Export for the admin server.
 */
size_t hhbc_arena_capacity() {
  if (!RuntimeOption::RepoAuthoritative) return 0;
  return get_readonly_arena().capacity();
}

///////////////////////////////////////////////////////////////////////////////

UnitEmitter::UnitEmitter(const MD5& md5)
  : m_mainReturn(make_tv<KindOfUninit>())
  , m_md5(md5)
  , m_bc((unsigned char*)malloc(BCMaxInit))
  , m_bclen(0)
  , m_bcmax(BCMaxInit)
  , m_nextFuncSn(0)
  , m_allClassesHoistable(true)
{}

UnitEmitter::~UnitEmitter() {
  if (m_bc) free(m_bc);

  for (auto& fe : m_fes) delete fe;
  for (auto& pce : m_pceVec) delete pce;
}


///////////////////////////////////////////////////////////////////////////////
// Basic data.

void UnitEmitter::setBc(const unsigned char* bc, size_t bclen) {
  if (m_bc) {
    free(m_bc);
  }
  m_bc = (unsigned char*)malloc(bclen);
  m_bcmax = bclen;
  memcpy(m_bc, bc, bclen);
  m_bclen = bclen;
}


///////////////////////////////////////////////////////////////////////////////
// Litstrs and Arrays.

const StringData* UnitEmitter::lookupLitstr(Id id) const {
  if (isGlobalLitstrId(id)) {
    return LitstrTable::get().lookupLitstrId(decodeGlobalLitstrId(id));
  }
  assert(id < m_litstrs.size());
  return m_litstrs[id];
}

const ArrayData* UnitEmitter::lookupArray(Id id) const {
  assert(id < m_arrays.size());
  return m_arrays[id].array;
}

Id UnitEmitter::mergeLitstr(const StringData* litstr) {
  if (Option::WholeProgram) {
    return encodeGlobalLitstrId(LitstrTable::get().mergeLitstr(litstr));
  }
  return mergeUnitLitstr(litstr);
}

Id UnitEmitter::mergeUnitLitstr(const StringData* litstr) {
  auto it = m_litstr2id.find(litstr);
  if (it == m_litstr2id.end()) {
    const StringData* str = makeStaticString(litstr);
    Id id = m_litstrs.size();
    m_litstrs.push_back(str);
    m_litstr2id[str] = id;
    return id;
  } else {
    return it->second;
  }
}

Id UnitEmitter::mergeArray(const ArrayData* a) {
  Variant v(const_cast<ArrayData*>(a));
  auto key = HHVM_FN(serialize)(v).toCppString();
  return mergeArray(a, key);
}

Id UnitEmitter::mergeArray(const ArrayData* a, const std::string& key) {
  auto const it = m_array2id.find(key);
  if (it != m_array2id.end()) {
    return it->second;
  }
  a = ArrayData::GetScalarArray(const_cast<ArrayData*>(a), key);
  Id id = m_arrays.size();
  ArrayVecElm ave = {key, a};
  m_arrays.push_back(ave);
  m_array2id[key] = id;
  return id;
}


///////////////////////////////////////////////////////////////////////////////
// FuncEmitters.

void UnitEmitter::initMain(int line1, int line2) {
  assert(m_fes.size() == 0);
  StringData* name = makeStaticString("");
  FuncEmitter* pseudomain = newFuncEmitter(name);
  Attr attrs = AttrMayUseVV;
  pseudomain->init(line1, line2, 0, attrs, false, name);
}

void UnitEmitter::addTrivialPseudoMain() {
  initMain(0, 0);
  auto const mfe = getMain();
  emitOp(OpInt);
  emitInt64(1);
  emitOp(OpRetC);
  mfe->maxStackCells = 1;
  mfe->finish(bcPos(), false);
  recordFunction(mfe);

  TypedValue mainReturn;
  mainReturn.m_data.num = 1;
  mainReturn.m_type = KindOfInt64;
  m_mainReturn = mainReturn;
  m_mergeOnly = true;
}

FuncEmitter* UnitEmitter::newFuncEmitter(const StringData* name) {
  // The pseudomain comes first.
  assert(m_fes.size() > 0 || !strcmp(name->data(), ""));

  FuncEmitter* fe = new FuncEmitter(*this, m_nextFuncSn++, m_fes.size(), name);
  m_fes.push_back(fe);
  return fe;
}

FuncEmitter* UnitEmitter::newMethodEmitter(const StringData* name,
                                           PreClassEmitter* pce) {
  return new FuncEmitter(*this, m_nextFuncSn++, name, pce);
}

void UnitEmitter::appendTopEmitter(FuncEmitter* fe) {
  fe->setIds(m_nextFuncSn++, m_fes.size());
  m_fes.push_back(fe);
}

void UnitEmitter::recordFunction(FuncEmitter* fe) {
  m_feTab.push_back(std::make_pair(fe->past, fe));
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit,
                           PreClass* preClass, int line1, int line2,
                           Offset base, Offset past,
                           const StringData* name, Attr attrs, bool top,
                           const StringData* docComment, int numParams,
                           bool needsNextClonedClosure) {
  Func* f = new (Func::allocFuncMem(name, numParams,
                                    needsNextClonedClosure,
                                    !preClass))
    Func(unit, preClass, line1, line2, base, past, name,
         attrs, top, docComment, numParams);
  m_fMap[fe] = f;
  return f;
}


///////////////////////////////////////////////////////////////////////////////
// PreClassEmitters.

PreClassEmitter* UnitEmitter::newPreClassEmitter(
  const StringData* name,
  PreClass::Hoistable hoistable
) {
  if (hoistable && m_hoistablePreClassSet.count(name)) {
    hoistable = PreClass::Mergeable;
  }

  PreClassEmitter* pce = new PreClassEmitter(*this, m_pceVec.size(),
                                             name, hoistable);

  if (hoistable >= PreClass::MaybeHoistable) {
    m_hoistablePreClassSet.insert(name);
    if (hoistable == PreClass::ClosureHoistable) {
      // Closures should appear at the VERY top of the file, so if any class in
      // the same file tries to use them, they are already defined. We had a
      // fun race where one thread was autoloading a file, finished parsing the
      // class, then another thread came along and saw the class was already
      // loaded and ran it before the first thread had time to parse the
      // closure class.
      m_hoistablePceIdList.push_front(pce->id());
    } else {
      m_hoistablePceIdList.push_back(pce->id());
    }
  } else {
    m_allClassesHoistable = false;
  }
  if (hoistable >= PreClass::Mergeable &&
      hoistable < PreClass::AlwaysHoistable) {
    if (m_returnSeen) {
      m_allClassesHoistable = false;
    } else {
      pushMergeableClass(pce);
    }
  }
  m_pceVec.push_back(pce);
  return pce;
}


///////////////////////////////////////////////////////////////////////////////
// Type aliases.

Id UnitEmitter::addTypeAlias(const TypeAlias& td) {
  Id id = m_typeAliases.size();
  m_typeAliases.push_back(td);
  return id;
}


///////////////////////////////////////////////////////////////////////////////
// Source locations.

SourceLocTable UnitEmitter::createSourceLocTable() const {
  SourceLocTable locations;
  for (size_t i = 0; i < m_sourceLocTab.size(); ++i) {
    Offset endOff = i < m_sourceLocTab.size() - 1
      ? m_sourceLocTab[i + 1].first
      : m_bclen;
    locations.push_back(SourceLocEntry(endOff, m_sourceLocTab[i].second));
  }
  return locations;
}

namespace {

using SrcLoc = std::vector<std::pair<Offset, SourceLoc>>;

/*
 * Create a LineTable from `srcLoc'.
 */
LineTable createLineTable(SrcLoc& srcLoc, Offset bclen) {
  LineTable lines;
  for (size_t i = 0; i < srcLoc.size(); ++i) {
    Offset endOff = i < srcLoc.size() - 1 ? srcLoc[i + 1].first : bclen;
    lines.push_back(LineEntry(endOff, srcLoc[i].second.line1));
  }
  return lines;
}

/*
 * Create a LineToOffsetRangeVecMap from `srcLoc'.
 */
LineToOffsetRangeVecMap createLineToOffsetMap(SrcLoc& srcLoc,
                                              Offset bclen) {
  LineToOffsetRangeVecMap map;
  for (size_t i = 0; i < srcLoc.size(); ++i) {
    Offset baseOff = srcLoc[i].first;
    Offset endOff = i < srcLoc.size() - 1 ? srcLoc[i + 1].first : bclen;
    OffsetRange range(baseOff, endOff);
    auto line0 = srcLoc[i].second.line0;
    auto line1 = srcLoc[i].second.line1;
    for (int line = line0; line <= line1; line++) {
      auto it = map.find(line);
      if (it != map.end()) {
        it->second.push_back(range);
      } else {
        OffsetRangeVec v(1);
        v.push_back(range);
        map[line] = v;
      }
    }
  }
  return map;
}

}

void UnitEmitter::recordSourceLocation(const Location* sLoc, Offset start) {
  // Some byte codes, such as for the implicit "return 0" at the end of a
  // a source file do not have valid source locations. This check makes
  // sure we don't record a (dummy) source location in this case.
  if (start > 0 && sLoc->line0 == 1 && sLoc->char0 == 1 &&
    sLoc->line1 == 1 && sLoc->char1 == 1 && strlen(sLoc->file) == 0) return;
  SourceLoc newLoc(*sLoc);
  if (!m_sourceLocTab.empty()) {
    if (m_sourceLocTab.back().second == newLoc) {
      // Combine into the interval already at the back of the vector.
      assert(start >= m_sourceLocTab.back().first);
      return;
    }
    assert(m_sourceLocTab.back().first < start &&
           "source location offsets must be added to UnitEmitter in "
           "increasing order");
  } else {
    // First record added should be for bytecode offset zero.
    assert(start == 0);
  }
  m_sourceLocTab.push_back(std::make_pair(start, newLoc));
}


///////////////////////////////////////////////////////////////////////////////
// Mergeables.

void UnitEmitter::pushMergeableClass(PreClassEmitter* e) {
  m_mergeableStmts.push_back(std::make_pair(MergeKind::Class, e->id()));
}

void UnitEmitter::pushMergeableInclude(Unit::MergeKind kind,
                                       const StringData* unitName) {
  m_mergeableStmts.push_back(
    std::make_pair(kind, mergeLitstr(unitName)));
  m_allClassesHoistable = false;
}

void UnitEmitter::insertMergeableInclude(int ix, Unit::MergeKind kind, Id id) {
  assert(size_t(ix) <= m_mergeableStmts.size());
  m_mergeableStmts.insert(m_mergeableStmts.begin() + ix,
                          std::make_pair(kind, id));
  m_allClassesHoistable = false;
}

void UnitEmitter::pushMergeableDef(Unit::MergeKind kind,
                                   const StringData* name,
                                   const TypedValue& tv) {
  m_mergeableStmts.push_back(std::make_pair(kind, m_mergeableValues.size()));
  m_mergeableValues.push_back(std::make_pair(mergeLitstr(name), tv));
  m_allClassesHoistable = false;
}

void UnitEmitter::insertMergeableDef(int ix, Unit::MergeKind kind,
                                     Id id, const TypedValue& tv) {
  assert(size_t(ix) <= m_mergeableStmts.size());
  m_mergeableStmts.insert(m_mergeableStmts.begin() + ix,
                          std::make_pair(kind, m_mergeableValues.size()));
  m_mergeableValues.push_back(std::make_pair(id, tv));
  m_allClassesHoistable = false;
}


///////////////////////////////////////////////////////////////////////////////
// Initialization and execution.

void UnitEmitter::commit(UnitOrigin unitOrigin) {
  Repo& repo = Repo::get();
  try {
    RepoTxn txn(repo);
    bool err = insert(unitOrigin, txn);
    if (!err) {
      txn.commit();
    }
  } catch (RepoExc& re) {
    int repoId = repo.repoIdForNewUnit(unitOrigin);
    if (repoId != RepoIdInvalid) {
      TRACE(3, "Failed to commit '%s' (0x%016" PRIx64 "%016" PRIx64 ") to '%s': %s\n",
               m_filepath->data(), m_md5.q[0], m_md5.q[1],
               repo.repoName(repoId).c_str(), re.msg().c_str());
    }
  }
}

bool UnitEmitter::insert(UnitOrigin unitOrigin, RepoTxn& txn) {
  Repo& repo = Repo::get();
  UnitRepoProxy& urp = repo.urp();
  int repoId = Repo::get().repoIdForNewUnit(unitOrigin);
  if (repoId == RepoIdInvalid) {
    return true;
  }
  m_repoId = repoId;

  try {
    {
      auto lines = createLineTable(m_sourceLocTab, m_bclen);
      urp.insertUnit(repoId).insert(txn, m_sn, m_md5, m_bc, m_bclen,
                                    &m_mainReturn, m_mergeOnly, lines,
                                    m_typeAliases);
    }
    int64_t usn = m_sn;
    for (unsigned i = 0; i < m_litstrs.size(); ++i) {
      urp.insertUnitLitstr(repoId).insert(txn, usn, i, m_litstrs[i]);
    }
    for (unsigned i = 0; i < m_arrays.size(); ++i) {
      urp.insertUnitArray(repoId).insert(txn, usn, i, m_arrays[i].serialized);
    }
    for (auto& fe : m_fes) {
      fe->commit(txn);
    }
    for (auto& pce : m_pceVec) {
      pce->commit(txn);
    }

    for (int i = 0, n = m_mergeableStmts.size(); i < n; i++) {
      switch (m_mergeableStmts[i].first) {
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          not_reached();
        case MergeKind::Class: break;
        case MergeKind::ReqDoc: {
          urp.insertUnitMergeable(repoId).insert(
            txn, usn, i,
            m_mergeableStmts[i].first, m_mergeableStmts[i].second, nullptr);
          break;
        }
        case MergeKind::Define:
        case MergeKind::PersistentDefine:
        case MergeKind::Global: {
          int ix = m_mergeableStmts[i].second;
          urp.insertUnitMergeable(repoId).insert(
            txn, usn, i,
            m_mergeableStmts[i].first,
            m_mergeableValues[ix].first, &m_mergeableValues[ix].second);
          break;
        }
      }
    }
    if (RuntimeOption::RepoDebugInfo) {
      for (size_t i = 0; i < m_sourceLocTab.size(); ++i) {
        SourceLoc& e = m_sourceLocTab[i].second;
        Offset endOff = i < m_sourceLocTab.size() - 1
                          ? m_sourceLocTab[i + 1].first
                          : m_bclen;

        urp.insertUnitSourceLoc(repoId)
           .insert(txn, usn, endOff, e.line0, e.char0, e.line1, e.char1);
      }
    }
    return false;
  } catch (RepoExc& re) {
    TRACE(3, "Failed to commit '%s' (0x%016" PRIx64 "%016" PRIx64 ") to '%s': %s\n",
             m_filepath->data(), m_md5.q[0], m_md5.q[1],
             repo.repoName(repoId).c_str(), re.msg().c_str());
    return true;
  }
}

static const unsigned char*
allocateBCRegion(const unsigned char* bc, size_t bclen) {
  if (RuntimeOption::RepoAuthoritative) {
    // In RepoAuthoritative, we assume we won't ever deallocate units
    // and that this is read-only, mostly cold data.  So we throw it
    // in a bump-allocator that's mprotect'd to prevent writes.
    return static_cast<const unsigned char*>(
      get_readonly_arena().allocate(bc, bclen)
    );
  }
  auto mem = static_cast<unsigned char*>(malloc(bclen));
  std::copy(bc, bc + bclen, mem);
  return mem;
}

Unit* UnitEmitter::create() {
  Unit* u = new Unit();
  u->m_repoId = m_repoId;
  u->m_sn = m_sn;
  u->m_bc = allocateBCRegion(m_bc, m_bclen);
  u->m_bclen = m_bclen;
  u->m_filepath = m_filepath;
  u->m_mainReturn = m_mainReturn;
  u->m_mergeOnly = m_mergeOnly;
  {
    const std::string& dirname = FileUtil::safe_dirname(m_filepath->data(),
                                                        m_filepath->size());
    u->m_dirpath = makeStaticString(dirname);
  }
  u->m_md5 = m_md5;
  for (unsigned i = 0; i < m_litstrs.size(); ++i) {
    NamedEntityPair np;
    np.first = m_litstrs[i];
    np.second = nullptr;
    u->m_namedInfo.push_back(np);
  }
  for (unsigned i = 0; i < m_arrays.size(); ++i) {
    u->m_arrays.push_back(m_arrays[i].array);
  }
  for (auto const& pce : m_pceVec) {
    u->m_preClasses.push_back(PreClassPtr(pce->create(*u)));
  }
  u->m_typeAliases = m_typeAliases;

  size_t ix = m_fes.size() + m_hoistablePceIdList.size();
  if (m_mergeOnly && !m_allClassesHoistable) {
    size_t extra = 0;
    for (auto& mergeable : m_mergeableStmts) {
      extra++;
      if (!RuntimeOption::RepoAuthoritative && SystemLib::s_inited) {
        if (mergeable.first != MergeKind::Class) {
          extra = 0;
          u->m_mergeOnly = false;
          break;
        }
      } else switch (mergeable.first) {
          case MergeKind::PersistentDefine:
          case MergeKind::Define:
          case MergeKind::Global:
            extra += sizeof(TypedValueAux) / sizeof(void*);
            break;
          default:
            break;
        }
    }
    ix += extra;
  }
  Unit::MergeInfo *mi = Unit::MergeInfo::alloc(ix);
  u->m_mergeInfo = mi;
  ix = 0;
  for (auto& fe : m_fes) {
    Func* func = fe->create(*u);
    if (func->top()) {
      if (!mi->m_firstHoistableFunc) {
        mi->m_firstHoistableFunc = ix;
      }
    } else {
      assert(!mi->m_firstHoistableFunc);
    }
    mi->mergeableObj(ix++) = func;
  }
  assert(u->getMain()->isPseudoMain());
  if (!mi->m_firstHoistableFunc) {
    mi->m_firstHoistableFunc =  ix;
  }
  mi->m_firstHoistablePreClass = ix;
  assert(m_fes.size());
  for (auto& id : m_hoistablePceIdList) {
    mi->mergeableObj(ix++) = u->m_preClasses[id].get();
  }
  mi->m_firstMergeablePreClass = ix;
  if (u->m_mergeOnly && !m_allClassesHoistable) {
    for (auto& mergeable : m_mergeableStmts) {
      switch (mergeable.first) {
        case MergeKind::Class:
          mi->mergeableObj(ix++) = u->m_preClasses[mergeable.second].get();
          break;
        case MergeKind::ReqDoc: {
          assert(RuntimeOption::RepoAuthoritative);
          void* name = u->lookupLitstrId(mergeable.second);
          mi->mergeableObj(ix++) = (char*)name + (int)mergeable.first;
          break;
        }
        case MergeKind::Define:
        case MergeKind::Global:
          assert(RuntimeOption::RepoAuthoritative);
        case MergeKind::PersistentDefine: {
          void* name = u->lookupLitstrId
            (m_mergeableValues[mergeable.second].first);
          mi->mergeableObj(ix++) = (char*)name + (int)mergeable.first;
          auto& tv = m_mergeableValues[mergeable.second].second;
          auto* tva = (TypedValueAux*)mi->mergeableData(ix);
          tva->m_data = tv.m_data;
          tva->m_type = tv.m_type;
          // leave tva->m_aux uninitialized
          ix += sizeof(*tva) / sizeof(void*);
          assert(sizeof(*tva) % sizeof(void*) == 0);
          break;
        }
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          not_reached();
      }
    }
  }
  assert(ix == mi->m_mergeablesSize);
  mi->mergeableObj(ix) = (void*)MergeKind::Done;
  u->m_sourceLocTable = createSourceLocTable();
  if (m_lineTable.size() == 0) {
    u->m_lineTable = createLineTable(m_sourceLocTab, m_bclen);
  } else {
    u->m_lineTable = m_lineTable;
  }
  u->m_lineToOffsetRangeVecMap = createLineToOffsetMap(m_sourceLocTab, m_bclen);
  for (size_t i = 0; i < m_feTab.size(); ++i) {
    assert(m_feTab[i].second->past == m_feTab[i].first);
    assert(m_fMap.find(m_feTab[i].second) != m_fMap.end());
    u->m_funcTable.push_back(
      FuncEntry(m_feTab[i].first, m_fMap.find(m_feTab[i].second)->second));
  }

  // Funcs can be recorded out of order when loading them from the
  // repo currently.  So sort 'em here.
  std::sort(u->m_funcTable.begin(), u->m_funcTable.end());

  m_fMap.clear();

  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    Trace::traceRelease("%s", u->toString().c_str());
  }
  if (RuntimeOption::EvalDumpHhas && SystemLib::s_inited) {
    std::printf("%s", disassemble(u).c_str());
    std::fflush(stdout);
    _Exit(0);
  }

  static const bool kVerify = debug || getenv("HHVM_VERIFY");
  static const bool kVerifyVerboseSystem =
    getenv("HHVM_VERIFY_VERBOSE_SYSTEM");
  static const bool kVerifyVerbose =
    kVerifyVerboseSystem || getenv("HHVM_VERIFY_VERBOSE");

  const bool isSystemLib = u->filepath()->empty() ||
    boost::ends_with(u->filepath()->data(), "systemlib.php");
  const bool doVerify =
    kVerify || boost::ends_with(u->filepath()->data(), "hhas");
  if (doVerify) {
    Verifier::checkUnit(
      u,
      isSystemLib ? kVerifyVerboseSystem : kVerifyVerbose
    );
  }

  return u;
}


///////////////////////////////////////////////////////////////////////////////
// UnitRepoProxy.

UnitRepoProxy::UnitRepoProxy(Repo& repo)
  : RepoProxy(repo)
#define URP_OP(c, o) \
  , m_##o##Local(repo, RepoIdLocal), m_##o##Central(repo, RepoIdCentral)
    URP_OPS
#undef URP_OP
{
#define URP_OP(c, o) \
  m_##o[RepoIdLocal] = &m_##o##Local; \
  m_##o[RepoIdCentral] = &m_##o##Central;
  URP_OPS
#undef URP_OP
}

UnitRepoProxy::~UnitRepoProxy() {
}

void UnitRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "Unit")
             << "(unitSn INTEGER PRIMARY KEY, md5 BLOB, bc BLOB, data BLOB,"
                "UNIQUE (md5));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitLitstr")
             << "(unitSn INTEGER, litstrId INTEGER, litstr TEXT,"
                " PRIMARY KEY (unitSn, litstrId));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitArray")
             << "(unitSn INTEGER, arrayId INTEGER, array BLOB,"
                " PRIMARY KEY (unitSn, arrayId));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitMergeables")
             << "(unitSn INTEGER, mergeableIx INTEGER,"
                " mergeableKind INTEGER, mergeableId INTEGER,"
                " mergeableValue BLOB,"
                " PRIMARY KEY (unitSn, mergeableIx));";
    txn.exec(ssCreate.str());
  }
  {
    std::stringstream ssCreate;
    ssCreate << "CREATE TABLE " << m_repo.table(repoId, "UnitSourceLoc")
             << "(unitSn INTEGER, pastOffset INTEGER, line0 INTEGER,"
                " char0 INTEGER, line1 INTEGER, char1 INTEGER,"
                " PRIMARY KEY (unitSn, pastOffset));";
    txn.exec(ssCreate.str());
  }
}

bool UnitRepoProxy::loadHelper(UnitEmitter& ue,
                               const std::string& name,
                               const MD5& md5) {
  ue.m_filepath = makeStaticString(name);
  // Look for a repo that contains a unit with matching MD5.
  int repoId;
  for (repoId = RepoIdCount - 1; repoId >= 0; --repoId) {
    if (!getUnit(repoId).get(ue, md5)) {
      break;
    }
  }
  if (repoId < 0) {
    TRACE(3, "No repo contains '%s' (0x%016" PRIx64  "%016" PRIx64 ")\n",
             name.c_str(), md5.q[0], md5.q[1]);
    return false;
  }
  try {
    getUnitLitstrs(repoId).get(ue);
    getUnitArrays(repoId).get(ue);
    m_repo.pcrp().getPreClasses(repoId).get(ue);
    getUnitMergeables(repoId).get(ue);
    m_repo.frp().getFuncs(repoId).get(ue);
  } catch (RepoExc& re) {
    TRACE(0,
          "Repo error loading '%s' (0x%016" PRIx64 "%016"
          PRIx64 ") from '%s': %s\n",
          name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str(),
          re.msg().c_str());
    return false;
  }
  TRACE(3, "Repo loaded '%s' (0x%016" PRIx64 "%016" PRIx64 ") from '%s'\n",
           name.c_str(), md5.q[0], md5.q[1], m_repo.repoName(repoId).c_str());
  return true;
}

std::unique_ptr<UnitEmitter>
UnitRepoProxy::loadEmitter(const std::string& name, const MD5& md5) {
  auto ue = folly::make_unique<UnitEmitter>(md5);
  if (!loadHelper(*ue, name, md5)) ue.reset();
  return ue;
}

Unit* UnitRepoProxy::load(const std::string& name, const MD5& md5) {
  UnitEmitter ue(md5);
  if (!loadHelper(ue, name, md5)) return nullptr;
  return ue.create();
}

void UnitRepoProxy::InsertUnitStmt
                  ::insert(RepoTxn& txn, int64_t& unitSn, const MD5& md5,
                           const unsigned char* bc, size_t bclen,
                           const TypedValue* mainReturn, bool mergeOnly,
                           const LineTable& lines,
                           const std::vector<TypeAlias>& typeAliases) {
  BlobEncoder dataBlob;

  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "Unit")
             << " VALUES(NULL, @md5, @bc, @data);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindMd5("@md5", md5);
  query.bindBlob("@bc", (const void*)bc, bclen);
  dataBlob(*mainReturn)
          (mergeOnly)
          (lines)
          (typeAliases);
  query.bindBlob("@data", dataBlob, /* static */ true);
  query.exec();
  unitSn = query.getInsertedRowid();
}

bool UnitRepoProxy::GetUnitStmt
                  ::get(UnitEmitter& ue, const MD5& md5) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT unitSn,bc,data FROM "
               << m_repo.table(m_repoId, "Unit")
               << " WHERE md5 == @md5;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindMd5("@md5", md5);
    query.step();
    if (!query.row()) {
      return true;
    }
    int64_t unitSn;                          /**/ query.getInt64(0, unitSn);
    const void* bc; size_t bclen;            /**/ query.getBlob(1, bc, bclen);
    BlobDecoder dataBlob =                   /**/ query.getBlob(2);

    ue.m_repoId = m_repoId;
    ue.m_sn = unitSn;
    ue.setBc((const unsigned char*)bc, bclen);

    TypedValue mainReturn;
    bool mergeOnly;
    LineTable lines;

    dataBlob(mainReturn)
            (mergeOnly)
            (lines)
            (ue.m_typeAliases);

    ue.m_mainReturn = mainReturn;
    ue.m_mergeOnly = mergeOnly;
    ue.m_lineTable = lines;

    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

void UnitRepoProxy::InsertUnitLitstrStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Id litstrId,
                           const StringData* litstr) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitLitstr")
             << " VALUES(@unitSn, @litstrId, @litstr);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@litstrId", litstrId);
  query.bindStaticString("@litstr", litstr);
  query.exec();
}

void UnitRepoProxy::GetUnitLitstrsStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT litstrId,litstr FROM "
             << m_repo.table(m_repoId, "UnitLitstr")
             << " WHERE unitSn == @unitSn ORDER BY litstrId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Id litstrId;        /**/ query.getId(0, litstrId);
      StringData* litstr; /**/ query.getStaticString(1, litstr);
      Id id UNUSED = ue.mergeUnitLitstr(litstr);
      assert(id == litstrId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitArrayStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Id arrayId,
                           const std::string& array) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitArray")
             << " VALUES(@unitSn, @arrayId, @array);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@arrayId", arrayId);
  query.bindStdString("@array", array);
  query.exec();
}

void UnitRepoProxy::GetUnitArraysStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT arrayId,array FROM "
             << m_repo.table(m_repoId, "UnitArray")
             << " WHERE unitSn == @unitSn ORDER BY arrayId ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Id arrayId;        /**/ query.getId(0, arrayId);
      std::string key;   /**/ query.getStdString(1, key);
      Variant v = unserialize_from_buffer(key.data(), key.size());
      Id id UNUSED = ue.mergeArray(v.asArrRef().get(), key);
      assert(id == arrayId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitMergeableStmt
                  ::insert(RepoTxn& txn, int64_t unitSn,
                           int ix, Unit::MergeKind kind, Id id,
                           TypedValue* value) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitMergeables")
             << " VALUES(@unitSn, @mergeableIx, @mergeableKind,"
                " @mergeableId, @mergeableValue);";
    txn.prepare(*this, ssInsert.str());
  }

  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@mergeableIx", ix);
  query.bindInt("@mergeableKind", (int)kind);
  query.bindId("@mergeableId", id);
  if (value) {
    assert(kind == MergeKind::Define ||
           kind == MergeKind::PersistentDefine ||
           kind == MergeKind::Global);
    query.bindTypedValue("@mergeableValue", *value);
  } else {
    assert(kind == MergeKind::ReqDoc);
    query.bindNull("@mergeableValue");
  }
  query.exec();
}

void UnitRepoProxy::GetUnitMergeablesStmt
                  ::get(UnitEmitter& ue) {
  RepoTxn txn(m_repo);
  if (!prepared()) {
    std::stringstream ssSelect;
    ssSelect << "SELECT mergeableIx,mergeableKind,mergeableId,mergeableValue"
                " FROM "
             << m_repo.table(m_repoId, "UnitMergeables")
             << " WHERE unitSn == @unitSn ORDER BY mergeableIx ASC;";
    txn.prepare(*this, ssSelect.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      int mergeableIx;           /**/ query.getInt(0, mergeableIx);
      int mergeableKind;         /**/ query.getInt(1, mergeableKind);
      Id mergeableId;            /**/ query.getInt(2, mergeableId);

      auto k = MergeKind(mergeableKind);

      if (UNLIKELY(!RuntimeOption::RepoAuthoritative)) {
        /*
         * We're using a repo generated in WholeProgram mode,
         * but we're not using it in RepoAuthoritative mode
         * (this is dodgy to start with). We're not going to
         * deal with requires at merge time, so drop them
         * here, and clear the mergeOnly flag for the unit.
         * The one exception is persistent constants are allowed in systemlib.
         */
        if (k != MergeKind::PersistentDefine || SystemLib::s_inited) {
          ue.m_mergeOnly = false;
        }
      }
      switch (k) {
        case MergeKind::ReqDoc:
          ue.insertMergeableInclude(mergeableIx, k, mergeableId);
          break;
        case MergeKind::PersistentDefine:
        case MergeKind::Define:
        case MergeKind::Global: {
          TypedValue mergeableValue; /**/ query.getTypedValue(3,
                                                              mergeableValue);
          ue.insertMergeableDef(mergeableIx, k, mergeableId, mergeableValue);
          break;
        }
        default: break;
      }
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitSourceLocStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Offset pastOffset,
                           int line0, int char0, int line1, int char1) {
  if (!prepared()) {
    std::stringstream ssInsert;
    ssInsert << "INSERT INTO " << m_repo.table(m_repoId, "UnitSourceLoc")
             << " VALUES(@unitSn, @pastOffset, @line0, @char0, @line1,"
                " @char1);";
    txn.prepare(*this, ssInsert.str());
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindOffset("@pastOffset", pastOffset);
  query.bindInt("@line0", line0);
  query.bindInt("@char0", char0);
  query.bindInt("@line1", line1);
  query.bindInt("@char1", char1);
  query.exec();
}

bool UnitRepoProxy::GetSourceLocTabStmt
     ::get(int64_t unitSn, SourceLocTable& sourceLocTab) {
  try {
    RepoTxn txn(m_repo);
    if (!prepared()) {
      std::stringstream ssSelect;
      ssSelect << "SELECT pastOffset,line0,char0,line1,char1 FROM "
               << m_repo.table(m_repoId, "UnitSourceLoc")
               << " WHERE unitSn == @unitSn"
                  " ORDER BY pastOffset ASC;";
      txn.prepare(*this, ssSelect.str());
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    do {
      query.step();
      if (!query.row()) {
        return true;
      }
      Offset pastOffset;
      query.getOffset(0, pastOffset);
      SourceLoc sLoc;
      query.getInt(1, sLoc.line0);
      query.getInt(2, sLoc.char0);
      query.getInt(3, sLoc.line1);
      query.getInt(4, sLoc.char1);
      SourceLocEntry entry(pastOffset, sLoc);
      sourceLocTab.push_back(entry);
    } while (!query.done());
    txn.commit();
  } catch (RepoExc& re) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
