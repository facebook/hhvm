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

#include "hphp/runtime/base/record-array.h"
#include "hphp/runtime/base/record-common.h"
#include "hphp/runtime/base/record-data.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-mutate.h"

namespace HPHP {
RecordBase::RecordBase(const RecordDesc* record)
  : m_record(record) {
  static_assert(KindOfUninit == static_cast<DataType>(0),
                "RecordBase assumes KindOfUninit == 0");
  memset(const_cast<TypedValue*>(fieldVec()), 0, fieldSize(record));
}

template
RecordData* RecordBase::newRecordImpl<RecordData>(const RecordDesc* rec,
                                                  uint32_t initSize,
                                                  const StringData* const *keys,
                                                  const TypedValue* values);
template
RecordArray* RecordBase::newRecordImpl<RecordArray>(
    const RecordDesc* rec, uint32_t initSize, const StringData* const *keys,
    const TypedValue* values);

template<class RecordType>
RecordType* RecordBase::newRecordImpl(const RecordDesc* rec,
                                      uint32_t initSize,
                                      const StringData* const *keys,
                                      const TypedValue* values) {
  if (rec->attrs() & AttrAbstract) {
    raise_error("Cannot instantiate abstract record %s", rec->name()->data());
  }
  auto const size = RecordType::sizeWithFields(rec);
  auto const recdata =
    new (NotNull{}, tl_heap->objMalloc(size)) RecordType(rec);
  assertx(recdata->hasExactlyOneRef());
  try {
    for (auto i = 0; i < initSize; ++i) {
      auto const name = keys[i];
      const auto&  field = rec->field(name);
      auto const& val = values[initSize - i -1];
      auto const& tc = field.typeConstraint();
      if (tc.isCheckable()) {
        tc.verifyRecField(&val, rec->name(), field.name());
      }
      auto const& tv = recdata->fieldLval(name);
      tvCopy(val, tv);
    }
    for (auto const &field : rec->allFields()) {
      auto const name = field.name();
      auto const& tv = recdata->fieldLval(name);
      if (type(tv) != KindOfUninit) continue;
      auto const& val = field.val();
      if (val.m_type != KindOfUninit) {
        tvDup(val, tv);
      } else {
        raise_record_init_error(rec->name(), name);
      }
    }
  } catch (...) {
    recdata->decRefAndRelease();
    throw;
  }
  return recdata;
}

tv_rval RecordBase::fieldRval(const StringData* fieldName) const {
  auto const idx = m_record->lookupField(fieldName);
  return tv_rval(&fieldVec()[idx]);
}

tv_lval RecordBase::fieldLval(const StringData* fieldName) {
  auto const idx = m_record->lookupField(fieldName);
  return tv_lval(const_cast<TypedValue*>(&fieldVec()[idx]));
}

} // namespace HPHP
