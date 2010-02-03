/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include <cpp/base/array/zend_array.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_array.h>
#include <util/hash.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS_CLS(ZendArray, Bucket);
IMPLEMENT_SMART_ALLOCATION(ZendArray, SmartAllocatorImpl::NeedRestoreOnce);
///////////////////////////////////////////////////////////////////////////////
// static members

StaticEmptyZendArray StaticEmptyZendArray::s_theEmptyArray;

///////////////////////////////////////////////////////////////////////////////
// construction/destruciton

ZendArray::ZendArray(uint nSize /* = 0 */) :
  m_nNumOfElements(0), m_nNextFreeElement(0),
  m_pListHead(NULL), m_pListTail(NULL), m_arBuckets(NULL), m_linear(false) {

  if (nSize >= 0x80000000) {
    m_nTableSize = 0x80000000; // prevent overflow
  } else {
    uint i = 3;
    while ((1U << i) < nSize) {
      i++;
    }
    m_nTableSize = 1 << i;
  }
  m_nTableMask = m_nTableSize - 1;
  m_arBuckets = (Bucket **)calloc(m_nTableSize, sizeof(Bucket *));
}

ZendArray::~ZendArray() {
  Bucket *p = m_pListHead;
  while (p) {
    Bucket *q = p;
    p = p->pListNext;
    DELETE(Bucket)(q);
  }
  if (!m_linear && m_arBuckets) {
    free(m_arBuckets);
  }
}

///////////////////////////////////////////////////////////////////////////////
// iterations

ssize_t ZendArray::iter_begin() const {
  Bucket *p = m_pListHead;
  return p ? reinterpret_cast<ssize_t>(p) : ArrayData::invalid_index;
}

ssize_t ZendArray::iter_end() const {
  Bucket *p = m_pListTail;
  return p ? reinterpret_cast<ssize_t>(p) : ArrayData::invalid_index;
}

ssize_t ZendArray::iter_advance(ssize_t prev) const {
  if (prev == 0 || prev == ArrayData::invalid_index) {
    return ArrayData::invalid_index;
  }
  Bucket *p = reinterpret_cast<Bucket *>(prev);
  p = p->pListNext;
  return p ? reinterpret_cast<ssize_t>(p) : ArrayData::invalid_index;
}

ssize_t ZendArray::iter_rewind(ssize_t prev) const {
  if (prev == 0 || prev == ArrayData::invalid_index) {
    return ArrayData::invalid_index;
  }
  Bucket *p = reinterpret_cast<Bucket *>(prev);
  p = p->pListLast;
  return p ? reinterpret_cast<ssize_t>(p) : ArrayData::invalid_index;
}

Variant ZendArray::getKey(ssize_t pos) const {
  ASSERT(pos && pos != ArrayData::invalid_index);
  Bucket *p = reinterpret_cast<Bucket *>(pos);
  if (p->key) {
    return p->key;
  }
  return (int64)p->h;
}

Variant ZendArray::getValue(ssize_t pos) const {
  ASSERT(pos && pos != ArrayData::invalid_index);
  Bucket *p = reinterpret_cast<Bucket *>(pos);
  return p->data;
}

CVarRef ZendArray::getValueRef(ssize_t pos) const {
  ASSERT(pos && pos != ArrayData::invalid_index);
  Bucket *p = reinterpret_cast<Bucket *>(pos);
  return p->data;
}

bool ZendArray::isVectorData() const {
  int64 index = 0;
  for (Bucket *p = m_pListHead; p; p = p->pListNext) {
    if (p->key || p->h != index++) return false;
  }
  return true;
}

Variant ZendArray::reset() {
  m_pos = (ssize_t)m_pListHead;
  if (m_pListHead) {
    return m_pListHead->data;
  }
  return false;
}

Variant ZendArray::prev() {
  if (m_pos) {
    Bucket *p = reinterpret_cast<Bucket *>(m_pos);
    p = p->pListLast;
    m_pos = (ssize_t)p;
    if (p) {
      return p->data;
    }
  }
  return false;
}

Variant ZendArray::next() {
  if (m_pos) {
    Bucket *p = reinterpret_cast<Bucket *>(m_pos);
    p = p->pListNext;
    m_pos = (ssize_t)p;
    if (p) {
      return p->data;
    }
  }
  return false;
}

Variant ZendArray::end() {
  m_pos = (ssize_t)m_pListTail;
  if (m_pListTail) {
    return m_pListTail->data;
  }
  return false;
}

Variant ZendArray::key() const {
  if (m_pos) {
    Bucket *p = reinterpret_cast<Bucket *>(m_pos);
    if (p->key) {
      return p->key;
    }
    return (int64)p->h;
  }
  return null;
}

Variant ZendArray::value(ssize_t &pos) const {
  if (pos && pos != ArrayData::invalid_index) {
    Bucket *p = reinterpret_cast<Bucket *>(pos);
    return p->data;
  }
  return false;
}

Variant ZendArray::current() const {
  if (m_pos) {
    Bucket *p = reinterpret_cast<Bucket *>(m_pos);
    return p->data;
  }
  return false;
}

Variant ZendArray::each() {
  if (m_pos) {
    Array ret;
    Bucket *p = reinterpret_cast<Bucket *>(m_pos);
    Variant key = getKey(m_pos);
    Variant value = getValue(m_pos);
    ret.set(1, value);
    ret.set("value", value);
    ret.set(0, key);
    ret.set("key", key);
    m_pos = (ssize_t)p->pListNext;
    return ret;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// lookups

ZendArray::Bucket *ZendArray::find(int64 h) const {
  for (Bucket *p = m_arBuckets[h & m_nTableMask]; p; p = p->pNext) {
    if (p->key == NULL && p->h == h) {
      return p;
    }
  }
  return NULL;
}

ZendArray::Bucket *ZendArray::find(const char *k, int len,
                                   int64 prehash /* = -1 */,
                                   int64 *h /* = NULL */) const {
  if (prehash < 0) {
    prehash = hash_string(k, len);
    if (h) {
      *h = prehash;
    }
  }
  for (Bucket *p = m_arBuckets[prehash & m_nTableMask]; p; p = p->pNext) {
    if (p->key && p->h == prehash && p->key->size() == len &&
        memcmp(p->key->data(), k, len) == 0) {
      return p;
    }
  }
  return NULL;
}

bool ZendArray::exists(int64 k, int64 prehash /* = -1 */) const {
  return find(k);
}

bool ZendArray::exists(litstr k, int64 prehash /* = -1 */) const {
  return find(k, strlen(k), prehash);
}

bool ZendArray::exists(CStrRef k, int64 prehash /* = -1 */) const {
  return find(k.data(), k.size(), prehash);
}

bool ZendArray::exists(CVarRef k, int64 prehash /* = -1 */) const {
  if (k.isNumeric()) return find(k.toInt64());
  String key = k.toString();
  return find(key.data(), key.size(), prehash);
}

bool ZendArray::idxExists(ssize_t idx) const {
  return (idx && idx != ArrayData::invalid_index);
}

Variant ZendArray::get(int64 k, int64 prehash /* = -1 */) const {
  Bucket *p = find(k);
  if (p) {
    return p->data;
  }
  return null;
}

Variant ZendArray::get(litstr k, int64 prehash /* = -1 */) const {
  Bucket *p = find(k, strlen(k), prehash);
  if (p) {
    return p->data;
  }
  return null;
}

Variant ZendArray::get(CStrRef k, int64 prehash /* = -1 */) const {
  Bucket *p = find(k.data(), k.size(), prehash);
  if (p) {
    return p->data;
  }
  return null;
}

Variant ZendArray::get(CVarRef k, int64 prehash /* = -1 */) const {
  Bucket *p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size(), prehash);
  }
  if (p) {
    return p->data;
  }
  return null;
}

ssize_t ZendArray::getIndex(int64 k, int64 prehash /* = -1 */) const {
  Bucket *p = find(k);
  if (p) {
    return (ssize_t)p;
  }
  return ArrayData::invalid_index;
}

ssize_t ZendArray::getIndex(litstr k, int64 prehash /* = -1 */) const {
  Bucket *p = find(k, strlen(k), prehash);
  if (p) {
    return (ssize_t)p;
  }
  return ArrayData::invalid_index;
}

ssize_t ZendArray::getIndex(CStrRef k, int64 prehash /* = -1 */) const {
  Bucket *p = find(k.data(), k.size(), prehash);
  if (p) {
    return (ssize_t)p;
  }
  return ArrayData::invalid_index;
}

ssize_t ZendArray::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  Bucket *p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size(), prehash);
  }
  if (p) {
    return (ssize_t)p;
  }
  return ArrayData::invalid_index;
}

///////////////////////////////////////////////////////////////////////////////
// append/insert/update

#define CONNECT_TO_BUCKET_DLLIST(element, list_head)                    \
  (element)->pNext = (list_head);                                       \
  (element)->pLast = NULL;                                              \
  if ((element)->pNext) {                                               \
    (element)->pNext->pLast = (element);				\
  }

#define CONNECT_TO_GLOBAL_DLLIST(element)				\
  (element)->pListLast = m_pListTail;                                   \
  m_pListTail = (element);                                              \
  (element)->pListNext = NULL;                                          \
  if ((element)->pListLast != NULL) {                                   \
    (element)->pListLast->pListNext = (element);                        \
  }                                                                     \
  if (!m_pListHead) {                                                   \
    m_pListHead = (element);                                            \
  }                                                                     \
  if (m_pos == 0) {                                                     \
    m_pos = (ssize_t)(element);                                         \
  }

#define SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p)                   \
do {                                                                    \
  if (m_linear) {                                                       \
    int nbytes = m_nTableSize * sizeof(Bucket *);                       \
    Bucket **t = (Bucket **)malloc(nbytes);                             \
    memcpy(t, m_arBuckets, nbytes);                                     \
    m_arBuckets = t;                                                    \
    m_linear = false;                                                   \
  }                                                                     \
  m_arBuckets[nIndex] = (p);                                            \
} while (0)

void ZendArray::resize() {
  int curSize = m_nTableSize * sizeof(Bucket *);
  if (m_linear) {
    Bucket **t = (Bucket **)calloc(m_nTableSize << 1, sizeof(Bucket *));
    memcpy(t, m_arBuckets, curSize);
    m_arBuckets = t;
    m_linear = false;
  } else {
    m_arBuckets = (Bucket **)realloc(m_arBuckets, curSize << 1);
    memset((char*)m_arBuckets + curSize, 0, curSize);
  }
  m_nTableSize <<= 1;
  m_nTableMask = m_nTableSize - 1;
  rehash();
}

void ZendArray::rehash() {
  memset(m_arBuckets, 0, m_nTableSize * sizeof(Bucket *));
  for (Bucket *p = m_pListHead; p; p = p->pListNext) {
    uint nIndex = (p->h & m_nTableMask);
    CONNECT_TO_BUCKET_DLLIST(p, m_arBuckets[nIndex]);
    SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p);
  }
}

bool ZendArray::update(OpFlag flag, int64 h, CVarRef data,
                       Variant **pDest /* = NULL */) {
  Bucket *p = NULL;
  if (flag & HASH_NEXT_INSERT) {
    h = m_nNextFreeElement;
  } else {
    p = find(h);
  }

  if (p) {
    if (pDest) {
      *pDest = &p->data;
    }
    if (flag & HASH_ADD) {
      return false;
    }
    p->data = data;
    if ((long)h >= (long)m_nNextFreeElement) {
      m_nNextFreeElement = h + 1;
    }
    return true;
  }

  p = NEW(Bucket)();
  p->h = h;
  p->data = data;
  if (pDest) {
    *pDest = &p->data;
  }

  uint nIndex = (h & m_nTableMask);
  CONNECT_TO_BUCKET_DLLIST(p, m_arBuckets[nIndex]);
  SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p);
  CONNECT_TO_GLOBAL_DLLIST(p);

  if ((long)h >= (long)m_nNextFreeElement) {
    m_nNextFreeElement = h + 1;
  }
  if (++m_nNumOfElements > m_nTableSize) {
    resize();
  }
  return true;
}

bool ZendArray::update(OpFlag flag, litstr key, int64 h, CVarRef data,
                       Variant **pDest /* = NULL */) {
  int len = strlen(key);
  Bucket *p = find(key, len, h, &h);
  if (p) {
    if (pDest) {
      *pDest = &p->data;
    }
    if (flag & HASH_ADD) {
      return false;
    }
    p->data = data;
    return true;
  }

  p = NEW(Bucket)();
  p->key = NEW(StringData)(key, len, AttachLiteral);
  p->key->incRefCount();
  p->h = h;
  p->data = data;
  if (pDest) {
    *pDest = &p->data;
  }

  uint nIndex = (h & m_nTableMask);
  CONNECT_TO_BUCKET_DLLIST(p, m_arBuckets[nIndex]);
  SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p);
  CONNECT_TO_GLOBAL_DLLIST(p);

  if (++m_nNumOfElements > m_nTableSize) {
    resize();
  }
  return true;
}

bool ZendArray::update(OpFlag flag, StringData *key, int64 h, CVarRef data,
                       Variant **pDest /* = NULL */) {
  Bucket *p = find(key->data(), key->size(), h, &h);
  if (p) {
    if (pDest) {
      *pDest = &p->data;
    }
    if (flag & HASH_ADD) {
      return false;
    }
    p->data = data;
    return true;
  }

  p = NEW(Bucket)();
  if (key->isShared()) {
    p->key = NEW(StringData)(key->data(), key->size(), CopyString);
  } else {
    p->key = key;
  }
  p->key->incRefCount();
  p->h = h;
  p->data = data;
  if (pDest) {
    *pDest = &p->data;
  }

  uint nIndex = (h & m_nTableMask);
  CONNECT_TO_BUCKET_DLLIST(p, m_arBuckets[nIndex]);
  SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p);
  CONNECT_TO_GLOBAL_DLLIST(p);

  if (++m_nNumOfElements > m_nTableSize) {
    resize();
  }
  return true;
}

ArrayData *ZendArray::lval(Variant *&ret, bool copy) {
  if (copy) {
    ZendArray *a = copyImpl();
    ASSERT(a->m_pListTail);
    ret = &a->m_pListTail->data;
    return a;
  }
  ASSERT(m_pListTail);
  ret = &m_pListTail->data;
  return NULL;
}

ArrayData *ZendArray::lval(int64 k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->update(HASH_ADD, k, null, &ret);
    return a;
  }
  update(HASH_ADD, k, null, &ret);
  return NULL;
}

ArrayData *ZendArray::lval(CStrRef k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  return lvalImpl(k.get(), ret, copy, prehash);
}

ArrayData *ZendArray::lval(litstr k, Variant *&ret, bool copy,
                           int64 prehash /* = -1 */) {
  return lvalImpl(k, ret, copy, prehash);
}

ArrayData *ZendArray::lval(CVarRef k, Variant *&ret, bool copy,
                         int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    return lval(k.toInt64(), ret, copy, prehash);
  } else {
    String key = k.toString();
    return lvalImpl(key.get(), ret, copy, prehash);
  }
}

ArrayData *ZendArray::set(int64 k, CVarRef v, bool copy,
                          int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->update(HASH_UPDATE, k, v);
    return a;
  }
  update(HASH_UPDATE, k, v);
  return NULL;
}

ArrayData *ZendArray::set(CStrRef k, CVarRef v, bool copy,
                          int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->update(HASH_UPDATE, k.get(), prehash, v);
    return a;
  }
  update(HASH_UPDATE, k.get(), prehash, v);
  return NULL;
}

ArrayData *ZendArray::set(litstr k, CVarRef v, bool copy,
                          int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->update(HASH_UPDATE, k, prehash, v);
    return a;
  }
  update(HASH_UPDATE, k, prehash, v);
  return NULL;
}

ArrayData *ZendArray::set(CVarRef k, CVarRef v, bool copy,
                          int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    if (copy) {
      ZendArray *a = copyImpl();
      a->update(HASH_UPDATE, k.toInt64(), v);
      return a;
    }
    update(HASH_UPDATE, k.toInt64(), v);
    return NULL;
  } else if (k.is(LiteralString)) {
    if (copy) {
      ZendArray *a = copyImpl();
      a->update(HASH_UPDATE, k.getLiteralString(), prehash, v);
      return a;
    }
    update(HASH_UPDATE, k.getLiteralString(), prehash, v);
    return NULL;
  } else {
    String sk = k.toString();
    StringData *sd = sk.get();
    if (copy) {
      ZendArray *a = copyImpl();
      a->update(HASH_UPDATE, sd, prehash, v);
      return a;
    }
    update(HASH_UPDATE, sd, prehash, v);
    return NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// delete

void ZendArray::erase(Bucket *p) {
  if (p) {
    uint nIndex = (p->h & m_nTableMask);
    if (p->pLast) {
      p->pLast->pNext = p->pNext;
    } else {
      SET_ARRAY_BUCKET_HEAD(m_arBuckets, nIndex, p->pNext);
    }
    if (p->pNext) {
      p->pNext->pLast = p->pLast;
    }
    if (p->pListLast) {
      p->pListLast->pListNext = p->pListNext;
    } else {
      /* Deleting the head of the list */
      ASSERT(m_pListHead == p);
      m_pListHead = p->pListNext;
    }
    if (p->pListNext) {
      p->pListNext->pListLast = p->pListLast;
    } else {
      ASSERT(m_pListTail == p);
      m_pListTail = p->pListLast;
    }
    if (m_pos == (ssize_t)p) {
      m_pos = (ssize_t)p->pListNext;
    }
    m_nNumOfElements--;

    DELETE(Bucket)(p);
  }
}

ArrayData *ZendArray::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->erase(a->find(k));
    return a;
  }
  erase(find(k));
  return NULL;
}

ArrayData *ZendArray::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->erase(a->find(k.data(), k.size(), prehash));
    return a;
  }
  erase(find(k.data(), k.size(), -1));
  return NULL;
}

ArrayData *ZendArray::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->erase(a->find(k, strlen(k), prehash));
    return a;
  }
  erase(find(k, strlen(k), -1));
  return NULL;
}

ArrayData *ZendArray::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    if (copy) {
      ZendArray *a = copyImpl();
      a->erase(a->find(k.toInt64()));
      return a;
    }
    erase(find(k.toInt64()));
    return NULL;
  } else {
    String key = k.toString();
    if (copy) {
      ZendArray *a = copyImpl();
      a->erase(a->find(key.data(), key.size(), prehash));
      return a;
    }
    erase(find(key.data(), key.size(), prehash));
    return NULL;
  }
}

ArrayData *ZendArray::copy() const {
  return copyImpl();
}

ZendArray *ZendArray::copyImpl() const {
  ZendArray *target = NEW(ZendArray)(m_nNumOfElements);
  for (Bucket *p = m_pListHead; p; p = p->pListNext) {
    if (p->data.isReferenced()) {
      p->data.setContagious();
    }
    if (p->key) {
      target->update(HASH_UPDATE, p->key, p->h, p->data);
    } else {
      target->update(HASH_UPDATE, p->h, p->data);
    }
  }
  target->m_pos = (ssize_t)target->m_pListHead;
  return target;
}

ArrayData *ZendArray::append(CVarRef v, bool copy) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->update(HASH_NEXT_INSERT, 0, v);
    return a;
  }
  update(HASH_NEXT_INSERT, 0, v);
  return NULL;
}

ArrayData *ZendArray::append(const ArrayData *elems, ArrayOp op, bool copy) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->append(elems, op, false);
    return a;
  }

  if (elems->supportValueRef()) {
    if (op == Plus) {
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        CVarRef value = it.secondRef();
        if (value.isReferenced()) value.setContagious();
        if (key.isNumeric()) {
          update(HASH_ADD, key.toInt64(), value);
        } else {
          String skey = key.toString();
          update(HASH_ADD, skey.get(), -1, value);
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        CVarRef value = it.secondRef();
        if (value.isReferenced()) value.setContagious();
        if (key.isNumeric()) {
          append(value, false);
        } else {
          set(key, value, false);
        }
      }
    }
  } else {
    if (op == Plus) {
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        if (key.isNumeric()) {
          update(HASH_ADD, key.toInt64(), it.second());
        } else {
          String skey = key.toString();
          update(HASH_ADD, skey.get(), -1, it.second());
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        if (key.isNumeric()) {
          append(it.second(), false);
        } else {
          set(key, it.second(), false);
        }
      }
    }
  }
  return NULL;
}

ArrayData *ZendArray::pop(Variant &value) {
  if (getCount() > 1) {
    ZendArray *a = copyImpl();
    a->pop(value);
    return a;
  }
  if (m_pListTail) {
    value = m_pListTail->data;
    if (!m_pListTail->key && (uint)m_pListTail->h == m_nNextFreeElement - 1) {
      m_nNextFreeElement--;
    }
    erase(m_pListTail);
  } else {
    value = null;
  }
  return NULL;
}

ArrayData *ZendArray::dequeue(Variant &value) {
  if (getCount() > 1) {
    ZendArray *a = copyImpl();
    a->dequeue(value);
    return a;
  }
  if (m_pListHead) {
    value = m_pListHead->data;
    erase(m_pListHead);
    renumber();
  } else {
    value = null;
  }
  return NULL;
}

ArrayData *ZendArray::insert(ssize_t pos, CVarRef v, bool copy) {
  if (copy) {
    ZendArray *a = copyImpl();
    a->insert(pos, v, false);
    return a;
  }

  Bucket *old_tail = m_pListTail;
  update(HASH_NEXT_INSERT, 0, v);
  if (m_nNumOfElements == 1) {
    return NULL; // only element in array, no need to move it.
  }

  // Move the newly inserted element from the tail of the ordering list to
  // whatever position was requested.
  Bucket *p = reinterpret_cast<Bucket *>(pos);
  if (p == old_tail) {
    return NULL; // already in the proper spot at the list end
  }

  if (pos == 0 || pos == ArrayData::invalid_index) {
    p = m_pListHead;
  }

  Bucket *new_elem = m_pListTail;

  // Remove from end of list
  m_pListTail = new_elem->pListLast;
  if (m_pListTail) {
    m_pListTail->pListNext = NULL;
  }

  // Insert before new position (p)
  new_elem->pListNext = p;
  new_elem->pListLast = p->pListLast;
  p->pListLast = new_elem;
  if (new_elem->pListLast) {
    new_elem->pListLast->pListNext = new_elem;
  } else {
    // no 'last' means we inserted at the front, so fix that pointer
    ASSERT(m_pListHead == p);
    m_pListHead = new_elem;
  }

  // Rewrite numeric keys to start from 0 and rehash
  renumber();
  return NULL;
}

void ZendArray::renumber() {
  unsigned long i = 0;
  for (Bucket *p = m_pListHead; p; p = p->pListNext) {
    if (p->key == NULL) {
      p->h = i++;
    }
  }
  m_nNextFreeElement = i;
  rehash();
}

void ZendArray::onSetStatic() {
  for (Bucket *p = m_pListHead; p; p = p->pListNext) {
    if (p->key) {
      p->key->setStatic();
    }
    p->data.setStatic();
  }
}

void ZendArray::getFullPos(FullPos &pos) {
  pos.primary = m_pos;
  if (m_pos) {
    Bucket *p = reinterpret_cast<Bucket *>(m_pos);
    pos.secondary = (ssize_t)p->h;
  }
}

bool ZendArray::setFullPos(const FullPos &pos) {
  // Only set if pos hasn't been invalidated.
  if (pos.primary) {
    for (Bucket *p = m_arBuckets[pos.secondary & m_nTableMask]; p;
         p = p->pNext) {
      if ((ssize_t)p == pos.primary) m_pos = (ssize_t)p;
    }
  }
  if (!m_pos) return false;
  return true;
}

CVarRef ZendArray::currentRef() {
  ASSERT(m_pos);
  Bucket *p = reinterpret_cast<Bucket *>(m_pos);
  return p->data;
}

CVarRef ZendArray::endRef() {
  ASSERT(m_pos);
  Bucket *p = reinterpret_cast<Bucket *>(m_pListTail);
  return p->data;
}

///////////////////////////////////////////////////////////////////////////////
// memory allocator methods.

bool ZendArray::calculate(int &size) {
  size += m_nTableSize * sizeof(Bucket *);
  return true;
}

void ZendArray::backup(LinearAllocator &allocator) {
  allocator.backup((const char*)m_arBuckets, m_nTableSize * sizeof(Bucket *));
}

void ZendArray::restore(const char *&data) {
  m_arBuckets = (Bucket**)data;
  data += m_nTableSize * sizeof(Bucket *);
  m_linear = true;
}

void ZendArray::sweep() {
  if (!m_linear && m_arBuckets) {
    free(m_arBuckets);
    m_arBuckets = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// class Bucket

ZendArray::Bucket::Bucket() :
  h(0), key(NULL), pListNext(NULL), pListLast(NULL), pNext(NULL), pLast(NULL) {
}

ZendArray::Bucket::~Bucket() {
  if (key && key->decRefCount() == 0) {
    DELETE(StringData)(key);
  }
}

void ZendArray::Bucket::dump() {
  printf("ZendArray::Bucket: %llx, %p, %p, %p, %p\n",
         h, pListNext, pListLast, pNext, pLast);
  if (key) {
    key->dump();
  }
  data.dump();
}

///////////////////////////////////////////////////////////////////////////////
}
