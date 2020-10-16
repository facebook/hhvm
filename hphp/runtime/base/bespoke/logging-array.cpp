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

constexpr LayoutIndex kLayoutIndex = {0};
auto const s_vtable = fromArray<LoggingArray>();
std::atomic<bool> g_emitLoggingArrays;

// The bespoke kind for a vanilla kind.
HeaderKind getBespokeKind(ArrayData::ArrayKind kind) {
  assertx(!(kind & ArrayData::kBespokeKindMask));
  return HeaderKind(kind | ArrayData::kBespokeKindMask);
}

template <typename... Ts>
void logEvent(const LoggingArray* lad, EntryTypes newTypes,
              ArrayOp op, Ts&&... args) {
  lad->profile->logEntryTypes(lad->entryTypes, newTypes);
  lad->profile->logEvent(op, std::forward<Ts>(args)...);
}

template <typename... Ts>
void logEvent(const LoggingArray* lad, ArrayOp op, Ts&&... args) {
  logEvent(lad, lad->entryTypes, op, std::forward<Ts>(args)...);
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
      return true;
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

void LoggingArray::InitializeLayouts() {
  auto const layout = new Layout("LoggingLayout", &s_vtable);
  always_assert(layout->index() == kLayoutIndex);
}

bespoke::LayoutIndex LoggingArray::GetLayoutIndex() {
  return kLayoutIndex;
}

LoggingArray* LoggingArray::As(ArrayData* ad) {
  auto const result = reinterpret_cast<LoggingArray*>(ad);
  assertx(result->checkInvariants());
  return result;
}
const LoggingArray* LoggingArray::As(const ArrayData* ad) {
  return LoggingArray::As(const_cast<ArrayData*>(ad));
}

LoggingArray* LoggingArray::Make(ArrayData* ad, LoggingProfile* profile,
                                 EntryTypes ms) {
  assertx(ad->isVanilla());

  auto lad = static_cast<LoggingArray*>(tl_heap->objMallocIndex(kSizeIndex));
  lad->initHeader_16(getBespokeKind(ad->kind()), OneReference, ad->auxBits());
  lad->m_size = ad->size();
  lad->setLayoutIndex(kLayoutIndex);
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
  lad->setLayoutIndex(kLayoutIndex);
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
  assertx(layoutIndex() == kLayoutIndex);
  assertx(m_kind == getBespokeKind(wrapped->kind()));
  assertx(isLegacyArray() == wrapped->isLegacyArray());
  return true;
}

void LoggingArray::logReachEvent(TransID transId, uint32_t guardIdx) {
  profile->logReach(transId, guardIdx);
}

void LoggingArray::updateKindAndLegacy() {
  assertx(hasExactlyOneRef());
  m_kind = getBespokeKind(wrapped->kind());
  setLegacyArrayInPlace(wrapped->isLegacyArray());
  assertx(checkInvariants());
}

void LoggingArray::updateSize() {
  if (hasExactlyOneRef()) {
    m_size = wrapped->size();
  }
  assertx(checkInvariants());
}

//////////////////////////////////////////////////////////////////////////////

size_t LoggingArray::HeapSize(const LoggingArray*) {
  return sizeof(LoggingArray);
}

void LoggingArray::Scan(const LoggingArray* lad, type_scan::Scanner& scanner) {
  logEvent(lad, ArrayOp::Scan);
  scanner.scan(lad->wrapped);
}

ArrayData* LoggingArray::EscalateToVanilla(
    const LoggingArray* lad, const char* reason) {
  logEvent(lad, ArrayOp::EscalateToVanilla, makeStaticString(reason));
  auto const wrapped = lad->wrapped;
  wrapped->incRefCount();
  return wrapped;
}

void LoggingArray::ConvertToUncounted(
    LoggingArray* lad, DataWalker::PointerMap* seen) {
  logEvent(lad, ArrayOp::ConvertToUncounted);
  auto tv = make_array_like_tv(lad->wrapped);
  ConvertTvToUncounted(&tv, seen);
  lad->wrapped = val(tv).parr;
}

void LoggingArray::ReleaseUncounted(LoggingArray* lad) {
  logEvent(lad, ArrayOp::ReleaseUncounted);
  auto tv = make_array_like_tv(lad->wrapped);
  ReleaseUncountedTv(&tv);
}

void LoggingArray::Release(LoggingArray* lad) {
  logEvent(lad, ArrayOp::Release);
  lad->wrapped->decRefAndRelease();
  tl_heap->objFreeIndex(lad, kSizeIndex);
}

//////////////////////////////////////////////////////////////////////////////
// Accessors

bool LoggingArray::IsVectorData(const LoggingArray* lad) {
  logEvent(lad, ArrayOp::IsVectorData);
  return lad->wrapped->isVectorData();
}
TypedValue LoggingArray::GetInt(const LoggingArray* lad, int64_t k) {
  logEvent(lad, ArrayOp::GetInt, k);
  return lad->wrapped->get(k);
}
TypedValue LoggingArray::GetStr(const LoggingArray* lad, const StringData* k) {
  logEvent(lad, ArrayOp::GetStr, k);
  return lad->wrapped->get(k);
}
TypedValue LoggingArray::GetKey(const LoggingArray* lad, ssize_t pos) {
  return lad->wrapped->nvGetKey(pos);
}
TypedValue LoggingArray::GetVal(const LoggingArray* lad, ssize_t pos) {
  return lad->wrapped->nvGetVal(pos);
}
ssize_t LoggingArray::GetIntPos(const LoggingArray* lad, int64_t k) {
  logEvent(lad, ArrayOp::GetIntPos, k);
  return lad->wrapped->nvGetIntPos(k);
}
ssize_t LoggingArray::GetStrPos(const LoggingArray* lad, const StringData* k) {
  logEvent(lad, ArrayOp::GetStrPos, k);
  return lad->wrapped->nvGetStrPos(k);
}

ssize_t LoggingArray::IterBegin(const LoggingArray* lad) {
  logEvent(lad, ArrayOp::IterBegin);
  return lad->wrapped->iter_begin();
}
ssize_t LoggingArray::IterLast(const LoggingArray* lad) {
  logEvent(lad, ArrayOp::IterLast);
  return lad->wrapped->iter_last();
}
ssize_t LoggingArray::IterEnd(const LoggingArray* lad) {
  logEvent(lad, ArrayOp::IterEnd);
  return lad->wrapped->iter_end();
}
ssize_t LoggingArray::IterAdvance(const LoggingArray* lad, ssize_t prev) {
  logEvent(lad, ArrayOp::IterAdvance);
  return lad->wrapped->iter_advance(prev);
}
ssize_t LoggingArray::IterRewind(const LoggingArray* lad, ssize_t prev) {
  logEvent(lad, ArrayOp::IterRewind);
  return lad->wrapped->iter_rewind(prev);
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
decltype(auto) mutate(LoggingArray* lad, F&& f) {
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  SCOPE_EXIT { if (cow) lad->wrapped->decRefCount(); };
  return escalate(lad, f(lad->wrapped));
}

template <typename F>
decltype(auto) mutate(LoggingArray* lad, EntryTypes ms, F&& f) {
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
arr_lval LoggingArray::LvalInt(LoggingArray* lad, int64_t k) {
  auto const val = lad->wrapped->get(k);
  auto const ms = val.is_init() ? lad->entryTypes.pessimizeValueTypes()
                                : lad->entryTypes;
  logEvent(lad, ms, ArrayOp::LvalInt, k, val);
  return mutate(lad, ms, [&](ArrayData* arr) { return arr->lval(k); });
}
arr_lval LoggingArray::LvalStr(LoggingArray* lad, StringData* k) {
  auto const val = lad->wrapped->get(k);
  auto const ms = val.is_init() ? lad->entryTypes.pessimizeValueTypes()
                                : lad->entryTypes;
  logEvent(lad, ms, ArrayOp::LvalStr, k, val);
  return mutate(lad, ms, [&](ArrayData* arr) { return arr->lval(k); });
}
arr_lval LoggingArray::ElemInt(LoggingArray* lad, int64_t k) {
  auto const val = lad->wrapped->get(k);
  auto const key = make_tv<KindOfInt64>(k);
  auto const ms = val.is_init() ? lad->entryTypes.with(key, countedValue(val))
                                : lad->entryTypes;
  logEvent(lad, ms, ArrayOp::ElemInt, k, val);
  return elem(mutate(lad, ms, [&](ArrayData* arr) { return arr->lval(k); }));
}
arr_lval LoggingArray::ElemStr(LoggingArray* lad, StringData* k) {
  auto const val = lad->wrapped->get(k);
  auto const key = make_tv<KindOfString>(k);
  auto const ms = val.is_init() ? lad->entryTypes.with(key, countedValue(val))
                                : lad->entryTypes;
  logEvent(lad, ms, ArrayOp::ElemStr, k, val);
  return elem(mutate(lad, ms, [&](ArrayData* arr) { return arr->lval(k); }));
}

ArrayData* LoggingArray::SetInt(LoggingArray* lad, int64_t k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = lad->entryTypes.with(make_tv<KindOfInt64>(k), v);
  logEvent(lad, ms, ArrayOp::SetInt, k, v);
  return mutate(lad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingArray::SetStr(LoggingArray* lad, StringData* k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = lad->entryTypes.with(make_tv<KindOfString>(k), v);
  logEvent(lad, ms, ArrayOp::SetStr, k, v);
  return mutate(lad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingArray::RemoveInt(LoggingArray* lad, int64_t k) {
  logEvent(lad, ArrayOp::RemoveInt, k);
  return mutate(lad, [&](ArrayData* w) { return w->remove(k); });
}
ArrayData* LoggingArray::RemoveStr(LoggingArray* lad, const StringData* k) {
  logEvent(lad, ArrayOp::RemoveStr, k);
  return mutate(lad, [&](ArrayData* w) { return w->remove(k); });
}

ArrayData* LoggingArray::Append(LoggingArray* lad, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  // NOTE: This key isn't always correct, but it's close enough for profiling.
  auto const k = make_tv<KindOfInt64>(lad->wrapped->size());
  auto const ms = lad->entryTypes.with(k, v);
  logEvent(lad, ms, ArrayOp::Append, v);
  return mutate(lad, ms, [&](ArrayData* w) { return w->append(v); });
}
ArrayData* LoggingArray::Pop(LoggingArray* lad, Variant& ret) {
  logEvent(lad, ArrayOp::Pop);
  return mutate(lad, [&](ArrayData* w) { return w->pop(ret); });
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

ArrayData* LoggingArray::ToDVArray(LoggingArray* lad, bool copy) {
  logEvent(lad, ArrayOp::ToDVArray);
  auto const cow = copy || lad->wrapped->cowCheck();
  return convert(lad, lad->wrapped->toDVArray(cow));
}
ArrayData* LoggingArray::ToHackArr(LoggingArray* lad, bool copy) {
  logEvent(lad, ArrayOp::ToHackArr);
  auto const cow = copy || lad->wrapped->cowCheck();
  return convert(lad, lad->wrapped->toHackArr(cow));
}
ArrayData* LoggingArray::SetLegacyArray(
    LoggingArray* lad, bool copy, bool legacy) {
  logEvent(lad, ArrayOp::SetLegacyArray);
  auto const cow = copy || lad->wrapped->cowCheck();
  return convert(lad, lad->wrapped->setLegacyArray(cow, legacy));
}

//////////////////////////////////////////////////////////////////////////////

}}
