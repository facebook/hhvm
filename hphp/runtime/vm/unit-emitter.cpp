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

#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/parser/location.h"
#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/std/ext_std_variable.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/litstr-table.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record-emitter.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/verifier/check.h"

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/read-only-arena.h"
#include "hphp/util/sha1.h"
#include "hphp/util/trace.h"

#include <boost/algorithm/string/predicate.hpp>

#include <folly/Memory.h>
#include <folly/FileUtil.h>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

using MergeKind = Unit::MergeKind;

///////////////////////////////////////////////////////////////////////////////

using BytecodeArena = ReadOnlyArena<VMColdAllocator<char>, false, 8>;
static BytecodeArena& bytecode_arena() {
  static BytecodeArena arena(RuntimeOption::EvalHHBCArenaChunkSize);
  return arena;
}

/*
 * Export for the admin server.
 */
size_t hhbc_arena_capacity() {
  if (!RuntimeOption::RepoAuthoritative) return 0;
  return bytecode_arena().capacity();
}

///////////////////////////////////////////////////////////////////////////////

UnitEmitter::UnitEmitter(const SHA1& sha1,
                         const SHA1& bcSha1,
                         const Native::FuncTable& nativeFuncs,
                         bool useGlobalIds)
  : m_useGlobalIds(useGlobalIds)
  , m_nativeFuncs(nativeFuncs)
  , m_sha1(sha1)
  , m_bcSha1(bcSha1)
  , m_bc((unsigned char*)malloc(BCMaxInit))
  , m_bclen(0)
  , m_bcmax(BCMaxInit)
  , m_nextFuncSn(0)
  , m_allClassesHoistable(true)
{}

UnitEmitter::~UnitEmitter() {
  if (m_bc) free(m_bc);

  for (auto& pce : m_pceVec) delete pce;
  for (auto& re : m_reVec) delete re;
}


///////////////////////////////////////////////////////////////////////////////
// Basic data.

void UnitEmitter::setBc(const unsigned char* bc, size_t bclen) {
  if (m_bc) {
    free(m_bc);
  }
  m_bc = (unsigned char*)malloc(bclen);
  m_bcmax = bclen;
  if (bclen) memcpy(m_bc, bc, bclen);
  m_bclen = bclen;
}


///////////////////////////////////////////////////////////////////////////////
// Litstrs and Arrays.

const StringData* UnitEmitter::lookupLitstr(Id id) const {
  if (!isUnitLitstrId(id)) {
    return LitstrTable::get().lookupLitstrId(id);
  }
  auto unitId = decodeUnitLitstrId(id);
  assertx(unitId < m_litstrs.size());
  return m_litstrs[unitId];
}

const ArrayData* UnitEmitter::lookupArray(Id id) const {
  assertx(id < m_arrays.size());
  return m_arrays[id];
}

const RepoAuthType::Array* UnitEmitter::lookupArrayType(Id id) const {
  return RuntimeOption::RepoAuthoritative
           ? globalArrayTypeTable().lookup(id)
           : m_arrayTypeTable.lookup(id);
}

void UnitEmitter::repopulateArrayTypeTable(
  const ArrayTypeTable::Builder& builder) {
  m_arrayTypeTable.repopulate(builder);
}

Id UnitEmitter::mergeLitstr(const StringData* litstr) {
  if (m_useGlobalIds) {
    return LitstrTable::get().mergeLitstr(litstr);
  }
  return encodeUnitLitstrId(mergeUnitLitstr(litstr));
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
  assertx(a->isStatic());
  auto const id = static_cast<Id>(m_arrays.size());
  m_array2id.emplace(a, id);
  m_arrays.push_back(a);
  return id;
}


///////////////////////////////////////////////////////////////////////////////
// FuncEmitters.

FuncEmitter* UnitEmitter::newFuncEmitter(const StringData* name) {
  auto fe = std::make_unique<FuncEmitter>(*this, m_nextFuncSn++, m_fes.size(),
                                          name);
  m_fes.push_back(std::move(fe));
  return m_fes.back().get();
}

FuncEmitter* UnitEmitter::newMethodEmitter(const StringData* name,
                                           PreClassEmitter* pce) {
  return new FuncEmitter(*this, m_nextFuncSn++, name, pce);
}

void UnitEmitter::appendTopEmitter(std::unique_ptr<FuncEmitter>&& fe) {
  fe->setIds(m_nextFuncSn++, m_fes.size());
  m_fes.push_back(std::move(fe));
}

Func* UnitEmitter::newFunc(const FuncEmitter* fe, Unit& unit,
                           const StringData* name, Attr attrs,
                           int numParams) {
  Func *func = nullptr;
  if (attrs & AttrIsMethCaller) {
    auto const pair = Func::getMethCallerNames(name);
    func = new (Func::allocFuncMem(numParams)) Func(
      unit, name, attrs, pair.first, pair.second);
  } else {
    func = new (Func::allocFuncMem(numParams)) Func(unit, name, attrs);
  }
  if (unit.m_extended) unit.getExtended()->m_funcTable.push_back(func);
  return func;
}


///////////////////////////////////////////////////////////////////////////////
// PreClassEmitters.

void UnitEmitter::addPreClassEmitter(PreClassEmitter* pce) {
  if (pce->hoistability() && m_hoistablePreClassSet.count(pce->name())) {
    pce->setHoistable(PreClass::Mergeable);
  }
  auto hoistable = pce->hoistability();
  if (hoistable <= PreClass::Mergeable) m_allClassesHoistable = false;
  if (hoistable >= PreClass::MaybeHoistable) {
    m_hoistablePreClassSet.insert(pce->name());
    m_hoistablePceIdList.push_back(pce->id());
  }
  if (hoistable >= PreClass::Mergeable &&
      hoistable < PreClass::AlwaysHoistable) {
    pushMergeableClass(pce);
  }
}

PreClassEmitter* UnitEmitter::newBarePreClassEmitter(
  const std::string& name,
  PreClass::Hoistable hoistable
) {
  auto pce = new PreClassEmitter(*this, m_pceVec.size(), name, hoistable);
  m_pceVec.push_back(pce);
  return pce;
}

PreClassEmitter* UnitEmitter::newPreClassEmitter(
  const std::string& name,
  PreClass::Hoistable hoistable
) {
  PreClassEmitter* pce = newBarePreClassEmitter(name, hoistable);
  addPreClassEmitter(pce);
  return pce;
}

RecordEmitter* UnitEmitter::newRecordEmitter(const std::string& name) {
  auto const re = new RecordEmitter(*this, m_reVec.size(), name);
  m_reVec.push_back(re);
  return re;
}

Id UnitEmitter::pceId(folly::StringPiece clsName) {
  Id id = 0;
  for (auto p : m_pceVec) {
    if (p->name()->slice() == clsName) return id;
    id++;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Type aliases.

TypeAliasEmitter* UnitEmitter::newTypeAliasEmitter(const std::string& name) {
  auto te = std::make_unique<TypeAliasEmitter>(*this, m_typeAliases.size(), name);
  m_typeAliases.push_back(std::move(te));
  return m_typeAliases.back().get();
}

///////////////////////////////////////////////////////////////////////////////
// Constants.

Id UnitEmitter::addConstant(const Constant& c) {
  Id id = m_constants.size();
  TRACE(1, "Add Constant %d %s %d\n", id, c.name->data(), c.attrs);
  m_constants.push_back(c);
  return id;
}

///////////////////////////////////////////////////////////////////////////////
// Source locations.

SourceLocTable UnitEmitter::createSourceLocTable() const {
  assertx(m_sourceLocTab.size() != 0);
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
LineTable createLineTable(const SrcLoc& srcLoc, Offset bclen) {
  LineTable lines;
  if (srcLoc.empty()) {
    return lines;
  }

  auto prev = srcLoc.begin();
  for (auto it = prev + 1; it != srcLoc.end(); ++it) {
    if (prev->second.line1 != it->second.line1) {
      lines.push_back(LineEntry(it->first, prev->second.line1));
      prev = it;
    }
  }

  lines.push_back(LineEntry(bclen, prev->second.line1));
  return lines;
}

}

void UnitEmitter::recordSourceLocation(const Location::Range& sLoc,
                                       Offset start) {
  // Some byte codes, such as for the implicit "return 0" at the end of a
  // a source file do not have valid source locations. This check makes
  // sure we don't record a (dummy) source location in this case.
  if (start > 0 && sLoc.line0 == -1) return;
  SourceLoc newLoc(sLoc);
  if (!m_sourceLocTab.empty()) {
    if (m_sourceLocTab.back().second == newLoc) {
      // Combine into the interval already at the back of the vector.
      assertx(start >= m_sourceLocTab.back().first);
      return;
    }
    assertx(m_sourceLocTab.back().first < start &&
           "source location offsets must be added to UnitEmitter in "
           "increasing order");
  } else {
    // First record added should be for bytecode offset zero or very rarely one
    // when the source starts with a label and a Nop is inserted.
    assertx(start == 0 || start == 1);
  }
  m_sourceLocTab.push_back(std::make_pair(start, newLoc));
}


///////////////////////////////////////////////////////////////////////////////
// Mergeables.

void UnitEmitter::pushMergeableClass(PreClassEmitter* e) {
  m_mergeableStmts.push_back(std::make_pair(MergeKind::Class, e->id()));
}

void UnitEmitter::pushMergeableId(Unit::MergeKind kind, const Id id) {
  m_mergeableStmts.push_back(std::make_pair(kind, id));
  m_allClassesHoistable = false;
}

void UnitEmitter::insertMergeableId(Unit::MergeKind kind, int ix, const Id id) {
  assertx(size_t(ix) <= m_mergeableStmts.size());
  m_mergeableStmts.insert(m_mergeableStmts.begin() + ix,
                          std::make_pair(kind, id));
  m_allClassesHoistable = false;
}

void UnitEmitter::pushMergeableRecord(const Id id) {
  m_mergeableStmts.push_back(std::make_pair(Unit::MergeKind::Record, id));
  m_allClassesHoistable = false;
}

void UnitEmitter::insertMergeableRecord(int ix, const Id id) {
  assertx(size_t(ix) <= m_mergeableStmts.size());
  m_mergeableStmts.insert(m_mergeableStmts.begin() + ix,
                          std::make_pair(Unit::MergeKind::Record, id));
  m_allClassesHoistable = false;
}

///////////////////////////////////////////////////////////////////////////////
// Initialization and execution.

void UnitEmitter::commit(UnitOrigin unitOrigin, bool usePreAllocatedUnitSn) {
  Repo& repo = Repo::get();
  try {
    auto txn = RepoTxn{repo.begin()};
    RepoStatus err = insert(unitOrigin, txn, usePreAllocatedUnitSn);
    if (err == RepoStatus::success) {
      txn.commit();
    }
  } catch (RepoExc& re) {
    tracing::addPointNoTrace("ue-commit-exn");
    int repoId = repo.repoIdForNewUnit(unitOrigin);
    if (repoId != RepoIdInvalid) {
      TRACE(3, "Failed to commit '%s' (%s) to '%s': %s\n",
               m_filepath->data(), m_sha1.toString().c_str(),
               repo.repoName(repoId).c_str(), re.msg().c_str());
    }
  }
}

RepoStatus UnitEmitter::insert(UnitOrigin unitOrigin, RepoTxn& txn,
                               bool usePreAllocatedUnitSn) {
  Repo& repo = Repo::get();
  UnitRepoProxy& urp = repo.urp();
  int repoId = Repo::get().repoIdForNewUnit(unitOrigin);
  if (repoId == RepoIdInvalid) {
    return RepoStatus::error;
  }
  m_repoId = repoId;

  try {
    {
      if (!m_sourceLocTab.empty()) {
        m_lineTable = createLineTable(m_sourceLocTab, m_bclen);
      }
      urp.insertUnit[repoId].insert(*this, txn, m_sn, m_sha1, m_bc,
                                    m_bclen, usePreAllocatedUnitSn);
    }
    int64_t usn = m_sn;
    urp.insertUnitLineTable[repoId].insert(txn, usn, m_lineTable);
    for (unsigned i = 0; i < m_litstrs.size(); ++i) {
      urp.insertUnitLitstr[repoId].insert(txn, usn, i, m_litstrs[i]);
    }
    for (unsigned i = 0; i < m_typeAliases.size(); ++i) {
      urp.insertUnitTypeAlias[repoId].insert(*this, txn, usn, i,
                                             *m_typeAliases[i]);
    }
    for (unsigned i = 0; i < m_constants.size(); ++i) {
      urp.insertUnitConstant[repoId].insert(*this, txn, usn, i, m_constants[i]);
    }
    for (unsigned i = 0; i < m_arrays.size(); ++i) {
      // We check that arrays do not exceed a configurable maximum size in the
      // assembler, so just assume that they're okay here.
      MemoryManager::SuppressOOM so(*tl_heap);

      auto const arr_str = [&]{
        VariableSerializer vs{VariableSerializer::Type::Internal};
        vs.setUnitFilename(m_filepath);
        return vs.serializeValue(
          VarNR(const_cast<ArrayData*>(m_arrays[i])), false
        ).toCppString();
      }();

      urp.insertUnitArray[repoId].insert(txn, usn, i, arr_str);
    }
    urp.insertUnitArrayTypeTable[repoId].insert(txn, usn, *this);
    for (auto& fe : m_fes) {
      fe->commit(txn);
    }
    for (auto& pce : m_pceVec) {
      pce->commit(txn);
    }
    for (auto& re : m_reVec) {
      re->commit(txn);
    }

    for (int i = 0, n = m_mergeableStmts.size(); i < n; i++) {
      switch (m_mergeableStmts[i].first) {
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          not_reached();
        case MergeKind::Class:
          break;
        case MergeKind::TypeAlias:
        case MergeKind::Record:
        case MergeKind::Define: {
          urp.insertUnitMergeable[repoId].insert(
            txn, usn, i,
            m_mergeableStmts[i].first, m_mergeableStmts[i].second);
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

        urp.insertUnitSourceLoc[repoId]
           .insert(txn, usn, endOff, e.line0, e.char0, e.line1, e.char1);
      }
    }
    return RepoStatus::success;
  } catch (RepoExc& re) {
    TRACE(3, "Failed to commit '%s' (%s) to '%s': %s\n",
             m_filepath->data(), m_sha1.toString().c_str(),
             repo.repoName(repoId).c_str(), re.msg().c_str());
    return RepoStatus::error;
  }
}

ServiceData::ExportedTimeSeries* g_hhbc_size = ServiceData::createTimeSeries(
  "vm.hhbc-size",
  {ServiceData::StatsType::AVG,
   ServiceData::StatsType::SUM,
   ServiceData::StatsType::COUNT}
);

static const unsigned char*
allocateBCRegion(const unsigned char* bc, size_t bclen) {
  g_hhbc_size->addValue(bclen);
  auto mem = static_cast<unsigned char*>(
    RuntimeOption::RepoAuthoritative ? bytecode_arena().allocate(bclen)
                                     : malloc(bclen));
  std::copy(bc, bc + bclen, mem);
  return mem;
}

bool UnitEmitter::check(bool verbose) const {
  return Verifier::checkUnit(
    this,
    verbose ? Verifier::kVerbose : Verifier::kStderr
  );
}

bool needs_extended_line_table() {
  return RuntimeOption::RepoDebugInfo &&
    (RuntimeOption::EvalDumpHhas ||
     RuntimeOption::EnableHphpdDebugger ||
     RuntimeOption::EnableVSDebugger ||
     RuntimeOption::EnableDebuggerServer);
}

std::unique_ptr<Unit> UnitEmitter::create(bool saveLineTable) const {
  INC_TPC(unit_load);

  tracing::BlockNoTrace _{"unit-create"};

  static const bool kVerify = debug || RuntimeOption::EvalVerify ||
    RuntimeOption::EvalVerifyOnly || RuntimeOption::EvalFatalOnVerifyError;
  static const bool kVerifyVerboseSystem =
    getenv("HHVM_VERIFY_VERBOSE_SYSTEM");
  static const bool kVerifyVerbose =
    kVerifyVerboseSystem || getenv("HHVM_VERIFY_VERBOSE");

  const bool isSystemLib = FileUtil::isSystemName(m_filepath->slice());
  const bool doVerify =
    kVerify || boost::ends_with(m_filepath->data(), ".hhas");
  if (doVerify) {
    auto const verbose = isSystemLib ? kVerifyVerboseSystem : kVerifyVerbose;
    if (!check(verbose)) {
      if (!verbose) {
        std::cerr << folly::format(
          "Verification failed for unit {}. Re-run with "
          "HHVM_VERIFY_VERBOSE{}=1 to see more details.\n",
          m_filepath->data(), isSystemLib ? "_SYSTEM" : ""
        );
      }
      if (RuntimeOption::EvalVerifyOnly) {
        if (!isSystemLib) {
          std::fflush(stdout);
          _Exit(1);
        }
      } else if (RuntimeOption::EvalFatalOnVerifyError) {
        return createFatalUnit(
          const_cast<StringData*>(m_filepath),
          m_sha1,
          FatalOp::Parse,
          "A bytecode verification error was detected"
        )->create(saveLineTable);
      }
    }
    if (!isSystemLib && RuntimeOption::EvalVerifyOnly) {
      std::fflush(stdout);
      _Exit(0);
    }
  }

  std::unique_ptr<Unit> u {
    RuntimeOption::RepoAuthoritative && !RuntimeOption::SandboxMode &&
      m_litstrs.empty() && m_arrayTypeTable.empty() ?
    new Unit : new UnitExtended
  };

  if (m_fatalUnit) {
    FatalInfo info{m_fatalLoc, m_fatalOp, m_fatalMsg};
    u->m_fatalInfo = std::make_unique<FatalInfo>(info);
  }

  u->m_repoId = saveLineTable ? RepoIdInvalid : m_repoId;
  u->m_sn = m_sn;
  u->m_bc = allocateBCRegion(m_bc, m_bclen);
  u->m_bclen = m_bclen;
  u->m_filepath = m_filepath;
  u->m_dirpath = makeStaticString(FileUtil::dirname(StrNR{m_filepath}));
  u->m_sha1 = m_sha1;
  u->m_bcSha1 = m_bcSha1;
  u->m_arrays = m_arrays;
  for (auto const& pce : m_pceVec) {
    u->m_preClasses.push_back(PreClassPtr(pce->create(*u)));
  }
  for (auto const& re : m_reVec) {
    u->m_preRecords.push_back(PreRecordDescPtr(re->create(*u)));
  }
  for (auto const& te : m_typeAliases) {
    u->m_typeAliases.push_back(te->create(*u));
  }
  u->m_constants = m_constants;
  u->m_metaData = m_metaData;
  u->m_fileAttributes = m_fileAttributes;
  u->m_ICE = m_ICE;

  size_t ix = m_fes.size() + m_hoistablePceIdList.size();
  if (!m_allClassesHoistable) ix += m_mergeableStmts.size();
  Unit::MergeInfo *mi = Unit::MergeInfo::alloc(ix);
  u->m_mergeInfo.store(mi, std::memory_order_relaxed);
  ix = 0;
  for (auto& fe : m_fes) {
    auto const func = fe->create(*u);
    assertx(ix == fe->id());
    mi->mergeableObj(ix++) = func;
  }
  mi->m_firstHoistablePreClass = ix;
  for (auto& id : m_hoistablePceIdList) {
    mi->mergeableObj(ix++) = u->m_preClasses[id].get();
  }
  mi->m_firstMergeablePreClass = ix;
  if (!m_allClassesHoistable) {
    for (auto& mergeable : m_mergeableStmts) {
      switch (mergeable.first) {
        case MergeKind::Class:
          mi->mergeableObj(ix++) = u->m_preClasses[mergeable.second].get();
          break;
        case MergeKind::Define:
        case MergeKind::Record:
        case MergeKind::TypeAlias:
          mi->mergeableObj(ix++) =
            (void*)((intptr_t(mergeable.second) << 3) + (int)mergeable.first);
          break;
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          not_reached();
      }
    }
  }
  assertx(ix == mi->m_mergeablesSize);
  mi->mergeableObj(ix) = (void*)MergeKind::Done;

  /*
   * What's going on is we're going to have a m_lineTable if this UnitEmitter
   * was loaded from the repo, and no m_sourceLocTab (it's demand-loaded by
   * unit.cpp because it's only used for the debugger).  Don't bother creating
   * the line table here, because we can retrieve it from the repo later.
   *
   * On the other hand, if this unit was just created by parsing a php file (or
   * whatnot) which was not committed to the repo, we'll have a m_sourceLocTab.
   * In this case we should populate m_lineTable (otherwise we might lose line
   * info altogether, since it may not be backed by a repo).
   */
  if (m_sourceLocTab.size() != 0) {
    SourceLocation::stashLineTable(u.get(), createLineTable(m_sourceLocTab, m_bclen));
    // If the debugger is enabled, or we plan to dump hhas we will
    // need the extended line table information in the output, and if
    // we're not writing the repo, stashing it here is necessary for
    // it to make it through.
    if (needs_extended_line_table()) {
      SourceLocation::stashExtendedLineTable(u.get(), createSourceLocTable());
    }
  } else if (saveLineTable) {
    SourceLocation::stashLineTable(u.get(), m_lineTable);
  }

  if (u->m_extended) {
    auto ux = u->getExtended();
    for (auto s : m_litstrs) {
      ux->m_namedInfo.emplace_back(LowStringPtr{s});
    }
    ux->m_arrayTypeTable = m_arrayTypeTable;

    // Funcs can be recorded out of order when loading them from the
    // repo currently.  So sort 'em here.
    std::sort(ux->m_funcTable.begin(), ux->m_funcTable.end(),
              [] (const Func* a, const Func* b) {
                return a->past() < b->past();
              });

    // If prefetching is enabled, store the symbol refs in the Unit so
    // the prefetcher can claim them. Reset the atomic flag to mark
    // them available. Otherwise set the atomic flag was already
    // claimed as a shortcut.
    if (!RO::RepoAuthoritative && unitPrefetchingEnabled()) {
      ux->m_symbolRefsForPrefetch = m_symbol_refs;
      ux->m_symbolRefsPrefetched.clear();
    } else {
      ux->m_symbolRefsPrefetched.test_and_set();
    }
  } else {
    assertx(!m_litstrs.size());
    assertx(m_arrayTypeTable.empty());
  }

  if (RuntimeOption::EvalDumpHhas > 1 ||
    (SystemLib::s_inited && RuntimeOption::EvalDumpHhas == 1)) {
    auto const& hhaspath = RuntimeOption::EvalDumpHhasToFile;
    if (!hhaspath.empty()) {
      static std::atomic<bool> first_unit{true};
      auto const flags = O_WRONLY | O_CREAT | (first_unit ? O_TRUNC : O_APPEND);
      if (!folly::writeFile(disassemble(u.get()), hhaspath.c_str(), flags)) {
        Logger::Error("Failed to write hhas to %s", hhaspath.c_str());
        _Exit(1);
      }
      first_unit = false;
    } else {
      std::printf("%s", disassemble(u.get()).c_str());
      std::fflush(stdout);
    }
    if (SystemLib::s_inited) {
      _Exit(0);
    }
  }

  if (RuntimeOption::EvalDumpBytecode) {
    // Dump human-readable bytecode.
    Trace::traceRelease("%s", u->toString().c_str());
  }

  return u;
}

template<class SerDe>
void UnitEmitter::serdeMetaData(SerDe& sd) {
  sd(m_metaData)
    (m_fileAttributes)
    (m_symbol_refs)
    (m_bcSha1)
    (m_fatalUnit)
    ;
  if (m_fatalUnit) {
    sd(m_fatalLoc)
      (m_fatalOp)
      (m_fatalMsg)
      ;
  }

  if (RuntimeOption::EvalLoadFilepathFromUnitCache) {
    /* May be different than the unit origin: e.g. for hhas files. */
    sd(m_filepath);
  }
}

template <typename SerDe>
void UnitEmitter::serde(SerDe& sd) {
  MemoryManager::SuppressOOM so(*tl_heap);

  serdeMetaData(sd);
  // These are not touched by serdeMetaData:
  sd(m_ICE);
  sd(m_allClassesHoistable);

  auto const seq = [&] (auto const& c, auto const& r, auto const& w) {
    if constexpr (SerDe::deserializing) {
      size_t size;
      sd(size);
      for (size_t i = 0; i < size; ++i) r(sd, i);
    } else {
      sd(c.size());
      for (auto const& x : c) w(sd, x);
    }
  };

  // Literal strings
  seq(
    m_litstrs,
    [&] (auto& sd, size_t i) {
      const StringData* s;
      sd(s);
      auto const id UNUSED = mergeUnitLitstr(s);
      assertx(id == i);
    },
    [&] (auto& sd, const StringData* s) { sd(s); }
  );

  // Arrays
  seq(
    m_arrays,
    [&] (auto& sd, size_t i) {
      std::string key;
      sd(key);

      auto v = [&]{
        VariableUnserializer vu{
          key.data(),
          key.size(),
          VariableUnserializer::Type::Internal
        };
        vu.setUnitFilename(m_filepath);
        return vu.unserialize();
      }();
      assertx(v.isArray());
      auto ad = v.detach().m_data.parr;
      ArrayData::GetScalarArray(&ad);
      auto const id DEBUG_ONLY = mergeArray(ad);
      assertx(id == i);
    },
    [&] (auto& sd, const ArrayData* a) {
      auto const str = [&]{
        VariableSerializer vs{VariableSerializer::Type::Internal};
        vs.setUnitFilename(m_filepath);
        return
          vs.serializeValue(VarNR(const_cast<ArrayData*>(a)), false)
          .toCppString();
      }();
      sd(str);
    }
  );

  // HHBBC array types
  sd(m_arrayTypeTable);

  // Pre-class emitters
  seq(
    m_pceVec,
    [&] (auto& sd, size_t i) {
      std::string name;
      int hoistable;
      sd(name);
      sd(hoistable);
      auto pce = newPreClassEmitter(name, (PreClass::Hoistable)hoistable);
      pce->serdeMetaData(sd);
      assertx(pce->id() == i);
    },
    [&] (auto& sd, PreClassEmitter* pce) {
      auto const nm = StripIdFromAnonymousClassName(pce->name()->slice());
      sd(nm.toString());
      sd((int)pce->hoistability());
      pce->serdeMetaData(sd);
    }
  );

  // Record emitters
  seq(
    m_reVec,
    [&] (auto& sd, size_t i) {
      std::string name;
      sd(name);
      auto re = newRecordEmitter(name);
      re->serdeMetaData(sd);
      assertx(re->id() == i);
    },
    [&] (auto& sd, RecordEmitter* re) {
      auto const nm = StripIdFromAnonymousClassName(re->name()->slice());
      sd(nm.toString());
      re->serdeMetaData(sd);
    }
  );

  // Type aliases
  seq(
    m_typeAliases,
    [&] (auto& sd, size_t i) {
      std::string name;
      sd(name);
      auto te = newTypeAliasEmitter(name);
      te->serdeMetaData(sd);
      assertx(te->id() == i);
    },
    [&] (auto& sd, const std::unique_ptr<TypeAliasEmitter>& te) {
      sd(te->name()->toCppString());
      te->serdeMetaData(sd);
    }
  );

  // Constants
  seq(
    m_constants,
    [&] (auto& sd, size_t i) {
      Constant cns;
      sd(cns.name);
      sd(cns);
      if (type(cns.val) == KindOfUninit) {
        cns.val.m_data.pcnt = reinterpret_cast<MaybeCountable*>(Unit::getCns);
      }
      auto const id UNUSED = addConstant(cns);
      assertx(id == i);
    },
    [&] (auto& sd, const Constant& cns) {
      sd(cns.name);
      sd(cns);
    }
  );

  // Line table
  sd(
    m_lineTable,
    [&] (const LineEntry& prev, const LineEntry& curDelta) {
      if (SerDe::deserializing) {
        return LineEntry {
          curDelta.pastOffset() + prev.pastOffset(),
          curDelta.val() + prev.val()
        };
      } else {
        return LineEntry {
          curDelta.pastOffset() - prev.pastOffset(),
          curDelta.val() - prev.val()
        };
      }
    }
  );

  // Mergeables (we cannot just serde the vector because some of the
  // entries are populated when creating PreClassEmitters).
  if constexpr (SerDe::deserializing) {
    size_t size;
    sd(size);

    for (size_t i = 0; i < size; ++i) {
      size_t idx;
      decltype(m_mergeableStmts)::value_type v;
      sd(idx);
      sd(v);
      switch (v.first) {
        case MergeKind::Define:
        case MergeKind::TypeAlias:
          insertMergeableId(v.first, idx, v.second);
          break;
        case MergeKind::Record:
          insertMergeableRecord(idx, v.second);
          break;
        case MergeKind::Class:
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          always_assert(false);
      }
    }
  } else {
    size_t size = 0;
    for (auto const& [k, _] : m_mergeableStmts) {
      switch (k) {
        case MergeKind::TypeAlias:
        case MergeKind::Record:
        case MergeKind::Define:
          ++size;
          break;
        case MergeKind::Class:
          break;
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          always_assert(false);
      }
    }

    sd(size);
    for (size_t i = 0; i < m_mergeableStmts.size(); ++i) {
      auto const& p = m_mergeableStmts[i];
      switch (p.first) {
        case MergeKind::TypeAlias:
        case MergeKind::Record:
        case MergeKind::Define:
          sd(i);
          sd(p);
          break;
        case MergeKind::Class:
          break;
        case MergeKind::Done:
        case MergeKind::UniqueDefinedClass:
          always_assert(false);
      }
    }
  }

  // Func emitters (we cannot use seq for these because they come from
  // both the pre-class emitters and m_fes.
  if constexpr (SerDe::deserializing) {
    size_t total;
    sd(total);
    for (size_t i = 0; i < total; ++i) {
      Id pceId;
      const StringData* name;
      sd(pceId);
      sd(name);

      FuncEmitter* fe;
      if (pceId < 0) {
        fe = newFuncEmitter(name);
      } else {
        auto funcPce = pce(pceId);
        fe = newMethodEmitter(name, funcPce);
        auto const added UNUSED = funcPce->addMethod(fe);
        assertx(added);
      }
      assertx(fe->sn() == i);
      fe->serdeMetaData(sd);
      fe->setEHTabIsSorted();
      fe->finish(fe->past);
    }
  } else {
    auto total = m_fes.size();
    for (auto const pce : m_pceVec) total += pce->methods().size();
    sd(total);

    auto const write = [&] (FuncEmitter* fe, Id pceId) {
      sd(pceId);
      sd(fe->name);
      fe->serdeMetaData(sd);
    };
    for (auto const& fe : m_fes) write(fe.get(), -1);
    for (auto const pce : m_pceVec) {
      for (auto const fe : pce->methods()) write(fe, pce->id());
    }
  }

  // Source location table
  sd(m_sourceLocTab);

  // Bytecode
  if constexpr (SerDe::deserializing) {
    size_t size;
    sd(size);
    assertx(sd.remaining() <= size);
    setBc(sd.data(), size);
    sd.advance(size);
  } else {
    sd(m_bclen);
    sd.writeRaw((const char*)m_bc, m_bclen);
  }
}

template void UnitEmitter::serde<>(BlobDecoder&);
template void UnitEmitter::serde<>(BlobEncoder&);

///////////////////////////////////////////////////////////////////////////////
// UnitRepoProxy.

UnitRepoProxy::UnitRepoProxy(Repo& repo)
    : RepoProxy(repo)
#define URP_OP(c, o) \
    , o{c##Stmt(repo, 0), c##Stmt(repo, 1)}
    URP_OPS
#undef URP_OP
{}

UnitRepoProxy::~UnitRepoProxy() {
}

void UnitRepoProxy::createSchema(int repoId, RepoTxn& txn) {
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER PRIMARY KEY, sha1 BLOB UNIQUE, globalids INTEGER,"
      " bc BLOB, data BLOB);",
      m_repo.table(repoId, "Unit"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, litstrId INTEGER, litstr TEXT,"
      " PRIMARY KEY (unitSn, litstrId));",
      m_repo.table(repoId, "UnitLitstr"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, typeAliasId INTEGER, name TEXT, data BLOB, "
      " PRIMARY KEY (unitSn, typeAliasId));",
      m_repo.table(repoId, "UnitTypeAlias"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, constantId INTEGER, name TEXT, data BLOB, "
      " PRIMARY KEY (unitSn, constantId));",
      m_repo.table(repoId, "UnitConstant"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, arrayId INTEGER, array BLOB, "
      " PRIMARY KEY (unitSn, arrayId));",
      m_repo.table(repoId, "UnitArray"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER PRIMARY KEY, arrayTypeTable BLOB);",
      m_repo.table(repoId, "UnitArrayTypeTable"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, mergeableIx INTEGER, mergeableKind INTEGER, "
      " mergeableId INTEGER, "
      " PRIMARY KEY (unitSn, mergeableIx));",
      m_repo.table(repoId, "UnitMergeables"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} "
      "(unitSn INTEGER, pastOffset INTEGER, line0 INTEGER,"
      " char0 INTEGER, line1 INTEGER, char1 INTEGER,"
      " PRIMARY KEY (unitSn, pastOffset));",
      m_repo.table(repoId, "UnitSourceLoc"));
    txn.exec(createQuery);
  }
  {
    auto createQuery = folly::sformat(
      "CREATE TABLE {} (unitSn INTEGER PRIMARY KEY, data BLOB);",
      m_repo.table(repoId, "UnitLineTable"));
    txn.exec(createQuery);
  }
}

std::unique_ptr<UnitEmitter> UnitRepoProxy::loadEmitter(
    const folly::StringPiece name,
    const SHA1& sha1,
    const Native::FuncTable& nativeFuncs) {
  // We set useGlobalIds to false as a placeholder; it will be set
  // correctly by UnitRepoProxy::GetUnitStmt::get.
  auto ue = std::make_unique<UnitEmitter>(sha1, SHA1{}, nativeFuncs, false);
  if (!RuntimeOption::EvalLoadFilepathFromUnitCache) {
    ue->m_filepath = makeStaticString(name);
  }
  // Look for a repo that contains a unit with matching SHA1.
  int repoId;
  for (repoId = m_repo.numOpenRepos() - 1; repoId >= 0; --repoId) {
    if (getUnit[repoId].get(*ue, sha1) == RepoStatus::success) {
      break;
    }
  }
  if (repoId < 0) {
    TRACE(3, "No repo contains '%s' (0x%s)\n",
             name.data(), sha1.toString().c_str());
    return nullptr;
  }
  try {
    getUnitLitstrs[repoId].get(*ue);
    getUnitArrays[repoId].get(*ue);
    getUnitArrayTypeTable[repoId].get(*ue);
    m_repo.pcrp().getPreClasses[repoId].get(*ue);
    m_repo.rrp().getRecords[repoId].get(*ue);
    getUnitTypeAliases[repoId].get(*ue);
    getUnitConstants[repoId].get(*ue);
    getUnitMergeables[repoId].get(*ue);
    getUnitLineTable[repoId].get(ue->m_sn, ue->m_lineTable);
    m_repo.frp().getFuncs[repoId].get(*ue);
  } catch (RepoExc& re) {
    TRACE(0,
          "Repo error loading '%s' (0x%s) from '%s': %s\n",
          name.data(), sha1.toString().c_str(),
          m_repo.repoName(repoId).c_str(), re.msg().c_str());
    return nullptr;
  }
  TRACE(3, "Repo loaded '%s' (0x%s) from '%s'\n",
           name.data(), sha1.toString().c_str(),
           m_repo.repoName(repoId).c_str());
  return ue;
}

std::unique_ptr<Unit>
UnitRepoProxy::load(const folly::StringPiece name, const SHA1& sha1,
                    const Native::FuncTable& nativeFuncs) {
  ARRPROV_USE_RUNTIME_LOCATION();
  auto ue = loadEmitter(name, sha1, nativeFuncs);
  if (!ue) return nullptr;

#ifdef USE_JEMALLOC
  if (RuntimeOption::TrackPerUnitMemory) {
    size_t len = sizeof(uint64_t*);
    uint64_t* alloc;
    uint64_t* del;
    mallctl("thread.allocatedp", static_cast<void*>(&alloc), &len, nullptr, 0);
    mallctl("thread.deallocatedp", static_cast<void*>(&del), &len, nullptr, 0);
    auto before = *alloc;
    auto debefore = *del;
    std::unique_ptr<Unit> result = ue->create();
    auto after = *alloc;
    auto deafter = *del;

    auto path = folly::sformat("/tmp/units-{}.map", getpid());
    auto change = (after - deafter) - (before - debefore);
    auto str = folly::sformat("{} {}\n", name, change);
    auto out = std::fopen(path.c_str(), "a");
    if (out) {
      std::fwrite(str.data(), str.size(), 1, out);
      std::fclose(out);
    }

    return result;
  }
#endif

  auto unit = ue->create();
  if (BuiltinSymbols::s_systemAr) {
    assertx(ue->m_filepath->data()[0] == '/' &&
            ue->m_filepath->data()[1] == ':');
    BuiltinSymbols::RecordSystemlibFile(std::move(ue));
  }
  FTRACE(1, "Creating unit {} for `{}`\n", unit.get(), name);
  return unit;
}

void UnitRepoProxy::InsertUnitStmt
                  ::insert(const UnitEmitter& ue,
                           RepoTxn& txn, int64_t& unitSn, const SHA1& sha1,
                           const unsigned char* bc, size_t bclen,
                           bool usePreAllocatedUnitSn) {
  BlobEncoder dataBlob{ue.useGlobalIds()};

  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@unitSn, @sha1, @globalids, @bc, @data);",
      m_repo.table(m_repoId, "Unit"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindSha1("@sha1", sha1);
  query.bindBool("@globalids", ue.m_useGlobalIds);
  query.bindBlob("@bc", (const void*)bc, bclen);
  const_cast<UnitEmitter&>(ue).serdeMetaData(dataBlob);
  query.bindBlob("@data", dataBlob, /* static */ true);
  if (!usePreAllocatedUnitSn) {
    query.bindNull("@unitSn");
  } else {
    always_assert(unitSn != -1);
    query.bindInt64("@unitSn", unitSn);
  }
  query.exec();
  if (!usePreAllocatedUnitSn) {
    unitSn = query.getInsertedRowid();
  }
}

RepoStatus UnitRepoProxy::GetUnitStmt::get(UnitEmitter& ue, const SHA1& sha1) {
  try {
    auto txn = RepoTxn{m_repo.begin()};
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT unitSn, globalids, bc, data FROM {} WHERE sha1 == @sha1;",
        m_repo.table(m_repoId, "Unit"));
      txn.prepare(*this, selectQuery);
    }
    RepoTxnQuery query(txn, *this);
    query.bindSha1("@sha1", sha1);
    query.step();
    if (!query.row()) {
      return RepoStatus::error;
    }
    int64_t unitSn;                     /**/ query.getInt64(0, unitSn);
    bool useGlobalIds;                  /**/ query.getBool(1, useGlobalIds);
    const void* bc; size_t bclen;       /**/ query.getBlob(2, bc, bclen);
    BlobDecoder dataBlob =              /**/ query.getBlob(3, useGlobalIds);

    ue.m_repoId = m_repoId;
    ue.m_sn = unitSn;
    ue.m_useGlobalIds = useGlobalIds;
    ue.setBc(static_cast<const unsigned char*>(bc), bclen);
    ue.serdeMetaData(dataBlob);

    txn.commit();
  } catch (RepoExc& re) {
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

void UnitRepoProxy::InsertUnitLitstrStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Id litstrId,
                           const StringData* litstr) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@unitSn, @litstrId, @litstr);",
      m_repo.table(m_repoId, "UnitLitstr"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@litstrId", litstrId);
  query.bindStaticString("@litstr", litstr);
  query.exec();
}

void UnitRepoProxy::GetUnitLitstrsStmt
                  ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT litstrId, litstr FROM {} "
      " WHERE unitSn == @unitSn ORDER BY litstrId ASC;",
      m_repo.table(m_repoId, "UnitLitstr"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Id litstrId;        /**/ query.getId(0, litstrId);
      StringData* litstr; /**/ query.getStaticString(1, litstr);
      Id id UNUSED = ue.mergeUnitLitstr(litstr);
      assertx(id == litstrId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitArrayTypeTableStmt::insert(
  RepoTxn& txn, int64_t unitSn, const UnitEmitter& ue) {

  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@unitSn, @arrayTypeTable);",
      m_repo.table(m_repoId, "UnitArrayTypeTable"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  BlobEncoder dataBlob{ue.useGlobalIds()};
  dataBlob(ue.m_arrayTypeTable);
  query.bindBlob("@arrayTypeTable", dataBlob, /* static */ true);
  query.exec();
}

void UnitRepoProxy::GetUnitArrayTypeTableStmt
                  ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT unitSn, arrayTypeTable FROM {} WHERE unitSn == @unitSn;",
      m_repo.table(m_repoId, "UnitArrayTypeTable"));
    txn.prepare(*this, selectQuery);
  }

  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);

  query.step();
  assertx(query.row());
  BlobDecoder dataBlob = query.getBlob(1, ue.useGlobalIds());
  dataBlob(ue.m_arrayTypeTable);
  dataBlob.assertDone();
  query.step();
  assertx(query.done());

  txn.commit();
}

void UnitRepoProxy::InsertUnitArrayStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Id arrayId,
                           const std::string& array) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@unitSn, @arrayId, @array);",
      m_repo.table(m_repoId, "UnitArray"));
    txn.prepare(*this, insertQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindId("@arrayId", arrayId);
  query.bindStdString("@array", array);
  query.exec();
}

void UnitRepoProxy::GetUnitArraysStmt
                  ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT arrayId, array FROM {} "
      " WHERE unitSn == @unitSn ORDER BY arrayId ASC;",
      m_repo.table(m_repoId, "UnitArray"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      // We check that arrays do not exceed a configurable maximum size in the
      // assembler, so just assume that they're okay here.
      MemoryManager::SuppressOOM so(*tl_heap);

      Id arrayId;        /**/ query.getId(0, arrayId);
      std::string key;   /**/ query.getStdString(1, key);

      Variant v = [&]{
        VariableUnserializer vu{
          key.data(),
          key.size(),
          VariableUnserializer::Type::Internal
        };
        vu.setUnitFilename(ue.m_filepath);
        return vu.unserialize();
      }();
      assertx(v.isArray());
      ArrayData* ad = v.detach().m_data.parr;
      ArrayData::GetScalarArray(&ad);
      Id id DEBUG_ONLY = ue.mergeArray(ad);
      assertx(id == arrayId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitMergeableStmt
                  ::insert(RepoTxn& txn, int64_t unitSn,
                           int ix, Unit::MergeKind kind, Id id) {
  assertx(kind == MergeKind::TypeAlias ||
          kind == MergeKind::Define ||
          kind == MergeKind::Record);
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES("
      " @unitSn, @mergeableIx, @mergeableKind, @mergeableId);",
      m_repo.table(m_repoId, "UnitMergeables"));
    txn.prepare(*this, insertQuery);
  }

  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt("@mergeableIx", ix);
  query.bindInt("@mergeableKind", (int)kind);
  query.bindId("@mergeableId", id);
  query.exec();
}

void UnitRepoProxy::GetUnitMergeablesStmt
                  ::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT mergeableIx, mergeableKind, mergeableId "
      "FROM {} "
      "WHERE unitSn == @unitSn ORDER BY mergeableIx ASC;",
      m_repo.table(m_repoId, "UnitMergeables"));
    txn.prepare(*this, selectQuery);
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
      switch (k) {
        case MergeKind::Define:
        case MergeKind::TypeAlias:
          ue.insertMergeableId(k, mergeableIx, mergeableId);
          break;
        case MergeKind::Record:
          ue.insertMergeableRecord(mergeableIx, mergeableId);
          break;
        default: break;
      }
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitLineTableStmt
                  ::insert(RepoTxn& txn,
                           int64_t unitSn,
                           LineTable& lineTable) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES(@unitSn, @data);",
      m_repo.table(m_repoId, "UnitLineTable"));
    txn.prepare(*this, insertQuery);
  }

  BlobEncoder dataBlob{false};
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  dataBlob(
    lineTable,
    [&](const LineEntry& prev, const LineEntry& cur) -> LineEntry {
      return LineEntry {
        cur.pastOffset() - prev.pastOffset(),
        cur.val() - prev.val()
      };
    }
  );

  query.bindBlob("@data", dataBlob, /* static */ true);
  query.exec();
}

void UnitRepoProxy::GetUnitLineTableStmt::get(int64_t unitSn,
                                              LineTable& lineTable) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT data FROM {} WHERE unitSn == @unitSn;",
      m_repo.table(m_repoId, "UnitLineTable"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.step();
  if (query.row()) {
    BlobDecoder dataBlob = query.getBlob(0, false);
    dataBlob(
      lineTable,
      [&](const LineEntry& prev, const LineEntry& delta) -> LineEntry {
        return LineEntry {
          delta.pastOffset() + prev.pastOffset(),
          delta.val() + prev.val()
        };
      }
    );
  }
  txn.commit();
}

void UnitRepoProxy::InsertUnitTypeAliasStmt
                  ::insert(const UnitEmitter& ue,
                           RepoTxn& txn,
                           int64_t unitSn,
                           Id typeAliasId,
                           const TypeAliasEmitter& te) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES (@unitSn, @typeAliasId, @name, @data);",
      m_repo.table(m_repoId, "UnitTypeAlias"));
    txn.prepare(*this, insertQuery);
  }

  BlobEncoder dataBlob{ue.useGlobalIds()};
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt64("@typeAliasId", typeAliasId);
  query.bindStaticString("@name", te.name());
  const_cast<TypeAliasEmitter&>(te).serdeMetaData(dataBlob);
  query.bindBlob("@data", dataBlob, /* static */ true);
  query.exec();
}

void UnitRepoProxy::GetUnitTypeAliasesStmt::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT typeAliasId, name, data FROM {} WHERE unitSn == @unitSn;",
      m_repo.table(m_repoId, "UnitTypeAlias"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);

  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Id typeAliasId;        /**/ query.getId(0, typeAliasId);
      std::string name;      /**/ query.getStdString(1, name);
      BlobDecoder dataBlob = /**/ query.getBlob(2, ue.useGlobalIds());

      TypeAliasEmitter* te = ue.newTypeAliasEmitter(name);
      te->serdeMetaData(dataBlob);

      assertx(te->id() == typeAliasId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitConstantStmt
                  ::insert(const UnitEmitter& ue,
                           RepoTxn& txn,
                           int64_t unitSn,
                           Id constantId,
                           const Constant& constant) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} VALUES (@unitSn, @constantId, @name, @data);",
      m_repo.table(m_repoId, "UnitConstant"));
    txn.prepare(*this, insertQuery);
  }

  BlobEncoder dataBlob{ue.useGlobalIds()};
  RepoTxnQuery query(txn, *this);
  query.bindInt64("@unitSn", unitSn);
  query.bindInt64("@constantId", constantId);
  query.bindStaticString("@name", constant.name);

  dataBlob(constant);
  query.bindBlob("@data", dataBlob, /* static */ true);
  query.exec();
}

void UnitRepoProxy::GetUnitConstantsStmt::get(UnitEmitter& ue) {
  auto txn = RepoTxn{m_repo.begin()};
  if (!prepared()) {
    auto selectQuery = folly::sformat(
      "SELECT constantId, name, data FROM {} WHERE unitSn == @unitSn;",
      m_repo.table(m_repoId, "UnitConstant"));
    txn.prepare(*this, selectQuery);
  }
  RepoTxnQuery query(txn, *this);

  query.bindInt64("@unitSn", ue.m_sn);
  do {
    query.step();
    if (query.row()) {
      Constant c;
      Id constantId;        /**/ query.getId(0, constantId);
      StringData *name;      /**/ query.getStaticString(1, name);
      c.name = makeStaticString(name);
      BlobDecoder dataBlob = /**/ query.getBlob(2, ue.useGlobalIds());

      // We check that arrays do not exceed a configurable maximum size in the
      // assembler, so just assume that they're okay here.
      MemoryManager::SuppressOOM so(*tl_heap);

      dataBlob(c);
      if (type(c.val) == KindOfUninit) {
        c.val.m_data.pcnt = reinterpret_cast<MaybeCountable*>(Unit::getCns);
      }
      Id id UNUSED = ue.addConstant(c);
      assertx(id == constantId);
    }
  } while (!query.done());
  txn.commit();
}

void UnitRepoProxy::InsertUnitSourceLocStmt
                  ::insert(RepoTxn& txn, int64_t unitSn, Offset pastOffset,
                           int line0, int char0, int line1, int char1) {
  if (!prepared()) {
    auto insertQuery = folly::sformat(
      "INSERT INTO {} "
      "VALUES(@unitSn, @pastOffset, @line0, @char0, @line1, @char1);",
      m_repo.table(m_repoId, "UnitSourceLoc"));
    txn.prepare(*this, insertQuery);
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

RepoStatus
UnitRepoProxy::GetSourceLocTabStmt::get(int64_t unitSn,
                                        SourceLocTable& sourceLocTab) {
  try {
    auto txn = RepoTxn{m_repo.begin()};
    if (!prepared()) {
      auto selectQuery = folly::sformat(
        "SELECT pastOffset, line0, char0, line1, char1 "
        "FROM {} "
        "WHERE unitSn == @unitSn "
        "ORDER BY pastOffset ASC;",
        m_repo.table(m_repoId, "UnitSourceLoc"));
      txn.prepare(*this, selectQuery);
    }
    RepoTxnQuery query(txn, *this);
    query.bindInt64("@unitSn", unitSn);
    do {
      query.step();
      if (!query.row()) {
        return RepoStatus::error;
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
    return RepoStatus::error;
  }
  return RepoStatus::success;
}

std::unique_ptr<UnitEmitter>
createFatalUnit(const StringData* filename, const SHA1& sha1, FatalOp op,
                std::string err, Location::Range loc) {
  auto ue = std::make_unique<UnitEmitter>(sha1, SHA1{}, Native::s_noNativeFuncs,
                                          false);
  ue->m_filepath = filename;

  ue->m_fatalUnit = true;
  ue->m_fatalLoc = loc;
  ue->m_fatalOp = op;
  ue->m_fatalMsg = err;

  return ue;
}

///////////////////////////////////////////////////////////////////////////////
}
