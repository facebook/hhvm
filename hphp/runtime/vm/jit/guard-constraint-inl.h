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
  assertx(isSpecialized());
  m_specialized |= kWantArrayKind;
  return *this;
}

inline bool GuardConstraint::wantArrayKind() const {
  return m_specialized & kWantArrayKind;
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
  return m_specialized && !wantArrayKind();
}

inline const Class* GuardConstraint::desiredClass() const {
  assertx(wantClass());
  return reinterpret_cast<const Class*>(m_specialized);
}

///////////////////////////////////////////////////////////////////////////////
}}
