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

#include "hphp/runtime/base/logging-array.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP {

namespace bespoke {

namespace {

LoggingLayout* s_phpArrayLayout = new LoggingLayout(KindOfArray);
LoggingLayout* s_dictLayout = new LoggingLayout(KindOfDict);
LoggingLayout* s_vecLayout = new LoggingLayout(KindOfVec);
LoggingLayout* s_keysetLayout = new LoggingLayout(KindOfKeyset);

}

bool LoggingArray::checkInvariants() const {
  assertx(wrapped->toDataType() == toDataType());
  assertx(wrapped->kindIsValid());
  assertx(wrapped->hasExactlyOneRef() || wrapped->isStatic());
  assertx(wrapped->isVanilla());
  assertx(dvArray() == wrapped->dvArray());
  return true;
}

LoggingArray* LoggingArray::fromBespoke(BespokeArray* bad) {
  return reinterpret_cast<LoggingArray*>(bad);
}

LoggingArray* LoggingArray::MakeFromVanilla(ArrayData* ad) {
  auto const si = MemoryManager::size2Index(sizeof(LoggingArray));
  auto lad = static_cast<LoggingArray*>(tl_heap->objMallocIndex(si));

  assertx(ad->isVanilla());
  auto const hk = [&]{
    switch (ad->kind()) {
    case kPackedKind:
    case kMixedKind:
    case kPlainKind:
    case kGlobalsKind:
    case kRecordKind:
      return HeaderKind::BespokeArray;
    case kDictKind:
      return HeaderKind::BespokeDict;
    case kVecKind:
      return HeaderKind::BespokeVec;
    case kKeysetKind:
      return HeaderKind::BespokeKeyset;
    case kBespokeArrayKind:
    case kBespokeDictKind:
    case kBespokeVecKind:
    case kBespokeKeysetKind:
    case kNumKinds:
      break;
    }
    always_assert(false);
  }();
  lad->initHeader(hk, OneReference);
  lad->setDVArray(ad->dvArray());

  lad->wrapped = ad;
  lad->setLayout(LoggingLayout::layoutForVanillaArray(ad));

  assertx(lad->checkInvariants());

  return lad;
}

Layout* LoggingLayout::layoutForVanillaArray(const ArrayData* ad) {
  switch (ad->kind()) {
  case ArrayData::kPackedKind:
  case ArrayData::kMixedKind:
  case ArrayData::kPlainKind:
  case ArrayData::kGlobalsKind:
    return s_phpArrayLayout;
  case ArrayData::kDictKind:
    return s_dictLayout;
  case ArrayData::kVecKind:
    return s_vecLayout;
  case ArrayData::kKeysetKind:
    return s_keysetLayout;
  case ArrayData::kRecordKind:
  case ArrayData::kBespokeArrayKind:
  case ArrayData::kBespokeDictKind:
  case ArrayData::kBespokeVecKind:
  case ArrayData::kBespokeKeysetKind:
  case ArrayData::kNumKinds:
    break;
  }
  always_assert(false);
}

DataType LoggingLayout::datatype() const {
  return m_datatype;
}

void LoggingLayout::scan(const BespokeArray* ad, type_scan::Scanner& scan) const {
  scan.scan(LoggingArray::fromBespoke(ad)->wrapped);
}

size_t LoggingLayout::heapSize(const BespokeArray* ad) const {
  return sizeof(BespokeArray);
}

ArrayData* LoggingLayout::escalate(const BespokeArray* ad) const {
  return LoggingArray::fromBespoke(ad)->wrapped->copy();
}

void LoggingLayout::release(BespokeArray* ad) const {
  auto lad = LoggingArray::fromBespoke(ad);
  assertx(lad->checkInvariants());
  lad->wrapped->decRefCount();
}

ArrayData* LoggingLayout::copy(const BespokeArray* ad) const {
  return LoggingArray::MakeFromVanilla(ad->copy());
}

ArrayData* LoggingLayout::copyStatic(const BespokeArray* ad) const {
  always_assert(false); // nyi
}

TypedValue LoggingLayout::getInt(const BespokeArray* ad, int64_t k) const {
  return LoggingArray::fromBespoke(ad)->wrapped->at(k);
}

TypedValue LoggingLayout::getStr(const BespokeArray* ad, const StringData* k) const {
  return LoggingArray::fromBespoke(ad)->wrapped->at(k);
}

ssize_t LoggingLayout::getIntPos(const BespokeArray* ad, int64_t k) const {
  return LoggingArray::fromBespoke(ad)->wrapped->nvGetIntPos(k);
}

ssize_t LoggingLayout::getStrPos(const BespokeArray* ad, const StringData* k) const {
  return LoggingArray::fromBespoke(ad)->wrapped->nvGetStrPos(k);
}

TypedValue LoggingLayout::getKey(const BespokeArray* ad, ssize_t pos) const {
  return LoggingArray::fromBespoke(ad)->wrapped->nvGetKey(pos);
}

TypedValue LoggingLayout::getVal(const BespokeArray* ad, ssize_t pos) const {
  return LoggingArray::fromBespoke(ad)->wrapped->nvGetVal(pos);
}

namespace {

template <typename F>
decltype(auto) simulateWrappedCow(LoggingArray* lad, F&& f) {
  auto const needsRefCount = lad->cowCheck();
  if (needsRefCount) {
    lad->wrapped->incRefCount();
  }
  SCOPE_EXIT {
    if (needsRefCount) {
      lad->wrapped->decRefCount();
    }
  };

  return f();
}

template <typename F>
ArrayData* mutate(LoggingArray* lad, F&& f) {
  return simulateWrappedCow(lad, [&]{
    ArrayData* ret = f(lad->wrapped);
    return ret != lad->wrapped
      ? LoggingArray::MakeFromVanilla(ret)
      : ret;
  });
}

}

arr_lval LoggingLayout::lvalInt(BespokeArray* ad, int64_t k) const {
  auto lad = LoggingArray::fromBespoke(ad);
  return simulateWrappedCow(lad, [&]{
    auto ret = lad->wrapped->lval(k);
    if (ret.arr != lad->wrapped) {
      return arr_lval{LoggingArray::MakeFromVanilla(ret.arr), ret};
    } else {
      return ret;
    }
  });
}

arr_lval LoggingLayout::lvalStr(BespokeArray* ad, StringData* k) const {
  auto lad = LoggingArray::fromBespoke(ad);
  return simulateWrappedCow(lad, [&]{
    auto ret = lad->wrapped->lval(k);
    if (ret.arr != lad->wrapped) {
      return arr_lval{LoggingArray::MakeFromVanilla(ret.arr), ret};
    } else {
      return ret;
    }
  });
}

size_t LoggingLayout::size(const BespokeArray* ad) const {
  return LoggingArray::fromBespoke(ad)->wrapped->size();
}

bool LoggingLayout::isVectorData(const BespokeArray* ad) const {
  return LoggingArray::fromBespoke(ad)->wrapped->isVectorData();
}

bool LoggingLayout::existsInt(const BespokeArray* ad, int64_t k) const {
  return LoggingArray::fromBespoke(ad)->wrapped->exists(k);
}

bool LoggingLayout::existsStr(const BespokeArray* ad, const StringData* k) const {
  return LoggingArray::fromBespoke(ad)->wrapped->exists(k);
}

ArrayData* LoggingLayout::setInt(BespokeArray* ad, int64_t k, TypedValue v) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->set(k, v);
                });
}

ArrayData* LoggingLayout::setStr(BespokeArray* ad, StringData* k, TypedValue v) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->set(k, v);
                });
}

ArrayData* LoggingLayout::deleteInt(BespokeArray* ad, int64_t k) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->remove(k);
                });
}

ArrayData* LoggingLayout::deleteStr(BespokeArray* ad, const StringData* k) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->remove(k);
                });
}

ssize_t LoggingLayout::iterBegin(const BespokeArray* ad) const {
  return LoggingArray::fromBespoke(ad)->wrapped->iter_begin();
}

ssize_t LoggingLayout::iterLast(const BespokeArray* ad) const {
  return LoggingArray::fromBespoke(ad)->wrapped->iter_last();
}

ssize_t LoggingLayout::iterEnd(const BespokeArray* ad) const {
  return LoggingArray::fromBespoke(ad)->wrapped->iter_end();
}

ssize_t LoggingLayout::iterAdvance(const BespokeArray* ad, ssize_t prev) const {
  return LoggingArray::fromBespoke(ad)->wrapped->iter_advance(prev);
}

ssize_t LoggingLayout::iterRewind(const BespokeArray* ad, ssize_t prev) const {
  return LoggingArray::fromBespoke(ad)->wrapped->iter_rewind(prev);
}

ArrayData* LoggingLayout::append(BespokeArray* ad, TypedValue v) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->append(v);
                });
}

ArrayData* LoggingLayout::prepend(BespokeArray* ad, TypedValue v) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->prepend(v);
                });
}

ArrayData* LoggingLayout::plusEq(BespokeArray* ad, const ArrayData* arr) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->plusEq(arr);
                });
}

ArrayData* LoggingLayout::merge(BespokeArray* ad, const ArrayData* arr) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->merge(arr);
                });
}

ArrayData* LoggingLayout::pop(BespokeArray* ad, Variant& ret) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->pop(ret);
                });
}

ArrayData* LoggingLayout::dequeue(BespokeArray* ad, Variant& ret) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->pop(ret);
                });
}

ArrayData* LoggingLayout::renumber(BespokeArray* ad) const {
  return mutate(LoggingArray::fromBespoke(ad),
                [&](ArrayData* w) {
                  return w->renumber();
                });
}

namespace {

template <typename Impl>
ArrayData* conversion(LoggingArray* lad, Impl&& impl) {
  auto ad = impl(lad->wrapped);
  if (ad != lad->wrapped) {
    return LoggingArray::MakeFromVanilla(ad);
  } else {
    lad->setLayout(LoggingLayout::layoutForVanillaArray(ad));
    assertx(lad->checkInvariants());
    return lad;
  }
}

}

ArrayData* LoggingLayout::toPHPArray(BespokeArray* ad, bool copy) const {
  assertx(IMPLIES(!copy, ad->hasExactlyOneRef()));
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toPHPArray(copy); }
  );
}

ArrayData* LoggingLayout::toPHPArrayIntishCast(BespokeArray* ad, bool copy) const {
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toPHPArrayIntishCast(copy); }
  );
}

ArrayData* LoggingLayout::toVec(BespokeArray* ad, bool copy) const {
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toVec(copy); }
  );
}

ArrayData* LoggingLayout::toDict(BespokeArray* ad, bool copy) const {
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toDict(copy); }
  );
}

ArrayData* LoggingLayout::toKeyset(BespokeArray* ad, bool copy) const {
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toKeyset(copy); }
  );
}

ArrayData* LoggingLayout::toVArray(BespokeArray* ad, bool copy) const {
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toVArray(copy); }
  );
}

ArrayData* LoggingLayout::toDArray(BespokeArray* ad, bool copy) const {
  return conversion(
    LoggingArray::fromBespoke(ad),
    [=](ArrayData* arr) { return arr->toDArray(copy); }
  );
}


} // namespace bespoke

} // namespace HPHP
