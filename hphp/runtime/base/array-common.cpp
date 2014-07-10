/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/array-common.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

ssize_t ArrayCommon::ReturnInvalidIndex(const ArrayData*) {
  return ArrayData::invalid_index;
}

bool ArrayCommon::ValidMArrayIter(const ArrayData* ad, const MArrayIter& fp) {
  if (ad->isPacked()) {
    assert(PackedArray::checkInvariants(ad));
  } else {
    assert(MixedArray::asMixed(ad));
  }
  assert(fp.getContainer() == ad);
  if (fp.getResetFlag()) return false;
  return fp.m_pos != ArrayData::invalid_index;
}

ArrayData* ArrayCommon::Pop(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_end();
    value = a->getValue(pos);
    return a->remove(a->getKey(pos), a->hasMultipleRefs());
  }
  value = uninit_null();
  return a;
}

ArrayData* ArrayCommon::Dequeue(ArrayData* a, Variant &value) {
  if (!a->empty()) {
    auto const pos = a->iter_begin();
    value = a->getValue(pos);
    auto const ret = a->remove(a->getKey(pos), a->hasMultipleRefs());
    // In PHP, array_shift() will cause all numerically key-ed values re-keyed
    ret->renumber();
    return ret;
  }
  value = uninit_null();
  return a;
}

//////////////////////////////////////////////////////////////////////

}
