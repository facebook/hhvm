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

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

bool cellIsPlausible(const Cell cell) {
  assert(cell.m_type != KindOfRef);

  auto assertPtr = [](void* ptr) {
    assert(ptr && (uintptr_t(ptr) % sizeof(ptr) == 0));
  };

  [&] {
    switch (cell.m_type) {
      case KindOfUninit:
      case KindOfNull:
        return;
      case KindOfBoolean:
        assert(cell.m_data.num == 0 || cell.m_data.num == 1);
        return;
      case KindOfInt64:
      case KindOfDouble:
        return;
      case KindOfPersistentString:
        assertPtr(cell.m_data.pstr);
        assert(cell.m_data.pstr->kindIsValid());
        assert(!cell.m_data.pstr->isRefCounted());
        return;
      case KindOfString:
        assertPtr(cell.m_data.pstr);
        assert(cell.m_data.pstr->kindIsValid());
        assert(cell.m_data.pstr->checkCount());
        return;
      case KindOfPersistentVec:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isVecArray());
        return;
      case KindOfVec:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isVecArray());
        return;
      case KindOfPersistentDict:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isDict());
        return;
      case KindOfDict:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isDict());
        return;
      case KindOfPersistentKeyset:
        assertPtr(cell.m_data.parr);
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isKeyset());
        return;
      case KindOfKeyset:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isKeyset());
        return;
      case KindOfPersistentArray:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->kindIsValid());
        assert(!cell.m_data.parr->isRefCounted());
        assert(cell.m_data.parr->isPHPArray());
        return;
      case KindOfArray:
        assertPtr(cell.m_data.parr);
        assert(cell.m_data.parr->kindIsValid());
        assert(cell.m_data.parr->checkCount());
        assert(cell.m_data.parr->isPHPArray());
        return;
      case KindOfObject:
        assertPtr(cell.m_data.pobj);
        assert(cell.m_data.pobj->kindIsValid());
        assert(cell.m_data.pobj->checkCount());
        return;
      case KindOfResource:
        assertPtr(cell.m_data.pres);
        assert(cell.m_data.pres->kindIsValid());
        assert(cell.m_data.pres->checkCount());
        return;
      case KindOfRef:
        assert(!"KindOfRef found in a Cell");
        break;
    }
    not_reached();
  }();

  return true;
}

bool tvIsPlausible(TypedValue tv) {
  if (tv.m_type == KindOfRef) {
    assert(tv.m_data.pref);
    assert(uintptr_t(tv.m_data.pref) % sizeof(void*) == 0);
    assert(tv.m_data.pref->kindIsValid());
    assert(tv.m_data.pref->checkCount());
    tv = *tv.m_data.pref->tv();
  }
  return cellIsPlausible(tv);
}

bool refIsPlausible(const Ref ref) {
  assert(ref.m_type == KindOfRef);
  return tvIsPlausible(ref);
}

///////////////////////////////////////////////////////////////////////////////

}
