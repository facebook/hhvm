/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
namespace req {

size_t req::root_handle::addRootHandle() {
  auto& handles = MM().m_root_handles;
  auto id = handles.size();
  handles.push_back(this);
  return id;
}

size_t req::root_handle::stealRootHandle(root_handle* s) {
  if (s->m_id == INVALID) return INVALID;
  auto& handles = MM().m_root_handles;
  auto id = s->m_id;
  handles[id] = this;
  s->m_id = INVALID;
  return id;
}

void req::root_handle::delRootHandle() {
  auto& handles = MM().m_root_handles;
  auto last = handles.back();
  handles[last->m_id = m_id] = last;
  m_id = INVALID;
  handles.pop_back();
}

template<class T> void root<T>::vscan(IMarker& mark) const {
  T::scan(mark);
}

template<class T> void root<T>::detach() {
  T::detach();
}

template<> void req::root<TypedValue>::vscan(IMarker& mark) const {
  mark(*static_cast<const TypedValue*>(this));
}

template<> void req::root<TypedValue>::detach() {
  m_type = KindOfNull;
}

template struct root<String>;
template struct root<Array>;
template struct root<Object>;
template struct root<Variant>;

}}
