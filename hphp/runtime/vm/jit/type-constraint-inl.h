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

#include "hphp/runtime/vm/class.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

inline TypeConstraint::TypeConstraint(DataTypeCategory cat
                                      /* = DataTypeGeneric */)
  : category(cat)
  , weak(false)
  , m_specialized(0)
{}

inline TypeConstraint::TypeConstraint(const Class* cls)
  : TypeConstraint(DataTypeSpecialized)
{
  setDesiredClass(cls);
}

inline TypeConstraint& TypeConstraint::setWeak(bool w /* = true */) {
  weak = w;
  return *this;
}

inline bool TypeConstraint::empty() const {
  return category == DataTypeGeneric && !weak;
}

inline bool TypeConstraint::operator==(TypeConstraint tc2) const {
  return category == tc2.category &&
         weak == tc2.weak &&
         m_specialized == tc2.m_specialized;
}

inline bool TypeConstraint::operator!=(TypeConstraint tc2) const {
  return !(*this == tc2);
}

///////////////////////////////////////////////////////////////////////////////
// Specialization.

inline bool TypeConstraint::isSpecialized() const {
  return category == DataTypeSpecialized;
}

inline TypeConstraint& TypeConstraint::setWantArrayKind() {
  assertx(!wantClass());
  assertx(isSpecialized());
  m_specialized |= kWantArrayKind;
  return *this;
}

inline bool TypeConstraint::wantArrayKind() const {
  return m_specialized & kWantArrayKind;
}

inline TypeConstraint& TypeConstraint::setWantArrayShape() {
  assertx(!wantClass());
  assertx(isSpecialized());
  setWantArrayKind();
  m_specialized |= kWantArrayShape;
  return *this;
}

inline bool TypeConstraint::wantArrayShape() const {
  return m_specialized & kWantArrayShape;
}

inline TypeConstraint& TypeConstraint::setDesiredClass(const Class* cls) {
  assertx(m_specialized == 0 ||
         desiredClass()->classof(cls) || cls->classof(desiredClass()));
  assertx(isSpecialized());
  m_specialized = reinterpret_cast<uintptr_t>(cls);
  assertx(wantClass());
  return *this;
}

inline bool TypeConstraint::wantClass() const {
  return m_specialized && !wantArrayKind();
}

inline const Class* TypeConstraint::desiredClass() const {
  assertx(wantClass());
  return reinterpret_cast<const Class*>(m_specialized);
}

///////////////////////////////////////////////////////////////////////////////
}}
