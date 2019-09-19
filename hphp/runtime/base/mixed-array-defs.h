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

#ifndef incl_HPHP_HPHP_ARRAY_DEFS_H_
#define incl_HPHP_HPHP_ARRAY_DEFS_H_

#include "hphp/runtime/base/mixed-array.h"

#include "hphp/runtime/base/array-helpers.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include "hphp/util/stacktrace-profiler.h"
#include "hphp/util/word-mem.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline void MixedArray::scan(type_scan::Scanner& scanner) const {
  if (isZombie()) return;
  auto data = this->data();
  scanner.scan(*data, m_used * sizeof(*data));
}

ALWAYS_INLINE
void MixedArray::InitSmall(MixedArray* a, uint32_t size, int64_t nextIntKey) {
  InitSmallHash(a);
  a->m_sizeAndPos = size; // pos=0
  a->initHeader_16(HeaderKind::Mixed, OneReference, ArrayData::kNotDVArray);
  a->m_scale_used = MixedArray::SmallScale | uint64_t(size) << 32;
  a->m_nextKI = nextIntKey;
}

inline void
MixedArray::copyElmsNextUnsafe(MixedArray* to, const MixedArray* from,
                               uint32_t nElems) {
  static_assert(offsetof(MixedArray, m_nextKI) + 8 == sizeof(MixedArray),
                "Revisit this if MixedArray layout changes");
  static_assert(sizeof(Elm) == 24, "");
  // Copy `m_nextKI' (8 bytes), data (oldUsed * 24), and optionally 24 more
  // bytes to make sure we can use bcopy32(), which rounds the length down to
  // 32-byte chunks. The additional bytes are guaranteed not to exceed the
  // space allocated for the array, because the hash table has at least 16
  // bytes, and when it is only 16 bytes (capacity = 3), we overrun the buffer
  // by only 16 bytes instead of 24.
  bcopy32_inline(&(to->m_nextKI), &(from->m_nextKI), sizeof(Elm) * nElems + 32);
}

extern int32_t* warnUnbalanced(MixedArray*, size_t n, int32_t* ei);

inline bool MixedArray::isTombstone(ssize_t pos) const {
  assertx(size_t(pos) <= m_used);
  return isTombstone(data()[pos].data.m_type);
}

ALWAYS_INLINE
Cell MixedArray::getElmKey(const Elm& e) {
  if (e.hasIntKey()) {
    return make_tv<KindOfInt64>(e.ikey);
  }
  auto str = e.skey;
  if (str->isRefCounted()) {
    str->rawIncRefCount();
    return make_tv<KindOfString>(str);
  }
  return make_tv<KindOfPersistentString>(str);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos,
                            TypedValue* valOut,
                            TypedValue* keyOut) const {
  assertx(size_t(pos) < m_used);
  auto& elm = data()[pos];
  auto const cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
  cellCopy(getElmKey(elm), *keyOut);
}

ALWAYS_INLINE
void MixedArray::getArrayElm(ssize_t pos, TypedValue* valOut) const {
  assertx(size_t(pos) < m_used);
  auto& elm = data()[pos];
  auto const cur = tvToCell(&elm.data);
  cellDup(*cur, *valOut);
}

ALWAYS_INLINE
const TypedValue* MixedArray::getArrayElmPtr(ssize_t pos) const {
  assertx(validPos(pos));
  if (size_t(pos) >= m_used) return nullptr;
  auto& elm = data()[pos];
  return !isTombstone(elm.data.m_type) ? &elm.data : nullptr;
}

ALWAYS_INLINE
TypedValue MixedArray::getArrayElmKey(ssize_t pos) const {
  assertx(validPos(pos));
  if (size_t(pos) >= m_used) return make_tv<KindOfUninit>();
  auto& elm = data()[pos];
  if (isTombstone(elm.data.m_type)) return make_tv<KindOfUninit>();
  return getElmKey(elm);
}

inline ArrayData* MixedArray::addVal(int64_t ki, Cell data) {
  assertx(!exists(ki));
  assertx(!isFull());
  auto h = hash_int64(ki);
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setIntKey(ki, h);
  recordIntKey();
  if (ki >= m_nextKI && m_nextKI >= 0) m_nextKI = ki + 1;
  cellDup(data, e->data);
  // TODO(#3888164): should avoid needing these KindOfUninit checks.
  if (UNLIKELY(e->data.m_type == KindOfUninit)) {
    e->data.m_type = KindOfNull;
  }
  return this;
}

inline ArrayData* MixedArray::addVal(StringData* key, Cell data) {
  assertx(!exists(key));
  assertx(!isFull());
  return addValNoAsserts(key, data);
}

inline ArrayData* MixedArray::addValNoAsserts(StringData* key, Cell data) {
  strhash_t h = key->hash();
  auto ei = findForNewInsert(h);
  auto e = allocElm(ei);
  e->setStrKey(key, h);
  recordStrKey(key);
  // TODO(#3888164): we should restructure things so we don't have to check
  // KindOfUninit here.
  initElem(e->data, data);
  return this;
}

template <class K>
ArrayData* MixedArray::updateWithRef(K k, TypedValue data) {
  assertx(!isFull());
  auto p = insert(k);
  if (p.found) {
    // TODO(#3888164): We should restructure things so we don't have to check
    // KindOfUninit here.
    setElemWithRef(p.tv, data);
    return this;
  }
  // TODO(#3888164): We should restructure things so we don't have to check
  // KindOfUninit here.
  tvDupWithRef(data, p.tv);
  if (p.tv.m_type == KindOfUninit) p.tv.m_type = KindOfNull;
  return this;
}

template <bool warn, class K>
arr_lval MixedArray::addLvalImpl(K k) {
  assertx(!isFull());
  auto p = insert(k);
  if (!p.found) {
    tvWriteNull(p.tv);
    if (warn && checkHACFalseyPromote()) {
      raise_hac_falsey_promote_notice("Lval on missing array element");
    }
  }
  return arr_lval { this, &p.tv };
}

//////////////////////////////////////////////////////////////////////

struct MixedArray::ValIter {

  ALWAYS_INLINE
  static bool isMixed(const ArrayData::ArrayKind& kind) {
    return kind == ArrayData::kMixedKind;
  }

  explicit ValIter(ArrayData* arr)
    : m_arr(arr)
    , m_kind(arr->kind())
  {
    assertx(isMixed(m_kind) || m_kind == kPackedKind || m_kind == kVecKind);
    if (isMixed(m_kind)) {
      m_iterMixed = asMixed(arr)->data();
      m_stopMixed = m_iterMixed + asMixed(arr)->m_used;
    } else {
      m_iterPacked = reinterpret_cast<TypedValue*>(arr + 1);
      m_stopPacked = m_iterPacked + arr->m_size;
    }
  }

  explicit ValIter(ArrayData* arr, ssize_t start_pos)
    : m_arr(arr)
    , m_kind(arr->kind())
  {
    assertx(isMixed(m_kind) || m_kind == kPackedKind || m_kind == kVecKind);
    if (isMixed(m_kind)) {
      m_iterMixed = asMixed(arr)->data() + start_pos;
      m_stopMixed = asMixed(arr)->data() + asMixed(arr)->m_used;
      assertx(m_iterMixed <= m_stopMixed);
    } else {
      m_iterPacked = reinterpret_cast<TypedValue*>(arr + 1) + start_pos;
      m_stopPacked = reinterpret_cast<TypedValue*>(arr + 1) + arr->m_size;
      assertx(m_iterPacked <= m_stopPacked);
    }
  }

   TypedValue* current() const {
     return UNLIKELY(isMixed(m_kind)) ? &currentElm()->data
                                      : m_iterPacked;
   }

   Elm* currentElm() const {
     assertx(isMixed(m_kind));
     return m_iterMixed;
   }

   bool empty() const {
     return isMixed(m_kind) ? m_iterMixed == m_stopMixed
                            : m_iterPacked == m_stopPacked;
   }

  ssize_t currentPos() const {
    if (isMixed(m_kind)) return m_iterMixed - asMixed(m_arr)->data();
    return m_iterPacked - reinterpret_cast<TypedValue*>(m_arr + 1);
  }

private:
  ArrayData* const m_arr;
  ArrayData::ArrayKind const m_kind;
  union {
    Elm* m_iterMixed;
    TypedValue* m_iterPacked;
  };
  union {
    Elm* m_stopMixed;
    TypedValue* m_stopPacked;
  };
};

//////////////////////////////////////////////////////////////////////

// Converts a TypedValue `source' to its uncounted form, so that its lifetime
// can go beyond the current request.  It is used after doing a raw copy of the
// array elements (without manipulating refcounts, as an uncounted won't hold
// any reference to refcounted values.
ALWAYS_INLINE
void ConvertTvToUncounted(
    tv_lval source,
    DataWalker::PointerMap* seen = nullptr) {
  auto& data = source.val();
  auto& type = source.type();
  if (isRefType(type)) {
    // unbox
    auto const inner = data.pref->cell();
    tvCopy(*inner, source);
  }

  auto const handlePersistent = [&] (MaybeCountable* elm) {
    if (elm->isRefCounted()) return false;
    if (elm->isStatic()) return true;
    if (elm->uncountedIncRef()) return true;
    if (seen) seen->emplace(elm, nullptr);
    return false;
  };

  // `source' cannot be Ref here as we already did an unbox.  It won't be
  // Object or Resource, as these should never appear in an uncounted array.
  // Thus we only need to deal with strings/arrays.
  switch (type) {
    case KindOfFunc:
    if (RuntimeOption::EvalAPCSerializeFuncs) {
      assertx(data.pfunc->isPersistent());
      break;
    }
    case KindOfClass:
      data.pstr = isFuncType(type)
        ? const_cast<StringData*>(funcToStringHelper(data.pfunc))
        : const_cast<StringData*>(classToStringHelper(data.pclass));
      // Fall-through
    case KindOfString:
      type = KindOfPersistentString;
      // Fall-through.
    case KindOfPersistentString: {
      auto& str = data.pstr;
      if (handlePersistent(str)) break;
      if (str->empty()) str = staticEmptyString();
      else if (auto const st = lookupStaticString(str)) str = st;
      else {
        HeapObject** seenStr = nullptr;
        if (seen && str->hasMultipleRefs()) {
          seenStr = &(*seen)[str];
          if (auto const st = static_cast<StringData*>(*seenStr)) {
            if (st->uncountedIncRef()) {
              str = st;
              break;
            }
          }
        }
        str = StringData::MakeUncounted(str->slice());
        if (seenStr) *seenStr = str;
      }
      break;
    }
    case KindOfVec:
      type = KindOfPersistentVec;
      // Fall-through.
    case KindOfPersistentVec: {
      auto& ad = data.parr;
      assertx(ad->isVecArray());
      if (handlePersistent(ad)) break;
      if (ad->empty()) ad = ArrayData::CreateVec();
      else ad = PackedArray::MakeUncounted(ad, false, seen);
      break;
    }

    case KindOfDict:
      type = KindOfPersistentDict;
      // Fall-through.
    case KindOfPersistentDict: {
      auto& ad = data.parr;
      assertx(ad->isDict());
      if (handlePersistent(ad)) break;
      if (ad->empty()) ad = ArrayData::CreateDict();
      else ad = MixedArray::MakeUncounted(ad, false, seen);
      break;
    }

    case KindOfKeyset:
      type = KindOfPersistentKeyset;
      // Fall-through.
    case KindOfPersistentKeyset: {
      auto& ad = data.parr;
      assertx(ad->isKeyset());
      if (handlePersistent(ad)) break;
      if (ad->empty()) ad = ArrayData::CreateKeyset();
      else ad = SetArray::MakeUncounted(ad, false, seen);
      break;
    }

    case KindOfArray:
      type = KindOfPersistentArray;
      // Fall-through.
    case KindOfPersistentArray: {
      auto& ad = data.parr;
      assertx(ad->isPHPArray());
      assertx(!RuntimeOption::EvalHackArrDVArrs || ad->isNotDVArray());
      if (handlePersistent(ad)) break;
      if (ad->empty()) {
        if (ad->isVArray()) ad = ArrayData::CreateVArray();
        else if (ad->isDArray()) ad = ArrayData::CreateDArray();
        else ad = ArrayData::Create();
      } else if (ad->hasPackedLayout()) {
        ad = PackedArray::MakeUncounted(ad, false, seen);
      } else {
        ad = MixedArray::MakeUncounted(ad, false, seen);
      }
      break;
    }
    case KindOfUninit: {
      type = KindOfNull;
      break;
    }
    case KindOfClsMeth: {
      if (RuntimeOption::EvalHackArrDVArrs) {
        tvCastToVecInPlace(source);
        type = KindOfPersistentVec;
        auto& ad = data.parr;
        if (handlePersistent(ad)) break;
        assertx(!ad->empty());
        ad = PackedArray::MakeUncounted(ad, false, seen);
        break;
      } else {
        tvCastToVArrayInPlace(source);
        type = KindOfPersistentArray;
        auto& ad = data.parr;
        if (handlePersistent(ad)) break;
        assertx(!ad->empty());
        ad = PackedArray::MakeUncounted(ad, false, seen);
        break;
      }
    }
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble: {
      break;
    }
    case KindOfRecord:
      raise_error(Strings::RECORD_NOT_SUPPORTED);
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      not_reached();
  }
}

ALWAYS_INLINE
void ReleaseUncountedTv(tv_lval lval) {
  if (isStringType(type(lval))) {
    auto const str = val(lval).pstr;
    assertx(!str->isRefCounted());
    if (str->isUncounted()) {
      StringData::ReleaseUncounted(str);
    }
    return;
  }
  if (isArrayLikeType(type(lval))) {
    auto const arr = val(lval).parr;
    assertx(!arr->isRefCounted());
    if (!arr->isStatic()) {
      if (arr->hasPackedLayout()) PackedArray::ReleaseUncounted(arr);
      else if (arr->isKeyset()) SetArray::ReleaseUncounted(arr);
      else MixedArray::ReleaseUncounted(arr);
    }
    return;
  }
  assertx(!isRefcountedType(type(lval)));
}

//////////////////////////////////////////////////////////////////////

}

#endif
