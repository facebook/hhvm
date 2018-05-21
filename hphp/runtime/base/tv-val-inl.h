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

#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

template<bool is_const>
inline tv_val<is_const>::tv_val()
  : tv_val{nullptr}
{}

template<bool is_const>
inline tv_val<is_const>::tv_val(tv_t* lval)
  : m_tv{lval}
{}

template<bool is_const>
inline bool tv_val<is_const>::operator==(tv_val other) const {
  return m_tv == other.m_tv;
}

template<bool is_const>
inline bool tv_val<is_const>::is_set() const {
  return m_tv;
}

template<bool is_const>
inline tv_val<is_const>::operator bool() const {
  return is_set();
}

template<bool is_const>
inline bool tv_val<is_const>::operator==(std::nullptr_t) const {
  return !is_set();
}

template<bool is_const>
inline bool tv_val<is_const>::operator!=(std::nullptr_t) const {
  return is_set();
}

template<bool is_const>
inline tv_val<is_const>::operator tv_val<true>() const {
  return tv_val<true>{m_tv};
}

template<bool is_const>
inline tv_val<false> tv_val<is_const>::as_lval() const {
  return tv_val<false>{const_cast<TypedValue*>(m_tv)};
}

template<bool is_const>
inline typename tv_val<is_const>::value_t& tv_val<is_const>::val() const {
  assertx(is_set());
  return m_tv->m_data;
}

template<bool is_const>
inline typename tv_val<is_const>::type_t& tv_val<is_const>::type() const {
  assertx(is_set());
  return m_tv->m_type;
}

template<bool is_const>
inline typename tv_val<is_const>::tv_t* tv_val<is_const>::tv_ptr() const {
  return m_tv;
}

template<bool is_const>
inline TypedValue tv_val<is_const>::tv() const {
  // Explicitly drop m_aux, since users of tv_val shouldn't care about it.
  assertx(is_set());
  return TypedValue{val(), type()};
}

template<bool is_const>
inline TypedValue tv_val<is_const>::operator*() const {
  return tv();
}

///////////////////////////////////////////////////////////////////////////////

}
