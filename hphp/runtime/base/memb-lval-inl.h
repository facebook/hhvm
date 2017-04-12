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

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline ArrayLval::ArrayLval()
  : m_base(nullptr)
  , m_ptr(nullptr)
{}

inline ArrayLval::ArrayLval(HeapObject* base, ArrayLval::ptr_union ptr)
  : m_base(base)
  , m_ptr(ptr)
{}

inline ArrayLval::ArrayLval(HeapObject* base, TypedValue* elem)
  : m_base(base)
  , m_ptr(elem)
{}

inline HeapObject* ArrayLval::base() const {
  return m_base;
}

inline ArrayData* ArrayLval::arr_base() const {
  assertx(isArrayKind(m_base->kind()));
  return m_arr;
}

inline bool ArrayLval::has_ref() const {
  return m_ptr.val != nullptr;
}

inline const Value& ArrayLval::val() const {
  return *m_ptr.val;
}
inline Value& ArrayLval::val() {
  return *m_ptr.val;
}

inline const DataType& ArrayLval::type() const {
  return m_ptr.tv->m_type;
}
inline DataType& ArrayLval::type() {
  return m_ptr.tv->m_type;
}

inline TypedValue* ArrayLval::tv() const {
  return m_ptr.tv;
}

inline ArrayLval::ptr_union ArrayLval::elem() const {
  return m_ptr;
}

///////////////////////////////////////////////////////////////////////////////

}
