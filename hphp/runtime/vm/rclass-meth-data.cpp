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

#include "hphp/runtime/vm/rclass-meth-data.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

RClsMethData::RClsMethData(Class* m_cls, Func* m_func, ArrayData* m_arr)
    : m_cls(m_cls), m_func(m_func), m_arr(m_arr) {
  initHeader(HeaderKind::RClsMeth, OneReference);
}

RClsMethData* RClsMethData::create(Class* cls, Func* func,
                                   ArrayData* reified_generics) {
  auto const rclsmeth =
    new (tl_heap->objMalloc(sizeof(RClsMethData)))
      RClsMethData(cls, func, reified_generics);
  return rclsmeth;
}


bool RClsMethData::Same(const RClsMethData* rc1, const RClsMethData* rc2) {
  if (rc1->m_cls != rc2->m_cls || rc1->m_func != rc2->m_func) {
     return false;
  }
  return ArrayData::Same(rc1->m_arr, rc2->m_arr);
}

void RClsMethData::release() noexcept {
  decRefArr(m_arr);
  tl_heap->objFree(this, sizeof(RClsMethData));
}

bool RClsMethData::kindIsValid() const {
  return
    m_kind == HeaderKind::RClsMeth &&
    m_cls->validate() &&
    m_func->validate() &&
    m_arr->kindIsValid();
}

} // namespace HPHP
