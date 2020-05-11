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

#ifndef HPHP_LOGGING_ARRAY_H_
#define HPHP_LOGGING_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke-layout.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

namespace bespoke {

struct LoggingArray : BespokeArray {
  static LoggingArray* fromBespoke(BespokeArray* ad);
  static const LoggingArray* fromBespoke(const BespokeArray* ad) {
    return fromBespoke(const_cast<BespokeArray*>(ad));
  }

  static LoggingArray* MakeFromVanilla(ArrayData* ad);

  bool checkInvariants() const;

  ArrayData* wrapped;
};

struct LoggingLayout : public Layout {
  using ArrayType = LoggingArray;

  explicit LoggingLayout(DataType type)
    : m_datatype(type) {}

  static Layout* layoutForVanillaArray(const ArrayData* ad);

  DataType datatype() const final;
  void scan(const BespokeArray* ad, type_scan::Scanner& scan) const final;
  size_t heapSize(const BespokeArray* ad) const final;

  ArrayData* escalate(const BespokeArray*) const final;
  void release(BespokeArray*) const final;
  ArrayData* copy(const BespokeArray*) const final;
  ArrayData* copyStatic(const BespokeArray*) const final;

  TypedValue getInt(const BespokeArray*, int64_t) const final;
  TypedValue getStr(const BespokeArray*, const StringData*) const final;
  ssize_t getIntPos(const BespokeArray*, int64_t ) const final;
  ssize_t getStrPos(const BespokeArray*, const StringData*) const final;
  TypedValue getKey(const BespokeArray*, ssize_t pos) const final;
  TypedValue getVal(const BespokeArray*, ssize_t pos) const final;

  arr_lval lvalInt(BespokeArray*, int64_t) const final;
  arr_lval lvalStr(BespokeArray*, StringData*) const final;

  size_t size(const BespokeArray*) const final;
  bool isVectorData(const BespokeArray*) const final;
  bool existsInt(const BespokeArray*, int64_t) const final;
  bool existsStr(const BespokeArray*, const StringData*) const final;

  ArrayData* setInt(BespokeArray*, int64_t k, TypedValue v) const final;
  ArrayData* setStr(BespokeArray*, StringData* k, TypedValue v) const final;
  ArrayData* deleteInt(BespokeArray*, int64_t) const final;
  ArrayData* deleteStr(BespokeArray*, const StringData*) const final;

  ssize_t iterBegin(const BespokeArray*) const final;
  ssize_t iterLast(const BespokeArray*) const final;
  ssize_t iterEnd(const BespokeArray*) const final;
  ssize_t iterAdvance(const BespokeArray*, ssize_t) const final;
  ssize_t iterRewind(const BespokeArray*, ssize_t) const final;

  ArrayData* append(BespokeArray*, TypedValue v) const final;
  ArrayData* prepend(BespokeArray*, TypedValue v) const final;
  ArrayData* plusEq(BespokeArray*, const ArrayData*) const final;
  ArrayData* merge(BespokeArray*, const ArrayData*) const final;
  ArrayData* pop(BespokeArray*, Variant&) const final;
  ArrayData* dequeue(BespokeArray*, Variant&) const final;
  ArrayData* renumber(BespokeArray*) const final;

  ArrayData* toPHPArray(BespokeArray*, bool copy) const final;
  ArrayData* toPHPArrayIntishCast(BespokeArray*, bool copy) const final;
  ArrayData* toVec(BespokeArray*, bool copy) const final;
  ArrayData* toDict(BespokeArray*, bool copy) const final;
  ArrayData* toKeyset(BespokeArray*, bool copy) const final;
  ArrayData* toVArray(BespokeArray*, bool copy) const final;
  ArrayData* toDArray(BespokeArray*, bool copy) const final;

private:
  const DataType m_datatype;
};

}

}


#endif // HPHP_LOGGING_ARRAY_H_
