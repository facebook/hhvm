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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT, class ArrayT>
SortFlavor genericPreSort(ArrayT& arr,
                          const AccessorT& acc,
                          bool checkTypes) {
  assertx(arr.m_size > 0);
  if (!checkTypes && arr.m_size == arr.m_used) {
    // No need to loop over the elements, we're done
    return GenericSort;
  }
  typename ArrayT::Elm* start = arr.data();
  typename ArrayT::Elm* end = arr.data() + arr.m_used;
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  for (;;) {
    if (checkTypes) {
      while (!start->isTombstone()) {
        allInts = (allInts && acc.isInt(*start));
        allStrs = (allStrs && acc.isStr(*start));
        ++start;
        if (start == end) {
          goto done;
        }
      }
    } else {
      while (!start->isTombstone()) {
        ++start;
        if (start == end) {
          goto done;
        }
      }
    }
    --end;
    if (start == end) {
      goto done;
    }
    while (end->isTombstone()) {
      --end;
      if (start == end) {
        goto done;
      }
    }
    memcpy(start, end, sizeof(typename ArrayT::Elm));
  }
done:
  arr.m_used = start - arr.data();
  assertx(arr.m_size == arr.m_used);
  if (checkTypes) {
    return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
  }
  return GenericSort;
}

template <typename AccessorT>
SortFlavor MixedArray::preSort(const AccessorT& acc, bool checkTypes) {
  return genericPreSort(*this, acc, checkTypes);
}

template <typename AccessorT>
SortFlavor SetArray::preSort(const AccessorT& acc, bool checkTypes) {
  auto const oldUsed UNUSED = m_used;
  auto flav = genericPreSort(*this, acc, checkTypes);
  assertx(ClearElms(data() + m_used, oldUsed - m_used));
  return flav;
}

/**
 * postSort() runs after the sort has been performed. For MixedArray, postSort()
 * handles rebuilding the hash. Also, if resetKeys is true, postSort() will
 * renumber the keys 0 thru n-1.
 */
void MixedArray::postSort(bool resetKeys) {   // nothrow guarantee
  assertx(m_size > 0);
  assertx(m_size == m_used);
  auto const ht = initHash(m_scale);
  auto const mask = this->mask();
  if (resetKeys) {
    mutableKeyTypes()->renumberKeys();
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data()[pos];
      if (e.hasStrKey()) decRefStr(e.skey);
      auto h = hash_int64(pos);
      e.setIntKey(pos, h);
      *findForNewInsert(ht, mask, h) = pos;
    }
    m_nextKI = m_size;
  } else {
    mutableKeyTypes()->makeCompact();
    auto data = this->data();
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data[pos];
      *findForNewInsert(ht, mask, e.probe()) = pos;
    }
  }
  assertx(checkInvariants());
}

/**
 * postSort() runs after the sort has been performed. For SetArray, postSort()
 * handles rebuilding the hash.
 */
void SetArray::postSort(bool) {   // nothrow guarantee
  assertx(m_size > 0);
  auto const ht = initHash(m_scale);
  auto const mask = this->mask();
  auto elms = data();
  assertx(m_used == m_size);
  for (uint32_t i = 0; i < m_used; ++i) {
    auto& elm = elms[i];
    assertx(!elm.isInvalid());
    *findForNewInsert(ht, mask, elm.hash()) = i;
  }
}

ArrayData* MixedArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto a = asMixed(ad);
  // We can uncomment later if we want this feature.
  // if (a->m_size <= 1 && !isSortFamily(sf)) {
  //   return a;
  // }
  if (UNLIKELY(hasUserDefinedCmp(sf) || a->cowCheck())) {
    auto ret = a->copyMixed();
    assertx(ret->hasExactlyOneRef());
    return ret;
  }
  return a;
}

ArrayData* SetArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto a = asSet(ad);
  if (UNLIKELY(hasUserDefinedCmp(sf) || a->cowCheck())) {
    auto ret = a->copySet();
    assertx(ret->hasExactlyOneRef());
    return ret;
  }
  return a;
}

ArrayData* PackedArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  if (sf == SORTFUNC_KSORT) {
    return ad;                          // trivial for packed arrays.
  }
  if (isSortFamily(sf)) {               // sort/rsort/usort
    if (UNLIKELY(ad->cowCheck())) {
      auto ret = PackedArray::Copy(ad);
      assertx(ret->hasExactlyOneRef());
      return ret;
    }
    return ad;
  }
  if (ad->m_size <= 1 && !(ad->isVecArrayKind() || ad->isVArray())) {
    return ad;
  }
  assertx(checkInvariants(ad));
  auto ret = ad->isVecArrayKind()
    // TODO(T39123862)
    ? PackedArray::ToDictVec(ad, ad->cowCheck())
    : ToMixedCopy(ad);
  assertx(ret->empty() || ret->hasExactlyOneRef());
  return ret;
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(data_begin, data_end, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(data_begin, data_end, comp); \
    } \
    break; \
  }
#define SORT_CASE_BLOCK(cmp_type, acc_type) \
  switch (sort_flags) { \
    default: /* fall through to SORT_REGULAR case */ \
    SORT_CASE(SORT_REGULAR, cmp_type, acc_type) \
    SORT_CASE(SORT_NUMERIC, cmp_type, acc_type) \
    SORT_CASE(SORT_STRING, cmp_type, acc_type) \
    SORT_CASE(SORT_STRING_CASE, cmp_type, acc_type) \
    SORT_CASE(SORT_LOCALE_STRING, cmp_type, acc_type) \
    SORT_CASE(SORT_NATURAL, cmp_type, acc_type) \
    SORT_CASE(SORT_NATURAL_CASE, cmp_type, acc_type) \
  }

#define CALL_SORT(acc_type)                     \
  if (flav == StringSort) {                     \
    SORT_CASE_BLOCK(StrElm, acc_type)           \
  } else if (flav == IntegerSort) {             \
    SORT_CASE_BLOCK(IntElm, acc_type)           \
  } else {                                      \
    SORT_CASE_BLOCK(Elm, acc_type)              \
  }

#define SORT_BODY(acc_type, resetKeys)                          \
  do {                                                          \
    if (!a->m_size) return;                                     \
    SortFlavor flav = a->preSort<acc_type>(acc_type(), true);   \
    a->m_pos = ssize_t(0);                                      \
    try {                                                       \
      CALL_SORT(acc_type);                                      \
    } catch (...) {                                             \
      /* Make sure we leave the array in a consistent state */  \
      a->postSort(resetKeys);                                   \
      throw;                                                    \
    }                                                           \
    a->postSort(resetKeys);                                     \
  } while (0)

void MixedArray::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asMixed(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocKeyAccessor<MixedArray::Elm>, false);
}

void MixedArray::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asMixed(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  a->m_nextKI = 0;
  SORT_BODY(AssocValAccessor<MixedArray::Elm>, true);
}

void MixedArray::Asort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asMixed(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocValAccessor<MixedArray::Elm>, false);
}

void SetArray::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asSet(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocKeyAccessor<SetArray::Elm>, false);
}

#undef SORT_BODY

void PackedArray::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  assertx(checkInvariants(ad));
  if (ad->m_size <= 1) {
    return;
  }
  assertx(!ad->hasMultipleRefs());
  auto a = ad;
  SortFlavor flav = preSort(ad);
  a->m_pos = 0;
  auto data_begin = packedData(ad);
  auto data_end = data_begin + a->m_size;
  CALL_SORT(TVAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

#define USER_SORT_BODY(acc_type, resetKeys)                     \
  do {                                                          \
    if (!a->m_size) return true;                                \
    CallCtx ctx;                                                \
    vm_decode_function(cmp_function, ctx);                      \
    if (!ctx.func) {                                            \
      return false;                                             \
    }                                                           \
    a->preSort<acc_type>(acc_type(), false);                    \
    a->m_pos = ssize_t(0);                                      \
    SCOPE_EXIT {                                                \
      /* Make sure we leave the array in a consistent state */  \
      a->postSort(resetKeys);                                   \
    };                                                          \
    ElmUCompare<acc_type> comp;                                 \
    comp.ctx = &ctx;                                            \
    HPHP::Sort::sort(a->data(), a->data() + a->m_size, comp);   \
    return true;                                                \
  } while (0)

bool MixedArray::Uksort(ArrayData* ad, const Variant& cmp_function) {
  auto a = asMixed(ad);
  USER_SORT_BODY(AssocKeyAccessor<MixedArray::Elm>, false);
}

bool MixedArray::Usort(ArrayData* ad, const Variant& cmp_function) {
  auto a = asMixed(ad);
  a->m_nextKI = 0;
  USER_SORT_BODY(AssocValAccessor<MixedArray::Elm>, true);
}

bool MixedArray::Uasort(ArrayData* ad, const Variant& cmp_function) {
  auto a = asMixed(ad);
  USER_SORT_BODY(AssocValAccessor<MixedArray::Elm>, false);
}

bool SetArray::Uksort(ArrayData* ad, const Variant& cmp_function) {
  auto a = asSet(ad);
  USER_SORT_BODY(AssocKeyAccessor<SetArray::Elm>, false);
}

#undef USER_SORT_BODY

SortFlavor PackedArray::preSort(ArrayData* ad) {
  assertx(checkInvariants(ad));
  assertx(ad->m_size > 0);
  TVAccessor acc;
  bool allInts = true;
  bool allStrs = true;
  auto elm = packedData(ad);
  auto const end = elm + ad->m_size;
  do {
    if (acc.isInt(*elm)) {
      if (!allInts) return GenericSort;
      allStrs = false;
    } else if (acc.isStr(*elm)) {
      if (!allStrs) return GenericSort;
      allInts = false;
    } else {
      return GenericSort;
    }
  } while (++elm < end);
  if (allInts) return IntegerSort;
  assertx(allStrs);
  return StringSort;
}

bool PackedArray::Usort(ArrayData* ad, const Variant& cmp_function) {
  assertx(checkInvariants(ad));
  if (ad->m_size <= 1) {
    return true;
  }
  assertx(!ad->hasMultipleRefs());
  ElmUCompare<TVAccessor> comp;
  CallCtx ctx;
  vm_decode_function(cmp_function, ctx);
  if (!ctx.func) {
    return false;
  }
  comp.ctx = &ctx;
  auto const data = packedData(ad);
  Sort::sort(data, data + ad->m_size, comp);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
