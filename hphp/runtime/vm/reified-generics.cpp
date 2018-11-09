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

#include "hphp/runtime/vm/reified-generics.h"

#include "hphp/runtime/vm/act-rec.h"

#include "hphp/util/debug.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {
ReifiedGenericsTable g_reified_generics_table;
} // namespace

void addToReifiedGenericsTable(
  const std::string& name,
  ArrayData* tsList
) {
  auto e = g_reified_generics_table.find(name);

  if (UNLIKELY(e == g_reified_generics_table.end())) {
    g_reified_generics_table[name] = tsList;
    return;
  }
  if (debug) {
    if (!tsList->equal(e->second, true)) {
      raise_error("Mismatched reified types");
    }
  }
  return;
}

ArrayData* getReifiedTypeList(const std::string& name) {
  auto e = g_reified_generics_table.find(name);
  if (LIKELY(e != g_reified_generics_table.end())) {
    return e->second;
  }
  raise_error("No such entry in the reified classes table");
}

ArrayData* getClsReifiedGenericsProp(Class* cls, ActRec* ar) {
  if (!cls->hasReifiedGenerics()) return nullptr;
  auto const this_ = ar->getThis();
  auto const slot = cls->lookupReifiedInitProp();
  assertx(slot != kInvalidSlot);
  auto tv = this_->propVec()[slot];
  assertx(tvIsVecOrVArray(tv));
  return tv.m_data.parr;
}

///////////////////////////////////////////////////////////////////////////////
}
