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

RecordData::RecordData(const RecordDesc* record)
  : RecordBase(record) {
  initHeader(HeaderKind::Record, OneReference);
  static_assert(sizeof(RecordData) == sizeof(RecordBase) + sizeof(Countable),
                "RecordData must not have any fields of its own");
}

RecordData* RecordData::newRecord(const RecordDesc* rec,
                                  uint32_t initSize,
                                  const StringData* const* keys,
                                  const TypedValue* values) {
  return newRecordImpl<RecordData>(rec, initSize, keys, values);
}

RecordData* RecordData::copyRecord() const {
  auto const rd = copyRecordBase(this, AllocMode::Request);
  rd->initHeader(this->kind(), OneReference);
  return rd;
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
