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
#include "hphp/runtime/base/bespoke/bespoke-top.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/punt.h"
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

LoggingLayout* s_layout;
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

// PRc|CRc method that returns a copy of the vanilla array `vad` with the
// sampled bit set, operating in place whenever possible.
ArrayData* makeSampledArray(ArrayData* vad) {
  assertx(vad->isVanilla());
  auto const result = [&]{
    if (!vad->cowCheck()) return vad;
    vad->decRefCount();
    if (vad->hasVanillaPackedLayout()) return PackedArray::Copy(vad);
    if (vad->hasVanillaMixedLayout())  return MixedArray::Copy(vad);
    return SetArray::Copy(vad);
  }();
  result->setSampledArrayInPlace();
  return result;
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

LoggingLayout::LoggingLayout()
  : ConcreteLayout("LoggingLayout", &s_vtable, {BespokeTop::GetLayoutIndex()},
                   /*liveable=*/ false)
{}

using namespace jit;
using namespace jit::irgen;

SSATmp* LoggingLayout::emitSet(IRGS& env, SSATmp* base, SSATmp* key,
                               SSATmp* val) const {
  auto const outputType = base->type().modified();
  return gen(env, BespokeSet, outputType, BespokeLayoutData { this }, base,
             key, val);
}

SSATmp* LoggingLayout::emitAppend(IRGS& env, SSATmp* base, SSATmp* val) const {
  auto const outputType = base->type().modified();
  return gen(env, BespokeAppend, outputType, BespokeLayoutData { this }, base,
             val);
}

//////////////////////////////////////////////////////////////////////////////

void setLoggingEnabled(bool val) {
  g_emitLoggingArrays.store(val, std::memory_order_relaxed);
}

ArrayData* maybeMakeLoggingArray(ArrayData* ad) {
  if (!g_emitLoggingArrays.load(std::memory_order_relaxed)) return ad;

  auto const profile = getLoggingProfile(getSrcKey());
  return profile ? maybeMakeLoggingArray(ad, profile) : ad;
}

const ArrayData* maybeMakeLoggingArray(const ArrayData* ad) {
  return maybeMakeLoggingArray(const_cast<ArrayData*>(ad));
}

ArrayData* maybeMakeLoggingArray(ArrayData* ad, LoggingProfile* profile) {
  if (!g_emitLoggingArrays.load(std::memory_order_relaxed)) return ad;

  if (ad->isSampledArray() || !ad->isVanilla()) {
    DEBUG_ONLY auto const op = profile->key.op();
    assertx(isArrLikeCastOp(op) || op == Op::NewObjD);
    FTRACE(5, "Skipping logging for {} array.\n",
           ad->isSampledArray() ? "sampled" : "bespoke");
    return ad;
  }

  auto const shouldEmitBespoke = [&]{
    if (shouldTestBespokeArrayLikes()) return true;
    if (RO::EvalEmitLoggingArraySampleRate == 0) return false;

    auto const skCount = profile->sampleCount++;
    FTRACE(5, "Observe SrcKey count: {}\n", skCount);
    return (skCount - 1) % RO::EvalEmitLoggingArraySampleRate == 0;
  }();

  if (!shouldEmitBespoke) {
    FTRACE(5, "Emit vanilla at {}\n", profile->key.toString());
    auto const cached = profile->staticSampledArray;
    return cached ? cached : makeSampledArray(ad);
  }

  FTRACE(5, "Emit bespoke at {}\n", profile->key.toString());
  profile->loggingArraysEmitted++;
  auto const cached = profile->staticLoggingArray;
  if (cached) return cached;

  // Non-static array constructors are basically a sequence of sets or appends.
  // We already log these events at the correct granularity; re-use that logic.
  IterateKVNoInc(ad, [&](auto k, auto v) {
    tvIsString(k) ? profile->logEvent(ArrayOp::ConstructStr, val(k).pstr, v)
                  : profile->logEvent(ArrayOp::ConstructInt, val(k).num, v);
  });
  return LoggingArray::Make(ad, profile, EntryTypes::ForArray(ad));
}

void profileArrLikeProps(ObjectData* obj) {
  if (!g_emitLoggingArrays.load(std::memory_order_relaxed)) return;

  auto const cls = obj->getVMClass();
  if (cls->needsInitThrowable()) return;

  for (auto slot = 0; slot < cls->numDeclProperties(); slot++) {
    if (cls->declProperties()[slot].attrs & AttrIsConst) continue;
    auto lval = obj->propLvalAtOffset(slot);
    if (!tvIsArrayLike(lval)) continue;
    auto const profile = getLoggingProfile(cls, slot);
    if (!profile) continue;
    auto const arr = maybeMakeLoggingArray(lval.val().parr, profile);
    tvCopy(make_array_like_tv(arr), lval);
  }
}

//////////////////////////////////////////////////////////////////////////////

void LoggingArray::InitializeLayouts() {
  s_layout = new LoggingLayout();
}

bespoke::LayoutIndex LoggingArray::GetLayoutIndex() {
  return s_layout->index();
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
  lad->setLayoutIndex(GetLayoutIndex());
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
  lad->setLayoutIndex(GetLayoutIndex());
  lad->wrapped = ad;
  lad->profile = profile;
  lad->entryTypes = EntryTypes::ForArray(ad);
  assertx(lad->checkInvariants());
  return lad;
}

bool LoggingArray::checkInvariants() const {
  assertx(wrapped->isVanilla());
  assertx(wrapped->kindIsValid());
  assertx(wrapped->size() == size());
  assertx(wrapped->toDataType() == toDataType());
  assertx(layoutIndex() == GetLayoutIndex());
  assertx(m_kind == getBespokeKind(wrapped->kind()));
  assertx(isLegacyArray() == wrapped->isLegacyArray());
  return true;
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

void LoggingArray::ZombieRelease(LoggingArray* lad) {
  logEvent(lad, ArrayOp::Release);
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

LoggingArray* escalate(LoggingArray* lad, ArrayData* result) {
  lad->updateSize();
  if (result == lad->wrapped) return lad;
  return LoggingArray::Make(result, lad->profile, lad->entryTypes);
}

LoggingArray* escalate(LoggingArray* lad, ArrayData* result, EntryTypes ms) {
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

tv_lval elem(tv_lval lvalIn, arr_lval result) {
  result.type() = dt_modulo_persistence(result.type());
  auto const ladIn = LoggingArray::As(lvalIn.val().parr);
  if (result.arr != ladIn) {
    lvalIn.type() = dt_with_rc(lvalIn.type());
    lvalIn.val().parr = result.arr;
    if (ladIn->decReleaseCheck()) LoggingArray::Release(ladIn);
  }
  return result;
}

template <typename F>
LoggingArray* mutateMove(LoggingArray* lad, EntryTypes ms, F&& f) {
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  auto const res = f(lad->wrapped);
  if (cow || res == lad->wrapped) {
    auto const ladNew = escalate(lad, res, ms);
    if (ladNew != lad && lad->decReleaseCheck()) {
      LoggingArray::ZombieRelease(lad);
    }
    return ladNew;
  }

  auto const profile = lad->profile;
  assertx(lad->decReleaseCheck());
  LoggingArray::ZombieRelease(lad);
  return LoggingArray::Make(res, profile, ms);
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
tv_lval LoggingArray::ElemInt(tv_lval lval, int64_t k, bool throwOnMissing) {
  auto const lad = As(lval.val().parr);
  auto const val = lad->wrapped->get(k);
  auto const key = make_tv<KindOfInt64>(k);
  auto const ms = val.is_init() ? lad->entryTypes.with(key, countedValue(val))
                                : lad->entryTypes;
  logEvent(lad, ms, ArrayOp::ElemInt, k, val);
  if (!val.is_init() && !throwOnMissing) {
    return const_cast<TypedValue*>(&immutable_null_base);
  }
  return elem(lval,
              mutate(lad, ms, [&](ArrayData* arr) { return arr->lval(k); }));
}
tv_lval LoggingArray::ElemStr(
    tv_lval lval, StringData* k, bool throwOnMissing) {
  auto const lad = As(lval.val().parr);
  auto const val = lad->wrapped->get(k);
  auto const key = make_tv<KindOfString>(k);
  auto const ms = val.is_init() ? lad->entryTypes.with(key, countedValue(val))
                                : lad->entryTypes;
  logEvent(lad, ms, ArrayOp::ElemStr, k, val);
  if (!val.is_init() && !throwOnMissing) {
    return const_cast<TypedValue*>(&immutable_null_base);
  }
  return elem(lval,
              mutate(lad, ms, [&](ArrayData* arr) { return arr->lval(k); }));
}

ArrayData* LoggingArray::SetInt(LoggingArray* lad, int64_t k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = lad->entryTypes.with(make_tv<KindOfInt64>(k), v);
  logEvent(lad, ms, ArrayOp::SetInt, k, v);
  return mutate(lad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingArray::SetIntMove(LoggingArray* lad, int64_t k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = lad->entryTypes.with(make_tv<KindOfInt64>(k), v);
  logEvent(lad, ms, ArrayOp::SetInt, k, v);
  return mutateMove(lad, ms, [&](ArrayData* w) { return w->setMove(k, v); });
}
ArrayData* LoggingArray::SetStr(LoggingArray* lad, StringData* k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = lad->entryTypes.with(make_tv<KindOfString>(k), v);
  logEvent(lad, ms, ArrayOp::SetStr, k, v);
  return mutate(lad, ms, [&](ArrayData* w) { return w->set(k, v); });
}
ArrayData* LoggingArray::SetStrMove(LoggingArray* lad, StringData* k, TypedValue v) {
  if (type(v) == KindOfUninit) type(v) = KindOfNull;
  auto const ms = lad->entryTypes.with(make_tv<KindOfString>(k), v);
  logEvent(lad, ms, ArrayOp::SetStr, k, v);
  return mutateMove(lad, ms, [&](ArrayData* w) { return w->setMove(k, v); });
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
ArrayData* LoggingArray::AppendMove(LoggingArray* lad, TypedValue v) {
  auto const result = Append(lad, v);
  if (result != lad && lad->decReleaseCheck()) Release(lad);
  tvDecRefGen(v);
  return result;
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
ArrayData* LoggingArray::PreSort(LoggingArray* lad, SortFunction sf) {
  logEvent(lad, ArrayOp::PreSort, makeStaticString(sortFunctionName(sf)));
  auto const cow = lad->cowCheck();
  if (cow) lad->wrapped->incRefCount();
  auto const result = lad->wrapped->escalateForSort(sf);
  if (cow) lad->wrapped->decRefCount();
  return result;
}
ArrayData* LoggingArray::PostSort(LoggingArray* lad, ArrayData* vad) {
  logEvent(lad, EntryTypes::ForArray(vad), ArrayOp::PostSort);
  return convert(lad, vad);
}
ArrayData* LoggingArray::SetLegacyArray(
    LoggingArray* lad, bool copy, bool legacy) {
  logEvent(lad, ArrayOp::SetLegacyArray);
  auto const cow = copy || lad->wrapped->cowCheck();
  return convert(lad, lad->wrapped->setLegacyArray(cow, legacy));
}

//////////////////////////////////////////////////////////////////////////////

}}
