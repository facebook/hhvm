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

#include <runtime/base/array/zend_array.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/array/sort_helpers.h>
#include <runtime/base/complex_types.h>
#include <util/hash.h>
#include <util/util.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct KeyAccessor {
  typedef const ZendArray::Bucket* ElmT;
  bool isInt(ElmT elm) const { return elm->hasIntKey(); }
  bool isStr(ElmT elm) const { return elm->hasStrKey(); }
  int64 getInt(ElmT elm) const { return elm->ikey; }
  StringData* getStr(ElmT elm) const { return elm->skey; }
  Variant getValue(ElmT elm) const {
    if (isInt(elm)) {
      return getInt(elm);
    }
    ASSERT(isStr(elm));
    return getStr(elm);
  }
};

struct ValAccessor {
  typedef const ZendArray::Bucket* ElmT;
  bool isInt(ElmT elm) const {
    return ((TypedValue*)&elm->data)->m_type == KindOfInt64;
  }
  bool isStr(ElmT elm) const {
    return IS_STRING_TYPE(((TypedValue*)&elm->data)->m_type);
  }
  int64 getInt(ElmT elm) const {
    return ((TypedValue*)&elm->data)->m_data.num;
  }
  StringData* getStr(ElmT elm) const {
    return ((TypedValue*)&elm->data)->m_data.pstr;
  }
  Variant getValue(ElmT elm) const {
    return elm->data;
  }
};

#define CONNECT_TO_BUCKET_LIST(element, list_head)                      \
  (element)->pNext = (list_head);                                       \

#define SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p)                   \
do {                                                                    \
  m_arBuckets[nIndex] = (p);                                            \
} while (0)

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
ZendArray::SortFlavor
ZendArray::preSort(Bucket** buffer, const AccessorT& acc, bool checkTypes) {
  ASSERT(m_size > 0);
  bool allInts UNUSED = true;
  bool allStrs UNUSED = true;
  uint i = 0;
  // Build up an auxillary array of Bucket pointers. We will
  // sort this auxillary array, and then we will rebuild the
  // linked list based on the result.
  if (checkTypes) {
    for (Bucket *p = m_pListHead; p; ++i, p = p->pListNext) {
      allInts = (allInts && acc.isInt(p));
      allStrs = (allStrs && acc.isStr(p));
      buffer[i] = p;
    }
    return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
  } else {
    for (Bucket *p = m_pListHead; p; ++i, p = p->pListNext) {
      buffer[i] = p;
    }
    return GenericSort;
  }
}

/**
 * postSort() runs after sorting has been performed. For ZendArray, postSort()
 * handles rewiring the linked list according to the results of the sort. Also,
 * if resetKeys is true, postSort() will renumber the keys 0 thru n-1.
 */
void ZendArray::postSort(Bucket** buffer, bool resetKeys) {
  uint last = m_size-1;
  m_pListHead = buffer[0];
  m_pListTail = buffer[last];
  m_pos = (ssize_t)m_pListHead;
  Bucket* b = buffer[0];
  b->pListLast = NULL;
  if (resetKeys) {
    memset(m_arBuckets, 0, tableSize() * sizeof(Bucket*));
    for (uint i = 0; i < last; ++i) {
      Bucket* bNext = buffer[i+1];
      b->pListNext = bNext;
      bNext->pListLast = b;
      if (b->hasStrKey() && b->skey->decRefCount() == 0) {
        DELETE(StringData)(b->skey);
      }
      b->setIntKey(i);
      uint nIndex = (i & m_nTableMask);
      CONNECT_TO_BUCKET_LIST(b, m_arBuckets[nIndex]);
      SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, b);
      b = bNext;
    }
    if (b->hasStrKey() && b->skey->decRefCount() == 0) {
      DELETE(StringData)(b->skey);
    }
    b->setIntKey(last);
    uint nIndex = (last & m_nTableMask);
    CONNECT_TO_BUCKET_LIST(b, m_arBuckets[nIndex]);
    SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, b);
    m_nNextFreeElement = m_size;
  } else {
    for (uint i = 0; i < last; ++i) {
      Bucket* bNext = buffer[i+1];
      b->pListNext = bNext;
      bNext->pListLast = b;
      b = bNext;
    }
  }
  b->pListNext = NULL;
}

ArrayData* ZendArray::escalateForSort() {
  if (getCount() > 1) {
    return copyImpl();
  }
  return this;
}

#define SORT_CASE(flag, cmp_type, acc_type) \
  case flag: { \
    if (ascending) { \
      cmp_type##Compare<acc_type, flag, true> comp; \
      HPHP::Sort::sort(buffer, buffer + m_size, comp); \
    } else { \
      cmp_type##Compare<acc_type, flag, false> comp; \
      HPHP::Sort::sort(buffer, buffer + m_size, comp); \
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
        m_nNextFreeElement = 0; \
      } \
      return; \
    } \
    /* Allocate an auxillary buffer of Bucket*'s */ \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    SortFlavor flav = preSort<acc_type>(buffer, acc_type(), true); \
    try { \
      /* Sort the auxillary buffer */ \
      CALL_SORT(acc_type); \
    } catch (...) { \
      /* Leave the array in a consistent state, and make */ \
      /* sure we don't leak the buffer */ \
      postSort(buffer, resetKeys); \
      smart_free(buffer); \
      throw; \
    } \
    postSort(buffer, resetKeys); \
    /* Free the buffer */ \
    smart_free(buffer); \
  } while(0)

void ZendArray::ksort(int sort_flags, bool ascending) {
  SORT_BODY(KeyAccessor, false);
}

void ZendArray::sort(int sort_flags, bool ascending) {
  SORT_BODY(ValAccessor, true);
}

void ZendArray::asort(int sort_flags, bool ascending) {
  SORT_BODY(ValAccessor, false);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT
#undef SORT_BODY

#define USER_SORT_BODY(acc_type, resetKeys) \
  do { \
    freeStrongIterators(); \
    if (!m_size) { \
      if (resetKeys) { \
        m_nNextFreeElement = 0; \
      } \
      return; \
    } \
    /* Allocate an auxillary buffer of Bucket*'s */ \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    preSort<acc_type>(buffer, acc_type(), false); \
    try { \
      /* Sort the auxillary buffer */ \
      ElmUCompare<acc_type> comp; \
      comp.callback = &cmp_function; \
      HPHP::Sort::sort(buffer, buffer + m_size, comp); \
    } catch (...) { \
      /* Leave the array in a consistent state, and make */ \
      /* sure we don't leak the buffer */ \
      postSort(buffer, resetKeys); \
      smart_free(buffer); \
      throw; \
    } \
    postSort(buffer, resetKeys); \
    /* Free the buffer */ \
    smart_free(buffer); \
  } while(0)

void ZendArray::uksort(CVarRef cmp_function) {
  USER_SORT_BODY(KeyAccessor, false);
}

void ZendArray::usort(CVarRef cmp_function) {
  USER_SORT_BODY(ValAccessor, true);
}

void ZendArray::uasort(CVarRef cmp_function) {
  USER_SORT_BODY(ValAccessor, false);
}

#undef USER_SORT_BODY

///////////////////////////////////////////////////////////////////////////////
}

