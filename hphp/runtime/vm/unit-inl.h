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

#ifndef incl_HPHP_VM_UNIT_INL_H_
#error "unit-inl.h should only be included by unit.h"
#endif

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/litstr-table.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// SourceLoc.

inline SourceLoc::SourceLoc(const Location::Range& r) {
  setLoc(&r);
}

inline void SourceLoc::reset() {
  line0 = char0 = line1 = char1 = 1;
}

inline bool SourceLoc::valid() const {
  return line0 != 1 || char0 != 1 || line1 != 1 || char1 != 1;
}

inline void SourceLoc::setLoc(const Location::Range* l) {
  line0 = l->line0;
  char0 = l->char0;
  line1 = l->line1;
  char1 = l->char1;
}

inline bool SourceLoc::same(const SourceLoc* l) const {
  return (this == l) ||
         (line0 == l->line0 && char0 == l->char0 &&
          line1 == l->line1 && char1 == l->char1);
}

inline bool SourceLoc::operator==(const SourceLoc& l) const {
  return same(&l);
}

///////////////////////////////////////////////////////////////////////////////
// Location tables.

template<typename T>
Offset TableEntry<T>::pastOffset() const {
  return m_pastOffset;
}

template<typename T>
T TableEntry<T>::val() const {
  return m_val;
}

template<typename T>
bool TableEntry<T>::operator<(const TableEntry& other) const {
  return m_pastOffset < other.m_pastOffset;
}

template<typename T>
template<class SerDe>
void TableEntry<T>::serde(SerDe& sd) {
  sd(m_pastOffset)(m_val);
}

///////////////////////////////////////////////////////////////////////////////
// Unit::MergeInfo.

inline Func** Unit::MergeInfo::funcBegin() const {
  return (Func**)m_mergeables;
}

inline Func** Unit::MergeInfo::funcEnd() const {
  return funcBegin() + m_firstHoistablePreClass;
}

inline Func** Unit::MergeInfo::funcHoistableBegin() const {
  return funcBegin() + m_firstHoistableFunc;
}

inline
Unit::MergeInfo::FuncRange Unit::MergeInfo::funcs() const {
  return FuncRange(funcBegin(), funcEnd());
}

inline
Unit::MergeInfo::MutableFuncRange Unit::MergeInfo::mutableFuncs() const {
  return MutableFuncRange(funcBegin(), funcEnd());
}

inline
Unit::MergeInfo::MutableFuncRange Unit::MergeInfo::nonMainFuncs() const {
  return MutableFuncRange(funcBegin() + 1, funcEnd());
}

inline
Unit::MergeInfo::MutableFuncRange Unit::MergeInfo::hoistableFuncs() const {
  return MutableFuncRange(funcHoistableBegin(), funcEnd());
}

inline void*& Unit::MergeInfo::mergeableObj(int idx) {
  return m_mergeables[idx];
}

inline void** Unit::MergeInfo::mergeableData(int idx) {
  return m_mergeables + idx;
}

///////////////////////////////////////////////////////////////////////////////
// Basic accessors.

inline int Unit::repoID() const {
  return m_repoId;
}

inline int64_t Unit::sn() const {
  return m_sn;
}

inline MD5 Unit::md5() const {
  return m_md5;
}

inline const StringData* Unit::filepath() const {
  assert(m_filepath);
  return m_filepath;
}

inline const StringData* Unit::dirpath() const {
  assert(m_dirpath);
  return m_dirpath;
}

///////////////////////////////////////////////////////////////////////////////
// Bytecode.

inline PC Unit::entry() const {
  return m_bc;
}

inline Offset Unit::bclen() const {
  return m_bclen;
}

inline PC Unit::at(Offset off) const {
  assert(off >= 0 && off <= Offset(m_bclen));
  return m_bc + off;
}

inline Offset Unit::offsetOf(PC pc) const {
  assert(contains(pc));
  return pc - m_bc;
}

inline bool Unit::contains(PC pc) const {
  return pc >= m_bc && pc <= m_bc + m_bclen;
}

inline Op Unit::getOpcode(size_t instrOffset) const {
  assert(instrOffset < m_bclen);
  return static_cast<Op>(m_bc[instrOffset]);
}

///////////////////////////////////////////////////////////////////////////////
// Litstrs and NamedEntitys.

inline size_t Unit::numLitstrs() const {
  return m_namedInfo.size();
}

inline bool Unit::isLitstrId(Id id) const {
  if (isGlobalLitstrId(id)) {
    auto globalID = decodeGlobalLitstrId(id);
    return LitstrTable::get().contains(globalID);
  }
  return m_namedInfo.contains(id);
}

inline StringData* Unit::lookupLitstrId(Id id) const {
  if (isGlobalLitstrId(id)) {
    auto globalID = decodeGlobalLitstrId(id);
    return LitstrTable::get().lookupLitstrId(globalID);
  }
  return m_namedInfo.lookupLitstr(id);
}

inline const NamedEntity* Unit::lookupNamedEntityId(Id id) const {
  return lookupNamedEntityPairId(id).second;
}

inline const NamedEntityPair& Unit::lookupNamedEntityPairId(Id id) const {
  if (isGlobalLitstrId(id)) {
    auto globalID = decodeGlobalLitstrId(id);
    return LitstrTable::get().lookupNamedEntityPairId(globalID);
  }
  return m_namedInfo.lookupNamedEntityPair(id);
}

///////////////////////////////////////////////////////////////////////////////
// Arrays.

inline size_t Unit::numArrays() const {
  return m_arrays.size();
}

inline ArrayData* Unit::lookupArrayId(Id id) const {
  assert(id < m_arrays.size());
  return const_cast<ArrayData*>(m_arrays[id]);
}

///////////////////////////////////////////////////////////////////////////////
// Funcs and PreClasses.

inline Func* Unit::lookupFuncId(Id id) const {
  assert(id < Id(m_mergeInfo->m_firstHoistablePreClass));
  return m_mergeInfo->funcBegin()[id];
}

inline PreClass* Unit::lookupPreClassId(Id id) const {
  assert(id < Id(m_preClasses.size()));
  return m_preClasses[id].get();
}

inline Unit::FuncRange Unit::funcs() const {
  return m_mergeInfo->funcs();
}

inline Unit::PreClassRange Unit::preclasses() const {
  return PreClassRange(m_preClasses);
}

inline Func* Unit::firstHoistable() const {
  return *m_mergeInfo->funcHoistableBegin();
}

///////////////////////////////////////////////////////////////////////////////
// Class lookup.

inline Class* Unit::lookupClass(const NamedEntity* ne) {
  return ne->getCachedClass();
}

inline Class* Unit::lookupClass(const StringData* name) {
  return lookupClass(NamedEntity::get(name));
}

inline Class* Unit::lookupClassOrUniqueClass(const NamedEntity* ne) {
  Class* cls = ne->clsList();
  if (LIKELY(cls != nullptr)) {
    if (cls->attrs() & AttrUnique && RuntimeOption::RepoAuthoritative) {
      return cls;
    }
    return cls->getCached();
  }
  return nullptr;
}

inline Class* Unit::lookupClassOrUniqueClass(const StringData* name) {
  return lookupClassOrUniqueClass(NamedEntity::get(name));
}

inline Class* Unit::loadClass(const StringData* name) {
  String normStr;
  auto ne = NamedEntity::get(name, true, &normStr);
  if (normStr) {
    name = normStr.get();
  }
  return loadClass(ne, name);
}

inline Class* Unit::getClass(const StringData* name, bool tryAutoload) {
  String normStr;
  auto ne = NamedEntity::get(name, true, &normStr);
  if (normStr) {
    name = normStr.get();
  }
  return getClass(ne, name, tryAutoload);
}

///////////////////////////////////////////////////////////////////////////////
// Merge.

inline bool Unit::isMergeOnly() const {
  return m_mergeOnly;
}

inline bool Unit::isEmpty() const {
  return m_mergeState & MergeState::Empty;
}

inline const TypedValue* Unit::getMainReturn() const {
  assert(isMergeOnly());
  return &m_mainReturn;
}

///////////////////////////////////////////////////////////////////////////////
// Other methods.

inline bool Unit::isInterpretOnly() const {
  return m_interpretOnly;
}

inline void Unit::setInterpretOnly() {
  m_interpretOnly = true;
}

inline bool Unit::isHHFile() const {
  return m_isHHFile;
}

///////////////////////////////////////////////////////////////////////////////
}
