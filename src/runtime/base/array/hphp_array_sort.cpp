/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/array/hphp_array.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/array/sort_helpers.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/execution_context.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct KeyAccessor {
  typedef const HphpArray::Elm& ElmT;
  bool isInt(ElmT elm) const { return elm.hasIntKey(); }
  bool isStr(ElmT elm) const { return elm.hasStrKey(); }
  int64 getInt(ElmT elm) const { return elm.ikey; }
  StringData* getStr(ElmT elm) const { return elm.key; }
  Variant getValue(ElmT elm) const {
    if (isInt(elm)) {
      return getInt(elm);
    }
    ASSERT(isStr(elm));
    return getStr(elm);
  }
};

struct ValAccessor {
  typedef const HphpArray::Elm& ElmT;
  bool isInt(ElmT elm) const { return elm.data.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm.data.m_type); }
  int64 getInt(ElmT elm) const { return elm.data.m_data.num; }
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
  ASSERT(m_size > 0);
  if (!checkTypes && ssize_t(m_size) == ssize_t(m_lastE + 1)) {
    // No need to loop over the elements, we're done
    return GenericSort;
  }
  Elm* start = m_data;
  Elm* end = m_data + m_lastE + 1;
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  for (;;) {
    if (checkTypes) {
      while (start->data.m_type != KindOfTombstone) {
        allInts = (allInts && acc.isInt(*start));
        allStrs = (allStrs && acc.isStr(*start));
        ++start;
        if (start == end) {
          goto done;
        }
      }
    } else {
      while (start->data.m_type != KindOfTombstone) {
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
    while (end->data.m_type == KindOfTombstone) {
      --end;
      if (start == end) {
        goto done;
      }
    }
    memcpy(start, end, sizeof(Elm));
  }
done:
  m_lastE = (start - m_data) - 1;
  ASSERT(ssize_t(m_size) == ssize_t(m_lastE + 1));
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
  ASSERT(m_size > 0);
  size_t tableSize = computeTableSize(m_tableMask);
  initHash(m_hash, tableSize);
  m_hLoad = 0;
  if (resetKeys) {
    for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
      Elm* e = &m_data[pos];
      if (e->hasStrKey() && e->key->decRefCount() == 0) {
        e->key->release();
      }
      e->setIntKey(pos);
      m_hash[pos] = pos;
    }
    m_nextKI = m_size;
  } else {
    for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
      Elm* e = &m_data[pos];
      ElmInd* ei = findForNewInsert(e->hasIntKey() ? e->ikey : e->hash);
      *ei = pos;
    }
  }
  m_hLoad = m_size;
}

ArrayData* HphpArray::escalateForSort() {
  if (getCount() > 1) {
    return copyImpl();
  }
  return this;
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(m_data, m_data + m_size, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(m_data, m_data + m_size, comp); \
    } \
    break; \
  }
#define SORT_CASE_BLOCK(cmp_type, acc_type) \
  switch (sort_flags) { \
    default: /* fall through to SORT_REGULAR case */ \
    SORT_CASE(SORT_REGULAR, cmp_type, acc_type) \
    SORT_CASE(SORT_NUMERIC, cmp_type, acc_type) \
    SORT_CASE(SORT_STRING, cmp_type, acc_type) \
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
    freeStrongIterators(); \
    if (!m_size) { \
      if (resetKeys) { \
        m_nextKI = 0; \
      } \
      return; \
    } \
    SortFlavor flav = preSort<acc_type>(acc_type(), true); \
    m_pos = ssize_t(0); \
    try { \
      CALL_SORT(acc_type); \
    } catch (...) { \
      /* Make sure we leave the array in a consistent state */ \
      postSort(resetKeys); \
      throw; \
    } \
    postSort(resetKeys); \
  } while (0)

void HphpArray::ksort(int sort_flags, bool ascending) {
  SORT_BODY(KeyAccessor, false);
}

void HphpArray::sort(int sort_flags, bool ascending) {
  SORT_BODY(ValAccessor, true);
}

void HphpArray::asort(int sort_flags, bool ascending) {
  SORT_BODY(ValAccessor, false);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

#define USER_SORT_BODY(acc_type, resetKeys) \
  do { \
    freeStrongIterators(); \
    if (!m_size) { \
      if (resetKeys) { \
        m_nextKI = 0; \
      } \
      return; \
    } \
    preSort<acc_type>(acc_type(), false); \
    m_pos = ssize_t(0); \
    try { \
      ElmUCompare<acc_type> comp; \
      comp.callback = &cmp_function; \
      HPHP::Sort::sort(m_data, m_data + m_size, comp); \
    } catch (...) { \
      /* Make sure we leave the array in a consistent state */ \
      postSort(resetKeys); \
      throw; \
    } \
    postSort(resetKeys); \
  } while (0)

void HphpArray::uksort(CVarRef cmp_function) {
  USER_SORT_BODY(KeyAccessor, false);
}

void HphpArray::usort(CVarRef cmp_function) {
  USER_SORT_BODY(ValAccessor, true);
}

void HphpArray::uasort(CVarRef cmp_function) {
  USER_SORT_BODY(ValAccessor, false);
}

#undef USER_SORT_BODY

///////////////////////////////////////////////////////////////////////////////
}

