/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

// inline methods of HphpArray
#include "hphp/runtime/base/hphp-array-defs.h"

#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct KeyAccessor {
  typedef const HphpArray::Elm& ElmT;
  bool isInt(ElmT elm) const { return elm.hasIntKey(); }
  bool isStr(ElmT elm) const { return elm.hasStrKey(); }
  int64_t getInt(ElmT elm) const { return elm.ikey; }
  StringData* getStr(ElmT elm) const { return elm.key; }
  Variant getValue(ElmT elm) const {
    if (isInt(elm)) {
      return getInt(elm);
    }
    assert(isStr(elm));
    return getStr(elm);
  }
};

struct ValAccessor {
  typedef const HphpArray::Elm& ElmT;
  bool isInt(ElmT elm) const { return elm.data.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm.data.m_type); }
  int64_t getInt(ElmT elm) const { return elm.data.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm.data.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm.data); }
};

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
HphpArray::SortFlavor
HphpArray::preSort(const AccessorT& acc, bool checkTypes) {
  assert(m_size > 0);
  if (isPacked()) {
    // todo t2607563: this is pessimistic.
    packedToMixed();
  }
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
 * postSort() runs after the sort has been performed. For HphpArray, postSort()
 * handles rebuilding the hash. Also, if resetKeys is true, postSort() will
 * renumber the keys 0 thru n-1.
 */
void HphpArray::postSort(bool resetKeys) {
  assert(m_size > 0);
  auto const ht = hashTab();
  initHash(ht, hashSize());
  m_hLoad = 0;
  if (resetKeys) {
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data()[pos];
      if (e.hasStrKey()) decRefStr(e.key);
      e.setIntKey(pos);
      ht[pos] = pos;
    }
    m_nextKI = m_size;
  } else {
    auto mask = m_tableMask;
    auto data = this->data();
    for (uint32_t pos = 0; pos < m_used; ++pos) {
      auto& e = data[pos];
      auto ei = findForNewInsert(ht, mask,
                                 e.hasIntKey() ? e.ikey : e.hash());
      *ei = pos;
    }
  }
  m_hLoad = m_size;
}

ArrayData* HphpArray::EscalateForSort(ArrayData* ad) {
  // task #1910931 only do this for refCount() > 1
  return asHphpArray(ad)->copyImpl();
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(a->data(), a->data() + a->m_size, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(a->data(), a->data() + a->m_size, comp); \
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
#define CALL_SORT(acc_type) \
  if (flav == StringSort) { \
    SORT_CASE_BLOCK(StrElm, acc_type) \
  } else if (flav == IntegerSort) { \
    SORT_CASE_BLOCK(IntElm, acc_type) \
  } else { \
    SORT_CASE_BLOCK(Elm, acc_type) \
  }
#define SORT_BODY(acc_type, resetKeys) \
  do { \
    a->freeStrongIterators(); \
    if (!a->m_size) { \
      if (resetKeys) { \
        a->m_nextKI = 0; \
      } \
      return; \
    } \
    SortFlavor flav = a->preSort<acc_type>(acc_type(), true); \
    a->m_pos = ssize_t(0); \
    try { \
      CALL_SORT(acc_type); \
    } catch (...) { \
      /* Make sure we leave the array in a consistent state */ \
      a->postSort(resetKeys); \
      throw; \
    } \
    a->postSort(resetKeys); \
  } while (0)

void HphpArray::Ksort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asHphpArray(ad);
  SORT_BODY(KeyAccessor, false);
}

void HphpArray::Sort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asHphpArray(ad);
  SORT_BODY(ValAccessor, true);
}

void HphpArray::Asort(ArrayData* ad, int sort_flags, bool ascending) {
  auto a = asHphpArray(ad);
  SORT_BODY(ValAccessor, false);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

#define USER_SORT_BODY(acc_type, resetKeys)                     \
  do {                                                          \
    a->freeStrongIterators();                                   \
    if (!a->m_size) {                                           \
      if (resetKeys) {                                          \
        a->m_nextKI = 0;                                        \
      }                                                         \
      return true;                                              \
    }                                                           \
    CallCtx ctx;                                                \
    JIT::CallerFrame cf;                                     \
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

bool HphpArray::Uksort(ArrayData* ad, CVarRef cmp_function) {
  auto a = asHphpArray(ad);
  USER_SORT_BODY(KeyAccessor, false);
}

bool HphpArray::Usort(ArrayData* ad, CVarRef cmp_function) {
  auto a = asHphpArray(ad);
  USER_SORT_BODY(ValAccessor, true);
}

bool HphpArray::Uasort(ArrayData* ad, CVarRef cmp_function) {
  auto a = asHphpArray(ad);
  USER_SORT_BODY(ValAccessor, false);
}

#undef USER_SORT_BODY

///////////////////////////////////////////////////////////////////////////////
}

