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
  // Check type hints
  for (auto i = 0; i < initSize; ++i) {
    const auto&  field = rec->field(keys[i]);
    auto const& val = values[initSize - i -1];
    auto const& tc = field.typeConstraint();
    if (tc.isCheckable()) {
      tc.verifyRecField(&val, rec->name(), field.name());
    }
  }

  auto const size = sizeWithFields(rec);
  auto const recdata =
    new (NotNull{}, tl_heap->objMalloc(size)) RecordData(rec);
  assertx(recdata->hasExactlyOneRef());
  for (auto i = 0; i < initSize; ++i) {
    auto const& tv = recdata->fieldLval(keys[i]);
    // TODO (T41489986): Handle default values
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

tv_rval RecordData::fieldRval(const StringData* fieldName) const {
  auto const idx = m_record->lookupField(fieldName);
  return tv_rval(&fieldVec()[idx]);
}

tv_lval RecordData::fieldLval(const StringData* fieldName) {
  auto const idx = m_record->lookupField(fieldName);
  return tv_lval(const_cast<TypedValue*>(&fieldVec()[idx]));
}

template<bool(*fieldEq)(TypedValue, TypedValue)>
ALWAYS_INLINE
bool RecordData::equalImpl(const RecordData* r1, const RecordData* r2) {
  if (r1->m_record != r2->m_record) {
    return false;
  }
  auto const fv1 = r1->fieldVec();
  auto const fv2 = r2->fieldVec();
  auto const numFields = r1->m_record->numFields();
  for (size_t i = 0; i < numFields; ++i) {
    if (!fieldEq(fv1[i], fv2[i])) {
      return false;
    }
  }
  return true;
}


bool RecordData::equal(const RecordData* r1, const RecordData* r2) {
  return equalImpl<tvEqual>(r1, r2);
}
bool RecordData::same(const RecordData* r1, const RecordData* r2) {
  return equalImpl<tvSame>(r1, r2);
}

}
