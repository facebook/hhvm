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

// don't mix this file up with runtime/base/bespoke-layout.h !
#ifndef HPHP_BESPOKEDIR_LAYOUT_H_
#define HPHP_BESPOKEDIR_LAYOUT_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/util/type-scan.h"

namespace HPHP { namespace bespoke {

struct Layout {
  Layout();
  virtual ~Layout() {}

  /* bespoke indexes are 16 bits wide, the last 3 values are reserved
   * (see jit::ArraySpec for why) */
  static uint16_t constexpr kMaxIndex = (1 << 16) - 4;

  uint16_t index() const { return m_index; }

  virtual std::string describe() const = 0;

  virtual size_t heapSize(const ArrayData* ad) const = 0;
  virtual size_t align(const ArrayData* ad) const = 0;
  virtual void scan(const ArrayData* ad, type_scan::Scanner& scan) const = 0;
  virtual ArrayData* escalateToVanilla(
    const ArrayData*, const char* reason) const = 0;

  // convertToUncounted only has to convert the array's values in place;
  // the array has already been copied bitwise into an uncounted allocation.
  // By the same token, releaseUncounted only has to free the array's values.
  virtual void convertToUncounted(
    ArrayData*, DataWalker::PointerMap* seen) const = 0;
  virtual void releaseUncounted(ArrayData*) const = 0;
  virtual void release(ArrayData*) const = 0;

  virtual bool isVectorData(const ArrayData*) const = 0;
  virtual TypedValue getInt(const ArrayData*, int64_t) const = 0;
  virtual TypedValue getStr(const ArrayData*, const StringData*) const = 0;
  virtual TypedValue getKey(const ArrayData*, ssize_t pos) const = 0;
  virtual TypedValue getVal(const ArrayData*, ssize_t pos) const = 0;
  virtual ssize_t getIntPos(const ArrayData*, int64_t) const = 0;
  virtual ssize_t getStrPos(const ArrayData*, const StringData*) const = 0;

  virtual arr_lval lvalInt(ArrayData* ad, int64_t k) const = 0;
  virtual arr_lval lvalStr(ArrayData* ad, StringData* k) const = 0;
  virtual ArrayData* setInt(ArrayData*, int64_t k, TypedValue v) const = 0;
  virtual ArrayData* setStr(ArrayData*, StringData* k, TypedValue v) const = 0;
  virtual ArrayData* removeInt(ArrayData*, int64_t) const = 0;
  virtual ArrayData* removeStr(ArrayData*, const StringData*) const = 0;

  virtual ssize_t iterBegin(const ArrayData*) const = 0;
  virtual ssize_t iterLast(const ArrayData*) const = 0;
  virtual ssize_t iterEnd(const ArrayData*) const = 0;
  virtual ssize_t iterAdvance(const ArrayData*, ssize_t) const = 0;
  virtual ssize_t iterRewind(const ArrayData*, ssize_t) const = 0;

  virtual ArrayData* append(ArrayData*, TypedValue v) const = 0;
  virtual ArrayData* prepend(ArrayData*, TypedValue v)  const = 0;
  virtual ArrayData* merge(ArrayData*, const ArrayData*)  const = 0;
  virtual ArrayData* pop(ArrayData*, Variant&)  const = 0;
  virtual ArrayData* dequeue(ArrayData*, Variant&)  const = 0;
  virtual ArrayData* renumber(ArrayData*)  const = 0;

  virtual ArrayData* copy(const ArrayData*) const = 0;
  virtual ArrayData* toVArray(ArrayData*, bool copy) const = 0;
  virtual ArrayData* toDArray(ArrayData*, bool copy) const = 0;
  virtual ArrayData* toVec(ArrayData*, bool copy) const = 0;
  virtual ArrayData* toDict(ArrayData*, bool copy) const = 0;
  virtual ArrayData* toKeyset(ArrayData*, bool copy) const = 0;

  virtual void setLegacyArrayInPlace(ArrayData*, bool legacy) const = 0;

private:
  uint16_t m_index;
};

const Layout* layoutForIndex(uint16_t index);

}}

#endif // HPHP_BESPOKEDIR_LAYOUT_H_
