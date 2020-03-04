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

#include "hphp/runtime/vm/class.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

inline GuardConstraint::GuardConstraint(DataTypeCategory cat
                                      /* = DataTypeGeneric */)
  : category(cat)
  , weak(false)
  , m_specialized(0)
{}

inline GuardConstraint::GuardConstraint(const Class* cls)
  : GuardConstraint(DataTypeSpecialized)
{
  setDesiredClass(cls);
}

inline GuardConstraint::GuardConstraint(const RecordDesc* rec)
  : GuardConstraint(DataTypeSpecialized)
{
  setDesiredRecord(rec);
}

inline GuardConstraint& GuardConstraint::setWeak(bool w /* = true */) {
  weak = w;
  return *this;
}

inline bool GuardConstraint::empty() const {
  return category == DataTypeGeneric && !weak;
}

inline bool GuardConstraint::operator==(GuardConstraint gc2) const {
  return category == gc2.category &&
         weak == gc2.weak &&
         m_specialized == gc2.m_specialized;
}

inline bool GuardConstraint::operator!=(GuardConstraint gc2) const {
  return !(*this == gc2);
}

///////////////////////////////////////////////////////////////////////////////
// Specialization.

inline bool GuardConstraint::isSpecialized() const {
  return category == DataTypeSpecialized;
}

inline GuardConstraint& GuardConstraint::setWantArrayKind() {
  assertx(!wantClass());
  assertx(!wantRecord());
  assertx(isSpecialized());
  m_specialized |= kWantArrayKind | kWantVanillaArray;
  return *this;
}

inline GuardConstraint& GuardConstraint::setWantVanillaArray() {
  assertx(!wantClass());
  assertx(isSpecialized());
  m_specialized |= kWantVanillaArray;
  return *this;
}

inline bool GuardConstraint::wantArrayKind() const {
  return m_specialized & kWantArrayKind;
}

inline bool GuardConstraint::wantVanillaArray() const {
  return m_specialized & kWantVanillaArray;
}

inline GuardConstraint& GuardConstraint::setDesiredClass(const Class* cls) {
  assertx(m_specialized == 0 ||
         desiredClass()->classof(cls) || cls->classof(desiredClass()));
  assertx(isSpecialized());
  m_specialized = reinterpret_cast<uintptr_t>(cls);
  assertx(wantClass());
  return *this;
}

inline bool GuardConstraint::wantClass() const {
  return m_specialized && !wantVanillaArray() && !wantRecord();
}

inline const Class* GuardConstraint::desiredClass() const {
  assertx(wantClass());
  return reinterpret_cast<const Class*>(m_specialized);
}

inline GuardConstraint&
GuardConstraint::setDesiredRecord(const RecordDesc* rec) {
  assertx(m_specialized == 0 ||
          desiredRecord()->recordDescOf(rec) ||
          rec->recordDescOf(desiredRecord()));
  assertx(isSpecialized());
  m_specialized = reinterpret_cast<uintptr_t>(rec) | kWantRecord;
  assertx(wantRecord());
  return *this;
}

inline bool GuardConstraint::wantRecord() const {
  return m_specialized & kWantRecord;
}

inline const RecordDesc* GuardConstraint::desiredRecord() const {
  assertx(wantRecord());
  return reinterpret_cast<const RecordDesc*>(m_specialized | ~kWantRecord);
}

///////////////////////////////////////////////////////////////////////////////
}}
