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
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "folly/SharedMutex.h"
#include "folly/container/F14Map.h"

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

ArrayData* maybeEnableLogging(ArrayData* ad) {
  if (!RO::EvalEmitBespokeArrayLikes) return ad;
  VMRegAnchor _;
  auto const fp = vmfp();
  auto const sk = SrcKey(fp->func(), vmpc(), resumeModeFromActRec(fp));
  return ad->isStatic() ? LoggingArray::MakeFromStatic(ad, sk)
                        : LoggingArray::MakeFromVanilla(ad, sk);
}

const ArrayData* maybeEnableLogging(const ArrayData* ad) {
  return maybeEnableLogging(const_cast<ArrayData*>(ad));
}

//////////////////////////////////////////////////////////////////////////////

namespace {

constexpr size_t kSizeIndex = 1;
static_assert(kSizeIndex2Size[kSizeIndex] >= sizeof(LoggingArray),
              "kSizeIndex must be large enough to fit a LoggingArray");
static_assert(kSizeIndex2Size[kSizeIndex - 1] < sizeof(LoggingArray),
              "kSizeIndex must be the smallest size for LoggingArray");

LoggingLayout* s_layout = new LoggingLayout();

// The bespoke kind for a vanilla kind. Assumes the kind supports bespokes.
HeaderKind getBespokeKind(ArrayData::ArrayKind kind) {
  switch (kind) {
    case ArrayData::kPackedKind: return HeaderKind::BespokeVArray;
    case ArrayData::kMixedKind:  return HeaderKind::BespokeDArray;
    case ArrayData::kVecKind:    return HeaderKind::BespokeVec;
    case ArrayData::kDictKind:   return HeaderKind::BespokeDict;
    case ArrayData::kKeysetKind: return HeaderKind::BespokeKeyset;

    case ArrayData::kBespokeVArrayKind:
    case ArrayData::kBespokeDArrayKind:
    case ArrayData::kBespokeVecKind:
    case ArrayData::kBespokeDictKind:
    case ArrayData::kBespokeKeysetKind:
    case ArrayData::kNumKinds:
      always_assert(false);
  }
  not_reached();
}

}

bool LoggingArray::checkInvariants() const {
  assertx(!isVanilla());
  assertx(kindIsValid());
  assertx(wrapped->isVanilla());
  assertx(wrapped->kindIsValid());
  assertx(wrapped->toDataType() == toDataType());
  assertx(asBespoke(this)->layout() == s_layout);
  assertx(m_kind == getBespokeKind(wrapped->kind()));
  assertx(isLegacyArray() == wrapped->isLegacyArray());
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

LoggingArray* LoggingArray::MakeFromVanilla(ArrayData* ad, SrcKey sk) {
  assertx(ad->isVanilla());
  assertx(ad->getPosition() == ad->iter_begin());

  auto lad = static_cast<LoggingArray*>(tl_heap->objMallocIndex(kSizeIndex));
  lad->initHeader_16(getBespokeKind(ad->kind()), OneReference,
                     ad->isLegacyArray() ? kLegacyArray : 0);
  lad->setLayout(s_layout);
  lad->wrapped = ad;
  lad->srckey = sk;
  assertx(lad->checkInvariants());
  return lad;
}

LoggingArray* LoggingArray::MakeFromStatic(ArrayData* ad, SrcKey sk) {
  assertx(ad->isStatic());
  assertx(ad->isVanilla());
  assertx(ad->getPosition() == ad->iter_begin());

  static folly::SharedMutex s_mutex;
  static folly::F14FastMap<SrcKey, LoggingArray*, SrcKey::Hasher> s_map;

  auto const result = [&](LoggingArray* lad) {
    assertx(lad->wrapped == ad);
    assertx(lad->checkInvariants());
    return lad;
  };

  {
    folly::SharedMutex::ReadHolder lock(s_mutex);
    auto const it = s_map.find(sk);
    if (it != s_map.end()) return result(it->second);
  }

  folly::SharedMutex::WriteHolder lock(s_mutex);
  auto insert = s_map.insert({sk, nullptr});
  if (!insert.second) return result(insert.first->second);

  auto const size = sizeof(LoggingArray);
  auto lad = static_cast<LoggingArray*>(
    RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size));
  MemoryStats::LogAlloc(AllocKind::StaticArray, size);
  insert.first->second = lad;

  lad->initHeader_16(getBespokeKind(ad->kind()), StaticValue,
                     ad->isLegacyArray() ? kLegacyArray : 0);
  lad->setLayout(s_layout);
  lad->wrapped = ad;
  lad->srckey = sk;
  return result(lad);
}

LoggingArray* LoggingArray::updateKind() {
  auto const kind = getBespokeKind(wrapped->kind());
  assertx(IMPLIES(kind != m_kind, hasExactlyOneRef()));
  m_kind = kind;
  assertx(checkInvariants());
  return this;
}

size_t LoggingLayout::heapSize(const ArrayData*) const {
  return sizeof(LoggingArray);
}
void LoggingLayout::scan(const ArrayData* ad, type_scan::Scanner& scan) const {
  scan.scan(LoggingArray::asLogging(ad)->wrapped);
}

ArrayData* LoggingLayout::escalateToVanilla(
    const ArrayData* ad, const char* /*reason*/) const {
  auto wrapped = LoggingArray::asLogging(ad)->wrapped;
  wrapped->incRefCount();
  return wrapped;
}

void LoggingLayout::convertToUncounted(
    ArrayData* ad, DataWalker::PointerMap* seen) const {
  auto lad = LoggingArray::asLogging(ad);
  auto tv = make_array_like_tv(lad->wrapped);
  ConvertTvToUncounted(&tv, seen);
  lad->wrapped = val(tv).parr;
}

void LoggingLayout::releaseUncounted(ArrayData* ad) const {
  auto tv = make_array_like_tv(LoggingArray::asLogging(ad)->wrapped);
  ReleaseUncountedTv(&tv);
}

//////////////////////////////////////////////////////////////////////////////

void LoggingLayout::release(ArrayData* ad) const {
  LoggingArray::asLogging(ad)->wrapped->decRefAndRelease();
  tl_heap->objFreeIndex(ad, kSizeIndex);
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
  if (result == lad->wrapped) return lad;
  return LoggingArray::MakeFromVanilla(result, lad->srckey);
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
ArrayData* LoggingLayout::merge(ArrayData* ad, const ArrayData* arr) const {
  return mutate(ad, [&](ArrayData* w) { return w->merge(arr); });
}
ArrayData* LoggingLayout::pop(ArrayData* ad, Variant& ret) const {
  return mutate(ad, [&](ArrayData* w) { return w->pop(ret); });
}
ArrayData* LoggingLayout::dequeue(ArrayData* ad, Variant& ret) const {
  return mutate(ad, [&](ArrayData* w) { return w->dequeue(ret); });
}
ArrayData* LoggingLayout::renumber(ArrayData* ad) const {
  return mutate(ad, [&](ArrayData* w) { return w->renumber(); });
}

namespace {

template <typename F>
ArrayData* conv(ArrayData* ad, F&& f) {
  auto const lad = LoggingArray::asLogging(ad);
  auto const result = f(lad->wrapped);
  if (result == lad->wrapped) return lad->updateKind();
  return LoggingArray::MakeFromVanilla(result, lad->srckey);
}

}

ArrayData* LoggingLayout::copy(const ArrayData* ad) const {
  auto const lad = LoggingArray::asLogging(ad);
  return LoggingArray::MakeFromVanilla(lad->wrapped->copy(), lad->srckey);
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

void LoggingLayout::setLegacyArrayInPlace(ArrayData* ad, bool legacy) const {
  assert(ad->hasExactlyOneRef());
  auto const lad = LoggingArray::asLogging(ad);
  if (lad->wrapped->cowCheck()) {
    auto const nad = lad->wrapped->copy();
    lad->wrapped->decRefCount();
    lad->wrapped = nad;
  }
  lad->wrapped->setLegacyArray(legacy);
}

//////////////////////////////////////////////////////////////////////////////

}}
