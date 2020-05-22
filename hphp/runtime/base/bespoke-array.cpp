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

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke-layout.h"

namespace HPHP {

const BespokeArray* BespokeArray::asBespoke(const ArrayData* ad) {
  return asBespoke(const_cast<ArrayData*>(ad));
}

BespokeArray* BespokeArray::asBespoke(ArrayData* ad) {
  assertx(!ad->isVanilla());
  return reinterpret_cast<BespokeArray*>(ad);
}

const bespoke::Layout* BespokeArray::layout() const {
  return bespoke::layoutForIndex(static_cast<uint16_t>(~m_size));
}

void BespokeArray::setLayout(const bespoke::Layout* layout) {
  m_size = ~static_cast<uint32_t>(layout->index());
}

DataType BespokeArray::toDataType() const {
  return layout()->datatype();
}

size_t BespokeArray::heapSize() const {
  return layout()->heapSize(this);
}

ArrayData* BespokeArray::escalate() const {
  return layout()->escalate(this);
}

void BespokeArray::scan(type_scan::Scanner& scan) const {
  return layout()->scan(this, scan);
}

void BespokeArray::Release(ArrayData* ad) {
  auto bad = asBespoke(ad);
  bad->layout()->release(bad);
}


// accessors
TypedValue BespokeArray::NvGetInt(const ArrayData* ad, int64_t key) {
  auto const bad = asBespoke(ad);
  return bad->layout()->getInt(bad, key);
}

TypedValue BespokeArray::NvGetStr(const ArrayData* ad, const StringData* key) {
  auto const bad = asBespoke(ad);
  return bad->layout()->getStr(bad, key);
}

ssize_t BespokeArray::NvGetIntPos(const ArrayData* ad, int64_t key) {
  auto bad = asBespoke(ad);
  return bad->layout()->getIntPos(bad, key);
}

ssize_t BespokeArray::NvGetStrPos(const ArrayData* ad, const StringData* key) {
  auto bad = asBespoke(ad);
  return bad->layout()->getStrPos(bad, key);
}

TypedValue BespokeArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  auto bad = asBespoke(ad);
  return bad->layout()->getKey(bad, pos);
}

TypedValue BespokeArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  auto bad = asBespoke(ad);
  return bad->layout()->getVal(bad, pos);
}

// inspection
size_t BespokeArray::Vsize(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->size(bad);
}

bool BespokeArray::IsVectorData(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->isVectorData(bad);
}

bool BespokeArray::ExistsInt(const ArrayData* ad, int64_t key) {
  auto bad = asBespoke(ad);
  return bad->layout()->existsInt(bad, key);
}

bool BespokeArray::ExistsStr(const ArrayData* ad, const StringData* key) {
  auto bad = asBespoke(ad);
  return bad->layout()->existsStr(bad, key);
}


// mutators
ArrayData* BespokeArray::SetInt(ArrayData* ad, int64_t key, TypedValue v) {
  auto bad = asBespoke(ad);
  return bad->layout()->setInt(bad, key, v);
}

ArrayData* BespokeArray::SetIntMove(ArrayData* ad, int64_t key, TypedValue v) {
  auto bad = asBespoke(ad);
  return bad->layout()->setInt(bad, key, v);
}

ArrayData* BespokeArray::SetStr(ArrayData* ad, StringData* key, TypedValue v) {
  auto bad = asBespoke(ad);
  return bad->layout()->setStr(bad, key, v);
}

ArrayData* BespokeArray::SetStrMove(ArrayData* ad, StringData* key, TypedValue v) {
  always_assert(false);
}


// rw access
arr_lval BespokeArray::LvalInt(ArrayData* ad, int64_t key) {
  auto bad = asBespoke(ad);
  return bad->layout()->lvalInt(bad, key);
}

arr_lval BespokeArray::LvalStr(ArrayData* ad, StringData* key) {
  auto bad = asBespoke(ad);
  return bad->layout()->lvalStr(bad, key);
}


// deletion
ArrayData* BespokeArray::RemoveInt(ArrayData* ad, int64_t key) {
  auto bad = asBespoke(ad);
  return bad->layout()->deleteInt(bad, key);
}

ArrayData* BespokeArray::RemoveStr(ArrayData* ad, const StringData* key) {
  auto bad = asBespoke(ad);
  return bad->layout()->deleteStr(bad, key);
}


// iteration
ssize_t BespokeArray::IterEnd(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->iterEnd(bad);
}

ssize_t BespokeArray::IterBegin(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->iterBegin(bad);
}

ssize_t BespokeArray::IterLast(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->iterLast(bad);
}

ssize_t BespokeArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  auto bad = asBespoke(ad);
  return bad->layout()->iterAdvance(bad, pos);
}

ssize_t BespokeArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  auto bad = asBespoke(ad);
  return bad->layout()->iterRewind(bad, pos);
}


// garbage
ArrayData* BespokeArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto const bad = asBespoke(ad);
  return bad->layout()->escalate(bad);
}


// copies
ArrayData* BespokeArray::Copy(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->copy(bad);
}

ArrayData* BespokeArray::CopyStatic(const ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->copy(bad);
}


// high level ops
ArrayData* BespokeArray::Append(ArrayData* ad, TypedValue v) {
  auto bad = asBespoke(ad);
  return bad->layout()->append(bad, v);
}

ArrayData* BespokeArray::Prepend(ArrayData* ad, TypedValue v) {
  auto bad = asBespoke(ad);
  return bad->layout()->prepend(bad, v);
}

ArrayData* BespokeArray::PlusEq(ArrayData* ad, const ArrayData* other) {
  auto bad = asBespoke(ad);
  return bad->layout()->plusEq(bad, other);
}

ArrayData* BespokeArray::Merge(ArrayData* ad, const ArrayData* elems) {
  auto bad = asBespoke(ad);
  return bad->layout()->plusEq(bad, elems);
}

ArrayData* BespokeArray::Pop(ArrayData* ad, Variant& out) {
  auto bad = asBespoke(ad);
  return bad->layout()->pop(bad, out);
}

ArrayData* BespokeArray::Dequeue(ArrayData* ad, Variant& out) {
  auto bad = asBespoke(ad);
  return bad->layout()->dequeue(bad, out);
}

ArrayData* BespokeArray::Renumber(ArrayData* ad) {
  auto bad = asBespoke(ad);
  return bad->layout()->renumber(bad);
}

void BespokeArray::OnSetEvalScalar(ArrayData*) {
  always_assert(false);
}


// conversions
ArrayData* BespokeArray::ToPHPArray(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toPHPArray(bad, copy);
}

ArrayData* BespokeArray::ToPHPArrayIntishCast(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toPHPArrayIntishCast(bad, copy);
}

ArrayData* BespokeArray::ToDict(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toDict(bad, copy);
}

ArrayData* BespokeArray::ToVec(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toVec(bad, copy);
}

ArrayData* BespokeArray::ToKeyset(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toKeyset(bad, copy);
}

ArrayData* BespokeArray::ToVArray(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toVArray(bad, copy);
}

ArrayData* BespokeArray::ToDArray(ArrayData* ad, bool copy) {
  auto bad = asBespoke(ad);
  return bad->layout()->toDArray(bad, copy);
}




} // namespace HPHP
