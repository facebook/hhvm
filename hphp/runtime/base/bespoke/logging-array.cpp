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

auto const s_vtable = fromArray<LoggingArray>();
Layout* s_layout = new Layout("LoggingLayout", &s_vtable);
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
  assertx(ad->isVanilla());
  if (!g_emitLoggingArrays.load(std::memory_order_relaxed)) return ad;
  auto const sk = getSrcKey();
  if (!sk.valid()) {
    FTRACE(5, "VMRegAnchor failed for maybleEnableLogging.\n");
    return ad;
  }

  auto const op = sk.op();
  auto const useStatic = op == Op::Array || op == Op::Vec ||
                         op == Op::Dict || op == Op::Keyset;
  assertx(IMPLIES(useStatic, ad->isStatic()));
  assertx(IMPLIES(ad->isStatic(), useStatic || isArrLikeCastOp(op)));

  // Don't profile static arrays used for TypeStruct tests. Rather than using
  // these arrays, we almost always just do a DataType check on the value.
  if ((op == Op::Array || op == Op::Dict) &&
      sk.advanced().op() == Op::IsTypeStructC) {
    FTRACE(5, "Skipping static array used for TypeStruct test.\n");
    return ad;
  }

  auto const profile = getLoggingProfile(sk, useStatic ? ad : nullptr);
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
  if (useStatic) return profile->staticArray;

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

const Layout* LoggingArray::layout() {
  return s_layout;
}

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

void LoggingArray::updateKindAndLegacy() {
  assertx(hasExactlyOneRef());
  auto const legacy = wrapped->isLegacyArray();
  m_kind = getBespokeKind(wrapped->kind());
  m_aux16 = (m_aux16 & ~kLegacyArray) | (legacy ? kLegacyArray : 0);
  assertx(checkInvariants());
}

void LoggingArray::updateSize() {
  if (hasExactlyOneRef()) {
    m_size = wrapped->size();
  }
  assertx(checkInvariants());
}

void LoggingArray::logReachEvent(TransID transId, uint32_t guardIdx) {
  profile->logReach(transId, guardIdx);
}

void LoggingArray::setLegacyArrayInPlace(bool legacy) {
  assert(hasExactlyOneRef());
  if (wrapped->cowCheck()) {
    wrapped->decRefCount();
    wrapped = wrapped->copy();
  }
  wrapped->setLegacyArray(legacy);
}

//////////////////////////////////////////////////////////////////////////////

size_t LoggingArray::heapSize(const ArrayData*) {
  return sizeof(LoggingArray);
}

size_t LoggingArray::align(const ArrayData*) {
  return alignof(LoggingArray);
}

void LoggingArray::scan(const ArrayData* ad, type_scan::Scanner& scanner) {
  logEvent(ad, ArrayOp::Scan);
  scanner.scan(asLogging(ad)->wrapped);
}

ArrayData* LoggingArray::escalateToVanilla(
    const ArrayData* ad, const char* reason) {
  logEvent(ad, ArrayOp::EscalateToVanilla, makeStaticString(reason));
  auto const wrapped = asLogging(ad)->wrapped;
  wrapped->incRefCount();
  return wrapped;
}

void LoggingArray::convertToUncounted(
    ArrayData* ad, DataWalker::PointerMap* seen) {
  logEvent(ad, ArrayOp::ConvertToUncounted);
  auto const lad = asLogging(ad);
  auto tv = make_array_like_tv(lad->wrapped);
  ConvertTvToUncounted(&tv, seen);
  lad->wrapped = val(tv).parr;
}

void LoggingArray::releaseUncounted(ArrayData* ad) {
  logEvent(ad, ArrayOp::ReleaseUncounted);
  auto tv = make_array_like_tv(asLogging(ad)->wrapped);
  ReleaseUncountedTv(&tv);
}

void LoggingArray::release(ArrayData* ad) {
  logEvent(ad, ArrayOp::Release);
  asLogging(ad)->wrapped->decRefAndRelease();
  tl_heap->objFreeIndex(ad, kSizeIndex);
}

//////////////////////////////////////////////////////////////////////////////
// Accessors

bool LoggingArray::isVectorData(const ArrayData* ad) {
  logEvent(ad, ArrayOp::IsVectorData);
  return asLogging(ad)->wrapped->isVectorData();
}
TypedValue LoggingArray::getInt(const ArrayData* ad, int64_t k) {
  logEvent(ad, ArrayOp::GetInt, k);
  return asLogging(ad)->wrapped->get(k);
}
TypedValue LoggingArray::getStr(const ArrayData* ad, const StringData* k) {
  logEvent(ad, ArrayOp::GetStr, k);
  return asLogging(ad)->wrapped->get(k);
}
TypedValue LoggingArray::getKey(const ArrayData* ad, ssize_t pos) {
  return asLogging(ad)->wrapped->nvGetKey(pos);
}
TypedValue LoggingArray::getVal(const ArrayData* ad, ssize_t pos) {
  return asLogging(ad)->wrapped->nvGetVal(pos);
}
ssize_t LoggingArray::getIntPos(const ArrayData* ad, int64_t k) {
  logEvent(ad, ArrayOp::GetIntPos, k);
  return asLogging(ad)->wrapped->nvGetIntPos(k);
}
ssize_t LoggingArray::getStrPos(const ArrayData* ad, const StringData* k) {
  logEvent(ad, ArrayOp::GetStrPos, k);
  return asLogging(ad)->wrapped->nvGetStrPos(k);
}

//////////////////////////////////////////////////////////////////////////////
// Mutations

namespace {
TypedValue countedValue(TypedValue val) {
  type(val) = dt_modulo_persistence(type(val));
  return val;
}

ArrayData* escalate(LoggingArray* lad, ArrayData* result) {
  lad->updateSize();
  if (result == lad->wrapped) return lad;
  return LoggingArray::Make(result, lad->profile, lad->entryTypes);
}

ArrayData* escalate(LoggingArray* lad, ArrayData* result, EntryTypes ms) {
  lad->updateSize();
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
  auto const lad = LoggingArray::asLogging(ad);
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  SCOPE_EXIT { if (cow) lad->wrapped->decRefCount(); };
  return escalate(lad, f(lad->wrapped));
}

template <typename F>
decltype(auto) mutate(ArrayData* ad, EntryTypes ms, F&& f) {
  auto const lad = LoggingArray::asLogging(ad);
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  SCOPE_EXIT { if (cow) lad->wrapped->decRefCount(); };
  return escalate(lad, f(lad->wrapped), ms);
}

arr_lval elem(arr_lval lval) {
  lval.type() = dt_modulo_persistence(lval.type());
  return lval;
}
}

// Lvals cannot insert new keys, so KeyTypes are unchanged. We must pessimize
// value types on doing an lval operation, but we can be more precise with our
// logging of the constrained "elem" operation.
arr_lval LoggingArray::lvalInt(ArrayData* ad, int64_t k) {
  auto const lad = asLogging(ad);
  auto const val = lad->wrapped->get(k);
  auto const ms = val.is_init() ? lad->entryTypes.pessimizeValueTypes()
                                : lad->entryTypes;
  logEvent(ad, ms, ArrayOp::LvalInt, k, val);
  return mutate(ad, ms, [&](ArrayData* arr) { return arr->lval(k); });
}
arr_lval LoggingArray::lvalStr(ArrayData* ad, StringData* k) {
  auto const lad = asLogging(ad);
  auto const val = lad->wrapped->get(k);
  auto const ms = val.is_init() ? lad->entryTypes.pessimizeValueTypes()
                                : lad->entryTypes;
  logEvent(ad, ms, ArrayOp::LvalStr, k, val);
  return mutate(ad, ms, [&](ArrayData* arr) { return arr->lval(k); });
}
arr_lval LoggingArray::elemInt(ArrayData* ad, int64_t k) {
  auto const lad = asLogging(ad);
  auto const val = lad->wrapped->get(k);
  auto const key = make_tv<KindOfInt64>(k);
  auto const ms = val.is_init() ? lad->entryTypes.with(key, countedValue(val))
                                : lad->entryTypes;
  logEvent(ad, ms, ArrayOp::ElemInt, k, val);
  return elem(mutate(ad, ms, [&](ArrayData* arr) { return arr->lval(k); }));
}
arr_lval LoggingArray::elemStr(ArrayData* ad, StringData* k) {
  auto const lad = asLogging(ad);
  auto const val = lad->wrapped->get(k);
  auto const key = make_tv<KindOfString>(k);
  auto const ms = val.is_init() ? lad->entryTypes.with(key, countedValue(val))
                                : lad->entryTypes;
  logEvent(ad, ms, ArrayOp::ElemStr, k, val);
  return elem(mutate(ad, ms, [&](ArrayData* arr) { return arr->lval(k); }));
}

ArrayData* LoggingArray::setInt(ArrayData* ad, int64_t k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = asLogging(ad)->entryTypes.with(make_tv<KindOfInt64>(k), v);
  logEvent(ad, ms, ArrayOp::SetInt, k, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingArray::setStr(ArrayData* ad, StringData* k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = asLogging(ad)->entryTypes.with(make_tv<KindOfString>(k), v);
  logEvent(ad, ms, ArrayOp::SetStr, k, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingArray::removeInt(ArrayData* ad, int64_t k) {
  logEvent(ad, ArrayOp::RemoveInt, k);
  return mutate(ad, [&](ArrayData* w) { return w->remove(k); });
}
ArrayData* LoggingArray::removeStr(ArrayData* ad, const StringData* k) {
  logEvent(ad, ArrayOp::RemoveStr, k);
  return mutate(ad, [&](ArrayData* w) { return w->remove(k); });
}

ssize_t LoggingArray::iterBegin(const ArrayData* ad) {
  logEvent(ad, ArrayOp::IterBegin);
  return asLogging(ad)->wrapped->iter_begin();
}
ssize_t LoggingArray::iterLast(const ArrayData* ad) {
  logEvent(ad, ArrayOp::IterLast);
  return asLogging(ad)->wrapped->iter_last();
}
ssize_t LoggingArray::iterEnd(const ArrayData* ad) {
  logEvent(ad, ArrayOp::IterEnd);
  return asLogging(ad)->wrapped->iter_end();
}
ssize_t LoggingArray::iterAdvance(const ArrayData* ad, ssize_t prev) {
  logEvent(ad, ArrayOp::IterAdvance);
  return asLogging(ad)->wrapped->iter_advance(prev);
}
ssize_t LoggingArray::iterRewind(const ArrayData* ad, ssize_t prev) {
  logEvent(ad, ArrayOp::IterRewind);
  return asLogging(ad)->wrapped->iter_rewind(prev);
}

ArrayData* LoggingArray::append(ArrayData* ad, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const lad = asLogging(ad);
  // NOTE: This key isn't always correct, but it's close enough for profiling.
  auto const k = make_tv<KindOfInt64>(lad->wrapped->size());
  auto const ms = lad->entryTypes.with(k, v);
  logEvent(ad, ms, ArrayOp::Append, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->append(v); });
}
ArrayData* LoggingArray::prepend(ArrayData* ad, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const lad = asLogging(ad);
  // NOTE: This key isn't always correct, but it's close enough for profiling.
  auto const k = make_tv<KindOfInt64>(lad->wrapped->size());
  auto const ms = lad->entryTypes.with(k, v);
  logEvent(ad, ms, ArrayOp::Prepend, v);
  return mutate(ad, ms, [&](ArrayData* w) { return w->prepend(v); });
}
ArrayData* LoggingArray::pop(ArrayData* ad, Variant& ret) {
  logEvent(ad, ArrayOp::Pop);
  return mutate(ad, [&](ArrayData* w) { return w->pop(ret); });
}
ArrayData* LoggingArray::dequeue(ArrayData* ad, Variant& ret) {
  logEvent(ad, ArrayOp::Dequeue);
  return mutate(ad, [&](ArrayData* w) { return w->dequeue(ret); });
}

//////////////////////////////////////////////////////////////////////////////

namespace {
ArrayData* convert(LoggingArray* lad, ArrayData* result) {
  if (result != lad->wrapped) {
    return LoggingArray::Make(result, lad->profile, lad->entryTypes);
  }
  lad->updateKindAndLegacy();
  return lad;
}
}

ArrayData* LoggingArray::copy(const ArrayData* ad) {
  logEvent(ad, ArrayOp::Copy);
  auto const lad = asLogging(ad);
  return Make(lad->wrapped->copy(), lad->profile, lad->entryTypes);
}
ArrayData* LoggingArray::toDVArray(ArrayData* ad, bool copy) {
  logEvent(ad, ArrayOp::ToDVArray);
  auto const lad = asLogging(ad);
  return convert(lad, lad->wrapped->toDVArray(copy));
}
ArrayData* LoggingArray::toHackArr(ArrayData* ad, bool copy) {
  logEvent(ad, ArrayOp::ToHackArr);
  auto const lad = asLogging(ad);
  return convert(lad, lad->wrapped->toHackArr(copy));
}

//////////////////////////////////////////////////////////////////////////////

}}
