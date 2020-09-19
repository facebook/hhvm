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
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <tbb/concurrent_hash_map.h>

#include <algorithm>
#include <atomic>

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////////////

constexpr size_t kSizeIndex = 2;
static_assert(kSizeIndex2Size[kSizeIndex] >= sizeof(LoggingArray),
              "kSizeIndex must be large enough to fit a LoggingArray");
static_assert(kSizeIndex == 0 ||
              kSizeIndex2Size[kSizeIndex - 1] < sizeof(LoggingArray),
              "kSizeIndex must be the smallest size for LoggingArray");

Layout* s_layout = new LoggingLayout();
std::atomic<bool> g_emitLoggingArrays;

// The bespoke kind for a vanilla kind.
HeaderKind getBespokeKind(ArrayData::ArrayKind kind) {
  assertx(!(kind & ArrayData::kBespokeKindMask));
  return HeaderKind(kind | ArrayData::kBespokeKindMask);
}

template <typename... Ts>
void logEvent(const ArrayData* ad, EntryTypes newTypes, ArrayOp op,
              Ts&&... args) {
  auto const lad = LoggingArray::asLogging(ad);
  lad->profile->logEntryTypes(lad->entryTypes, newTypes);
  lad->profile->logEvent(op, std::forward<Ts>(args)...);
}

template <typename... Ts>
void logEvent(const ArrayData* ad, ArrayOp op, Ts&&... args) {
  auto const lad = LoggingArray::asLogging(ad);
  logEvent(ad, lad->entryTypes, op, std::forward<Ts>(args)...);
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

void setLoggingEnabled(bool val) {
  g_emitLoggingArrays.store(val, std::memory_order_relaxed);
}

ArrayData* maybeMakeLoggingArray(ArrayData* ad) {
  if (!g_emitLoggingArrays.load(std::memory_order_relaxed)) return ad;
  auto const sk = getSrcKey();
  if (!sk.valid()) {
    FTRACE(5, "VMRegAnchor failed for maybleEnableLogging.\n");
    return ad;
  }

  auto const profile = getLoggingProfile(sk, ad);
  if (!profile) return ad;

  auto const shouldEmitBespoke = [&] {
    if (shouldTestBespokeArrayLikes()) {
      FTRACE(5, "Observe rid: {}\n", requestCount());
      return !jit::mcgen::retranslateAllEnabled() || requestCount() % 2 == 1;
    } else {
      if (RO::EvalEmitLoggingArraySampleRate == 0) return false;

      auto const skCount = profile->sampleCount++;
      FTRACE(5, "Observe SrcKey count: {}\n", skCount);
      return (skCount - 1) % RO::EvalEmitLoggingArraySampleRate == 0;
    }
  }();

  if (!shouldEmitBespoke) {
    FTRACE(5, "Emit vanilla at {}\n", sk.getSymbol());
    return ad;
  }

  FTRACE(5, "Emit bespoke at {}\n", sk.getSymbol());
  profile->loggingArraysEmitted++;
  if (ad->isStatic()) return profile->staticArray;

  // Non-static array constructors are basically a sequence of sets or appends.
  // We already log these events at the correct granularity; re-use that logic.
  IterateKVNoInc(ad, [&](auto k, auto v) {
    tvIsString(k) ? profile->logEvent(ArrayOp::ConstructStr, val(k).pstr, v)
                  : profile->logEvent(ArrayOp::ConstructInt, val(k).num, v);
  });
  return LoggingArray::Make(ad, profile, EntryTypes::ForArray(ad));
}

const ArrayData* maybeMakeLoggingArray(const ArrayData* ad) {
  return maybeMakeLoggingArray(const_cast<ArrayData*>(ad));
}

//////////////////////////////////////////////////////////////////////////////

LoggingArray* LoggingArray::Make(ArrayData* ad, LoggingProfile* profile,
                                 EntryTypes ms) {
  assertx(ad->isVanilla());

  auto lad = static_cast<LoggingArray*>(tl_heap->objMallocIndex(kSizeIndex));
  lad->initHeader_16(getBespokeKind(ad->kind()), OneReference, ad->auxBits());
  lad->m_size = ad->size();
  lad->setLayoutRaw(s_layout);
  lad->wrapped = ad;
  lad->profile = profile;
  lad->entryTypes = ms;
  assertx(lad->checkInvariants());
  return lad;
}

LoggingArray* LoggingArray::MakeStatic(ArrayData* ad, LoggingProfile* profile) {
  assertx(ad->isVanilla());
  assertx(ad->isStatic());

  auto const size = sizeof(LoggingArray);
  auto lad = static_cast<LoggingArray*>(
      RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size));
  lad->initHeader_16(getBespokeKind(ad->kind()), StaticValue, ad->auxBits());
  lad->m_size = ad->size();
  lad->setLayoutRaw(s_layout);
  lad->wrapped = ad;
  lad->profile = profile;
  lad->entryTypes = EntryTypes::ForArray(ad);
  assertx(lad->checkInvariants());
  return lad;
}

void LoggingArray::FreeStatic(LoggingArray* lad) {
  assertx(lad->wrapped->isStatic());
  RO::EvalLowStaticArrays ? low_free(lad) : uncounted_free(lad);
}

bool LoggingArray::checkInvariants() const {
  assertx(!isVanilla());
  assertx(kindIsValid());
  assertx(wrapped->isVanilla());
  assertx(wrapped->kindIsValid());
  assertx(wrapped->size() == size());
  assertx(wrapped->toDataType() == toDataType());
  assertx(layoutRaw() == s_layout);
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

void LoggingArray::updateKindAndSize() {
  if (hasExactlyOneRef()) {
    m_kind = getBespokeKind(wrapped->kind());
    m_size = wrapped->size();
  }
  assertx(checkInvariants());
}

void LoggingArray::logReachEvent(TransID tid, size_t guardIdx) {
  profile->logReach(tid, guardIdx);
}

std::string LoggingLayout::describe() const {
  return "Logging";
}

size_t LoggingLayout::heapSize(const ArrayData*) const {
  return sizeof(LoggingArray);
}

size_t LoggingLayout::align(const ArrayData*) const {
  return alignof(LoggingArray);
}

void LoggingLayout::scan(const ArrayData* ad, type_scan::Scanner& scan) const {
  logEvent(ad, ArrayOp::Scan);
  scan.scan(LoggingArray::asLogging(ad)->wrapped);
}

ArrayData* LoggingLayout::escalateToVanilla(
    const ArrayData* ad, const char* reason) const {
  logEvent(ad, ArrayOp::EscalateToVanilla, makeStaticString(reason));
  auto wrapped = LoggingArray::asLogging(ad)->wrapped;
  wrapped->incRefCount();
  return wrapped;
}

void LoggingLayout::convertToUncounted(
    ArrayData* ad, DataWalker::PointerMap* seen) const {
  logEvent(ad, ArrayOp::ConvertToUncounted);
  auto lad = LoggingArray::asLogging(ad);
  auto tv = make_array_like_tv(lad->wrapped);
  ConvertTvToUncounted(&tv, seen);
  lad->wrapped = val(tv).parr;
}

void LoggingLayout::releaseUncounted(ArrayData* ad) const {
  logEvent(ad, ArrayOp::ReleaseUncounted);
  auto tv = make_array_like_tv(LoggingArray::asLogging(ad)->wrapped);
  ReleaseUncountedTv(&tv);
}

void LoggingLayout::release(ArrayData* ad) const {
  logEvent(ad, ArrayOp::Release);
  LoggingArray::asLogging(ad)->wrapped->decRefAndRelease();
  tl_heap->objFreeIndex(ad, kSizeIndex);
}

//////////////////////////////////////////////////////////////////////////////
// Accessors

bool LoggingLayout::isVectorData(const ArrayData* ad) const {
  logEvent(ad, ArrayOp::IsVectorData);
  return LoggingArray::asLogging(ad)->wrapped->isVectorData();
}
TypedValue LoggingLayout::getInt(const ArrayData* ad, int64_t k) const {
  logEvent(ad, ArrayOp::GetInt, k);
  return LoggingArray::asLogging(ad)->wrapped->get(k);
}
TypedValue LoggingLayout::getStr(const ArrayData* ad, const StringData* k) const {
  logEvent(ad, ArrayOp::GetStr, k);
  return LoggingArray::asLogging(ad)->wrapped->get(k);
}
TypedValue LoggingLayout::getKey(const ArrayData* ad, ssize_t pos) const {
  return LoggingArray::asLogging(ad)->wrapped->nvGetKey(pos);
}
TypedValue LoggingLayout::getVal(const ArrayData* ad, ssize_t pos) const {
  return LoggingArray::asLogging(ad)->wrapped->nvGetVal(pos);
}
ssize_t LoggingLayout::getIntPos(const ArrayData* ad, int64_t k) const {
  logEvent(ad, ArrayOp::GetIntPos, k);
  return LoggingArray::asLogging(ad)->wrapped->nvGetIntPos(k);
}
ssize_t LoggingLayout::getStrPos(const ArrayData* ad, const StringData* k) const {
  logEvent(ad, ArrayOp::GetStrPos, k);
  return LoggingArray::asLogging(ad)->wrapped->nvGetStrPos(k);
}

//////////////////////////////////////////////////////////////////////////////
// Mutations

namespace {
ArrayData* escalate(LoggingArray* lad, ArrayData* result) {
  lad->updateKindAndSize();
  if (result == lad->wrapped) return lad;
  return LoggingArray::Make(result, lad->profile, lad->entryTypes);
}

ArrayData* escalate(LoggingArray* lad, ArrayData* result, EntryTypes ms) {
  lad->updateKindAndSize();
  if (result == lad->wrapped) {
    lad->entryTypes = ms;
    return lad;
  }

  return LoggingArray::Make(result, lad->profile, ms);
}

arr_lval escalate(LoggingArray* lad, arr_lval result) {
  return arr_lval{escalate(lad, result.arr), result};
}

arr_lval escalate(LoggingArray* lad, arr_lval result, EntryTypes ms) {
  return arr_lval{escalate(lad, result.arr, ms), result};
}

template <typename F>
decltype(auto) mutate(ArrayData* ad, F&& f) {
  auto lad = LoggingArray::asLogging(ad);
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  SCOPE_EXIT { if (cow) lad->wrapped->decRefCount(); };
  return escalate(lad, f(lad->wrapped));
}

template <typename F>
decltype(auto) mutate(ArrayData* ad, EntryTypes ms, F&& f) {
  auto lad = LoggingArray::asLogging(ad);
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  SCOPE_EXIT { if (cow) lad->wrapped->decRefCount(); };
  return escalate(lad, f(lad->wrapped), ms);
}
}

arr_lval LoggingLayout::lvalInt(ArrayData* ad, int64_t k) const {
  // Lvals cannot insert new keys, so the KeyTypes are unchanged, but outside
  // code can change the value types arbitrarily with the lval
  auto const ms = LoggingArray::asLogging(ad)->entryTypes.pessimizeValueTypes();
  auto const val = getInt(ad, k);
  logEvent(ad, ms, ArrayOp::LvalInt, k, val);
  return mutate(ad, ms, [&](ArrayData* arr) { return arr->lval(k); });
}
arr_lval LoggingLayout::lvalStr(ArrayData* ad, StringData* k) const {
  // Lvals cannot insert new keys, so the KeyTypes are unchanged, but outside
  // code can change the value types arbitrarily with the lval
  auto const ms = LoggingArray::asLogging(ad)->entryTypes.pessimizeValueTypes();
  auto const val = getStr(ad, k);
  logEvent(ad, ms, ArrayOp::LvalInt, k, val);
  return mutate(ad, ms, [&](ArrayData* arr) { return arr->lval(k); });
}
ArrayData* LoggingLayout::setInt(ArrayData* ad, int64_t k, TypedValue v) const {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = LoggingArray::asLogging(ad)->
    entryTypes.withKV(make_tv<KindOfInt64>(k), v);
  logEvent(ad, ms, ArrayOp::SetInt, k, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingLayout::setStr(ArrayData* ad, StringData* k, TypedValue v) const {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = LoggingArray::asLogging(ad)->
    entryTypes.withKV(make_tv<KindOfString>(k), v);
  logEvent(ad, ms, ArrayOp::SetStr, k, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingLayout::removeInt(ArrayData* ad, int64_t k) const {
  logEvent(ad, ArrayOp::RemoveInt, k);
  return mutate(ad, [&](ArrayData* w) { return w->remove(k); });
}
ArrayData* LoggingLayout::removeStr(ArrayData* ad, const StringData* k) const {
  logEvent(ad, ArrayOp::RemoveStr, k);
  return mutate(ad, [&](ArrayData* w) { return w->remove(k); });
}

ssize_t LoggingLayout::iterBegin(const ArrayData* ad) const {
  logEvent(ad, ArrayOp::IterBegin);
  return LoggingArray::asLogging(ad)->wrapped->iter_begin();
}
ssize_t LoggingLayout::iterLast(const ArrayData* ad) const {
  logEvent(ad, ArrayOp::IterLast);
  return LoggingArray::asLogging(ad)->wrapped->iter_last();
}
ssize_t LoggingLayout::iterEnd(const ArrayData* ad) const {
  logEvent(ad, ArrayOp::IterEnd);
  return LoggingArray::asLogging(ad)->wrapped->iter_end();
}
ssize_t LoggingLayout::iterAdvance(const ArrayData* ad, ssize_t prev) const {
  logEvent(ad, ArrayOp::IterAdvance);
  return LoggingArray::asLogging(ad)->wrapped->iter_advance(prev);
}
ssize_t LoggingLayout::iterRewind(const ArrayData* ad, ssize_t prev) const {
  logEvent(ad, ArrayOp::IterRewind);
  return LoggingArray::asLogging(ad)->wrapped->iter_rewind(prev);
}

ArrayData* LoggingLayout::append(ArrayData* ad, TypedValue v) const {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = LoggingArray::asLogging(ad)->entryTypes.withV(v);
  logEvent(ad, ms, ArrayOp::Append, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->append(v); });
}
ArrayData* LoggingLayout::prepend(ArrayData* ad, TypedValue v) const {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = LoggingArray::asLogging(ad)->entryTypes.withV(v);
  logEvent(ad, ms, ArrayOp::Prepend, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->prepend(v); });
}
ArrayData* LoggingLayout::merge(ArrayData* ad, const ArrayData* arr) const {
  logEvent(ad, ArrayOp::Merge);
  return mutate(ad, [&](ArrayData* w) { return w->merge(arr); });
}
ArrayData* LoggingLayout::pop(ArrayData* ad, Variant& ret) const {
  logEvent(ad, ArrayOp::Pop);
  return mutate(ad, [&](ArrayData* w) { return w->pop(ret); });
}
ArrayData* LoggingLayout::dequeue(ArrayData* ad, Variant& ret) const {
  logEvent(ad, ArrayOp::Dequeue);
  return mutate(ad, [&](ArrayData* w) { return w->dequeue(ret); });
}
ArrayData* LoggingLayout::renumber(ArrayData* ad) const {
  logEvent(ad, ArrayOp::Renumber);
  return mutate(ad, [&](ArrayData* w) { return w->renumber(); });
}

//////////////////////////////////////////////////////////////////////////////

namespace {
template <typename F>
ArrayData* convert(ArrayData* ad, F&& f) {
  auto const lad = LoggingArray::asLogging(ad);
  auto const wrapped = lad->wrapped;
  auto const result = f(wrapped);
  lad->updateKindAndSize();

  // Reuse existing profile for in-place conversions.
  if (result == wrapped) return lad;

  // Reuse existing profile for conversions that don't change array layout.
  if ((wrapped->hasVanillaMixedLayout() && result->hasVanillaMixedLayout()) ||
      (wrapped->hasVanillaPackedLayout() && result->hasVanillaPackedLayout()) ||
      (wrapped->isKeysetKind() && result->isKeysetKind())) {
    return LoggingArray::Make(result, lad->profile, lad->entryTypes);
  }

  // If the layout has changed, make a fresh profile at the new creation site.
  auto const sk = getSrcKey();
  if (!sk.valid()) {
    FTRACE(5, "VMRegAnchor failed for convert operation.\n");
    return result;
  }

  auto const profile = getLoggingProfile(sk, result);
  if (!profile) return result;

  // We expect 1 / SampleRate LoggingArrays to make it here. Bump sampleCount
  // for the cast site accordingly. Since we sample the second array created
  // at each site, we start the count here at 2, not SampleRate.
  //
  // TODO(kshaunak): Treat this site like a constructor and log pseudo-ops.
  uint64_t expected = 0;
  if (!profile->sampleCount.compare_exchange_strong(expected, 2)) {
    profile->sampleCount += RO::EvalEmitLoggingArraySampleRate;
  }
  profile->loggingArraysEmitted++;

  return LoggingArray::Make(result, profile, lad->entryTypes);
}
}

ArrayData* LoggingLayout::copy(const ArrayData* ad) const {
  logEvent(ad, ArrayOp::Copy);
  auto const lad = LoggingArray::asLogging(ad);
  return LoggingArray::Make(lad->wrapped->copy(), lad->profile,
                            lad->entryTypes);
}
ArrayData* LoggingLayout::toVArray(ArrayData* ad, bool copy) const {
  logEvent(ad, ArrayOp::ToVArray);
  return convert(ad, [=](ArrayData* w) { return w->toVArray(copy); });
}
ArrayData* LoggingLayout::toDArray(ArrayData* ad, bool copy) const {
  logEvent(ad, ArrayOp::ToDArray);
  return convert(ad, [=](ArrayData* w) { return w->toDArray(copy); });
}
ArrayData* LoggingLayout::toVec(ArrayData* ad, bool copy) const {
  logEvent(ad, ArrayOp::ToVec);
  return convert(ad, [=](ArrayData* w) { return w->toVec(copy); });
}
ArrayData* LoggingLayout::toDict(ArrayData* ad, bool copy) const {
  logEvent(ad, ArrayOp::ToDict);
  return convert(ad, [=](ArrayData* w) { return w->toDict(copy); });
}
ArrayData* LoggingLayout::toKeyset(ArrayData* ad, bool copy) const {
  logEvent(ad, ArrayOp::ToKeyset);
  return convert(ad, [=](ArrayData* w) { return w->toKeyset(copy); });
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

}}
