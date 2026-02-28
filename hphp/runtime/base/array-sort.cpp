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

#include "hphp/runtime/base/vanilla-sort.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/vm/coeffects.h"

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
SortFlavor VanillaDict::preSort(const AccessorT& acc, bool checkTypes) {
  return genericPreSort(*this, acc, checkTypes);
}

template <typename AccessorT>
SortFlavor VanillaKeyset::preSort(const AccessorT& acc, bool checkTypes) {
  auto const oldUsed UNUSED = m_used;
  auto flav = genericPreSort(*this, acc, checkTypes);
  assertx(ClearElms(data() + m_used, oldUsed - m_used));
  return flav;
}

/**
 * postSort() runs after the sort has been performed. For VanillaDict, postSort()
 * handles rebuilding the hash. Also, if resetKeys is true, postSort() will
 * renumber the keys 0 thru n-1.
 */
void VanillaDict::postSort(bool resetKeys) {   // nothrow guarantee
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
 * postSort() runs after the sort has been performed. For VanillaKeyset,
 * postSort() handles rebuilding the hash.
 */
void VanillaKeyset::postSort(bool) {   // nothrow guarantee
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

ArrayData* VanillaDict::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto const a = as(ad);
  return a->cowCheck() ? a->copyMixed() : a;
}

ArrayData* VanillaKeyset::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto const a = asSet(ad);
  return a->cowCheck() ? a->copySet() : a;
}

ArrayData* VanillaVec::EscalateForSort(ArrayData* ad, SortFunction sf) {
  assertx(checkInvariants(ad));
  assertx(sf != SORTFUNC_KSORT);
  if (isSortFamily(sf)) { // sort/rsort/usort
    return ad->cowCheck() ? VanillaVec::Copy(ad) : ad;
  }
  return ToMixedCopy(ad);
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
    try {                                                       \
      CALL_SORT(acc_type);                                      \
    } catch (...) {                                             \
      /* Make sure we leave the array in a consistent state */  \
      a->postSort(resetKeys);                                   \
      throw;                                                    \
    }                                                           \
    a->postSort(resetKeys);                                     \
  } while (0)

void VanillaDict::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = as(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocKeyAccessor<VanillaDict::Elm>, false);
}

void VanillaDict::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = as(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  a->m_nextKI = 0;
  SORT_BODY(AssocValAccessor<VanillaDict::Elm>, true);
}

void VanillaDict::Asort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = as(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocValAccessor<VanillaDict::Elm>, false);
}

void VanillaKeyset::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asSet(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocKeyAccessor<VanillaKeyset::Elm>, false);
}

#undef SORT_BODY

void VanillaVec::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  assertx(checkInvariants(ad));
  if (ad->m_size <= 1) {
    return;
  }
  assertx(!ad->hasMultipleRefs());
  auto a = ad;
  SortFlavor flav = preSort(ad);
  auto data_begin = VanillaLvalIterator { ad, 0 };
  auto data_end = data_begin + a->m_size;
  CALL_SORT(VanillaLvalAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

#define USER_SORT_BODY(acc_type, resetKeys)                     \
  do {                                                          \
    CoeffectsAutoGuard _;                                       \
    if (!a->m_size) return true;                                \
    CallCtx ctx;                                                \
    vm_decode_function(cmp_function, ctx);                      \
    if (!ctx.func) {                                            \
      return false;                                             \
    }                                                           \
    a->preSort<acc_type>(acc_type(), false);                    \
    SCOPE_EXIT {                                                \
      /* Make sure we leave the array in a consistent state */  \
      a->postSort(resetKeys);                                   \
    };                                                          \
    ElmUCompare<acc_type> comp;                                 \
    comp.ctx = &ctx;                                            \
    HPHP::Sort::sort(a->data(), a->data() + a->m_size, comp);   \
    return true;                                                \
  } while (0)

bool VanillaDict::Uksort(ArrayData* ad, const Variant& cmp_function) {
  auto a = as(ad);
  USER_SORT_BODY(AssocKeyAccessor<VanillaDict::Elm>, false);
}

bool VanillaDict::Usort(ArrayData* ad, const Variant& cmp_function) {
  auto a = as(ad);
  a->m_nextKI = 0;
  USER_SORT_BODY(AssocValAccessor<VanillaDict::Elm>, true);
}

bool VanillaDict::Uasort(ArrayData* ad, const Variant& cmp_function) {
  auto a = as(ad);
  USER_SORT_BODY(AssocValAccessor<VanillaDict::Elm>, false);
}

bool VanillaKeyset::Uksort(ArrayData* ad, const Variant& cmp_function) {
  auto a = asSet(ad);
  USER_SORT_BODY(AssocKeyAccessor<VanillaKeyset::Elm>, false);
}

#undef USER_SORT_BODY

SortFlavor VanillaVec::preSort(ArrayData* ad) {
  assertx(checkInvariants(ad));
  assertx(ad->m_size > 0);
  bool allInts = true;
  bool allStrs = true;
  uint32_t i = 0;
  uint32_t size = ad->m_size;
  do {
    const auto type = VanillaVec::LvalUncheckedInt(ad, i).type();
    if (type == KindOfInt64) {
      if (!allInts) return GenericSort;
      allStrs = false;
    } else if (isStringType(type)) {
      if (!allStrs) return GenericSort;
      allInts = false;
    } else {
      return GenericSort;
    }
  } while (++i < size);
  if (allInts) return IntegerSort;
  assertx(allStrs);
  return StringSort;
}

bool VanillaVec::Usort(ArrayData* ad, const Variant& cmp_function) {
  assertx(checkInvariants(ad));
  if (ad->m_size <= 1) {
    return true;
  }
  assertx(!ad->hasMultipleRefs());
  CoeffectsAutoGuard _;
  CallCtx ctx;
  vm_decode_function(cmp_function, ctx);
  if (!ctx.func) {
    return false;
  }
  ElmUCompare<VanillaLvalAccessor> comp;
  comp.ctx = &ctx;
  auto data = VanillaLvalIterator { ad, 0 };
  Sort::sort(data, data + ad->m_size, comp);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
