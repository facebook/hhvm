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

#include "hphp/runtime/vm/rfunc-data.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {

RFuncData::RFuncData(Func* m_func, ArrayData* m_arr) : m_func(m_func), m_arr(m_arr) {
  initHeader(HeaderKind::RFunc, OneReference);
}

RFuncData* RFuncData::newInstance(Func* func, ArrayData* reified_generics) {
  reified_generics->incRefCount();
  auto const rfunc =
    new (tl_heap->objMalloc(sizeof(RFuncData))) RFuncData(func, reified_generics);
  return rfunc;
}

void RFuncData::release() noexcept {
  decRefArr(m_arr);
  tl_heap->objFree(this, sizeof(RFuncData));
}

} // namespace HPHP
