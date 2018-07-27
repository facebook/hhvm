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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

bool cellIsPlausible(const Cell cell) {
  assertx(!isRefType(cell.m_type));

  auto assertPtr = [](const void* ptr) {
    assertx(ptr && (uintptr_t(ptr) % sizeof(ptr) == 0));
  };

  [&] {
    switch (cell.m_type) {
      case KindOfUninit:
      case KindOfNull:
        return;
      case KindOfBoolean:
        assertx(cell.m_data.num == 0 || cell.m_data.num == 1);
        return;
      case KindOfInt64:
      case KindOfDouble:
        return;
      case KindOfPersistentString:
        assertPtr(cell.m_data.pstr);
        assertx(cell.m_data.pstr->kindIsValid());
        assertx(!cell.m_data.pstr->isRefCounted());
        return;
      case KindOfString:
        assertPtr(cell.m_data.pstr);
        assertx(cell.m_data.pstr->kindIsValid());
        assertx(cell.m_data.pstr->checkCount());
        return;
      case KindOfPersistentVec:
        assertPtr(cell.m_data.parr);
        assertx(!cell.m_data.parr->isRefCounted());
        assertx(cell.m_data.parr->isVecArray());
        assertx(cell.m_data.parr->isNotDVArray());
        return;
      case KindOfVec:
        assertPtr(cell.m_data.parr);
        assertx(cell.m_data.parr->checkCount());
        assertx(cell.m_data.parr->isVecArray());
        assertx(cell.m_data.parr->isNotDVArray());
        return;
      case KindOfPersistentDict:
        assertPtr(cell.m_data.parr);
        assertx(!cell.m_data.parr->isRefCounted());
        assertx(cell.m_data.parr->isDict());
        assertx(cell.m_data.parr->isNotDVArray());
        return;
      case KindOfDict:
        assertPtr(cell.m_data.parr);
        assertx(cell.m_data.parr->checkCount());
        assertx(cell.m_data.parr->isDict());
        assertx(cell.m_data.parr->isNotDVArray());
        return;
      case KindOfPersistentKeyset:
        assertPtr(cell.m_data.parr);
        assertx(!cell.m_data.parr->isRefCounted());
        assertx(cell.m_data.parr->isKeyset());
        assertx(cell.m_data.parr->isNotDVArray());
        return;
      case KindOfKeyset:
        assertPtr(cell.m_data.parr);
        assertx(cell.m_data.parr->checkCount());
        assertx(cell.m_data.parr->isKeyset());
        assertx(cell.m_data.parr->isNotDVArray());
        return;
      case KindOfPersistentArray:
        assertPtr(cell.m_data.parr);
        assertx(cell.m_data.parr->kindIsValid());
        assertx(!cell.m_data.parr->isRefCounted());
        assertx(cell.m_data.parr->isPHPArray());
        assertx(cell.m_data.parr->dvArraySanityCheck());
        return;
      case KindOfArray:
        assertPtr(cell.m_data.parr);
        assertx(cell.m_data.parr->kindIsValid());
        assertx(cell.m_data.parr->checkCount());
        assertx(cell.m_data.parr->isPHPArray());
        assertx(cell.m_data.parr->dvArraySanityCheck());
        return;
      case KindOfObject:
        assertPtr(cell.m_data.pobj);
        assertx(cell.m_data.pobj->kindIsValid());
        assertx(cell.m_data.pobj->checkCount());
        return;
      case KindOfResource:
        assertPtr(cell.m_data.pres);
        assertx(cell.m_data.pres->kindIsValid());
        assertx(cell.m_data.pres->checkCount());
        return;
      case KindOfFunc:
        assertPtr(cell.m_data.pfunc);
        assertx(cell.m_data.pfunc->validate());
        return;
      case KindOfClass:
        assertPtr(cell.m_data.pclass);
        assertx(cell.m_data.pclass->validate());
        return;
      case KindOfRef:
        assertx(!"KindOfRef found in a Cell");
        break;
    }
    not_reached();
  }();

  return true;
}

bool tvIsPlausible(TypedValue tv) {
  if (isRefType(tv.m_type)) {
    assertx(tv.m_data.pref);
    assertx(uintptr_t(tv.m_data.pref) % sizeof(void*) == 0);
    assertx(tv.m_data.pref->kindIsValid());
    assertx(tv.m_data.pref->checkCount());
    tv = *tv.m_data.pref->tv();
  }
  return cellIsPlausible(tv);
}

bool refIsPlausible(const Ref ref) {
  assertx(isRefType(ref.m_type));
  return tvIsPlausible(ref);
}

///////////////////////////////////////////////////////////////////////////////

}
