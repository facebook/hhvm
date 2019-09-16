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

#ifndef incl_HPHP_RECORD_ARRAY_H_
#define incl_HPHP_RECORD_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/record-data.h"
#include "hphp/runtime/base/string-hash-map.h"

namespace HPHP {
struct RecordArray : ArrayData,
                     RecordBase,
                     type_scan::MarkCollectable<RecordArray>  {
  explicit RecordArray(const RecordDesc*);
  RecordArray(const RecordArray&) = delete;
  RecordArray& operator=(const RecordArray&) = delete;
  ~RecordArray() = delete;


  static size_t sizeWithFields(const RecordDesc* rec) {
    return sizeof(RecordArray) + fieldSize(rec) + sizeof(ExtraFieldMap);
  }
  size_t heapSize() const;
  bool kindIsValid() const;

  void scan(type_scan::Scanner&) const;

  static RecordArray* newRecordArray(const RecordDesc*,
                                     uint32_t initSize,
                                     const StringData* const* keys,
                                     const TypedValue* values);
  RecordArray* copyRecordArray() const;

  // Array interface
  static void Release(ArrayData*);
  static tv_rval NvGetInt(const ArrayData*, int64_t key);
  static tv_rval NvTryGetInt(const ArrayData*, int64_t key);
  static tv_rval NvGetStr(const ArrayData*, const StringData*);
  static constexpr auto NvTryGetStr = &NvGetStr;
  static ssize_t NvGetIntPos(const ArrayData*, int64_t k);
  static ssize_t NvGetStrPos(const ArrayData*, const StringData* k);
  static Cell NvGetKey(const ArrayData*, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t key, Cell v);
  static ArrayData* SetIntInPlace(ArrayData*, int64_t key, Cell v);
  static ArrayData* SetStr(ArrayData*, StringData*, Cell v);
  static ArrayData* SetStrInPlace(ArrayData*, StringData*, Cell v);
  static ArrayData* SetWithRefInt(ArrayData*, int64_t k, Cell v);
  static ArrayData* SetWithRefIntInPlace(ArrayData*, int64_t k, Cell v);
  static ArrayData* SetWithRefStr(ArrayData*, StringData* k, Cell v);
  static ArrayData* SetWithRefStrInPlace(ArrayData*, StringData* k, Cell v);
  static size_t Vsize(const ArrayData*);
  static tv_rval GetValueRef(const ArrayData*, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static bool ExistsInt(const ArrayData*, int64_t key);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static arr_lval LvalInt(ArrayData*, int64_t k, bool copy);
  static arr_lval LvalStr(ArrayData*, StringData* key, bool copy);
  static arr_lval LvalNew(ArrayData*, bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t key);
  static ArrayData* RemoveIntInPlace(ArrayData*, int64_t key);
  static ArrayData* RemoveStr(ArrayData*, const StringData*);
  static ArrayData* RemoveStrInPlace(ArrayData*, const StringData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);
  static ArrayData* EscalateForSort(ArrayData*, SortFunction);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant&);
  static bool Usort(ArrayData*, const Variant&);
  static bool Uasort(ArrayData*, const Variant&);
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell v);
  static ArrayData* AppendInPlace(ArrayData*, Cell v);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v);
  static ArrayData* AppendWithRefInPlace(ArrayData*, TypedValue v);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, Cell v);
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData*);
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToPHPArrayIntishCast(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToVec(ArrayData*, bool);
  static ArrayData* ToKeyset(ArrayData*, bool);
  static ArrayData* ToVArray(ArrayData*, bool);
  static ArrayData* ToDArray(ArrayData*, bool);

  static RecordArray* asRecordArray(ArrayData*);
  static const RecordArray* asRecordArray(const ArrayData*);

private:
  using ExtraFieldMap = req::StringFastMap<TypedValue>;
  const ExtraFieldMap* extraFieldMap() const;
  bool checkInvariants() const;
  /**
   * Returns index of the field with name `key` if it exists and
   * `val` passes the typehint for the field.
   * Returns kInvalidSlot if the field does not exist.
   * Raises a typehint error if the field exists but the type check fails.
   */
  Slot checkFieldForWrite(const StringData* key, Cell val) const;
  /*
   * Updates the field at idx if idx is valid. Otherwise inserts key, val pair
   * in the extra field map.
   */
  void updateField(StringData* key, Cell val, Slot idx);
};
} // namespace HPHP
#endif // incl_HPHP_RECORD_ARRAY_H_
