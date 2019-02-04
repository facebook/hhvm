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
#include "hphp/runtime/vm/class-meth-data.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"

namespace HPHP {

ClsMethData::ClsMethData(Class* cls, Func* func)
  : m_cls(cls)
  , m_func(func) {
  assertx(cls);
  assertx(func);
  initHeader_16(HeaderKind::ClsMeth, OneReference, 0);
};

ClsMethData* ClsMethData::make(Class* cls, Func* func) {
  return new (tl_heap->objMalloc(sizeof(ClsMethData)))
    ClsMethData(cls, func);
}

void ClsMethData::release() noexcept {
  assertx(validate());
  tl_heap->objFree(this, sizeof(ClsMethData));
  AARCH64_WALKABLE_FRAME();
}

bool ClsMethData::validate() const {
  m_cls->validate();
  m_func->validate();
  return m_kind == HeaderKind::ClsMeth;
}

} // namespace HPHP
