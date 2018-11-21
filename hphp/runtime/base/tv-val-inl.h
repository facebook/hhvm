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

template<bool is_const, typename tag_t>
tv_val<is_const, tag_t>::tv_val()
    : m_s{}
{}

template<bool is_const, typename tag_t>
tv_val<is_const, tag_t>::tv_val(type_t* type, value_t* val)
  : m_s{type, val}
{}

template<bool is_const, typename tag_t>
tv_val<is_const, tag_t>::tv_val(tv_t* tv)
  : tv_val{&tv->m_type, &tv->m_data}
{}

template<bool is_const, typename tag_t>
tv_val<is_const, tag_t>::tv_val(std::nullptr_t)
    : tv_val{}
{}

template<bool is_const, typename tag_t>
template<typename Tag>
tv_val<is_const, tag_t>::tv_val(
    tv_val<is_const> lval, with_tag_t<Tag> t
) : m_s{lval.m_s.type(), lval.m_s.val(), t}
{}

template<bool is_const, typename tag_t>
bool tv_val<is_const, tag_t>::operator==(tv_val other) const {
  return m_s == other.m_s;
}

template<bool is_const, typename tag_t>
bool tv_val<is_const, tag_t>::operator!=(tv_val other) const {
  return m_s != other.m_s;
}

template<bool is_const, typename tag_t>
bool tv_val<is_const, tag_t>::is_set() const {
  return m_s.is_set();
}

template<bool is_const, typename tag_t>
tv_val<is_const, tag_t>::operator bool() const {
  return is_set();
}

template<bool is_const, typename tag_t>
bool tv_val<is_const, tag_t>::operator==(std::nullptr_t) const {
  return !is_set();
}

template<bool is_const, typename tag_t>
bool tv_val<is_const, tag_t>::operator!=(std::nullptr_t) const {
  return is_set();
}

template<bool is_const, typename tag_t>
tv_val<is_const, tag_t>::operator tv_val<true>() const {
  return tv_val<true>{m_s.type(), m_s.val()};
}

template<bool is_const, typename tag_t>
tv_val<false> tv_val<is_const, tag_t>::as_lval() const {
  return tv_val<false>{
    const_cast<DataType*>(m_s.type()), const_cast<Value*>(m_s.val())
  };
}

template<bool is_const, typename tag_t>
auto tv_val<is_const, tag_t>::val() const -> value_t& {
  assertx(is_set());
  return *m_s.val();
}

template<bool is_const, typename tag_t>
auto tv_val<is_const, tag_t>::type() const -> type_t& {
  assertx(is_set());
  return *m_s.type();
}

template<bool is_const, typename tag_t>
TypedValue tv_val<is_const, tag_t>::tv() const {
  // Explicitly drop m_aux, since users of tv_val shouldn't care about it.
  assertx(is_set());
  return TypedValue{val(), type()};
}

template<bool is_const, typename tag_t>
TypedValue tv_val<is_const, tag_t>::operator*() const {
  return tv();
}

template<bool is_const, typename tag_t>
template<typename Tag>
auto tv_val<is_const, tag_t>::tag() const -> with_tag_t<Tag> {
  return m_s.tag();
}

template<bool is_const, typename tag_t>
template<typename Tag>
auto tv_val<is_const, tag_t>::drop_tag() const ->
  with_tag_t<Tag, tv_val<is_const>> {
  return tv_val<is_const>{m_s.type(), m_s.val()};
}

///////////////////////////////////////////////////////////////////////////////

}
