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

#ifndef HPHP_BESPOKE_LAYOUT_H_
#define HPHP_BESPOKE_LAYOUT_H_

#include "hphp/runtime/base/array-data.h"

namespace HPHP {

namespace type_scan { struct Scanner; }

struct BespokeArray;

namespace bespoke {

struct Layout;

struct BespokeFunctions {
  void (*scan)(const Layout*, const BespokeArray* ad, type_scan::Scanner& scan);
  size_t (*heapSize)(const Layout*, const BespokeArray* ad);

  // lifetime
  ArrayData* (*escalate)(const Layout*, const BespokeArray* ad);
  void (*release)(const Layout*, BespokeArray* ad);
  ArrayData* (*copy)(const Layout*, const BespokeArray* ad);
  ArrayData* (*copyStatic)(const Layout*, const BespokeArray* ad);

  // accessors
  TypedValue (*getInt)(const Layout*, const BespokeArray* ad, int64_t k);
  TypedValue (*getStr)(const Layout*, const BespokeArray* ad, const StringData* k);
  ssize_t (*getIntPos)(const Layout*, const BespokeArray* ad, int64_t k);
  ssize_t (*getStrPos)(const Layout*, const BespokeArray* ad, const StringData* k);
  TypedValue (*getKey)(const Layout*, const BespokeArray* ad, ssize_t p);
  TypedValue (*getVal)(const Layout*, const BespokeArray* ad, ssize_t p);
  arr_lval (*lvalInt)(const Layout*, const BespokeArray* ad, int64_t k);

  // inspection
  size_t (*size)(const Layout*, const BespokeArray* ad);
  size_t (*isVectorData)(const Layout*, const BespokeArray* ad);
  size_t (*existsInt)(const Layout*, const BespokeArray* ad, int64_t k);
  size_t (*existsStr)(const Layout*, const BespokeArray* ad, const StringData* k);

  // mutators
  ArrayData* (*setInt)(const Layout*, BespokeArray* ad, int64_t k, TypedValue v);
  ArrayData* (*setStr)(const Layout*, BespokeArray* ad, StringData* k, TypedValue v);
  ArrayData* (*setNew)(const Layout*, BespokeArray* ad, TypedValue v);
  ArrayData* (*deleteInt)(const Layout*, BespokeArray* ad, int64_t k);
  ArrayData* (*deleteStr)(const Layout*, BespokeArray* ad, const StringData* k);

  // iteration
  ssize_t (*iterBegin)(const Layout*, const BespokeArray* ad);
  ssize_t (*iterEnd)(const Layout*, const BespokeArray* ad);
  ssize_t (*iterAdvance)(const Layout*, const BespokeArray* ad, ssize_t in);
  ssize_t (*iterRewind)(const Layout*, const BespokeArray* ad, ssize_t in);
};

struct Layout {
  Layout();
  virtual ~Layout() {}

  int64_t index() const { return m_index; }
  virtual DataType datatype() const = 0;

  virtual void scan(const BespokeArray* ad, type_scan::Scanner& scan) const = 0;
  virtual size_t heapSize(const BespokeArray* ad) const = 0;

  virtual ArrayData* escalate(const BespokeArray*) const = 0;
  virtual void release(BespokeArray*) const = 0;
  virtual ArrayData* copy(const BespokeArray*) const = 0;
  virtual ArrayData* copyStatic(const BespokeArray*) const = 0;

  virtual TypedValue getInt(const BespokeArray*, int64_t) const = 0;
  virtual TypedValue getStr(const BespokeArray*, const StringData*) const = 0;
  virtual ssize_t getIntPos(const BespokeArray*, int64_t ) const = 0;
  virtual ssize_t getStrPos(const BespokeArray*, const StringData*) const = 0;
  virtual TypedValue getKey(const BespokeArray*, ssize_t pos) const = 0;
  virtual TypedValue getVal(const BespokeArray*, ssize_t pos) const = 0;

  virtual arr_lval lvalInt(BespokeArray* ad, int64_t k) const = 0;
  virtual arr_lval lvalStr(BespokeArray* ad, StringData* k) const = 0;

  virtual size_t size(const BespokeArray*) const = 0;
  virtual bool isVectorData(const BespokeArray*) const = 0;
  virtual bool existsInt(const BespokeArray*, int64_t) const = 0;
  virtual bool existsStr(const BespokeArray*, const StringData*) const = 0;

  virtual ArrayData* setInt(BespokeArray*, int64_t k, TypedValue v) const = 0;
  virtual ArrayData* setStr(BespokeArray*, StringData* k, TypedValue v) const = 0;
  virtual ArrayData* deleteInt(BespokeArray*, int64_t) const = 0;
  virtual ArrayData* deleteStr(BespokeArray*, const StringData*) const = 0;

  virtual ssize_t iterBegin(const BespokeArray*) const = 0;
  virtual ssize_t iterLast(const BespokeArray*) const = 0;
  virtual ssize_t iterEnd(const BespokeArray*) const = 0;
  virtual ssize_t iterAdvance(const BespokeArray*, ssize_t) const = 0;
  virtual ssize_t iterRewind(const BespokeArray*, ssize_t) const = 0;

  virtual ArrayData* toPHPArray(BespokeArray*, bool copy) const = 0;
  virtual ArrayData* toPHPArrayIntishCast(BespokeArray*, bool copy) const = 0;
  virtual ArrayData* toVec(BespokeArray*, bool copy) const = 0;
  virtual ArrayData* toDict(BespokeArray*, bool copy) const = 0;
  virtual ArrayData* toKeyset(BespokeArray*, bool copy) const = 0;
  virtual ArrayData* toVArray(BespokeArray*, bool copy) const = 0;
  virtual ArrayData* toDArray(BespokeArray*, bool copy) const = 0;

  virtual ArrayData* append(BespokeArray*, TypedValue v) const = 0;
  virtual ArrayData* prepend(BespokeArray*, TypedValue v)  const = 0;
  virtual ArrayData* plusEq(BespokeArray*, const ArrayData*)  const = 0;
  virtual ArrayData* merge(BespokeArray*, const ArrayData*)  const = 0;
  virtual ArrayData* pop(BespokeArray*, Variant&)  const = 0;
  virtual ArrayData* dequeue(BespokeArray*, Variant&)  const = 0;
  virtual ArrayData* renumber(BespokeArray*)  const = 0;

private:
  int64_t m_index;
};

const Layout* layoutForIndex(int16_t bespoke_idx);

} // namespace bespoke
} // namespace HPHP

#endif // HPHP_BESPOKE_LAYOUT_H_
