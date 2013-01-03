/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/base/variable_serializer.h>
#include <runtime/base/array/sort_helpers.h>
#include <runtime/ext/ext_collection.h>
#include <runtime/ext/ext_array.h>
#include <runtime/ext/ext_math.h>
#include <runtime/ext/ext_intl.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

c_Vector::c_Vector(const ObjectStaticCallbacks *cb) :
    ExtObjectDataFlags<ObjectData::VectorAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_data(NULL), m_size(0), m_capacity(0), m_versionNumber(0) {
}

c_Vector::~c_Vector() {
  int sz = m_size;
  for (int i = 0; i < sz; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  c_Vector::freeData();
}

void c_Vector::freeData() {
  if (m_data) {
    smart_free(m_data);
    m_data = NULL;
  }
}

void c_Vector::t___construct() {
}

Variant c_Vector::t___destruct() {
  return null;
}

void c_Vector::grow() {
  if (m_capacity) {
    m_capacity += m_capacity;
  } else {
    m_capacity = 8;
  }
  m_data = (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
}

void c_Vector::resize(int64 sz, TypedValue* val) {
  ASSERT(val && val->m_type != KindOfRef);
  ++m_versionNumber;
  ASSERT(sz >= 0);
  if (m_capacity < sz) {
    m_capacity = sz;
    m_data =
      (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
  }
  for (int64 i = m_size-1; i >= sz; --i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  for (int64 i = m_size; i < sz; ++i) {
    tvDup(val, &m_data[i]);
  }
  m_size = sz;
}

void c_Vector::reserve(int64 sz) {
  ++m_versionNumber;
  if (sz <= 0) return;
  if (m_capacity < sz) {
    m_capacity = sz;
    m_data =
      (TypedValue*)smart_realloc(m_data, m_capacity * sizeof(TypedValue));
  }
}

Array c_Vector::toArrayImpl() const {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  int sz = m_size;
  for (int i = 0; i < sz; ++i) {
    ai.set(tvAsCVarRef(&m_data[i]));
  }
  return ai.create();
}

Array c_Vector::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_Vector::clone() {
  ObjectData* obj = ObjectData::clone();
  c_Vector* vec = static_cast<c_Vector*>(obj);
  int sz = m_size;
  TypedValue* data;
  vec->m_capacity = vec->m_size = sz;
  vec->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  for (int i = 0; i < sz; ++i) {
    tvDup(&m_data[i], &data[i]);
  }
  return obj;
}

Object c_Vector::t_add(CVarRef val) {
  TypedValue* tv = (TypedValue*)(&val);
  if (UNLIKELY(tv->m_type == KindOfRef)) {
    tv = tv->m_data.pref->tv();
  }
  add(tv);
  return this;
}

Object c_Vector::t_append(CVarRef val) {
  TypedValue* tv = (TypedValue*)(&val);
  if (UNLIKELY(tv->m_type == KindOfRef)) {
    tv = tv->m_data.pref->tv();
  }
  add(tv);
  return this;
}

Variant c_Vector::t_pop() {
  ++m_versionNumber;
  if (m_size) {
    --m_size;
    Variant ret = tvAsCVarRef(&m_data[m_size]);
    tvRefcountedDecRef(&m_data[m_size]);
    return ret;
  } else {
    Object e(SystemLib::AllocRuntimeExceptionObject(
      "Cannot pop empty Vector"));
    throw e;
  }
}

void c_Vector::t_resize(CVarRef sz, CVarRef value) {
  if (!sz.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter sz must be a non-negative integer"));
    throw e;
  }
  int64 intSz = sz.toInt64();
  if (intSz < 0) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter sz must be a non-negative integer"));
    throw e;
  }
  TypedValue* val = (TypedValue*)(&value);
  if (UNLIKELY(val->m_type == KindOfRef)) {
    val = val->m_data.pref->tv();
  }
  resize(intSz, val);
}

Object c_Vector::t_clear() {
  ++m_versionNumber;
  int sz = m_size;
  for (int i = 0; i < sz; ++i) {
    tvRefcountedDecRef(&m_data[i]);
  }
  smart_free(m_data);
  m_data = NULL;
  m_size = 0;
  m_capacity = 0;
  return this;
}

bool c_Vector::t_isempty() {
  return (m_size == 0);
}

int64 c_Vector::t_count() {
  return m_size;
}

Variant c_Vector::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  }
  throwBadKeyType();
  return null;
}

Variant c_Vector::t_get(CVarRef key) {
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return null;
    }
  }
  throwBadKeyType();
  return null;
}

bool c_Vector::t_contains(CVarRef key) {
  if (key.isInteger()) {
    return contains(key.toInt64());
  }
  throwBadKeyType();
  return false;
}

Array c_Vector::t_toarray() {
  return toArrayImpl();
}

void c_Vector::t_sort(CVarRef col /* = null */) {
  raise_warning("Vector::sort() is deprecated, please use the builtin sort() "
                "function or usort() function instead");
  // Terribly inefficient, but produces correct results for now
  Variant arr = t_toarray();
  if (col.isNull()) {
    f_sort(ref(arr));
  } else {
    if (!col.isObject()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected col to be an instance of Collator"));
      throw e;
    }
    ObjectData* obj = col.getObjectData();
    if (!obj->o_instanceof("Collator")) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Expected col to be an instance of Collator"));
      throw e;
    }
    c_Collator* collator = static_cast<c_Collator*>(obj);
    // TODO Task #1429976: What do we do if the Collator encountered errors
    // while sorting? How is this reported to the user?
    collator->t_sort(ref(arr));
  }
  if (!arr.isArray()) {
    ASSERT(false);
    return;
  }
  ArrayData* ad = arr.getArrayData();
  int sz = ad->size();
  ssize_t pos = ad->iter_begin();
  for (int i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    ASSERT(pos != ArrayData::invalid_index);
    tvAsVariant(&m_data[i]) = ad->getValue(pos);
  }
}

void c_Vector::t_reverse() {
  TypedValue* start = m_data;
  TypedValue* end = m_data + m_size - 1;
  for (; start < end; ++start, --end) {
    std::swap(start->m_data.num, end->m_data.num);
    std::swap(start->m_type, end->m_type);
  }
}

void c_Vector::t_splice(CVarRef offset, CVarRef len /* = null */,
                        CVarRef replacement /* = null */) {
  if (!offset.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter offset must be an integer"));
    throw e;
  }
  if (!len.isNull() && !len.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be null or an integer"));
    throw e;
  }
  if (!replacement.isNull()) {
    Object e(SystemLib::AllocRuntimeExceptionObject(
      "Vector::splice does not support replacement parameter"));
    throw e;
  }
  int64 startPos = offset.toInt64();
  if (UNLIKELY(uint64_t(startPos) >= uint64_t(m_size))) {
    if (startPos >= 0) {
      return;
    }
    startPos += m_size;
    if (startPos < 0) {
      startPos = 0;
    }
  }
  int64 endPos;
  if (len.isInteger()) {
    int64 intLen = len.toInt64();
    if (LIKELY(intLen > 0)) {
      endPos = startPos + intLen;
      if (endPos > m_size) {
        endPos = m_size;
      }
    } else {
      if (intLen == 0) {
        return;
      }
      endPos = m_size + intLen;
      if (endPos <= startPos) {
        return;
      }
    }
  } else {
    endPos = m_size;
  }
  // Null out each element before decreffing it. We need to do this in case
  // a __destruct method reenters and accesses this Vector object.
  for (int64 i = startPos; i < endPos; ++i) {
    uint64_t datum = m_data[i].m_data.num;
    DataType t = m_data[i].m_type;
    tvWriteNull(&m_data[i]);
    tvRefcountedDecRefHelper(t, datum);
  }
  // Move elements that came after the deleted elements (if there are any)
  if (endPos < m_size) {
    memmove(&m_data[startPos], &m_data[endPos],
            (m_size-endPos) * sizeof(TypedValue));
  }
  m_size -= (endPos - startPos);
}

int64 c_Vector::t_linearsearch(CVarRef search_value) {
  int sz = m_size;
  for (int i = 0; i < sz; ++i) {
    if (search_value.same(tvAsCVarRef(&m_data[i]))) {
      return i;
    }
  }
  return -1;
}

void c_Vector::t_shuffle() {
  for (int64 i = 1; i < m_size; ++i) {
    int64 j = f_mt_rand(0, i);
    std::swap(m_data[i], m_data[j]);
  }
}

Object c_Vector::t_getiterator() {
  c_VectorIterator* it = NEWOBJ(c_VectorIterator)();
  it->m_obj = this;
  it->m_pos = 0;
  it->m_versionNumber = getVersionNumber();
  return it;
}

Object c_Vector::t_put(CVarRef key, CVarRef value) {
  if (key.isInteger()) {
    TypedValue* tv = (TypedValue*)(&value);
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    put(key.toInt64(), tv);
    return this;
  }
  throwBadKeyType();
  return this;
}

Variant c_Vector::ti_fromarray(const char* cls, CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_Vector* vec;
  Object ret = vec = NEWOBJ(c_Vector)();
  ArrayData* ad = arr.getArrayData();
  int sz = ad->size();
  vec->m_capacity = vec->m_size = sz;
  TypedValue* data;
  vec->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  ssize_t pos = ad->iter_begin();
  for (int i = 0; i < sz; ++i, pos = ad->iter_advance(pos)) {
    ASSERT(pos != ArrayData::invalid_index);
    TypedValue* tv = (TypedValue*)(&ad->getValueRef(pos));
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    tvRefcountedIncRef(tv);
    data[i].m_data.num = tv->m_data.num;
    data[i].m_type = tv->m_type;
  }
  return ret;
}

Variant c_Vector::ti_fromvector(const char* cls, CVarRef vec) {
  if (!vec.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  ObjectData* obj = vec.getObjectData();
  if (!obj->o_instanceof("Vector")) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  c_Vector* v = static_cast<c_Vector*>(obj);
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int sz = v->m_size;
  TypedValue* data;
  target->m_capacity = target->m_size = sz;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  for (int i = 0; i < sz; ++i) {
    tvDup(&v->m_data[i], &data[i]);
  }
  return ret;
}

Variant c_Vector::ti_slice(const char* cls, CVarRef vec, CVarRef offset,
                           CVarRef len /* = null */) {
  if (!vec.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  ObjectData* obj = vec.getObjectData();
  if (!obj->o_instanceof("Vector")) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "vec must be an instance of Vector"));
    throw e;
  }
  if (!offset.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter offset must be an integer"));
    throw e;
  }
  if (!len.isNull() && !len.isInteger()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter len must be null or an integer"));
    throw e;
  }
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  c_Vector* v = static_cast<c_Vector*>(obj);
  int64 startPos = offset.toInt64();
  if (UNLIKELY(uint64_t(startPos) >= uint64_t(v->m_size))) {
    if (startPos >= 0) {
      return ret;
    }
    startPos += v->m_size;
    if (startPos < 0) {
      startPos = 0;
    }
  }
  int64 endPos;
  if (len.isInteger()) {
    int64 intLen = len.toInt64();
    if (LIKELY(intLen > 0)) {
      endPos = startPos + intLen;
      if (endPos > v->m_size) {
        endPos = v->m_size;
      }
    } else {
      if (intLen == 0) {
        return ret;
      }
      endPos = v->m_size + intLen;
      if (endPos <= startPos) {
        return ret;
      }
    }
  } else {
    endPos = v->m_size;
  }
  ASSERT(startPos < endPos);
  int64 sz = endPos - startPos;
  TypedValue* data;
  target->m_capacity = target->m_size = sz;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  for (int64 i = 0; i < sz; ++i, ++startPos) {
    tvDup(&v->m_data[startPos], &data[i]);
  }
  return ret;
}

struct VectorValAccessor {
  typedef const TypedValue& ElmT;
  bool isInt(ElmT elm) const { return elm.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm.m_type); }
  int64 getInt(ElmT elm) const { return elm.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm); }
};

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
c_Vector::SortFlavor c_Vector::preSort(const AccessorT& acc) {
  ASSERT(m_size > 0);
  int64 sz = m_size;
  bool allInts = true;
  bool allStrs = true;
  for (int64 i = 0; i < sz; ++i) {
    allInts = (allInts && acc.isInt(m_data[i]));
    allStrs = (allStrs && acc.isStr(m_data[i]));
  }
  return allStrs ? StringSort : allInts ? IntegerSort : GenericSort;
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

void c_Vector::sort(int sort_flags, bool ascending) {
  if (!m_size) {
    return;
  }
  SortFlavor flav = preSort<VectorValAccessor>(VectorValAccessor());
  CALL_SORT(VectorValAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT

void c_Vector::usort(CVarRef cmp_function) {
  if (!m_size) {
    return;
  }
  ElmUCompare<VectorValAccessor> comp;
  comp.callback = &cmp_function;
  HPHP::Sort::sort(m_data, m_data + m_size, comp);
}

void c_Vector::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys may be used with Vectors"));
  throw e;
}

TypedValue* c_Vector::OffsetGet(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Vector* vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    return vec->at(key->m_data.num);
  }
  throwBadKeyType();
  return NULL;
}

void c_Vector::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  ASSERT(key->m_type != KindOfRef);
  ASSERT(val->m_type != KindOfRef);
  c_Vector* vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    vec->put(key->m_data.num, val);
    return;
  }
  throwBadKeyType();
}

bool c_Vector::OffsetIsset(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Vector* vec = static_cast<c_Vector*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = vec->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = NULL;
  }
  return result ? isset(tvAsCVarRef(result)) : false;
}

bool c_Vector::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Vector* vec = static_cast<c_Vector*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = vec->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = NULL;
  }
  return result ? empty(tvAsCVarRef(result)) : true;
}

bool c_Vector::OffsetContains(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Vector* vec = static_cast<c_Vector*>(obj);
  if (key->m_type == KindOfInt64) {
    return vec->contains(key->m_data.num);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_Vector::OffsetAppend(ObjectData* obj, TypedValue* val) {
  ASSERT(val->m_type != KindOfRef);
  c_Vector* vec = static_cast<c_Vector*>(obj);
  vec->add(val);
}

void c_Vector::OffsetUnset(ObjectData* obj, TypedValue* key) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "Cannot unset element of a Vector"));
  throw e;
}

c_VectorIterator::c_VectorIterator(const ObjectStaticCallbacks *cb) :
    ExtObjectData(cb) {
}

c_VectorIterator::~c_VectorIterator() {
}

void c_VectorIterator::t___construct() {
}

Variant c_VectorIterator::t_current() {
  c_Vector* vec = m_obj.get();
  if (UNLIKELY(m_versionNumber != vec->getVersionNumber())) {
    throw_collection_modified();
  }
  if (!vec->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return tvAsCVarRef(&vec->m_data[m_pos]);
}

Variant c_VectorIterator::t_key() {
  c_Vector* vec = m_obj.get();
  if (!vec->contains(m_pos)) {
    throw_iterator_not_valid();
  }
  return m_pos;
}

bool c_VectorIterator::t_valid() {
  ASSERT(m_pos >= 0);
  c_Vector* vec = m_obj.get();
  return vec && (m_pos < vec->m_size);
}

void c_VectorIterator::t_next() {
  m_pos++;
}

void c_VectorIterator::t_rewind() {
  m_pos = 0;
}

///////////////////////////////////////////////////////////////////////////////

static const char emptyMapSlot[sizeof(c_Map::Bucket)] = { 0 };

c_Map::c_Map(const ObjectStaticCallbacks *cb) :
    ExtObjectDataFlags<ObjectData::MapAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_size(0), m_load(0), m_nLastSlot(0), m_versionNumber(0) {
  m_data = (Bucket*)emptyMapSlot;
}

c_Map::~c_Map() {
  deleteBuckets();
  freeData();
}

void c_Map::freeData() {
  if (m_data != (Bucket*)emptyMapSlot) {
    smart_free(m_data);
  }
  m_data = (Bucket*)emptyMapSlot;
}

void c_Map::deleteBuckets() {
  if (!m_size) return;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      tvRefcountedDecRef(&p.data);
      if (p.hasStrKey() && p.skey->decRefCount() == 0) {
        DELETE(StringData)(p.skey);
      }
    }
  }
}

void c_Map::t___construct() {
}

Variant c_Map::t___destruct() {
  return null;
}

Array c_Map::toArrayImpl() const {
  ArrayInit ai(m_size);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      if (p.hasIntKey()) {
        ai.set((int64)p.ikey, tvAsCVarRef(&p.data));
      } else {
        ai.set(*(const String*)(&p.skey), tvAsCVarRef(&p.data));
      }
    }
  }
  return ai.create();
}

Array c_Map::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_Map::clone() {
  ObjectData* obj = ObjectData::clone();
  c_Map* target = static_cast<c_Map*>(obj);

  if (!m_size) return obj;

  ASSERT(m_nLastSlot != 0);
  target->m_size = m_size;
  target->m_load = m_load;
  target->m_nLastSlot = m_nLastSlot;
  target->m_data = (Bucket*)smart_malloc(numSlots() * sizeof(Bucket));
  memcpy(target->m_data, m_data, numSlots() * sizeof(Bucket));

  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      if (IS_REFCOUNTED_TYPE(p.data.m_type)) {
        tvIncRef(&p.data);
      }
      if (p.hasStrKey()) {
        p.skey->incRefCount();
      }
    }
  }

  return obj;
}

Object c_Map::t_clear() {
  deleteBuckets();
  freeData();
  m_size = 0;
  m_load = 0;
  m_nLastSlot = 0;
  m_data = (Bucket*)emptyMapSlot;
  return this;
}

bool c_Map::t_isempty() {
  return (m_size == 0);
}

int64 c_Map::t_count() {
  return m_size;
}

Variant c_Map::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return null;
}

Variant c_Map::t_get(CVarRef key) {
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return null;
    }
  } else if (key.isString()) {
    TypedValue* tv = get(key.getStringData());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return null;
    }
  }
  throwBadKeyType();
  return null;
}

Object c_Map::t_put(CVarRef key, CVarRef value) {
  TypedValue* val = (TypedValue*)(&value);
  if (UNLIKELY(val->m_type == KindOfRef)) {
    val = val->m_data.pref->tv();
  }
  if (key.isInteger()) {
    update(key.toInt64(), val);
  } else if (key.isString()) {
    update(key.getStringData(), val);
  } else {
    throwBadKeyType();
  }
  return this;
}

bool c_Map::t_contains(CVarRef key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    return contains(key.toInt64());
  }
  if (IS_STRING_TYPE(t)) {
    return contains(key.getStringData());
  }
  throwBadKeyType();
  return false;
}

Object c_Map::t_remove(CVarRef key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    remove(key.toInt64());
  } else if (IS_STRING_TYPE(t)) {
    remove(key.getStringData());
  } else {
    throwBadKeyType();
  }
  return this;
}

Object c_Map::t_discard(CVarRef key) {
  return t_remove(key);
}

Array c_Map::t_toarray() {
  return toArrayImpl();
}

Array c_Map::t_copyasarray() {
  return toArrayImpl();
}

Array c_Map::t_tokeysarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      if (p.hasIntKey()) {
        ai.set((int64)p.ikey);
      } else {
        ai.set(*(const String*)(&p.skey));
      }
    }
  }
  return ai.create();
}

Object c_Map::t_values() {
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int64 sz = m_size;
  if (!sz) {
    return ret;
  }
  TypedValue* data;
  target->m_capacity = target->m_size = sz;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));

  int64 j = 0;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      TypedValue* tv = &p.data;
      tvRefcountedIncRef(tv);
      data[j].m_data.num = tv->m_data.num;
      data[j].m_type = tv->m_type;
      ++j;
    }
  }
  return ret;
}

Array c_Map::t_tovaluesarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket& p = m_data[i];
    if (p.validValue()) {
      ai.set(tvAsCVarRef(&p.data));
    }
  }
  return ai.create();
}

Object c_Map::t_updatefromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Expected arr to be an array"));
    throw e;
  }
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    TypedValue* tv = (TypedValue*)(&ad->getValueRef(pos));
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_Map::t_updatefromiterable(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  if (obj->getCollectionType() == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    for (uint i = 0; i <= mp->m_nLastSlot; ++i) {
      c_Map::Bucket& p = mp->m_data[i];
      if (p.validValue()) {
        if (p.hasIntKey()) {
          update((int64)p.ikey, &p.data);
        } else {
          update(p.skey, &p.data);
        }
      }
    }
    return this;
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = (TypedValue*)(&v);
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_Map::t_differencebykey(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  c_Map* target;
  Object ret = target = static_cast<c_Map*>(clone());
  if (obj->getCollectionType() == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    for (uint i = 0; i <= mp->m_nLastSlot; ++i) {
      c_Map::Bucket& p = mp->m_data[i];
      if (p.validValue()) {
        if (p.hasIntKey()) {
          target->remove((int64)p.ikey);
        } else {
          target->remove(p.skey);
        }
      }
    }
    return ret;
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    if (k.isInteger()) {
      target->remove(k.toInt64());
    } else {
      ASSERT(k.isString());
      target->remove(k.getStringData());
    }
  }
  return ret;
}

Object c_Map::t_getiterator() {
  c_MapIterator* it = NEWOBJ(c_MapIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_versionNumber = getVersionNumber();
  return it;
}

Variant c_Map::ti_fromarray(const char* cls, CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_Map* mp;
  Object ret = mp = NEWOBJ(c_Map)();
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    TypedValue* tv = (TypedValue*)(&ad->getValueRef(pos));
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      mp->update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      mp->update(k.getStringData(), tv);
    }
  }
  return ret;
}

Variant c_Map::ti_fromiterable(const char* cls, CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  Object ret;
  if (obj->getCollectionType() == Collection::MapType) {
    ret = obj->clone();
    return ret;
  }
  c_Map* target;
  ret = target = NEWOBJ(c_Map)();
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = (TypedValue*)(&v);
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      target->update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      target->update(k.getStringData(), tv);
    }
  }
  return ret;
}

void c_Map::throwOOB() {
  Object e(SystemLib::AllocOutOfBoundsExceptionObject(
    "Attempted to subscript a non-key"));
  throw e;
}

#define STRING_HASH(x)   (int32_t(x) | 0x80000000)

bool inline hitStringKey(const c_Map::Bucket* p, const char* k,
                         int len, int32_t hash) ALWAYS_INLINE;
bool inline hitStringKey(const c_Map::Bucket* p, const char* k,
                  int len, int32_t hash) {
  ASSERT(p->validValue());
  if (p->hasIntKey()) return false;
  const char* data = p->skey->data();
  return data == k || (p->hash() == hash &&
                       p->skey->size() == len &&
                       memcmp(data, k, len) == 0);
}

bool inline hitIntKey(const c_Map::Bucket* p, int64 ki) ALWAYS_INLINE;
bool inline hitIntKey(const c_Map::Bucket* p, int64 ki) {
  ASSERT(p->validValue());
  return p->ikey == ki && p->hasIntKey();
}

#define FIND_BODY(h0, hit) \
  size_t tableMask = m_nLastSlot; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Bucket* p = fetchBucket(probeIndex); \
  if (LIKELY(p->validValue() && (hit))) { \
    return p; \
  } \
  if (LIKELY(p->empty())) { \
    return NULL; \
  } \
  for (size_t i = 1;; ++i) { \
    ASSERT(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    ASSERT(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    p = fetchBucket(probeIndex); \
    if (p->validValue() && (hit)) { \
      return p; \
    } \
    if (p->empty()) { \
      return NULL; \
    } \
  }

#define FIND_FOR_INSERT_BODY(h0, hit) \
  size_t tableMask = m_nLastSlot; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Bucket* p = fetchBucket(h0 & tableMask); \
  if (LIKELY((p->validValue() && (hit)) || \
             p->empty())) { \
    return p; \
  } \
  Bucket* ts = NULL; \
  for (size_t i = 1;; ++i) { \
    if (UNLIKELY(p->tombstone() && !ts)) { \
      ts = p; \
    } \
    ASSERT(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    ASSERT(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    p = fetchBucket(probeIndex); \
    if (LIKELY(p->validValue() && (hit))) { \
      return p; \
    } \
    if (LIKELY(p->empty())) { \
      if (LIKELY(!ts)) { \
        return p; \
      } \
      return ts; \
    } \
  }

c_Map::Bucket* c_Map::find(int64 h) const {
  FIND_BODY(h, hitIntKey(p, h));
}

c_Map::Bucket* c_Map::find(const char* k, int len, strhash_t prehash) const {
  FIND_BODY(prehash, hitStringKey(p, k, len, STRING_HASH(prehash)));
}

c_Map::Bucket* c_Map::findForInsert(int64 h) const {
  FIND_FOR_INSERT_BODY(h, hitIntKey(p, h));
}

c_Map::Bucket* c_Map::findForInsert(const char* k, int len,
                                    strhash_t prehash) const {
  FIND_FOR_INSERT_BODY(prehash, hitStringKey(p, k, len, STRING_HASH(prehash)));
}

inline ALWAYS_INLINE
c_Map::Bucket* c_Map::findForNewInsert(size_t h0) const {
  size_t tableMask = m_nLastSlot;
  size_t probeIndex = h0 & tableMask;
  Bucket* p = fetchBucket(probeIndex);
  if (LIKELY(p->empty())) {
    return p;
  }
  for (size_t i = 1;; ++i) {
    ASSERT(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    ASSERT(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex);
    p = fetchBucket(probeIndex);
    if (LIKELY(p->empty())) {
      return p;
    }
  }
}

#undef STRING_HASH
#undef FIND_BODY
#undef FIND_FOR_INSERT_BODY

bool c_Map::update(int64 h, TypedValue* data) {
  ASSERT(data->m_type != KindOfRef);
  Bucket* p = findForInsert(h);
  ASSERT(p);
  if (p->validValue()) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
  }
  ++m_versionNumber;
  ++m_size;
  if (!p->tombstone()) {
    if (UNLIKELY(++m_load >= computeMaxLoad())) {
      resize();
      p = findForInsert(h);
      ASSERT(p);
    }
  }
  tvRefcountedIncRef(data);
  p->data.m_data.num = data->m_data.num;
  p->data.m_type = data->m_type;
  p->setIntKey(h);
  return true;
}

bool c_Map::update(StringData *key, TypedValue* data) {
  strhash_t h = key->hash();
  Bucket* p = findForInsert(key->data(), key->size(), h);
  ASSERT(p);
  if (p->validValue()) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
  }
  ++m_versionNumber;
  ++m_size;
  if (!p->tombstone()) {
    if (UNLIKELY(++m_load >= computeMaxLoad())) {
      resize();
      p = findForInsert(key->data(), key->size(), h);
      ASSERT(p);
    }
  }
  tvRefcountedIncRef(data);
  p->data.m_data.num = data->m_data.num;
  p->data.m_type = data->m_type;
  p->setStrKey(key, h);
  return true;
}

void c_Map::erase(Bucket* p) {
  if (p == NULL) {
    return;
  }
  if (p->validValue()) {
    m_size--;
    tvRefcountedDecRef(&p->data);
    if (p->hasStrKey() && p->skey->decRefCount() == 0) {
      DELETE(StringData)(p->skey);
    }
    p->data.m_type = (DataType)KindOfTombstone;
    if (m_size < computeMinElements() && m_size) {
      resize();
    }
  }
}

void c_Map::resize() {
  reserve(m_size);
}

void c_Map::reserve(int64 sz) {
  ++m_versionNumber;
  if (sz < 2) {
    if (sz <= 0) return;
    sz = 2;
  }
  if (m_nLastSlot == 0) {
    ASSERT(m_data == (Bucket*)emptyMapSlot);
    m_nLastSlot = Util::roundUpToPowerOfTwo(sz << 1) - 1;
    m_data = (Bucket*)smart_calloc(numSlots(), sizeof(Bucket));
    return;
  }
  uint oldNumSlots = numSlots();
  m_nLastSlot = Util::roundUpToPowerOfTwo(sz << 1) - 1;
  m_load = m_size;
  Bucket* oldBuckets = m_data;
  m_data = (Bucket*)smart_calloc(numSlots(), sizeof(Bucket));
  for (uint i = 0; i < oldNumSlots; ++i) {
    Bucket* p = &oldBuckets[i];
    if (p->validValue()) {
      Bucket* np = findForNewInsert(p->hasIntKey() ? p->ikey : p->hash());
      memcpy(np, p, sizeof(Bucket));
    }
  }
  smart_free(oldBuckets);
}

ssize_t c_Map::iter_begin() const {
  if (!m_size) return 0;
  for (uint i = 0; i <= m_nLastSlot; ++i) {
    Bucket* p = fetchBucket(i);
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
  }
  return 0;
}

ssize_t c_Map::iter_next(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  Bucket* pLast = fetchBucket(m_nLastSlot);
  ++p;
  while (p <= pLast) {
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
    ++p;
  }
  return 0;
}

ssize_t c_Map::iter_prev(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  Bucket* pStart = m_data;
  --p;
  while (p >= pStart) {
    if (p->validValue()) {
      return reinterpret_cast<ssize_t>(p);
    }
    --p;
  }
  return 0;
}

Variant c_Map::iter_key(ssize_t pos) const {
  ASSERT(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  if (p->hasStrKey()) {
    return p->skey;
  }
  return (int64)p->ikey;
}

Variant c_Map::iter_value(ssize_t pos) const {
  ASSERT(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return tvAsCVarRef(&p->data);
}

void c_Map::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps"));
  throw e;
}

void c_Map::Bucket::dump() {
  if (!validValue()) {
    printf("c_Map::Bucket: %s\n", (empty() ? "empty" : "tombstone"));
    return;
  }
  printf("c_Map::Bucket: %llx\n", hashKey());
  if (hasStrKey()) {
    skey->dump();
  }
  tvAsCVarRef(&data).dump();
}

TypedValue* c_Map::OffsetGet(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Map* mp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    return mp->at(key->m_data.num);
  }
  if (IS_STRING_TYPE(key->m_type)) {
    return mp->at(key->m_data.pstr);
  }
  throwBadKeyType();
  return NULL;
}

void c_Map::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  ASSERT(key->m_type != KindOfRef);
  ASSERT(val->m_type != KindOfRef);
  c_Map* mp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    mp->put(key->m_data.num, val);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    mp->put(key->m_data.pstr, val);
    return;
  }
  throwBadKeyType();
}

bool c_Map::OffsetIsset(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Map* mp = static_cast<c_Map*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = mp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = mp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = NULL;
  }
  return result ? isset(tvAsCVarRef(result)) : false;
}

bool c_Map::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Map* mp = static_cast<c_Map*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = mp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = mp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = NULL;
  }
  return result ? empty(tvAsCVarRef(result)) : true;
}

bool c_Map::OffsetContains(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Map* mp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    return mp->contains(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    return mp->contains(key->m_data.pstr);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_Map::OffsetAppend(ObjectData* obj, TypedValue* val) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "[] operator not supported for Maps"));
  throw e;
}

void c_Map::OffsetUnset(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Map* mp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    mp->remove(key->m_data.num);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    mp->remove(key->m_data.pstr);
    return;
  }
  throwBadKeyType();
}

c_MapIterator::c_MapIterator(const ObjectStaticCallbacks *cb) :
    ExtObjectData(cb) {
}

c_MapIterator::~c_MapIterator() {
}

void c_MapIterator::t___construct() {
}

Variant c_MapIterator::t_current() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return mp->iter_value(m_pos);
}

Variant c_MapIterator::t_key() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return mp->iter_key(m_pos);
}

bool c_MapIterator::t_valid() {
  return m_pos != 0;
}

void c_MapIterator::t_next() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
    throw_collection_modified();
  }
  m_pos = mp->iter_next(m_pos);
}

void c_MapIterator::t_rewind() {
  c_Map* mp = m_obj.get();
  if (UNLIKELY(m_versionNumber != mp->getVersionNumber())) {
    throw_collection_modified();
  }
  m_pos = mp->iter_begin();
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION_CLS(c_StableMap, Bucket);

#define CONNECT_TO_GLOBAL_DLLIST(element)                               \
do {                                                                    \
  (element)->pListLast = m_pListTail;                                   \
  m_pListTail = (element);                                              \
  (element)->pListNext = NULL;                                          \
  if ((element)->pListLast != NULL) {                                   \
    (element)->pListLast->pListNext = (element);                        \
  }                                                                     \
  if (!m_pListHead) {                                                   \
    m_pListHead = (element);                                            \
  }                                                                     \
} while (false)

static const char emptyStableMapSlot[sizeof(c_StableMap::Bucket*)] = { 0 };

c_StableMap::c_StableMap(const ObjectStaticCallbacks *cb) :
    ExtObjectDataFlags<ObjectData::StableMapAttrInit|
                       ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset>(cb),
    m_versionNumber(0), m_pListHead(NULL), m_pListTail(NULL) {
  m_size = 0;
  m_nTableSize = 0;
  m_nTableMask = 0;
  m_arBuckets = (Bucket**)emptyStableMapSlot;
}

c_StableMap::~c_StableMap() {
  deleteBuckets();
  freeData();
}

void c_StableMap::freeData() {
  if (m_arBuckets != (Bucket**)emptyStableMapSlot) {
    smart_free(m_arBuckets);
  }
  m_arBuckets = (Bucket**)emptyStableMapSlot;
}

void c_StableMap::deleteBuckets() {
  Bucket* p = m_pListHead;
  while (p) {
    Bucket* q = p;
    p = p->pListNext;
    DELETE(Bucket)(q);
  }
}

void c_StableMap::t___construct() {
}

Variant c_StableMap::t___destruct() {
  return null;
}

Array c_StableMap::toArrayImpl() const {
  ArrayInit ai(m_size);
  Bucket* p = m_pListHead;
  while (p) {
    if (p->hasIntKey()) {
      ai.set((int64)p->ikey, tvAsCVarRef(&p->data));
    } else {
      ai.set(*(const String*)(&p->skey), tvAsCVarRef(&p->data));
    }
    p = p->pListNext;
  }
  return ai.create();
}

Array c_StableMap::o_toArray() const {
  check_collection_cast_to_array();
  return toArrayImpl();
}

ObjectData* c_StableMap::clone() {
  ObjectData* obj = ObjectData::clone();
  c_StableMap* target = static_cast<c_StableMap*>(obj);

  if (!m_size) return obj;

  target->m_size = m_size;
  target->m_nTableSize = m_nTableSize;
  target->m_nTableMask = m_nTableMask;
  target->m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));

  Bucket *last = NULL;
  for (Bucket *p = m_pListHead; p; p = p->pListNext) {
    Bucket *np = NEW(Bucket)();
    tvDup(&p->data, &np->data);
    uint nIndex;
    if (p->hasIntKey()) {
      np->setIntKey(p->ikey);
      nIndex = p->ikey & target->m_nTableMask;
    } else {
      np->setStrKey(p->skey, p->hash());
      nIndex = p->hash() & target->m_nTableMask;
    }

    np->pNext = target->m_arBuckets[nIndex];
    target->m_arBuckets[nIndex] = np;

    if (last) {
      last->pListNext = np;
      np->pListLast = last;
    } else {
      target->m_pListHead = np;
      np->pListLast = NULL;
    }
    last = np;
  }
  if (last) last->pListNext = NULL;
  target->m_pListTail = last;

  return obj;
}

Object c_StableMap::t_clear() {
  deleteBuckets();
  freeData();
  m_pListHead = NULL;
  m_pListTail = NULL;
  m_size = 0;
  m_nTableSize = 0;
  m_nTableMask = 0;
  m_arBuckets = (Bucket**)emptyStableMapSlot;
  return this;
}

bool c_StableMap::t_isempty() {
  return (m_size == 0);
}

int64 c_StableMap::t_count() {
  return m_size;
}

Variant c_StableMap::t_at(CVarRef key) {
  if (key.isInteger()) {
    return tvAsCVarRef(at(key.toInt64()));
  } else if (key.isString()) {
    return tvAsCVarRef(at(key.getStringData()));
  }
  throwBadKeyType();
  return null;
}

Variant c_StableMap::t_get(CVarRef key) {
  if (key.isInteger()) {
    TypedValue* tv = get(key.toInt64());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return null;
    }
  } else if (key.isString()) {
    TypedValue* tv = get(key.getStringData());
    if (tv) {
      return tvAsCVarRef(tv);
    } else {
      return null;
    }
  }
  throwBadKeyType();
  return null;
}

Object c_StableMap::t_put(CVarRef key, CVarRef value) {
  TypedValue* val = (TypedValue*)(&value);
  if (UNLIKELY(val->m_type == KindOfRef)) {
    val = val->m_data.pref->tv();
  }
  if (key.isInteger()) {
    update(key.toInt64(), val);
  } else if (key.isString()) {
    update(key.getStringData(), val);
  } else {
    throwBadKeyType();
  }
  return this;
}

bool c_StableMap::t_contains(CVarRef key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    return contains(key.toInt64());
  }
  if (IS_STRING_TYPE(t)) {
    return contains(key.getStringData());
  }
  throwBadKeyType();
  return false;
}

Object c_StableMap::t_remove(CVarRef key) {
  DataType t = key.getType();
  if (t == KindOfInt64) {
    remove(key.toInt64());
  } else if (IS_STRING_TYPE(t)) {
    remove(key.getStringData());
  } else {
    throwBadKeyType();
  }
  return this;
}

Object c_StableMap::t_discard(CVarRef key) {
  return t_remove(key);
}

Array c_StableMap::t_toarray() {
  return toArrayImpl();
}

Array c_StableMap::t_copyasarray() {
  return toArrayImpl();
}

Array c_StableMap::t_tokeysarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  Bucket* p = m_pListHead;
  while (p) {
    if (p->hasIntKey()) {
      ai.set((int64)p->ikey);
    } else {
      ai.set(*(const String*)(&p->skey));
    }
    p = p->pListNext;
  }
  return ai.create();
}

Object c_StableMap::t_values() {
  c_Vector* target;
  Object ret = target = NEWOBJ(c_Vector)();
  int64 sz = m_size;
  if (!sz) {
    return ret;
  }
  TypedValue* data;
  target->m_capacity = target->m_size = sz;
  target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
  Bucket* p = m_pListHead;
  for (int64 i = 0; i < sz; ++i) {
    ASSERT(p);
    tvDup(&p->data, &data[i]);
    p = p->pListNext;
  }
  return ret;
}

Array c_StableMap::t_tovaluesarray() {
  ArrayInit ai(m_size, ArrayInit::vectorInit);
  Bucket* p = m_pListHead;
  while (p) {
    ai.set(tvAsCVarRef(&p->data));
    p = p->pListNext;
  }
  return ai.create();
}

Object c_StableMap::t_updatefromarray(CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    TypedValue* tv = (TypedValue*)(&ad->getValueRef(pos));
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_StableMap::t_updatefromiterable(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  if (obj->getCollectionType() == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    c_StableMap::Bucket* p = smp->m_pListHead;
    while (p) {
      if (p->hasIntKey()) {
        update((int64)p->ikey, &p->data);
      } else {
        update(p->skey, &p->data);
      }
      p = p->pListNext;
    }
    return this;
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = (TypedValue*)(&v);
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      update(k.getStringData(), tv);
    }
  }
  return this;
}

Object c_StableMap::t_differencebykey(CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  c_StableMap* target;
  Object ret = target = static_cast<c_StableMap*>(clone());
  if (obj->getCollectionType() == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    c_StableMap::Bucket* p = smp->m_pListHead;
    while (p) {
      if (p->hasIntKey()) {
        target->remove((int64)p->ikey);
      } else {
        target->remove(p->skey);
      }
      p = p->pListNext;
    }
  }
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    if (k.isInteger()) {
      target->remove(k.toInt64());
    } else {
      ASSERT(k.isString());
      target->remove(k.getStringData());
    }
  }
  return ret;
}

Object c_StableMap::t_getiterator() {
  c_StableMapIterator* it = NEWOBJ(c_StableMapIterator)();
  it->m_obj = this;
  it->m_pos = iter_begin();
  it->m_versionNumber = getVersionNumber();
  return it;
}

Variant c_StableMap::ti_fromarray(const char* cls, CVarRef arr) {
  if (!arr.isArray()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter arr must be an array"));
    throw e;
  }
  c_StableMap* smp;
  Object ret = smp = NEWOBJ(c_StableMap)();
  ArrayData* ad = arr.getArrayData();
  for (ssize_t pos = ad->iter_begin(); pos != ArrayData::invalid_index;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    Variant v = ad->getValue(pos);
    TypedValue* tv = (TypedValue*)(&v);
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      smp->update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      smp->update(k.getStringData(), tv);
    }
  }
  return ret;
}

Variant c_StableMap::ti_fromiterable(const char* cls, CVarRef it) {
  if (!it.isObject()) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Parameter it must be an instance of Iterable"));
    throw e;
  }
  ObjectData* obj = it.getObjectData();
  Object ret;
  if (obj->getCollectionType() == Collection::StableMapType) {
    ret = obj->clone();
    return ret;
  }
  c_StableMap* target;
  ret = target = NEWOBJ(c_StableMap)();
  for (ArrayIter iter = obj->begin(); iter; ++iter) {
    Variant k = iter.first();
    Variant v = iter.second();
    TypedValue* tv = (TypedValue*)(&v);
    if (UNLIKELY(tv->m_type == KindOfRef)) {
      tv = tv->m_data.pref->tv();
    }
    if (k.isInteger()) {
      target->update(k.toInt64(), tv);
    } else {
      ASSERT(k.isString());
      target->update(k.getStringData(), tv);
    }
  }
  return ret;
}

bool inline sm_hit_string_key(const c_StableMap::Bucket* p,
                              const char* k, int len, int32 hash) ALWAYS_INLINE;
bool inline sm_hit_string_key(const c_StableMap::Bucket* p,
                              const char* k, int len, int32 hash) {
  if (p->hasIntKey()) return false;
  const char* data = p->skey->data();
  return data == k || (p->hash() == hash &&
                       p->skey->size() == len &&
                       memcmp(data, k, len) == 0);
}

c_StableMap::Bucket* c_StableMap::find(int64 h) const {
  for (Bucket* p = m_arBuckets[h & m_nTableMask]; p; p = p->pNext) {
    if (p->hasIntKey() && p->ikey == h) {
      return p;
    }
  }
  return NULL;
}

c_StableMap::Bucket* c_StableMap::find(const char* k, int len,
                                       strhash_t prehash) const {
  int32_t hash = c_StableMap::Bucket::encodeHash(prehash);
  for (Bucket* p = m_arBuckets[prehash & m_nTableMask]; p; p = p->pNext) {
    if (sm_hit_string_key(p, k, len, hash)) return p;
  }
  return NULL;
}

c_StableMap::Bucket** c_StableMap::findForErase(int64 h) const {
  Bucket** ret = &(m_arBuckets[h & m_nTableMask]);
  Bucket* p = *ret;
  while (p) {
    if (p->hasIntKey() && p->ikey == h) {
      return ret;
    }
    ret = &(p->pNext);
    p = *ret;
  }
  return NULL;
}

c_StableMap::Bucket** c_StableMap::findForErase(const char* k, int len,
                                                strhash_t prehash) const {
  Bucket** ret = &(m_arBuckets[prehash & m_nTableMask]);
  Bucket* p = *ret;
  int32_t hash = c_StableMap::Bucket::encodeHash(prehash);
  while (p) {
    if (sm_hit_string_key(p, k, len, hash)) return ret;
    ret = &(p->pNext);
    p = *ret;
  }
  return NULL;
}

bool c_StableMap::update(int64 h, TypedValue* data) {
  Bucket* p = find(h);
  if (p) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
  }
  ++m_versionNumber;
  if (++m_size > m_nTableSize) {
    resize();
  }
  p = NEW(Bucket)(data);
  p->setIntKey(h);
  uint nIndex = (h & m_nTableMask);
  p->pNext = m_arBuckets[nIndex];
  m_arBuckets[nIndex] = p;
  CONNECT_TO_GLOBAL_DLLIST(p);
  return true;
}

bool c_StableMap::update(StringData *key, TypedValue* data) {
  strhash_t h = key->hash();
  Bucket* p = find(key->data(), key->size(), h);
  if (p) {
    tvRefcountedIncRef(data);
    tvRefcountedDecRef(&p->data);
    p->data.m_data.num = data->m_data.num;
    p->data.m_type = data->m_type;
    return true;
  }
  ++m_versionNumber;
  if (++m_size > m_nTableSize) {
    resize();
  }
  p = NEW(Bucket)(data);
  p->setStrKey(key, h);
  uint nIndex = (h & m_nTableMask);
  p->pNext = m_arBuckets[nIndex];
  m_arBuckets[nIndex] = p;
  CONNECT_TO_GLOBAL_DLLIST(p);
  return true;
}

void c_StableMap::erase(Bucket** prev) {
  if (prev == NULL) {
    return;
  }
  Bucket* p = *prev;
  if (p) {
    *prev = p->pNext;
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
    m_size--;
    DELETE(Bucket)(p);
  }
}

void c_StableMap::resize() {
  reserve(m_size);
}

void c_StableMap::reserve(int64 sz) {
  ++m_versionNumber;
  if (sz < 4) {
    if (sz <= 0) return;
    sz = 4;
  } else {
    sz = Util::roundUpToPowerOfTwo(sz);
  }
  if (m_nTableSize == 0) {
    m_nTableSize = sz;
    m_nTableMask = m_nTableSize - 1;
    m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));
    return;
  }
  m_nTableSize = sz;
  m_nTableMask = m_nTableSize - 1;
  smart_free(m_arBuckets);
  m_arBuckets = (Bucket**)smart_calloc(m_nTableSize, sizeof(Bucket*));
  for (Bucket* p = m_pListHead; p; p = p->pListNext) {
    uint nIndex = (p->hashKey() & m_nTableMask);
    p->pNext = m_arBuckets[nIndex];
    m_arBuckets[nIndex] = p;
  }
}

ssize_t c_StableMap::iter_begin() const {
  Bucket* p = m_pListHead;
  return reinterpret_cast<ssize_t>(p);
}

ssize_t c_StableMap::iter_next(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  p = p->pListNext;
  return reinterpret_cast<ssize_t>(p);
}

ssize_t c_StableMap::iter_prev(ssize_t pos) const {
  if (pos == 0) {
    return 0;
  }
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  p = p->pListLast;
  return reinterpret_cast<ssize_t>(p);
}

Variant c_StableMap::iter_key(ssize_t pos) const {
  ASSERT(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  if (p->hasStrKey()) {
    return p->skey;
  }
  return (int64)p->ikey;
}

Variant c_StableMap::iter_value(ssize_t pos) const {
  ASSERT(pos);
  Bucket* p = reinterpret_cast<Bucket*>(pos);
  return tvAsCVarRef(&p->data);
}

struct StableMapKeyAccessor {
  typedef const c_StableMap::Bucket* ElmT;
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

struct StableMapValAccessor {
  typedef const c_StableMap::Bucket* ElmT;
  bool isInt(ElmT elm) const { return elm->data.m_type == KindOfInt64; }
  bool isStr(ElmT elm) const { return IS_STRING_TYPE(elm->data.m_type); }
  int64 getInt(ElmT elm) const { return elm->data.m_data.num; }
  StringData* getStr(ElmT elm) const { return elm->data.m_data.pstr; }
  Variant getValue(ElmT elm) const { return tvAsCVarRef(&elm->data); }
};

/**
 * preSort() does an initial pass over the array to do some preparatory work
 * before the sort algorithm runs. For sorts that use builtin comparators, the
 * types of values are also observed during this first pass. By observing the
 * types during this initial pass, we can often use a specialized comparator
 * and avoid performing type checks during the actual sort.
 */
template <typename AccessorT>
c_StableMap::SortFlavor c_StableMap::preSort(Bucket** buffer,
                                             const AccessorT& acc,
                                             bool checkTypes) {
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
 * postSort() runs after sorting has been performed. For StableMap, postSort()
 * handles rewiring the linked list according to the results of the sort.
 */
void c_StableMap::postSort(Bucket** buffer) {
  uint last = m_size-1;
  Bucket* b = m_pListHead = buffer[0];
  b->pListLast = NULL;
  for (uint i = 0; i < last; ++i) {
    Bucket* bNext = buffer[i+1];
    b->pListNext = bNext;
    bNext->pListLast = b;
    b = bNext;
  }
  m_pListTail = b;
  b->pListNext = NULL;
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
#define SORT_BODY(acc_type) \
  do { \
    if (!m_size) { \
      return; \
    } \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    SortFlavor flav = preSort<acc_type>(buffer, acc_type(), true); \
    try { \
      CALL_SORT(acc_type); \
    } catch (...) { \
      postSort(buffer); \
      smart_free(buffer); \
      throw; \
    } \
    postSort(buffer); \
    smart_free(buffer); \
  } while(0)

void c_StableMap::asort(int sort_flags, bool ascending) {
  SORT_BODY(StableMapValAccessor);
}

void c_StableMap::ksort(int sort_flags, bool ascending) {
  SORT_BODY(StableMapKeyAccessor);
}

#undef SORT_CASE
#undef SORT_CASE_BLOCK
#undef CALL_SORT
#undef SORT_BODY

#define USER_SORT_BODY(acc_type) \
  do { \
    if (!m_size) { \
      return; \
    } \
    Bucket** buffer = (Bucket**)smart_malloc(m_size * sizeof(Bucket*)); \
    preSort<acc_type>(buffer, acc_type(), false); \
    ElmUCompare<acc_type> comp; \
    comp.callback = &cmp_function; \
    try { \
      HPHP::Sort::sort(buffer, buffer + m_size, comp); \
    } catch (...) { \
      postSort(buffer); \
      smart_free(buffer); \
      throw; \
    } \
    postSort(buffer); \
    smart_free(buffer); \
  } while (0)

void c_StableMap::uasort(CVarRef cmp_function) {
  USER_SORT_BODY(StableMapValAccessor);
}

void c_StableMap::uksort(CVarRef cmp_function) {
  USER_SORT_BODY(StableMapKeyAccessor);
}

#undef USER_SORT_BODY

void c_StableMap::throwBadKeyType() {
  Object e(SystemLib::AllocInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with StableMaps"));
  throw e;
}

c_StableMap::Bucket::~Bucket() {
  if (hasStrKey() && skey->decRefCount() == 0) {
    DELETE(StringData)(skey);
  }
  tvRefcountedDecRef(&data);
}

void c_StableMap::Bucket::dump() {
  printf("c_StableMap::Bucket: %llx, %p, %p, %p\n",
         hashKey(), pListNext, pListLast, pNext);
  if (hasStrKey()) {
    skey->dump();
  }
  tvAsCVarRef(&data).dump();
}

TypedValue* c_StableMap::OffsetGet(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_StableMap* smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return smp->at(key->m_data.num);
  }
  if (IS_STRING_TYPE(key->m_type)) {
    return smp->at(key->m_data.pstr);
  }
  throwBadKeyType();
  return NULL;
}

void c_StableMap::OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  ASSERT(key->m_type != KindOfRef);
  ASSERT(val->m_type != KindOfRef);
  c_StableMap* smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    smp->put(key->m_data.num, val);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    smp->put(key->m_data.pstr, val);
    return;
  }
  throwBadKeyType();
}

bool c_StableMap::OffsetIsset(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_StableMap* smp = static_cast<c_StableMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = smp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = smp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = NULL;
  }
  return result ? isset(tvAsCVarRef(result)) : false;
}

bool c_StableMap::OffsetEmpty(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_StableMap* smp = static_cast<c_StableMap*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = smp->get(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    result = smp->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = NULL;
  }
  return result ? empty(tvAsCVarRef(result)) : true;
}

bool c_StableMap::OffsetContains(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_Map* smp = static_cast<c_Map*>(obj);
  if (key->m_type == KindOfInt64) {
    return smp->contains(key->m_data.num);
  } else if (IS_STRING_TYPE(key->m_type)) {
    return smp->contains(key->m_data.pstr);
  } else {
    throwBadKeyType();
    return false;
  }
}

void c_StableMap::OffsetAppend(ObjectData* obj, TypedValue* val) {
  Object e(SystemLib::AllocRuntimeExceptionObject(
    "[] operator not supported for StableMaps"));
  throw e;
}

void c_StableMap::OffsetUnset(ObjectData* obj, TypedValue* key) {
  ASSERT(key->m_type != KindOfRef);
  c_StableMap* smp = static_cast<c_StableMap*>(obj);
  if (key->m_type == KindOfInt64) {
    smp->remove(key->m_data.num);
    return;
  }
  if (IS_STRING_TYPE(key->m_type)) {
    smp->remove(key->m_data.pstr);
    return;
  }
  throwBadKeyType();
}

#undef CONNECT_TO_GLOBAL_DLLIST

c_StableMapIterator::c_StableMapIterator(const ObjectStaticCallbacks *cb) :
    ExtObjectData(cb) {
}

c_StableMapIterator::~c_StableMapIterator() {
}

void c_StableMapIterator::t___construct() {
}

Variant c_StableMapIterator::t_current() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return smp->iter_value(m_pos);
}

Variant c_StableMapIterator::t_key() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
    throw_collection_modified();
  }
  if (!m_pos) {
    throw_iterator_not_valid();
  }
  return smp->iter_key(m_pos);
}

bool c_StableMapIterator::t_valid() {
  return m_pos != 0;
}

void c_StableMapIterator::t_next() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
    throw_collection_modified();
  }
  m_pos = smp->iter_next(m_pos);
}

void c_StableMapIterator::t_rewind() {
  c_StableMap* smp = m_obj.get();
  if (UNLIKELY(m_versionNumber != smp->getVersionNumber())) {
    throw_collection_modified();
  }
  m_pos = smp->iter_begin();
}

#define COLLECTION_MAGIC_METHODS(cls) \
  String c_##cls::t___tostring() { \
    return #cls; \
  } \
  Variant c_##cls::t___get(Variant name) { \
    throw_collection_property_exception(); \
  } \
  Variant c_##cls::t___set(Variant name, Variant value) { \
    throw_collection_property_exception(); \
  } \
  bool c_##cls::t___isset(Variant name) { \
    throw_collection_property_exception(); \
  } \
  Variant c_##cls::t___unset(Variant name) { \
    throw_collection_property_exception(); \
  }

COLLECTION_MAGIC_METHODS(Vector)
COLLECTION_MAGIC_METHODS(Map)
COLLECTION_MAGIC_METHODS(StableMap)

#undef COLLECTION_MAGIC_METHODS

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer) {
  ASSERT(obj->isCollection());
  int64 sz = collectionSize(obj);
  if (obj->getCollectionType() == Collection::VectorType) {
    serializer->setObjectInfo(obj->o_getClassName(), obj->o_getId(), 'V');
    serializer->writeArrayHeader(sz, true);
    if (serializer->getType() == VariableSerializer::Serialize ||
        serializer->getType() == VariableSerializer::APCSerialize ||
        serializer->getType() == VariableSerializer::DebuggerSerialize) {
      // For the 'V' serialization format, we don't print out keys
      // for Serialize, APCSerialize, DebuggerSerialize
      for (ArrayIter iter(obj); iter; ++iter) {
        serializer->writeArrayValue(iter.second());
      }
    } else {
      for (ArrayIter iter(obj); iter; ++iter) {
        serializer->writeArrayKey(iter.first());
        serializer->writeArrayValue(iter.second());
      }
    }
    serializer->writeArrayFooter();
  } else {
    ASSERT(obj->getCollectionType() == Collection::MapType ||
           obj->getCollectionType() == Collection::StableMapType);
    serializer->setObjectInfo(obj->o_getClassName(), obj->o_getId(), 'K');
    serializer->writeArrayHeader(sz, false);
    for (ArrayIter iter(obj); iter; ++iter) {
      serializer->writeArrayKey(iter.first());
      serializer->writeArrayValue(iter.second());
    }
    serializer->writeArrayFooter();
  }
}

void collectionUnserialize(ObjectData* obj,
                           VariableUnserializer* uns,
                           int64 sz,
                           char type) {
  ASSERT(obj->isCollection());
  collectionReserve(obj, sz);
  if (type == 'V') {
    for (int64 i = 0; i < sz; ++i) {
      Variant v;
      v.unserialize(uns);
      collectionOffsetAppend(obj, v);
    }
  } else {
    ASSERT(type == 'K');
    for (int64 i = 0; i < sz; ++i) {
      Variant k;
      k.unserialize(uns);
      Variant v;
      v.unserialize(uns);
      collectionOffsetSet(obj, k, v);
    }
  }
}

CollectionInit::CollectionInit(int cType, ssize_t nElems) {
  switch (cType) {
    case Collection::VectorType: m_data = NEWOBJ(c_Vector)(); break;
    case Collection::MapType: m_data = NEWOBJ(c_Map)(); break;
    case Collection::StableMapType: m_data = NEWOBJ(c_StableMap)(); break;
    default:
      ASSERT(false);
      break;
  }
  // Reserve enough room for nElems elements in advance
  if (nElems) {
    collectionReserve(m_data, nElems);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
