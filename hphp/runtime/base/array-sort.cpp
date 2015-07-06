/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"
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
template <typename AccessorT>
SortFlavor MixedArray::preSort(const AccessorT& acc, bool checkTypes) {
  assert(m_size > 0);
  assert(!isPacked());
  if (!checkTypes && m_size == m_used) {
    // No need to loop over the elements, we're done
    return GenericSort;
  }
  Elm* start = data();
  Elm* end = data() + m_used;
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  for (;;) {
    if (checkTypes) {
      while (!isTombstone(start->data.m_type)) {
        allInts = (allInts && acc.isInt(*start));
        allStrs = (allStrs && acc.isStr(*start));
        ++start;
        if (start == end) {
          goto done;
        }
      }
    } else {
      while (!isTombstone(start->data.m_type)) {
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
    while (isTombstone(end->data.m_type)) {
      --end;
      if (start == end) {
        goto done;
      }
    }
    memcpy(start, end, sizeof(Elm));
  }
done:
  m_used = start - data();
  assert(m_size == m_used);
  if (checkTypes) {
    return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
  } else {
    return GenericSort;
  }
}

/**
 * postSort() runs after the sort has been performed. For MixedArray, postSort()
 * handles rebuilding the hash. Also, if resetKeys is true, postSort() will
 * renumber the keys 0 thru n-1.
 */
void MixedArray::postSort(bool resetKeys) {   // nothrow guarantee
  assert(m_size > 0);
  auto const ht = hashTab();
  initHash(ht, m_scale);
  if (resetKeys) {
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data()[pos];
      if (e.hasStrKey()) decRefStr(e.skey);
      e.setIntKey(pos);
      ht[pos] = pos;
    }
    m_nextKI = m_size;
  } else {
    auto mask = this->mask();
    auto data = this->data();
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data[pos];
      *findForNewInsert(ht, mask, e.probe()) = pos;
    }
  }
}

ArrayData* MixedArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  auto a = asMixed(ad);
  // We can uncomment later if we want this feature.
  // if (a->m_size <= 1 && !isSortFamily(sf)) {
  //   return a;
  // }
  if (UNLIKELY(hasUserDefinedCmp(sf) || a->hasMultipleRefs())) {
    auto ret = a->copyMixed();
    assert(ret->hasExactlyOneRef());
    return ret;
  } else {
    return a;
  }
}

ArrayData* PackedArray::EscalateForSort(ArrayData* ad, SortFunction sf) {
  if (ad->m_size <= 1) {
    return ad;
  }
  if (sf == SORTFUNC_KSORT) {
    return ad;                          // trivial for packed arrays.
  }
  if (isSortFamily(sf)) {               // sort/rsort/usort
    if (UNLIKELY(ad->hasMultipleRefs())) {
      auto ret = PackedArray::Copy(ad);
      assert(ret->hasExactlyOneRef());
      return ret;
    } else {
      return ad;
    }
  }
  assert(checkInvariants(ad));
  auto ret = ToMixedCopy(ad);
  assert(ret->hasExactlyOneRef());
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
    if (UNLIKELY(strong_iterators_exist())) {                   \
      free_strong_iterators(a);                                 \
    }                                                           \
    if (!a->m_size) {                                           \
      if (resetKeys) {                                          \
        a->m_nextKI = 0;                                        \
      }                                                         \
      return;                                                   \
    }                                                           \
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
  SORT_BODY(AssocValAccessor<MixedArray::Elm>, true);
}

void MixedArray::Asort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asMixed(ad);
  auto data_begin = a->data();
  auto data_end = data_begin + a->m_size;
  SORT_BODY(AssocValAccessor<MixedArray::Elm>, false);
}

void PackedArray::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  assert(ad->isPacked());
  if (ad->m_size <= 1) {
    return;
  }
  assert(!ad->hasMultipleRefs());
  auto a = ad;
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(a);
  }
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
    if (UNLIKELY(strong_iterators_exist())) {                   \
      free_strong_iterators(a);                                 \
    }                                                           \
    if (!a->m_size) {                                           \
      if (resetKeys) {                                          \
        a->m_nextKI = 0;                                        \
      }                                                         \
      return true;                                              \
    }                                                           \
    CallCtx ctx;                                                \
    CallerFrame cf;                                             \
    vm_decode_function(cmp_function, cf(), false, ctx);         \
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
  USER_SORT_BODY(AssocValAccessor<MixedArray::Elm>, true);
}

bool MixedArray::Uasort(ArrayData* ad, const Variant& cmp_function) {
  auto a = asMixed(ad);
  USER_SORT_BODY(AssocValAccessor<MixedArray::Elm>, false);
}

SortFlavor PackedArray::preSort(ArrayData* ad) {
  assert(ad->isPacked());
  auto const data = packedData(ad);
  TVAccessor acc;
  uint32_t sz = ad->m_size;
  bool allInts = true;
  bool allStrs = true;
  for (uint32_t i = 0; i < sz; ++i) {
    allInts = (allInts && acc.isInt(data[i]));
    allStrs = (allStrs && acc.isStr(data[i]));
  }
  return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
}

bool PackedArray::Usort(ArrayData* ad, const Variant& cmp_function) {
  assert(ad->isPacked());
  if (ad->m_size <= 1) {
    return true;
  }
  assert(!ad->hasMultipleRefs());
  if (UNLIKELY(strong_iterators_exist())) {
    free_strong_iterators(ad);
  }
  ElmUCompare<TVAccessor> comp;
  CallCtx ctx;
  CallerFrame cf;
  vm_decode_function(cmp_function, cf(), false, ctx);
  if (!ctx.func) {
    return false;
  }
  comp.ctx = &ctx;
  auto const data = packedData(ad);
  Sort::sort(data, data + ad->m_size, comp);
  return true;
}

#undef USER_SORT_BODY

///////////////////////////////////////////////////////////////////////////////
}
