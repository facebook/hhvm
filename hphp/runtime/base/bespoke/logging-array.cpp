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

#include "hphp/runtime/base/bespoke/logging-array.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

namespace {
LoggingLayout* s_layout = new LoggingLayout();
}

bool LoggingArray::checkInvariants() const {
  assertx(!isVanilla());
  assertx(kindIsValid());
  assertx(wrapped->isVanilla());
  assertx(wrapped->kindIsValid());
  assertx(wrapped->toDataType() == toDataType());
  assertx(wrapped->hasExactlyOneRef() || wrapped->isStatic());
  assertx(asBespoke(this)->layout() == s_layout);
  return true;
}

LoggingArray* LoggingArray::asLogging(ArrayData* ad) {
  auto const result = reinterpret_cast<LoggingArray*>(ad);
  result->checkInvariants();
  return result;
}
const LoggingArray* LoggingArray::asLogging(const ArrayData* ad) {
  return asLogging(const_cast<ArrayData*>(ad));
}

LoggingArray* LoggingArray::MakeFromVanilla(ArrayData* ad) {
  assertx(ad->getPosition() == 0);
  auto const size_index = MemoryManager::size2Index(sizeof(LoggingArray));
  auto lad = static_cast<LoggingArray*>(tl_heap->objMallocIndex(size_index));

  assertx(ad->isVanilla());
  auto const kind = [&]{
    switch (ad->kind()) {
      case kPackedKind: return HeaderKind::BespokeVArray;
      case kMixedKind:  return HeaderKind::BespokeDArray;
      case kPlainKind:  return HeaderKind::BespokeArray;
      case kVecKind:    return HeaderKind::BespokeVec;
      case kDictKind:   return HeaderKind::BespokeDict;
      case kKeysetKind: return HeaderKind::BespokeKeyset;
      default: always_assert(false);
    }
  }();

  lad->initHeader(kind, OneReference);
  lad->setLayout(s_layout);
  lad->wrapped = ad;
  assertx(lad->checkInvariants());
  return lad;
}

size_t LoggingLayout::heapSize(const ArrayData*) const {
  return sizeof(LoggingArray);
}
void LoggingLayout::scan(const ArrayData* ad, type_scan::Scanner& scan) const {
  scan.scan(LoggingArray::asLogging(ad)->wrapped);
}

ArrayData* LoggingLayout::escalateToVanilla(const ArrayData* ad) const {
  auto wrapped = LoggingArray::asLogging(ad)->wrapped;
  wrapped->incRefCount();
  return wrapped;
}

//////////////////////////////////////////////////////////////////////////////

void LoggingLayout::release(ArrayData* ad) const {
  LoggingArray::asLogging(ad)->wrapped->decRefCount();
}
size_t LoggingLayout::size(const ArrayData* ad) const {
  return LoggingArray::asLogging(ad)->wrapped->size();
}
bool LoggingLayout::isVectorData(const ArrayData* ad) const {
  return LoggingArray::asLogging(ad)->wrapped->isVectorData();
}

TypedValue LoggingLayout::getInt(const ArrayData* ad, int64_t k) const {
  return LoggingArray::asLogging(ad)->wrapped->get(k);
}
TypedValue LoggingLayout::getStr(const ArrayData* ad, const StringData* k) const {
  return LoggingArray::asLogging(ad)->wrapped->get(k);
}
TypedValue LoggingLayout::getKey(const ArrayData* ad, ssize_t pos) const {
  return LoggingArray::asLogging(ad)->wrapped->nvGetKey(pos);
}
TypedValue LoggingLayout::getVal(const ArrayData* ad, ssize_t pos) const {
  return LoggingArray::asLogging(ad)->wrapped->nvGetVal(pos);
}
ssize_t LoggingLayout::getIntPos(const ArrayData* ad, int64_t k) const {
  return LoggingArray::asLogging(ad)->wrapped->nvGetIntPos(k);
}
ssize_t LoggingLayout::getStrPos(const ArrayData* ad, const StringData* k) const {
  return LoggingArray::asLogging(ad)->wrapped->nvGetStrPos(k);
}

namespace {

ArrayData* escalate(LoggingArray* lad, ArrayData* result) {
  return result == lad->wrapped ? lad : LoggingArray::MakeFromVanilla(result);
}

arr_lval escalate(LoggingArray* lad, arr_lval result) {
  return arr_lval{escalate(lad, result.arr), result};
}

template <typename F>
decltype(auto) mutate(ArrayData* ad, F&& f) {
  auto lad = LoggingArray::asLogging(ad);
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  SCOPE_EXIT { if (cow) lad->wrapped->decRefCount(); };
  return escalate(lad, f(lad->wrapped));
}

}

arr_lval LoggingLayout::lvalInt(ArrayData* ad, int64_t k) const {
  return mutate(ad, [&](ArrayData* arr) { return arr->lval(k); });
}
arr_lval LoggingLayout::lvalStr(ArrayData* ad, StringData* k) const {
  return mutate(ad, [&](ArrayData* arr) { return arr->lval(k); });
}
ArrayData* LoggingLayout::setInt(ArrayData* ad, int64_t k, TypedValue v) const {
  return mutate(ad, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingLayout::setStr(ArrayData* ad, StringData* k, TypedValue v) const {
  return mutate(ad, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingLayout::removeInt(ArrayData* ad, int64_t k) const {
  return mutate(ad, [&](ArrayData* w) { return w->remove(k); });
}
ArrayData* LoggingLayout::removeStr(ArrayData* ad, const StringData* k) const {
  return mutate(ad, [&](ArrayData* w) { return w->remove(k); });
}

ssize_t LoggingLayout::iterBegin(const ArrayData* ad) const {
  return LoggingArray::asLogging(ad)->wrapped->iter_begin();
}
ssize_t LoggingLayout::iterLast(const ArrayData* ad) const {
  return LoggingArray::asLogging(ad)->wrapped->iter_last();
}
ssize_t LoggingLayout::iterEnd(const ArrayData* ad) const {
  return LoggingArray::asLogging(ad)->wrapped->iter_end();
}
ssize_t LoggingLayout::iterAdvance(const ArrayData* ad, ssize_t prev) const {
  return LoggingArray::asLogging(ad)->wrapped->iter_advance(prev);
}
ssize_t LoggingLayout::iterRewind(const ArrayData* ad, ssize_t prev) const {
  return LoggingArray::asLogging(ad)->wrapped->iter_rewind(prev);
}

ArrayData* LoggingLayout::append(ArrayData* ad, TypedValue v) const {
  return mutate(ad, [&](ArrayData* w) { return w->append(v); });
}
ArrayData* LoggingLayout::prepend(ArrayData* ad, TypedValue v) const {
  return mutate(ad, [&](ArrayData* w) { return w->prepend(v); });
}
ArrayData* LoggingLayout::plusEq(ArrayData* ad, const ArrayData* arr) const {
  return mutate(ad, [&](ArrayData* w) { return w->plusEq(arr); });
}
ArrayData* LoggingLayout::merge(ArrayData* ad, const ArrayData* arr) const {
  return mutate(ad, [&](ArrayData* w) { return w->merge(arr); });
}
ArrayData* LoggingLayout::pop(ArrayData* ad, Variant& ret) const {
  return mutate(ad, [&](ArrayData* w) { return w->pop(ret); });
}
ArrayData* LoggingLayout::dequeue(ArrayData* ad, Variant& ret) const {
  return mutate(ad, [&](ArrayData* w) { return w->pop(ret); });
}
ArrayData* LoggingLayout::renumber(ArrayData* ad) const {
  return mutate(ad, [&](ArrayData* w) { return w->renumber(); });
}

namespace {

template <typename F>
ArrayData* conv(ArrayData* ad, F&& f) {
  auto lad = LoggingArray::asLogging(ad);
  auto ret = f(lad->wrapped);
  return ret == lad->wrapped ? lad : LoggingArray::MakeFromVanilla(ret);
}

}

ArrayData* LoggingLayout::copy(const ArrayData* ad) const {
  return LoggingArray::MakeFromVanilla(ad->copy());
}
ArrayData* LoggingLayout::toPHPArray(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toPHPArray(copy); });
}
ArrayData* LoggingLayout::toPHPArrayIntishCast(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toPHPArrayIntishCast(copy); });
}
ArrayData* LoggingLayout::toVArray(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toVArray(copy); });
}
ArrayData* LoggingLayout::toDArray(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toDArray(copy); });
}
ArrayData* LoggingLayout::toVec(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toVec(copy); });
}
ArrayData* LoggingLayout::toDict(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toDict(copy); });
}
ArrayData* LoggingLayout::toKeyset(ArrayData* ad, bool copy) const {
  return conv(ad, [=](ArrayData* w) { return w->toKeyset(copy); });
}

//////////////////////////////////////////////////////////////////////////////

}}
