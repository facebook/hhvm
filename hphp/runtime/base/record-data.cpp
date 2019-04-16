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

#include "hphp/runtime/base/record-data.h"

#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/vm/record.h"

namespace HPHP {

RecordData::RecordData(const Record* record)
  : m_record(record) {
  initHeader(HeaderKind::Record, OneReference);
}

RecordData* RecordData::newRecord(const Record* rec,
                                  uint32_t initSize,
                                  const StringData* const *keys,
                                  const TypedValue* values) {
  auto const size = sizeWithFields(rec);
  auto recdata = new (NotNull{}, tl_heap->objMalloc(size)) RecordData(rec);
  assertx(recdata->hasExactlyOneRef());
  for (auto i = 0; i < initSize; ++i) {
    auto const tv = recdata->getFieldLval(keys[i]);
    // TODO(arnabde): Type check
    // TODO(arnabde): What if there is an initial value?
    // Its refcount needs to be decremented.
    tvCopy(values[initSize - i - 1], tv);
  }
  return recdata;
}

NEVER_INLINE
void RecordData::release() noexcept {
  assertx(kindIsValid());
  auto const numRecFields = m_record->numFields();
  auto const recFields = fieldVec();
  for (auto idx = 0; idx < numRecFields; ++idx) {
    tvDecRefGen(recFields[idx]);
  }
  tl_heap->objFree(this, heapSize());
  AARCH64_WALKABLE_FRAME();
}

tv_rval RecordData::getFieldRval(const StringData* fieldName) const {
  auto const idx = m_record->lookupField(fieldName);
  return tv_rval(&fieldVec()[idx]);
}

tv_lval RecordData::getFieldLval(const StringData* fieldName) const {
  auto const idx = m_record->lookupField(fieldName);
  return tv_lval(const_cast<TypedValue*>(&fieldVec()[idx]));
}

}
