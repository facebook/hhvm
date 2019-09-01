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

#ifndef incl_HPHP_RECORD_DATA_H_
#define incl_HPHP_RECORD_DATA_H_

#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/record-common.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/tv-val.h"

#include "hphp/util/type-scan.h"

namespace HPHP {
struct RecordData : Countable,
                    RecordBase,
                    type_scan::MarkCollectable<RecordData> {
  explicit RecordData(const RecordDesc*);

  RecordData(const RecordData&) = delete;
  RecordData& operator=(const RecordData&) = delete;
  ~RecordData() = delete;

  size_t heapSize() const;
  bool kindIsValid() const;

  static size_t sizeWithFields(const RecordDesc* rec) {
    return sizeof(RecordData) + fieldSize(rec);
  }
  static constexpr ptrdiff_t getVMRecordOffset() {
    return offsetof(RecordData, m_record);
  }


  /* Given a Record type, an array of keys and an array of corresponding
   * initial values, return a fully initialized instance of that Record.
   * The initial ref-count will be set to one.
   */
  static RecordData* newRecord(const RecordDesc*,
                               uint32_t initSize,
                               const StringData* const* keys,
                               const TypedValue* values);
  RecordData* copyRecord() const;
  // Decrement ref-counts of all fields of the record and free the memory.
  void release() noexcept;
  ALWAYS_INLINE void decRefAndRelease() {
    assertx(kindIsValid());
    if (decReleaseCheck()) release();
  }

  static bool equal(const RecordData*, const RecordData*);
  static bool same(const RecordData*, const RecordData*);

  bool instanceof(const RecordDesc* rec) const {
    return m_record->recordDescOf(rec);
  }

  void scan(type_scan::Scanner&) const;

private:

  template<bool (*fieldEq)(TypedValue, TypedValue)>
  static bool equalImpl(const RecordData*, const RecordData*);

};

ALWAYS_INLINE void decRefRec(RecordData* rec) {
  rec->decRefAndRelease();
}

}

#define incl_HPHP_RECORD_DATA_INL_H_
#include "hphp/runtime/base/record-data-inl.h"
#undef incl_HPHP_RECORD_DATA_INL_H_

#endif // incl_HPHP_RECORD_DATA_H_
