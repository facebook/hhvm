/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/array/small_array.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/runtime_option.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(SmallArray);

///////////////////////////////////////////////////////////////////////////////
// constructor

SmallArray::SmallArray() : m_nNumOfElements(0),
                           m_nListHead(ArrayData::invalid_index),
                           m_nListTail(ArrayData::invalid_index),
                           m_nNextFreeElement(0) {
  m_pos = ArrayData::invalid_index;
}

///////////////////////////////////////////////////////////////////////////////
// iterations

ssize_t SmallArray::iter_advance(ssize_t prev) const {
  if (prev >= 0 && m_arBuckets[prev].kind != Empty) {
    ASSERT(prev < SARR_TABLE_SIZE);
    return m_arBuckets[prev].next;
  }
  return ArrayData::invalid_index;
}

ssize_t SmallArray::iter_rewind(ssize_t prev) const {
  if (prev >= 0 && m_arBuckets[prev].kind != Empty) {
    ASSERT(prev < SARR_TABLE_SIZE);
    return m_arBuckets[prev].prev;
  }
  return ArrayData::invalid_index;
}

Variant SmallArray::getKey(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < SARR_TABLE_SIZE && m_arBuckets[pos].kind != Empty);
  const Bucket &b = m_arBuckets[pos];
  if (b.kind == IntKey) {
    return b.h;
  }
  ASSERT(b.key);
  return b.key;
}

Variant SmallArray::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < SARR_TABLE_SIZE && m_arBuckets[pos].kind != Empty);
  return m_arBuckets[pos].data;
}

void SmallArray::fetchValue(ssize_t pos, Variant &v) const {
  ASSERT(pos >= 0 && pos < SARR_TABLE_SIZE && m_arBuckets[pos].kind != Empty);
  v = m_arBuckets[pos].data;
}

CVarRef SmallArray::getValueRef(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < SARR_TABLE_SIZE && m_arBuckets[pos].kind != Empty);
  return m_arBuckets[pos].data;
}

bool SmallArray::isVectorData() const {
  int64 index = 0;
  for (int i = m_nListHead; i >= 0; ) {
    const Bucket &b = m_arBuckets[i];
    if (b.kind != IntKey || b.h != index++) return false;
    i = b.next;
  }
  return true;
}

Variant SmallArray::reset() {
  m_pos = m_nListHead;
  ASSERT(m_pos == ArrayData::invalid_index ||
         m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
  if (m_pos >= 0 && m_arBuckets[m_pos].kind != Empty) {
    return m_arBuckets[m_pos].data;
  }
  return false;
}

Variant SmallArray::prev() {
  if (m_pos >= 0) {
    ASSERT(m_pos < SARR_TABLE_SIZE && m_arBuckets[m_pos].kind != Empty);
    m_pos = m_arBuckets[m_pos].prev;
    ASSERT(m_pos == ArrayData::invalid_index ||
           m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
    if (m_pos >= 0) {
      ASSERT(m_arBuckets[m_pos].kind != Empty);
      return m_arBuckets[m_pos].data;
    }
  }
  return false;
}

Variant SmallArray::next() {
  if (m_pos >= 0) {
    ASSERT(m_pos < SARR_TABLE_SIZE && m_arBuckets[m_pos].kind != Empty);
    m_pos = m_arBuckets[m_pos].next;
    ASSERT(m_pos == ArrayData::invalid_index ||
           m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
    if (m_pos >= 0) {
      ASSERT(m_arBuckets[m_pos].kind != Empty);
      return m_arBuckets[m_pos].data;
    }
  }
  return false;
}

Variant SmallArray::end() {
  m_pos = m_nListTail;
  ASSERT(m_pos == ArrayData::invalid_index ||
         m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
  if (m_pos >= 0) {
    ASSERT(m_arBuckets[m_pos].kind != Empty);
    return m_arBuckets[m_pos].data;
  }
  return false;
}

Variant SmallArray::key() const {
  if (m_pos >= 0) {
    ASSERT(m_pos < SARR_TABLE_SIZE && m_arBuckets[m_pos].kind != Empty);
    const Bucket &b = m_arBuckets[m_pos];
    if (b.kind == IntKey) {
      return b.h;
    }
    ASSERT(b.key);
    return b.key;
  }
  return null;
}

Variant SmallArray::value(ssize_t &pos) const {
  if (pos >= 0) {
    ASSERT(pos < SARR_TABLE_SIZE && m_arBuckets[pos].kind != Empty);
    return m_arBuckets[pos].data;
  }
  return false;
}

Variant SmallArray::current() const {
  if (m_pos >= 0) {
    ASSERT(m_pos < SARR_TABLE_SIZE && m_arBuckets[m_pos].kind != Empty);
    return m_arBuckets[m_pos].data;
  }
  return false;
}

Variant SmallArray::each() {
  if (m_pos >= 0) {
    ArrayInit init(4, false);
    Variant key(getKey(m_pos));
    Variant value(getValue(m_pos));
    init.set(0, 1LL, value);
    init.set(1, "value", value, -1, true);
    init.set(2, 0LL, key);
    init.set(3, "key", key, -1, true);
    m_pos = m_arBuckets[m_pos].next;
    ASSERT(m_pos == ArrayData::invalid_index ||
           m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
    return Array(init.create());
  }
  return false;
}

void SmallArray::getFullPos(FullPos &pos) {
  ASSERT(m_pos == ArrayData::invalid_index ||
         m_pos >= 0 && m_pos < SARR_TABLE_SIZE &&
         m_arBuckets[m_pos].kind != Empty);
  pos.primary = m_pos;
}

bool SmallArray::setFullPos(const FullPos &pos) {
  // Only set if pos hasn't been invalidated.
  if (pos.primary >= 0) {
    Bucket &b = m_arBuckets[pos.primary];
    if (b.kind != Empty) m_pos = pos.primary;
  }
  return m_pos >= 0;
}

CVarRef SmallArray::currentRef() {
  ASSERT(m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
  return m_arBuckets[m_pos].data;
}

CVarRef SmallArray::endRef() {
  ASSERT(m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
  return m_arBuckets[(int)m_nListTail].data;
}

///////////////////////////////////////////////////////////////////////////////
// lookups (linear probing)

// The find() function can always find a slot, as the load is never 100%.
int SmallArray::find(int64 h) const {
  int start = int_ihash(h);
  if (m_nNumOfElements == 0) return start;
  int ret = ArrayData::invalid_index;
  for (int i = start; i < SARR_TABLE_SIZE; i++) {
    const Bucket &b = m_arBuckets[i];
    if (b.kind == IntKey && b.h == h) return i;
    if (ret < 0 && b.kind == Empty) ret = i;
  }
  for (int i = 0; i < start; i++) {
    const Bucket &b = m_arBuckets[i];
    if (b.kind == IntKey && b.h == h) return i;
    if (ret < 0 && b.kind == Empty) ret = i;
  }
  return ret;
}

// The find() function can always find a slot, as the load is never 100%.
int SmallArray::find(const char *k, int len) const {
  int prehash = str_ohash(k, len);
  int start = str_ihash(prehash);
  if (m_nNumOfElements == 0) return start;
  int ret = ArrayData::invalid_index;
  for (int i = start; i < SARR_TABLE_SIZE; i++) {
    const Bucket &b = m_arBuckets[i];
    if (b.kind == StrKey &&
        (b.key->data() == k ||
         b.h == prehash && b.key->size() == len &&
         memcmp(b.key->data(), k, len) == 0)) {
      return i;
    }
    if (ret < 0 && b.kind == Empty) ret = i;
  }
  for (int i = 0; i < start; i++) {
    const Bucket &b = m_arBuckets[i];
    if (b.kind == StrKey &&
        (b.key->data() == k ||
         b.h == prehash && b.key->size() == len &&
         memcmp(b.key->data(), k, len) == 0)) {
      return i;
    }
    if (ret < 0 && b.kind == Empty) ret = i;
  }
  return ret;
}

bool SmallArray::exists(int64 k, int64 prehash /* = -1 */) const {
  int p = find(k);
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::exists(litstr k, int64 prehash /* = -1 */) const {
  int p = find(k, strlen(k));
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::exists(CStrRef k, int64 prehash /* = -1 */) const {
  int p = find(k.data(), k.size());
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::exists(CVarRef k, int64 prehash /* = -1 */) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size());
  }
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::idxExists(ssize_t idx) const {
  return (idx >= 0 && m_arBuckets[idx].kind != Empty);
}

Variant SmallArray::get(int64 k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  int p = find(k);
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %lld", k);
  }
  return null;
}

Variant SmallArray::get(litstr k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  int p = find(k, strlen(k));
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %s", k);
  }
  return null;
}

Variant SmallArray::get(CStrRef k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  int p = find(k.data(), k.size());
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %s", k.data());
  }
  return null;
}

Variant SmallArray::get(CVarRef k, int64 prehash /* = -1 */,
                        bool error /* = false */) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size());
  }
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null;
}

ssize_t SmallArray::getIndex(int64 k, int64 prehash /* = -1 */) const {
  int p = find(k);
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

ssize_t SmallArray::getIndex(litstr k, int64 prehash /* = -1 */) const {
  int p = find(k, strlen(k));
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

ssize_t SmallArray::getIndex(CStrRef k, int64 prehash /* = -1 */) const {
  int p = find(k.data(), k.size());
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

ssize_t SmallArray::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  }
  String key = k.toString();
  p = find(key.data(), key.size());
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

///////////////////////////////////////////////////////////////////////////////
// append/insert/update

ArrayData *SmallArray::escalate() {
  // Assume UseZendArray for now
  ASSERT(RuntimeOption::UseZendArray);
  ZendArray *ret = NEW(ZendArray)(m_nNumOfElements);
  for (int p = m_nListHead; p >= 0; p = m_arBuckets[p].next) {
    Bucket &b = m_arBuckets[p];
    ASSERT(b.kind != Empty);
    if (b.data.isReferenced()) b.data.setContagious();
    if (b.kind == IntKey) {
      ret->set(b.h, b.data, false);
    } else {
      ASSERT(b.key);
      ret->set(String(b.key), b.data, false, -1);
    }
  }
  // Set m_pos in the escalated array
  if (m_pos != ArrayData::invalid_index) {
    Bucket &b = m_arBuckets[m_pos];
    if (b.kind == IntKey) {
      ret->setPosition(ret->getIndex(b.h));
    } else {
      ASSERT(b.key);
      ret->setPosition(ret->getIndex(String(b.key)));
    }
  }
  return ret;
}

SmallArray::Bucket *SmallArray::addKey(int p, int64 h) {
  ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind == Empty &&
         m_nNumOfElements < SARR_SIZE);
  Bucket &b = m_arBuckets[p];
  b.kind = IntKey;
  b.h = h;

  connect_to_global_dllist(p, b);
  m_nNumOfElements++;

  if ((long)h >= (long)m_nNextFreeElement) {
    m_nNextFreeElement = h + 1;
  }
  return &b;
}

SmallArray::Bucket *SmallArray::addKey(int p, litstr key, int len) {
  ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind == Empty &&
         m_nNumOfElements < SARR_SIZE);
  Bucket &b = m_arBuckets[p];
  b.kind = StrKey;
  b.h = str_ohash(key, len);
  b.key = NEW(StringData)(key, len, AttachLiteral);
  b.key->incRefCount();

  connect_to_global_dllist(p, b);
  m_nNumOfElements++;
  return &b;
}

SmallArray::Bucket *SmallArray::addKey(int p, StringData *key) {
  ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind == Empty &&
         m_nNumOfElements < SARR_SIZE);
  const char *k = key->data();
  int len = key->size();
  Bucket &b = m_arBuckets[p];
  b.kind = StrKey;
  b.h = str_ohash(k, len);
  if (key->isShared()) {
    b.key = NEW(StringData)(k, len, CopyString);
  } else {
    b.key = key;
  }
  b.key->incRefCount();

  connect_to_global_dllist(p, b);
  m_nNumOfElements++;
  return &b;
}

void SmallArray::renumber() {
  unsigned long i = 0;
  for (int p = m_nListHead; p >= 0; ) {
    Bucket &b = m_arBuckets[p];
    p = b.next;
    if (b.kind == IntKey) {
      b.h = i++;
    }
  }
  m_nNextFreeElement = i;
}

ArrayData *SmallArray::lval(Variant *&ret, bool copy) {
  if (copy) {
    SmallArray *a = copyImpl();
    int p = a->m_nListTail;
    ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind != Empty);
    ret = &a->m_arBuckets[p].data;
    return a;
  }
  int p = m_nListTail;
  ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind != Empty);
  ret = &m_arBuckets[p].data;
  return NULL;
}

ArrayData *SmallArray::lval(int64 k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalate();
      a->lval(k, ret, false, prehash);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, k);
    } else {
      addKey(p, k);
    }
    ret = &pb->data;
    return result;
  }

  if (copy && !checkExist) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lval(litstr k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  int len = strlen(k);
  int p = find(k, len);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalate();
      a->lval(k, ret, false, prehash);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, k, len);
    } else {
      addKey(p, k, len);
    }
    ret = &pb->data;
    return result;
  }

  if (copy && !checkExist) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lval(CStrRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  StringData *key = k.get();
  int p = find(key->data(), key->size());
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalate();
      a->lval(k, ret, false, prehash);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, key);
    } else {
      addKey(p, key);
    }
    ret = &pb->data;
    return result;
  }

  if (copy && !checkExist) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lval(CVarRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */,
                            bool checkExist /* = false */) {
  if (k.isNumeric()) {
    return lval(k.toInt64(), ret, copy, prehash, checkExist);
  } else if (k.is(LiteralString)) {
    return lval(k.getLiteralString(), ret, copy, prehash, checkExist);
  } else {
    return lval(k.toString(), ret, copy, prehash, checkExist);
  }
}

ArrayData *SmallArray::set(int64 k, CVarRef v, bool copy,
                           int64 prehash /* = -1 */) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalate();
      a->set(k, v, false, prehash);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, k);
    } else {
      addKey(p, k);
    }
    pb->data = v;
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data = v;
  return result;
}

ArrayData *SmallArray::set(litstr k, CVarRef v, bool copy,
                           int64 prehash /* = -1 */) {
  int len = strlen(k);
  int p = find(k, len);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalate();
      a->set(k, v, false, prehash);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, k, len);
    } else {
      addKey(p, k, len);
    }
    pb->data = v;
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data = v;
  return result;
}

ArrayData *SmallArray::set(CStrRef k, CVarRef v, bool copy,
                           int64 prehash /* = -1 */) {
  StringData *key = k.get();
  int p = find(key->data(), key->size());
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalate();
      a->set(k, v, false, prehash);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, key);
    } else {
      addKey(p, key);
    }
    pb->data = v;
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data = v;
  return result;
}

ArrayData *SmallArray::set(CVarRef k, CVarRef v, bool copy,
                           int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    return set(k.toInt64(), v, copy, prehash);
  }
  if (k.is(LiteralString)) {
    return set(k.getLiteralString(), v, copy, prehash);
  }
  return set(k.toString(), v, copy, prehash);
}

ArrayData *SmallArray::copy() const {
  return copyImpl();
}

void SmallArray::nextInsert(CVarRef v) {
  int64 h = m_nNextFreeElement;
  int p = find(h);
  Bucket *pb = addKey(p, h);
  pb->data = v;
  m_nNextFreeElement = h + 1;
}

ArrayData *SmallArray::append(CVarRef v, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalate();
    a->append(v, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    a->nextInsert(v);
    return a;
  }
  nextInsert(v);
  return NULL;
}

bool SmallArray::add(int64 h, CVarRef data) {
  int p = find(h);
  Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) return false;
  addKey(p, h);
  b.data = data;
  return true;
}

bool SmallArray::add(litstr key, int64 h, CVarRef data) {
  int len = strlen(key);
  int p = find(key, len);
  Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) return false;
  addKey(p, key, len);
  b.data = data;
  return true;
}

bool SmallArray::add(StringData *key, int64 h, CVarRef data) {
  int p = find(key->data(), key->size());
  Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) return false;
  addKey(p, key);
  b.data = data;
  return true;
}

ArrayData *SmallArray::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ssize_t elems_size = elems->size();
  if (elems_size == 0) return NULL;
  if (m_nNumOfElements + elems_size >= SARR_SIZE) {
    ArrayData *a = escalate();
    a->append(elems, op, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
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
          add(key.toInt64(), value);
        } else {
          String strkey = key.toString();
          add(strkey.get(), -1, value);
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        CVarRef value = it.secondRef();
        if (value.isReferenced()) value.setContagious();
        if (key.isNumeric()) {
          nextInsert(value);
        } else {
          String strkey = key.toString();
          set(strkey, value, false);
        }
      }
    }
  } else {
    if (op == Plus) {
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        if (key.isNumeric()) {
          add(key.toInt64(), it.second());
        } else {
          String strkey = key.toString();
          add(strkey.get(), -1, it.second());
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        if (key.isNumeric()) {
          nextInsert(it.second());
        } else {
          String strkey = key.toString();
          set(strkey, it.second(), false);
        }
      }
    }
  }
  return NULL;
}

ArrayData *SmallArray::prepend(CVarRef v, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalate();
    a->prepend(v, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    a->prepend(v, false);
    return a;
  }

  int p = 0;
  while (m_arBuckets[p].kind != Empty) p++;
  Bucket &b = m_arBuckets[p];
  b.kind = IntKey;
  b.data = v;
  b.prev = ArrayData::invalid_index;
  b.next = m_nListHead;
  if (m_nListHead >= 0) m_arBuckets[(int)m_nListHead].prev = p;
  m_nListHead = p;
  if (m_nNumOfElements == 0) {
    b.h = 0;
    m_nListTail = p;
    m_pos = p;
    m_nNumOfElements = 1;
    m_nNextFreeElement = 1;
  } else {
    m_nNumOfElements++;
    // Rewrite numeric keys to start from 0.
    renumber();
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// delete

void SmallArray::erase(Bucket *pb) {
  if (pb->key) {
    if (pb->key->decRefCount() == 0) DELETE(StringData)(pb->key);
    pb->key = NULL;
  }
  pb->kind = Empty;
  pb->data.unset();
  if (pb->prev >= 0) {
    m_arBuckets[(int)pb->prev].next = pb->next;
  } else {
    ASSERT(m_nListHead >= 0 && m_nListHead < SARR_TABLE_SIZE &&
           m_arBuckets + m_nListHead == pb);
    m_nListHead = pb->next;
  }
  if (pb->next >= 0) {
    m_arBuckets[(int)pb->next].prev = pb->prev;
  } else {
    ASSERT(m_nListTail >= 0 && m_nListTail < SARR_TABLE_SIZE &&
           m_arBuckets + m_nListTail == pb);
    m_nListTail = pb->prev;
  }
  if (m_arBuckets + m_pos == pb) {
    m_pos = pb->next;
    ASSERT(m_pos == ArrayData::invalid_index ||
           m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
  }
  m_nNumOfElements--;
}

ArrayData *SmallArray::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;
  if (pb->kind == Empty) return NULL;
  if (copy) {
    SmallArray *a = copyImpl();
    pb = a->m_arBuckets + p;
    a->erase(pb);
    return a;
  }
  erase(pb);
  return NULL;
}

ArrayData *SmallArray::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  int p = find(k, strlen(k));
  Bucket *pb = m_arBuckets + p;
  if (pb->kind == Empty) return NULL;
  if (copy) {
    SmallArray *a = copyImpl();
    pb = a->m_arBuckets + p;
    a->erase(pb);
    return a;
  }
  erase(pb);
  return NULL;
}

ArrayData *SmallArray::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  StringData *key = k.get();
  int p = find(key->data(), key->size());
  Bucket *pb = m_arBuckets + p;
  if (pb->kind == Empty) return NULL;
  if (copy) {
    SmallArray *a = copyImpl();
    pb = a->m_arBuckets + p;
    a->erase(pb);
    return a;
  }
  erase(pb);
  return NULL;
}

ArrayData *SmallArray::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  if (k.isNumeric()) {
    return remove(k.toInt64(), copy, prehash);
  }
  if (k.is(LiteralString)) {
    return remove(k.getLiteralString(), copy, prehash);
  }
  return remove(k.toString(), copy, prehash);
}

ArrayData *SmallArray::pop(Variant &value) {
  if (m_nNumOfElements == 0) {
    value = null;
    return NULL;
  }
  if (getCount() > 1) {
    SmallArray *a = copyImpl();
    a->pop(value);
    return a;
  }

  ASSERT(m_nListTail >= 0 && m_nListTail < SARR_TABLE_SIZE &&
         m_arBuckets[(int)m_nListTail].kind != Empty);
  Bucket &b = m_arBuckets[(int)m_nListTail];
  value = b.data;
  if (b.kind == IntKey && (uint)b.h == m_nNextFreeElement - 1) {
    m_nNextFreeElement--;
  }
  erase(&b);
  return NULL;
}

ArrayData *SmallArray::dequeue(Variant &value) {
  if (m_nNumOfElements == 0) {
    value = null;
    return NULL;
  }
  if (getCount() > 1) {
    SmallArray *a = copyImpl();
    a->dequeue(value);
    return a;
  }

  ASSERT(m_nListHead >= 0 && m_nListHead < SARR_TABLE_SIZE &&
         m_arBuckets[(int)m_nListHead].kind != Empty);
  Bucket &b = m_arBuckets[(int)m_nListHead];
  value = b.data;
  erase(&b);
  renumber();
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// misc

void SmallArray::onSetStatic() {
  for (int p = m_nListHead; p >= 0; ) {
    Bucket &b = m_arBuckets[p];
    if (b.key) b.key->setStatic();
    b.data.setStatic();
    p = b.next;
  }
}

///////////////////////////////////////////////////////////////////////////////
// the empty array

StaticEmptySmallArray StaticEmptySmallArray::s_theEmptyArray;

///////////////////////////////////////////////////////////////////////////////
}
