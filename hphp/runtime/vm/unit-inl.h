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

#ifndef incl_HPHP_VM_UNIT_INL_H_
#error "unit-inl.h should only be included by unit.h"
#endif

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/litstr-table.h"
#include "hphp/runtime/vm/unit-util.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Unit::MergeInfo.

inline Func** Unit::MergeInfo::funcBegin() const {
  return (Func**)m_mergeables;
}

inline Func** Unit::MergeInfo::funcEnd() const {
  return funcBegin() + m_firstHoistablePreClass;
}

inline
Unit::MergeInfo::FuncRange Unit::MergeInfo::funcs() const {
  return { funcBegin(), funcEnd() };
}

inline
Unit::MergeInfo::MutableFuncRange Unit::MergeInfo::mutableFuncs() const {
  return { funcBegin(), funcEnd() };
}

inline void*& Unit::MergeInfo::mergeableObj(int idx) {
  return m_mergeables[idx];
}

///////////////////////////////////////////////////////////////////////////////
// Basic accessors.

inline UnitExtended* Unit::getExtended() {
  assertx(m_extended);
  return static_cast<UnitExtended*>(this);
}

inline SymbolRefs* Unit::claimSymbolRefsForPrefetch() {
  if (!m_extended) return nullptr; // No symbol refs
  auto extended = getExtended();
  // Atomically mark them as taken and check if they were already
  // taken
  if (extended->m_symbolRefsPrefetched.test_and_set()) return nullptr;
  return &extended->m_symbolRefsForPrefetch;
}

inline const UnitExtended* Unit::getExtended() const {
  assertx(m_extended);
  return static_cast<const UnitExtended*>(this);
}

inline const FatalInfo* Unit::getFatalInfo() const {
  return m_fatalInfo.get();
}

inline int Unit::repoID() const {
  return m_repoId;
}

inline int64_t Unit::sn() const {
  return m_sn;
}

inline SHA1 Unit::sha1() const {
  return m_sha1;
}

inline SHA1 Unit::bcSha1() const {
  return m_bcSha1;
}

inline bool Unit::isICE() const {
  return m_ICE;
}

///////////////////////////////////////////////////////////////////////////////
// File paths.

inline const StringData* Unit::origFilepath() const {
  assertx(m_origFilepath);
  return m_origFilepath;
}

inline const StringData* Unit::perRequestFilepath() const {
  if (!m_extended) return nullptr;
  auto const u = getExtended();
  if (!u->m_perRequestFilepath.bound()) return nullptr;
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(RuntimeOption::EvalReuseUnitsByHash);
  if (!u->m_perRequestFilepath.isInit()) return nullptr;
  return *u->m_perRequestFilepath;
}

inline const StringData* Unit::filepath() const {
  if (auto const p = perRequestFilepath()) return p;
  return origFilepath();
}

inline rds::Handle Unit::perRequestFilepathHandle() const {
  if (!m_extended) return rds::kUninitHandle;
  return getExtended()->m_perRequestFilepath.maybeHandle();
}

inline bool Unit::hasPerRequestFilepath() const {
  return perRequestFilepathHandle() != rds::kUninitHandle;
}

inline void Unit::bindPerRequestFilepath(const StringData* p) {
  assertx(p);
  assertx(p->isStatic());
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(RuntimeOption::EvalReuseUnitsByHash);
  assertx(m_extended);
  auto u = getExtended();
  assertx(u->m_perRequestFilepath.bound());
  assertx(!u->m_perRequestFilepath.isInit());
  u->m_perRequestFilepath.initWith(p);
}

inline void Unit::makeFilepathPerRequest() {
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(RuntimeOption::EvalReuseUnitsByHash);
  assertx(m_extended);
  auto u = getExtended();
  assertx(!u->m_perRequestFilepath.bound());
  u->m_perRequestFilepath = rds::alloc<LowStringPtr>();
}

/////////////////////////////////////////////////////////////////////////////
// Unit cache ref-counting

inline void Unit::acquireCacheRefCount() {
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(m_extended);
  ++getExtended()->m_cacheRefCount;
}

inline bool Unit::releaseCacheRefCount() {
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(m_extended);
  assertx(getExtended()->m_cacheRefCount > 0);
  return !(--getExtended()->m_cacheRefCount);
}

inline bool Unit::hasCacheRef() const {
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(m_extended);
  return getExtended()->m_cacheRefCount > 0;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode.

inline PC Unit::entry() const {
  return m_bc;
}

inline Offset Unit::bclen() const {
  return m_bclen;
}

///////////////////////////////////////////////////////////////////////////////
// Litstrs and NamedEntitys.

inline size_t Unit::numLitstrs() const {
  if (!m_extended) return 0;
  return getExtended()->m_namedInfo.size();
}

inline bool Unit::isLitstrId(Id id) const {
  if (!isUnitLitstrId(id)) {
    return LitstrTable::get().contains(id);
  }
  auto unitID = decodeUnitLitstrId(id);
  return m_extended && getExtended()->m_namedInfo.contains(unitID);
}

inline StringData* Unit::lookupLitstrId(Id id) const {
  if (!isUnitLitstrId(id)) {
    return LitstrTable::get().lookupLitstrId(id);
  }
  auto unitID = decodeUnitLitstrId(id);
  return getExtended()->m_namedInfo.lookupLitstr(unitID);
}

inline const NamedEntity* Unit::lookupNamedEntityId(Id id) const {
  return lookupNamedEntityPairId(id).second;
}

inline NamedEntityPair Unit::lookupNamedEntityPairId(Id id) const {
  if (!isUnitLitstrId(id)) {
    return LitstrTable::get().lookupNamedEntityPairId(id);
  }
  auto unitID = decodeUnitLitstrId(id);
  return getExtended()->m_namedInfo.lookupNamedEntityPair(unitID);
}

///////////////////////////////////////////////////////////////////////////////
// Arrays.

inline size_t Unit::numArrays() const {
  return m_arrays.size();
}

inline const ArrayData* Unit::lookupArrayId(Id id) const {
  assertx(id < m_arrays.size());
  return m_arrays[id];
}

inline const RepoAuthType::Array* Unit::lookupArrayTypeId(Id id) const {
  return RuntimeOption::RepoAuthoritative ?
    globalArrayTypeTable().lookup(id) :
    getExtended()->m_arrayTypeTable.lookup(id);
}

///////////////////////////////////////////////////////////////////////////////
// Funcs and PreClasses and RecordDescs.

inline Func* Unit::lookupFuncId(Id id) const {
  assertx(id < Id(mergeInfo()->m_firstHoistablePreClass));
  return mergeInfo()->funcBegin()[id];
}

inline PreClass* Unit::lookupPreClassId(Id id) const {
  assertx(id < Id(m_preClasses.size()));
  return m_preClasses[id].get();
}

inline PreRecordDesc* Unit::lookupPreRecordId(Id id) const {
  assertx(id < Id(m_preRecords.size()));
  return m_preRecords[id].get();
}

inline Unit::FuncRange Unit::funcs() const {
  return mergeInfo()->funcs();
}

inline folly::Range<PreClassPtr*> Unit::preclasses() {
  return { m_preClasses.data(), m_preClasses.size() };
}

inline folly::Range<const PreClassPtr*> Unit::preclasses() const {
  return { m_preClasses.data(), m_preClasses.size() };
}

inline folly::Range<PreRecordDescPtr*> Unit::prerecords() {
  return { m_preRecords.data(), m_preRecords.size() };
}

inline folly::Range<const PreRecordDescPtr*> Unit::prerecords() const {
  return { m_preRecords.data(), m_preRecords.size() };
}

template<class Fn> void Unit::forEachFunc(Fn fn) const {
  for (auto& func : funcs()) {
    fn(func);
  }
  for (auto& c : preclasses()) {
    auto methods = FuncRange{c->methods(), c->methods() + c->numMethods()};
    for (auto& method : methods) {
      fn(method);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Type aliases

inline folly::Range<PreTypeAlias*> Unit::typeAliases() {
  return { m_typeAliases.begin(), m_typeAliases.end() };
}

inline folly::Range<const PreTypeAlias*> Unit::typeAliases() const {
  return { m_typeAliases.begin(), m_typeAliases.end() };
}

///////////////////////////////////////////////////////////////////////////////
// Constants

inline folly::Range<Constant*> Unit::constants() {
  return { m_constants.begin(), m_constants.end() };
}

inline folly::Range<const Constant*> Unit::constants() const {
  return { m_constants.begin(), m_constants.end() };
}

///////////////////////////////////////////////////////////////////////////////
// File attributes

inline const UserAttributeMap& Unit::fileAttributes() const {
  return m_fileAttributes;
}

///////////////////////////////////////////////////////////////////////////////
// RecordDesc lookup.

inline RecordDesc* Unit::lookupRecordDesc(const NamedEntity* ne) {
  return ne->getCachedRecordDesc();
}

inline RecordDesc* Unit::lookupRecordDesc(const StringData* name) {
  return lookupRecordDesc(NamedEntity::get(name));
}

inline const RecordDesc* Unit::lookupUniqueRecDesc(const StringData* name) {
  auto ne = NamedEntity::get(name);
  auto rec = ne->recordList();
  if (LIKELY(rec && (rec->attrs() & AttrUnique))) return rec;
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Merge.

inline bool Unit::isEmpty() const {
  return m_mergeState.load(std::memory_order_relaxed) & MergeState::Empty;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool Unit::isInterpretOnly() const {
  return m_interpretOnly;
}

inline void Unit::setInterpretOnly() {
  m_interpretOnly = true;
}

inline UserAttributeMap Unit::metaData() const {
  return m_metaData;
}

///////////////////////////////////////////////////////////////////////////////
}
