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

#include <runtime/base/array/small_array.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/runtime_option.h>
#include <util/hash.h>

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

SmallArray::SmallArray(unsigned int nSize, int64 n,
                       StringData *keys[],
                       const Variant *values[]) :
  m_nNumOfElements(nSize),
  m_nListHead(ArrayData::invalid_index),
  m_nListTail(ArrayData::invalid_index),
  m_nNextFreeElement(n) {
  const Variant **v = values;
  for (StringData **k = keys; *k; k++, v++) {
    ASSERT((*k)->isStatic());
    int64 h = (*k)->getPrecomputedHash();
    Bucket *pb = m_arBuckets + findEmpty(h);
    pb->h = h;
    pb->key = *k;
    pb->kind = StrKey;
    pb->data = **v;
    connect_to_global_dllist(pb - m_arBuckets, *pb);
  }
}

SmallArray::SmallArray(unsigned int nSize, int64 n,
                       int64 keys[], const Variant *values[]) :
  m_nNumOfElements(nSize),
  m_nListHead(ArrayData::invalid_index),
  m_nListTail(ArrayData::invalid_index),
  m_nNextFreeElement(n) {
  int64 *k = keys;
  for (const Variant **v = values; *v; v++, k++) {
    int64 h = *k;
    Bucket *pb = m_arBuckets + findEmpty(h);
    pb->h = h;
    pb->kind = IntKey;
    pb->data = **v;
    connect_to_global_dllist(pb - m_arBuckets, *pb);
  }
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

StaticString s_value("value");
StaticString s_key("key");

Variant SmallArray::each() {
  if (m_pos >= 0) {
    ArrayInit init(4, false);
    Variant key(getKey(m_pos));
    Variant value(getValue(m_pos));
    init.set(1LL, value);
    init.set(s_value, value, true);
    init.set(0LL, key);
    init.set(s_key, key, true);
    m_pos = m_arBuckets[m_pos].next;
    ASSERT(m_pos == ArrayData::invalid_index ||
           m_pos >= 0 && m_pos < SARR_TABLE_SIZE);
    return Array(init.create());
  }
  return false;
}

void SmallArray::getFullPos(FullPos &fp) {
  ASSERT(fp.container == (ArrayData *)this);
  ASSERT(m_pos == ArrayData::invalid_index ||
         m_pos >= 0 && m_pos < SARR_TABLE_SIZE &&
         m_arBuckets[m_pos].kind != Empty);
  fp.pos = m_pos;
  if (fp.pos == ArrayData::invalid_index) {
    // Record that there is a strong iterator out there
    // that is past the end
    m_siPastEnd = 1;
  }
}

bool SmallArray::setFullPos(const FullPos &fp) {
  ASSERT(fp.container == (ArrayData *) this);
  if (fp.pos >= 0) {
    Bucket &b = m_arBuckets[fp.pos];
    if (b.kind != Empty) m_pos = fp.pos;
  }
  return m_pos >= 0;
}

void SmallArray::updateStrongIterators(int p) {
  ASSERT(m_siPastEnd);
  m_siPastEnd = 0;
  int sz = m_strongIterators.size();
  bool shouldWarn = false;
  for (int i = 0; i < sz; i++) {
    if (m_strongIterators.get(i)->pos == ArrayData::invalid_index) {
      m_strongIterators.get(i)->pos = p;
      shouldWarn = true;
    }
  }
  if (shouldWarn) {
    raise_warning("An element was added to an array inside foreach "
                  "by reference when iterating over the last "
                  "element. This may lead to unexpeced results.");
  }
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
  int start = h & (SARR_TABLE_SIZE - 1);
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
int SmallArray::find(const char *k, int len, int64 prehash) const {
  int start = prehash & (SARR_TABLE_SIZE - 1);
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

int SmallArray::findEmpty(int64 h) const {
  int start = h & (SARR_TABLE_SIZE - 1);
  for (int i = start; i < SARR_TABLE_SIZE; i++) {
    if (m_arBuckets[i].kind == Empty) return i;
  }
  for (int i = 0; i < start; i++) {
    if (m_arBuckets[i].kind == Empty) return i;
  }
  ASSERT(false);
  return -1;
}

bool SmallArray::exists(int64 k) const {
  int p = find(k);
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::exists(litstr k) const {
  int len = strlen(k);
  int64 hash = hash_string(k, len);
  int p = find(k, len, hash);
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::exists(CStrRef k) const {
  int p = find(k.data(), k.size(), k->hash());
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::exists(CVarRef k) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size(), key->hash());
  }
  return m_arBuckets[p].kind != Empty;
}

bool SmallArray::idxExists(ssize_t idx) const {
  return (idx >= 0 && m_arBuckets[idx].kind != Empty);
}

CVarRef SmallArray::get(int64 k, bool error /* = false */) const {
  int p = find(k);
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %lld", k);
  }
  return null_variant;
}

CVarRef SmallArray::get(litstr k, bool error /* = false */) const {
  int len = strlen(k);
  int64 hash = hash_string(k, len);
  int p = find(k, len, hash);
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %s", k);
  }
  return null_variant;
}

CVarRef SmallArray::get(CStrRef k, bool error /* = false */) const {
  int p = find(k.data(), k.size(), k->hash());
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %s", k.data());
  }
  return null_variant;
}

CVarRef SmallArray::get(CVarRef k, bool error /* = false */) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size(), key->hash());
  }
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    return b.data;
  }
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null_variant;
}

void SmallArray::load(CVarRef k, Variant &v) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  } else {
    String key = k.toString();
    p = find(key.data(), key.size(), key->hash());
  }
  const Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) {
    v.setWithRef(b.data);
  }
}

ssize_t SmallArray::getIndex(int64 k) const {
  int p = find(k);
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

ssize_t SmallArray::getIndex(litstr k) const {
  int len = strlen(k);
  int64 hash = hash_string(k, len);
  int p = find(k, len, hash);
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

ssize_t SmallArray::getIndex(CStrRef k) const {
  int p = find(k.data(), k.size(), k->hash());
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

ssize_t SmallArray::getIndex(CVarRef k) const {
  int p;
  if (k.isNumeric()) {
    p = find(k.toInt64());
  }
  String key = k.toString();
  p = find(key.data(), key.size(), key->hash());
  if (m_arBuckets[p].kind != Empty) return p;
  return ArrayData::invalid_index;
}

///////////////////////////////////////////////////////////////////////////////
// append/insert/update

ArrayData *SmallArray::escalate(bool mutableIteration /* = false */) const {
  // SmallArray doesn't need to be escalated for most of the time.
  return const_cast<SmallArray *>(this);
}

ArrayData *SmallArray::escalateToZendArray() const {
  ZendArray *ret = NEW(ZendArray)(m_nNumOfElements);
  for (int p = m_nListHead; p >= 0; p = m_arBuckets[p].next) {
    const Bucket &b = m_arBuckets[p];
    ASSERT(b.kind != Empty);
    Variant *v;
    if (b.kind == IntKey) {
      ret->addLval(b.h, v, false);
    } else {
      ASSERT(b.key);
      ret->addLval(String(b.key), v, false);
    }
    v->setWithRef(b.data);
  }
  // Set m_pos in the escalated array
  if (m_pos != ArrayData::invalid_index) {
    const Bucket &b = m_arBuckets[m_pos];
    if (b.kind == IntKey) {
      ret->setPosition(ret->getIndex(b.h));
    } else {
      ASSERT(b.key);
      ret->setPosition(ret->getIndex(String(b.key)));
    }
  } else {
    ret->setPosition(0);
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

  if (h >= m_nNextFreeElement && m_nNextFreeElement >= 0) {
    m_nNextFreeElement = h + 1;
  }
  return &b;
}

SmallArray::Bucket *SmallArray::addKey(int p, StringData *key) {
  ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind == Empty &&
         m_nNumOfElements < SARR_SIZE);
  Bucket &b = m_arBuckets[p];
  b.kind = StrKey;
  b.h = key->hash();
  b.key = key;
  b.key->incRefCount();

  connect_to_global_dllist(p, b);
  m_nNumOfElements++;
  return &b;
}

void SmallArray::renumber() {
  int64 i = 0;
  for (int p = m_nListHead; p >= 0; ) {
    Bucket &b = m_arBuckets[p];
    p = b.next;
    if (b.kind == IntKey) {
      b.h = i++;
    }
  }
  m_nNextFreeElement = i;
}

ArrayData *SmallArray::lvalNew(Variant *&ret, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData* a = escalateToZendArray();
    a->lvalNew(ret, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    if (!a->nextInsert(null)) {
      ret = &(Variant::lvalBlackHole());
      return a;
    }
    int p = a->m_nListTail;
    ASSERT(p >= 0 && p < SARR_TABLE_SIZE && a->m_arBuckets[p].kind != Empty);
    ret = &a->m_arBuckets[p].data;
    return a;
  }
  if (!nextInsert(null)) {
    ret = &(Variant::lvalBlackHole());
    return NULL;
  }
  int p = m_nListTail;
  ASSERT(p >= 0 && p < SARR_TABLE_SIZE && m_arBuckets[p].kind != Empty);
  ret = &m_arBuckets[p].data;
  return NULL;
}

ArrayData *SmallArray::lval(int64 k, Variant *&ret, bool copy,
                            bool checkExist /* = false */) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->lval(k, ret, false);
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

  if (copy && (!checkExist ||
               (!pb->data.isReferenced() && !pb->data.isObject()))) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lval(litstr k, Variant *&ret, bool copy,
                            bool checkExist /* = false */) {
  String key(k, AttachLiteral);
  int64 prehash = key->hash();
  int p = find(k, key.size(), prehash);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->lval(key, ret, false);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, key.get());
    } else {
      addKey(p, key.get());
    }
    ret = &pb->data;
    return result;
  }

  if (copy && (!checkExist ||
               (!pb->data.isReferenced() && !pb->data.isObject()))) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lval(CStrRef k, Variant *&ret, bool copy,
                            bool checkExist /* = false */) {
  StringData *key = k.get();
  int64 prehash = key->hash();
  int p = find(key->data(), key->size(), prehash);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->lval(k, ret, false);
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

  if (copy && (!checkExist ||
               (!pb->data.isReferenced() && !pb->data.isObject()))) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lvalPtr(CStrRef k, Variant *&ret, bool copy,
                               bool create) {
  StringData *key = k.get();
  int64 prehash = key->hash();
  int p = find(key->data(), key->size(), prehash);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (create && m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->lvalPtr(k, ret, false, create);
      return a;
    }
    ret = NULL;
    if (copy) {
      result = copyImpl();
      if (create) {
        pb = result->addKey(p, key);
        ret = &pb->data;
      }
    } else if (create) {
      addKey(p, key);
      ret = &pb->data;
    }
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lvalPtr(int64 k, Variant *&ret, bool copy,
                               bool create) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (create && m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->lvalPtr(k, ret, false, create);
      return a;
    }
    ret = NULL;
    if (copy) {
      result = copyImpl();
      if (create) {
        pb = result->addKey(p, k);
        ret = &pb->data;
      }
    } else if (create) {
      addKey(p, k);
      ret = &pb->data;
    }
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  ret = &pb->data;
  return result;
}

ArrayData *SmallArray::lval(CVarRef k, Variant *&ret, bool copy,
                            bool checkExist /* = false */) {
  if (k.isNumeric()) {
    return lval(k.toInt64(), ret, copy, checkExist);
  } else {
    return lval(k.toString(), ret, copy, checkExist);
  }
}

ArrayData *SmallArray::set(int64 k, CVarRef v, bool copy) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->set(k, v, false);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, k);
    } else {
      addKey(p, k);
    }
    pb->data.assignVal(v);
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data.assignVal(v);
  return result;
}

ArrayData *SmallArray::set(CStrRef k, CVarRef v, bool copy) {
  StringData *key = k.get();
  int p = find(key->data(), key->size(), key->hash());
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->set(k, v, false);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, key);
    } else {
      addKey(p, key);
    }
    pb->data.assignVal(v);
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data.assignVal(v);
  return result;
}

ArrayData *SmallArray::set(CVarRef k, CVarRef v, bool copy) {
  if (k.isNumeric()) {
    return set(k.toInt64(), v, copy);
  }
  return set(k.toString(), v, copy);
}

ArrayData *SmallArray::setRef(int64 k, CVarRef v, bool copy) {
  int p = find(k);
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->setRef(k, v, false);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, k);
    } else {
      addKey(p, k);
    }
    pb->data.assignRef(v);
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data.assignRef(v);
  return result;
}

ArrayData *SmallArray::setRef(CStrRef k, CVarRef v, bool copy) {
  StringData *key = k.get();
  int p = find(key->data(), key->size(), key->hash());
  Bucket *pb = m_arBuckets + p;

  SmallArray *result = NULL;
  if (pb->kind == Empty) {
    if (m_nNumOfElements >= SARR_SIZE) {
      ArrayData *a = escalateToZendArray();
      a->setRef(k, v, false);
      return a;
    }
    if (copy) {
      result = copyImpl();
      pb = result->addKey(p, key);
    } else {
      addKey(p, key);
    }
    pb->data.assignRef(v);
    return result;
  }

  if (copy) {
    result = copyImpl();
    pb = result->m_arBuckets + p;
  }
  pb->data.assignRef(v);
  return result;
}

ArrayData *SmallArray::setRef(CVarRef k, CVarRef v, bool copy) {
  if (k.isNumeric()) {
    return setRef(k.toInt64(), v, copy);
  }
  return setRef(k.toString(), v, copy);
}

ArrayData *SmallArray::add(int64 k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->add(k, v, false);
    return a;
  }
  if (copy) {
    SmallArray *result = copyImpl();
    Bucket *pb = result->addKey(findEmpty(k), k);
    pb->data = v;
    return result;
  }
  Bucket *pb = addKey(findEmpty(k), k);
  pb->data = v;
  return NULL;
}

ArrayData *SmallArray::add(CStrRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->add(k, v, false);
    return a;
  }
  if (copy) {
    SmallArray *result = copyImpl();
    Bucket *pb = result->addKey(findEmpty(k->hash()), k.get());
    pb->data = v;
    return result;
  }
  Bucket *pb = addKey(findEmpty(k->hash()), k.get());
  pb->data = v;
  return NULL;
}

ArrayData *SmallArray::add(CVarRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->add(k, v, false);
    return a;
  }
  if (k.isNumeric()) return add(k.toInt64(), v, copy);
  return add(k.toString(), v, copy);
}

ArrayData *SmallArray::addLval(int64 k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->addLval(k, ret, false);
    return a;
  }
  if (copy) {
    SmallArray *result = copyImpl();
    Bucket *pb = result->addKey(findEmpty(k), k);
    ret = &pb->data;
    return result;
  }
  Bucket *pb = addKey(findEmpty(k), k);
  ret = &pb->data;
  return NULL;
}

ArrayData *SmallArray::addLval(CStrRef k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->addLval(k, ret, false);
    return a;
  }
  if (copy) {
    SmallArray *result = copyImpl();
    Bucket *pb = result->addKey(findEmpty(k->hash()), k.get());
    ret = &pb->data;
    return result;
  }
  Bucket *pb = addKey(findEmpty(k->hash()), k.get());
  ret = &pb->data;
  return NULL;
}

ArrayData *SmallArray::addLval(CVarRef k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->addLval(k, ret, false);
    return a;
  }
  if (k.isNumeric()) return addLval(k.toInt64(), ret, copy);
  return addLval(k.toString(), ret, copy);
}

ArrayData *SmallArray::copy() const {
  return copyImpl();
}

bool SmallArray::nextInsert(CVarRef v) {
  if (m_nNextFreeElement < 0) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  int64 h = m_nNextFreeElement;
  int p = find(h);
  Bucket *pb = addKey(p, h);
  pb->data.assignVal(v);
  m_nNextFreeElement = h + 1;
  return true;
}

bool SmallArray::nextInsertRef(CVarRef v) {
  if (m_nNextFreeElement < 0) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  int64 h = m_nNextFreeElement;
  int p = find(h);
  Bucket *pb = addKey(p, h);
  pb->data.assignRef(v);
  m_nNextFreeElement = h + 1;
  return true;
}

void SmallArray::nextInsertWithRef(CVarRef v) {
  int64 h = m_nNextFreeElement;
  int p = find(h);
  Bucket *pb = addKey(p, h);
  pb->data.setWithRef(v);
  m_nNextFreeElement = h + 1;
}

ArrayData *SmallArray::append(CVarRef v, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
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

ArrayData *SmallArray::appendRef(CVarRef v, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->appendRef(v, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    a->nextInsertRef(v);
    return a;
  }
  nextInsertRef(v);
  return NULL;
}

ArrayData *SmallArray::appendWithRef(CVarRef v, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->appendWithRef(v, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    a->nextInsertWithRef(v);
    return a;
  }
  nextInsertWithRef(v);
  return NULL;
}

bool SmallArray::addValWithRef(int64 h, CVarRef data) {
  int p = find(h);
  Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) return false;
  addKey(p, h);
  b.data.setWithRef(data);
  return true;
}

bool SmallArray::addValWithRef(StringData *key, CVarRef data) {
  int p = find(key->data(), key->size(), key->hash());
  Bucket &b = m_arBuckets[p];
  if (b.kind != Empty) return false;
  addKey(p, key);
  b.data.setWithRef(data);
  return true;
}

ArrayData *SmallArray::append(const ArrayData *elems, ArrayOp op, bool copy) {
  ssize_t elems_size = elems->size();
  if (elems_size == 0) return NULL;
  if (m_nNumOfElements + elems_size >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->append(elems, op, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    a->append(elems, op, false);
    return a;
  }

  if (op == Plus) {
    for (ArrayIter it(elems); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      if (key.isNumeric()) {
        addValWithRef(key.toInt64(), value);
      } else {
        addValWithRef(key.getStringData(), value);
      }
    }
  } else {
    ASSERT(op == Merge);
    for (ArrayIter it(elems); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      if (key.isNumeric()) {
        nextInsertWithRef(value);
      } else {
        Variant *p;
        String strkey = key.toString();
        lval(strkey, p, false, false);
        p->setWithRef(value);
      }
    }
  }
  return NULL;
}

ArrayData *SmallArray::prepend(CVarRef v, bool copy) {
  if (m_nNumOfElements >= SARR_SIZE) {
    ArrayData *a = escalateToZendArray();
    a->prepend(v, false);
    return a;
  }
  if (copy) {
    SmallArray *a = copyImpl();
    a->prepend(v, false);
    return a;
  }

  // To match PHP-like semantics, we invalidate all strong iterators
  // when an element is added to the beginning of the array
  if (!m_strongIterators.empty()) {
    freeStrongIterators();
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
    m_nNumOfElements = 1;
    m_nNextFreeElement = 1;
  } else {
    m_nNumOfElements++;
    // Rewrite numeric keys to start from 0.
    renumber();
  }
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator
  m_pos = (ssize_t)m_nListHead;
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// delete

void SmallArray::erase(Bucket *pb, bool updateNext /* = false */) {
  if (pb->key) {
    if (pb->key->decRefCount() == 0) DELETE(StringData)(pb->key);
    pb->key = NULL;
  } else {
    // Match PHP 5.3.1 semantics
    if (pb->h == m_nNextFreeElement-1 &&
        (pb->h == 0x7fffffffffffffffLL || updateNext)) {
      --m_nNextFreeElement;
    }
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

  bool nextElementUnsetInsideForeachByReference = false;
  int p = pb - m_arBuckets;
  int sz = m_strongIterators.size();
  for (int i = 0; i < sz; ++i) {
    if (m_strongIterators.get(i)->pos == p) {
      nextElementUnsetInsideForeachByReference = true;
      m_strongIterators.get(i)->pos = pb->next;
      if (m_strongIterators.get(i)->pos == ArrayData::invalid_index) {
        m_siPastEnd = 1;
      }
    }
  }
  if (nextElementUnsetInsideForeachByReference) {
    if (RuntimeOption::EnableHipHopErrors) {
      raise_warning("The next element was unset inside foreach by reference. "
                    "This may lead to unexpeced results.");
    }
  }
}

ArrayData *SmallArray::remove(int64 k, bool copy) {
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

ArrayData *SmallArray::remove(CStrRef k, bool copy) {
  StringData *key = k.get();
  int p = find(key->data(), key->size(), key->hash());
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

ArrayData *SmallArray::remove(CVarRef k, bool copy) {
  if (k.isNumeric()) {
    return remove(k.toInt64(), copy);
  }
  return remove(k.toString(), copy);
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
  erase(&b, true);
  // To match PHP-like semantics, the pop operation resets the array's
  // internal iterator
  m_pos = (ssize_t)m_nListHead;
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

  // To match PHP-like semantics, we invalidate all strong iterators
  // when an element is removed from the beginning of the array
  if (!m_strongIterators.empty()) {
    freeStrongIterators();
  }

  ASSERT(m_nListHead >= 0 && m_nListHead < SARR_TABLE_SIZE &&
         m_arBuckets[(int)m_nListHead].kind != Empty);
  Bucket &b = m_arBuckets[(int)m_nListHead];
  value = b.data;
  erase(&b);
  renumber();
  // To match PHP-like semantics, the dequeue operation resets the array's
  // internal iterator
  m_pos = (ssize_t)m_nListHead;
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
