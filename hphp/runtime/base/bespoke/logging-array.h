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
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace bespoke {

struct LoggingProfile;

struct LoggingArray : BespokeArray {
  static LoggingArray* asLogging(ArrayData* ad);
  static const LoggingArray* asLogging(const ArrayData* ad);
  static LoggingArray* MakeStatic(ArrayData* ad, LoggingProfile* prof);
  static void FreeStatic(LoggingArray* lad);

  // Updates m_kind in place to match the wrapped array's kind. Returns this.
  LoggingArray* updateKind();

  bool checkInvariants() const;

  ArrayData* wrapped;
  LoggingProfile* profile;
};

struct LoggingLayout : public Layout {
  size_t heapSize(const ArrayData* ad) const final;
  void scan(const ArrayData* ad, type_scan::Scanner& scan) const final;
  ArrayData* escalateToVanilla(
    const ArrayData*, const char* reason) const final;

  void convertToUncounted(
    ArrayData*, DataWalker::PointerMap* seen) const final;
  void releaseUncounted(ArrayData*) const final;
  void release(ArrayData*) const final;

  size_t size(const ArrayData*) const final;
  bool isVectorData(const ArrayData*) const final;

  TypedValue getInt(const ArrayData*, int64_t) const final;
  TypedValue getStr(const ArrayData*, const StringData*) const final;
  TypedValue getKey(const ArrayData*, ssize_t pos) const final;
  TypedValue getVal(const ArrayData*, ssize_t pos) const final;
  ssize_t getIntPos(const ArrayData*, int64_t) const final;
  ssize_t getStrPos(const ArrayData*, const StringData*) const final;

  arr_lval lvalInt(ArrayData*, int64_t) const final;
  arr_lval lvalStr(ArrayData*, StringData*) const final;
  ArrayData* setInt(ArrayData*, int64_t k, TypedValue v) const final;
  ArrayData* setStr(ArrayData*, StringData* k, TypedValue v) const final;
  ArrayData* removeInt(ArrayData*, int64_t) const final;
  ArrayData* removeStr(ArrayData*, const StringData*) const final;

  ssize_t iterBegin(const ArrayData*) const final;
  ssize_t iterLast(const ArrayData*) const final;
  ssize_t iterEnd(const ArrayData*) const final;
  ssize_t iterAdvance(const ArrayData*, ssize_t) const final;
  ssize_t iterRewind(const ArrayData*, ssize_t) const final;

  ArrayData* append(ArrayData*, TypedValue v) const final;
  ArrayData* prepend(ArrayData*, TypedValue v) const final;
  ArrayData* merge(ArrayData*, const ArrayData*) const final;
  ArrayData* pop(ArrayData*, Variant&) const final;
  ArrayData* dequeue(ArrayData*, Variant&) const final;
  ArrayData* renumber(ArrayData*) const final;

  ArrayData* copy(const ArrayData*) const final;
  ArrayData* toVArray(ArrayData*, bool copy) const final;
  ArrayData* toDArray(ArrayData*, bool copy) const final;
  ArrayData* toVec(ArrayData*, bool copy) const final;
  ArrayData* toDict(ArrayData*, bool copy) const final;
  ArrayData* toKeyset(ArrayData*, bool copy) const final;

  void setLegacyArrayInPlace(ArrayData*, bool legacy) const final;
};

}}

#endif // HPHP_LOGGING_ARRAY_H_
