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

#include <runtime/base/complex_types.h>
#include <runtime/base/types.h>
#include <runtime/base/comparisons.h>
#include <util/exception.h>
#include <runtime/base/array/vector_variant.h>
#include <runtime/base/array/map_variant.h>
#include <runtime/base/shared/shared_map.h>
#include <system/gen/php/classes/stdclass.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/array/array_util.h>
#include <runtime/base/runtime_option.h>
#include <runtime/ext/ext_iconv.h>
#include <unicode/coll.h> // icu

using namespace std;

namespace HPHP {

const Array Array::s_nullArray = Array();

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(Array);
///////////////////////////////////////////////////////////////////////////////
// constructors

Array Array::Create(CVarRef name, CVarRef var) {
  return ArrayData::Create(name.isString() ? name.toKey() : name, var);
}

Array::Array(ArrayData *data) {
  m_data.parr = data;
  if (m_data.parr) {
    m_data.parr->incRefCount();
  }
}

Array::Array(CArrRef arr) {
  m_data.parr = arr.m_data.parr;
  if (m_data.parr) {
    m_data.parr->incRefCount();
  }
}

///////////////////////////////////////////////////////////////////////////////
// operators

Array &Array::operator=(ArrayData *data) {
  set(data);
  return *this;
}

Array &Array::operator=(CArrRef arr) {
  set(arr.m_data.parr);
  return *this;
}

Array &Array::operator=(CVarRef var) {
  return operator=(var.toArray());
}

Array Array::operator+(CVarRef var) const {
  if (var.getType() != KindOfArray) {
    throw BadArrayMergeException();
  }
  return operator+(var.toArray());
}

Array Array::operator+(CArrRef arr) const {
  return Array(m_data.parr).operator+=(arr);
}

Array &Array::operator+=(CVarRef var) {
  if (var.getType() != KindOfArray) {
    throw BadArrayMergeException();
  }
  return operator+=(var.toArray());
}

Array &Array::operator+=(CArrRef arr) {
  return mergeImpl(arr, ArrayData::Plus);
}

Array Array::diff(CArrRef array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function /* = NULL */,
                  const void *key_data /* = NULL */,
                  PFUNC_CMP value_cmp_function /* = NULL */,
                  const void *value_data /* = NULL */) const {
  return diffImpl(array, by_key, by_value, false, key_cmp_function, key_data,
                  value_cmp_function, value_data);
}

Array Array::intersect(CArrRef array, bool by_key, bool by_value,
                       PFUNC_CMP key_cmp_function /* = NULL */,
                       const void *key_data /* = NULL */,
                       PFUNC_CMP value_cmp_function /* = NULL */,
                       const void *value_data /* = NULL */) const {
  return diffImpl(array, by_key, by_value, true, key_cmp_function, key_data,
                  value_cmp_function, value_data);
}


static void _sort(vector<int> &indices, CArrRef source, Array::SortData &opaque,
                  Array::PFUNC_CMP cmp_func,
                  bool by_key, const void *data /* = NULL */);

Array Array::diffImpl(CArrRef array, bool by_key, bool by_value, bool match,
                      PFUNC_CMP key_cmp_function,
                      const void *key_data,
                      PFUNC_CMP value_cmp_function,
                      const void *value_data) const {
  ASSERT(by_key || by_value);
  ASSERT(by_key || key_cmp_function == NULL);
  ASSERT(by_value || value_cmp_function == NULL);

  if (!value_cmp_function) {
    value_cmp_function = SortStringAscending;
  }

  Array ret = Array::Create();
  if (by_key && !key_cmp_function) {
    // Fast case
    for (ArrayIter iter(*this); iter; ++iter) {
      Variant key(iter.first());
      bool found = false;
      if (array.exists(key)) {
        if (by_value) {
          found = value_cmp_function(iter.second(),
                                     array.rvalAt(key), value_data) == 0;
        } else {
          found = true;
        }
      }
      if (found == match) {
        ret.set(key, iter.second());
      }
    }
    return ret;
  }

  if (!key_cmp_function) {
    key_cmp_function = SortRegularAscending;
  }

  vector<int> perm1;
  SortData opaque1;
  int bottom = 0;
  int top = array.size();
  PFUNC_CMP cmp;
  const void *cmp_data;
  if (by_key) {
    cmp = key_cmp_function;
    cmp_data = key_data;
  } else {
    cmp = value_cmp_function;
    cmp_data = value_data;
  }
  _sort(perm1, array, opaque1, cmp, by_key, cmp_data);

  for (ArrayIter iter(*this); iter; ++iter) {
    Variant target;
    if (by_key) {
      target = iter.first();
    } else {
      target = iter.second();
    }

    int mid = -1;
    int min = bottom;
    int max = top;
    while (min < max) {
      mid = (max + min) / 2;
      ssize_t pos = opaque1.positions[perm1[mid]];
      int cmp_res =  cmp(target,
                         by_key ? array->getKey(pos) : array->getValue(pos),
                         cmp_data);
      if (cmp_res > 0) { // outer is bigger
        min = mid + 1;
      } else if (cmp_res == 0) {
        break;
      } else {
        max = mid;
      }
    }
    bool found = false;
    if (min < max) { // found
      // if checking both, check value
      if (by_key && by_value) {
        Variant val(iter.second());
        // Have to look up and down for matches
        for (int i = mid; i < max; i++) {
          ssize_t pos = opaque1.positions[perm1[i]];
          if (key_cmp_function(target, array->getKey(pos), key_data) != 0) {
            break;
          }
          if (value_cmp_function(val, array->getValue(pos), value_data) == 0) {
            found = true;
            break;
          }
        }
        if (!found) {
          for (int i = mid-1; i >= min; i--) {
            ssize_t pos = opaque1.positions[perm1[i]];
            if (key_cmp_function(target, array->getKey(pos), key_data) != 0) {
              break;
            }
            if (value_cmp_function(val, array->getValue(pos),
                                   value_data) == 0) {
              found = true;
              break;
            }
          }
        }
      } else {
        // found at mid
        found = true;
      }
    }

    if (found == match) {
      ret.set(iter.first(), iter.second());
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// manipulations

Array &Array::merge(CArrRef arr) {
  return mergeImpl(arr, ArrayData::Merge);
}

Array &Array::mergeImpl(CArrRef arr, ArrayData::ArrayOp op) {
  if (m_data.parr == NULL || arr.m_data.parr == NULL) {
    throw BadArrayMergeException();
  }
  if (!arr.m_data.parr->empty()) {
    if (op != ArrayData::Merge && m_data.parr->empty()) {
      set(arr.m_data.parr);
    } else {
      ArrayData *escalated = m_data.parr->append(arr.m_data.parr, op,
                                                 m_data.parr->getCount() > 1);
      if (escalated) {
        setPtr(escalated);
      }
    }
  } else if (op == ArrayData::Merge) {
    m_data.parr->renumber();
  }
  return *this;
}

Array Array::slice(int offset, int length, bool preserve_keys) const {
  if (!m_data.parr) return Array();
  return ArrayUtil::Slice(m_data.parr, offset, length, preserve_keys);
}

///////////////////////////////////////////////////////////////////////////////
// type conversions

Object Array::toObject() const {
  if (m_data.parr) {
    return m_data.parr->toObject();
  }
  return Object(NEW(c_stdclass)());
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

bool Array::same(CArrRef v2) const {
  if (m_data.parr == NULL && v2.m_data.parr == NULL) return true;
  if (m_data.parr && v2.m_data.parr) {
    return m_data.parr->compare(v2.m_data.parr, true) == 0;
  }
  return false;
}

bool Array::same(CObjRef v2) const {
  return false;
}

bool Array::equal(CArrRef v2) const {
  if (m_data.parr == NULL || v2.m_data.parr == NULL) {
    return HPHP::equal(toBoolean(), v2.toBoolean());
  }
  return m_data.parr->compare(v2.get(), false) == 0;
}

bool Array::equal(CObjRef v2) const {
  if (m_data.parr == NULL || v2.get() == NULL) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  return false;
}

bool Array::less(CArrRef v2, bool flip /* = false */) const {
  if (m_data.parr == NULL || v2.m_data.parr == NULL) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(m_data.parr, false) > 0;
  }
  return m_data.parr->compare(v2.get(), false) < 0;
}

bool Array::less(CObjRef v2) const {
  if (m_data.parr == NULL || v2.get() == NULL) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  return false;
}

bool Array::less(CVarRef v2) const {
  if (m_data.parr == NULL || v2.isNull()) {
    return HPHP::less(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return m_data.parr->compare(v2.toArray().get(), false) < 0;
  }
  return v2.more(*this);
}

bool Array::more(CArrRef v2, bool flip /* = true */) const {
  if (m_data.parr == NULL || v2.m_data.parr == NULL) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (flip) {
    return v2.get()->compare(m_data.parr, false) < 0;
  }
  return m_data.parr->compare(v2.get(), false) > 0;
}

bool Array::more(CObjRef v2) const {
  if (m_data.parr == NULL || v2.get() == NULL) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  return true;
}

bool Array::more(CVarRef v2) const {
  if (m_data.parr == NULL || v2.isNull()) {
    return HPHP::more(toBoolean(), v2.toBoolean());
  }
  if (v2.getType() == KindOfArray) {
    return v2.toArray().get()->compare(m_data.parr, false) < 0;
  }
  return v2.less(*this);
}

///////////////////////////////////////////////////////////////////////////////
// iterator

void Array::escalate() {
  if (m_data.parr) {
    SharedMap *mapShared = dynamic_cast<SharedMap *>(m_data.parr);
    if (mapShared) {
      setPtr(mapShared->escalate());
      return;
    }
    if (!RuntimeOption::UseZendArray) {
      VectorVariant *vecVariant = dynamic_cast<VectorVariant *>(m_data.parr);
      if (vecVariant) {
        setPtr(NEW(MapVariant)(vecVariant));
        return;
      }

      ASSERT(dynamic_cast<VectorVariant *>(m_data.parr) ||
             dynamic_cast<MapVariant *>(m_data.parr));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// offset functions

Variant Array::rvalAt(bool key, int64 prehash /* = -1 */,
                      bool error /* = false*/) const {
  if (m_data.parr) return m_data.parr->get(key ? 1LL : 0LL, prehash, error);
  return null_variant;
}

Variant Array::rvalAt(char key, int64 prehash /* = -1 */,
                      bool error /* = false */) const {
  if (m_data.parr) return m_data.parr->get((int64)key, prehash, error);
  return null_variant;
}

Variant Array::rvalAt(short key, int64 prehash /* = -1 */,
                      bool error /* = false */) const {
  if (m_data.parr) return m_data.parr->get((int64)key, prehash, error);
  return null_variant;
}

Variant Array::rvalAt(int key, int64 prehash /* = -1 */,
                      bool error /* = false */) const {
  if (m_data.parr) return m_data.parr->get((int64)key, prehash, error);
  return null_variant;
}

Variant Array::rvalAt(int64 key, int64 prehash /* = -1 */,
                      bool error /* = false */) const {
  if (m_data.parr) return m_data.parr->get(key, prehash, error);
  return null_variant;
}

Variant Array::rvalAt(ssize_t key, int64 prehash /* = -1 */,
                      bool error /* = false */) const {
  if (m_data.parr) return m_data.parr->get((int64)key, error);
  return null_variant;
}

Variant Array::rvalAt(double key, int64 prehash /* = -1 */,
                      bool error /* = false */) const {
  if (m_data.parr) return m_data.parr->get((int64)key, prehash, error);
  return null_variant;
}

Variant Array::rvalAt(litstr key, int64 prehash /* = -1 */,
                      bool error /* = false */,
                      bool isString /* = false */) const {
  if (m_data.parr) {
    if (isString) return m_data.parr->get(key, prehash, error);
    int64 n;
    int len = strlen(key);
    if (!is_strictly_integer(key, len, n)) {
      return m_data.parr->get(key, prehash, error);
    } else {
      return m_data.parr->get(n, prehash, error);
    }
  }
  return null_variant;
}

Variant Array::rvalAt(CStrRef key, int64 prehash /* = -1 */,
                      bool error /* = false */,
                      bool isString /* = false */) const {
  if (m_data.parr) {
    if (isString) return m_data.parr->get(key, prehash, error);
    int64 n;
    if (!key->isStrictlyInteger(n)) {
      return m_data.parr->get(key, prehash, error);
    } else {
      return m_data.parr->get(n, prehash, error);
    }
  }
  return null_variant;
}

Variant Array::rvalAt(CVarRef key, int64 prehash /* = -1 */,
                      bool error /* = false*/) const {
  if (!m_data.parr) return null_variant;
  switch (key.m_type) {
  case KindOfNull:
    return m_data.parr->get("", prehash, error);
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    return m_data.parr->get(key.m_data.num, prehash, error);
  case KindOfDouble:
    return m_data.parr->get((int64)key.m_data.dbl, prehash, error);
  case LiteralString:
    key.escalateString();
    // fall through
  case KindOfString: {
    int64 n;
    if (key.m_data.pstr->isStrictlyInteger(n)) {
      return m_data.parr->get(n, prehash, error);
    } else {
      return m_data.parr->get(String(key.m_data.pstr), prehash, error);
    }
  }
  case KindOfArray:
    throw_bad_type_exception("Invalid type used as key");
    break;
  case KindOfObject:
    if (key.isResource()) {
      return m_data.parr->get(key.toInt64(), prehash, error);
    }
    throw_bad_type_exception("Invalid type used as key");
    break;
  case KindOfVariant:
    return rvalAt(*(key.m_data.pvar), prehash, error);
  default:
    ASSERT(false);
    break;
  }
  return null_variant;
}

Variant &Array::lvalAt(litstr  key, int64 prehash /* = -1 */,
                       bool checkExist /* = false */,
                       bool isString /* = false */) {
  if (isString) return lvalAtImpl(String(key), prehash, checkExist);
  return lvalAtImpl(String(key).toKey(), prehash, checkExist);
}
Variant &Array::lvalAt(CStrRef key, int64 prehash /* = -1 */,
                       bool checkExist /* = false */,
                       bool isString /* = false */) {
  if (isString) return lvalAtImpl(key, prehash, checkExist);
  return lvalAtImpl(key.toKey(), prehash, checkExist);
}
Variant &Array::lvalAt(CVarRef key, int64 prehash /* = -1 */,
                       bool checkExist /* = false */) {
  Variant k(key.toKey());
  if (!k.isNull()) {
    return lvalAtImpl(k, prehash, checkExist);
  }
  return Variant::lvalBlackHole();
}

CVarRef Array::set(litstr  key, CVarRef v, int64 prehash /* = -1 */,
                   bool isString /* = false */) {
  if (isString) return setImpl(String(key), v, prehash);
  return setImpl(String(key).toKey(), v, prehash);
}
CVarRef Array::set(CStrRef key, CVarRef v, int64 prehash /* = -1 */,
                   bool isString /* = false */) {
  if (isString) return setImpl(key, v, prehash);
  return setImpl(key.toKey(), v, prehash);
}

CVarRef Array::set(CVarRef key, CVarRef v, int64 prehash /* = -1 */) {
  if (key.getRawType() == KindOfInt64)
    return setImpl(key.getNumData(), v, prehash);
  Variant k(key.toKey());
  if (!k.isNull()) {
    return setImpl(k, v, prehash);
  }
  return null_variant;
}

Variant Array::refvalAt(CStrRef key, int64 prehash /* = -1 */,
                        bool isString /* = false */) {
  return ref(lvalAt(key, prehash, false, isString));
}

///////////////////////////////////////////////////////////////////////////////
// membership functions

bool Array::valueExists(CVarRef search_value,
                        bool strict /* = false */) const {
  for (ArrayIter iter(*this); iter; ++iter) {
    if ((strict && iter.second().same(search_value)) ||
        (!strict && iter.second().equal(search_value))) {
      return true;
    }
  }
  return false;
}

Variant Array::key(CVarRef search_value, bool strict /* = false */) const {
  for (ArrayIter iter(*this); iter; ++iter) {
    if ((strict && iter.second().same(search_value)) ||
        (!strict && iter.second().equal(search_value))) {
      return iter.first();
    }
  }
  return false; // PHP uses "false" over null in many places
}

Array Array::keys(CVarRef search_value /* = null_variant */,
                  bool strict /* = false */) const {
  Array ret = Array::Create();
  if (search_value.isNull()) {
    for (ArrayIter iter(*this); iter; ++iter) {
      ret.append(iter.first());
    }
  } else {
    for (ArrayIter iter(*this); iter; ++iter) {
      if ((strict && iter.second().same(search_value)) ||
          (!strict && iter.second().equal(search_value))) {
        ret.append(iter.first());
      }
    }
  }
  return ret;
}

Array Array::values() const {
  Array ret = Array::Create();
  for (ArrayIter iter(*this); iter; ++iter) {
    ret.append(iter.second());
  }
  return ret;
}

bool Array::exists(litstr key, int64 prehash /* = -1 */,
                   bool isString /* = false */) const {
  if (isString) return existsImpl(key, prehash);
  return existsImpl(String(key).toKey(), prehash);
}

bool Array::exists(CStrRef key, int64 prehash /* = -1 */,
                   bool isString /* = false */) const {
  if (isString) return existsImpl(key, prehash);
  return existsImpl(key.toKey(), prehash);
}

bool Array::exists(CVarRef key, int64 prehash /* = -1 */) const {
  switch(key.getType()) {
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    return existsImpl(key.toInt64(), prehash);
  default:
    break;
  }
  Variant k(key.toKey());
  if (!k.isNull()) {
    return existsImpl(k, prehash);
  }
  return false;
}

void Array::remove(litstr  key, int64 prehash /* = -1 */) {
  removeImpl(String(key).toKey(), prehash);
}
void Array::remove(CStrRef key, int64 prehash /* = -1 */) {
  removeImpl(key.toKey(), prehash);
}

void Array::remove(CVarRef key, int64 prehash /* = -1 */) {
  switch(key.getType()) {
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    removeImpl(key.toInt64(), prehash);
    return;
  default:
    break;
  }
  Variant k(key.toKey());
  if (!k.isNull()) {
    removeImpl(k, prehash);
  }
}

void Array::removeAll() {
  operator=(Create());
}

Variant Array::append(CVarRef v) {
  if (!m_data.parr) {
    setPtr(ArrayData::Create(v));
  } else {
    if (v.isContagious()) {
      escalate();
    }
    bool copy = (m_data.parr->getCount() > 1);
    ArrayData *escalated = m_data.parr->append(v, copy);
    if (escalated) {
      setPtr(escalated);
    }
  }
  return v;
}

Variant Array::pop() {
  if (m_data.parr) {
    Variant ret;
    ArrayData *newarr = m_data.parr->pop(ret);
    if (newarr) {
      setPtr(newarr);
    }
    return ret;
  }
  return null_variant;
}

Variant Array::dequeue() {
  if (m_data.parr) {
    Variant ret;
    ArrayData *newarr = m_data.parr->dequeue(ret);
    if (newarr) {
      setPtr(newarr);
    }
    return ret;
  }
  return null_variant;
}

void Array::prepend(CVarRef v) {
  if (!m_data.parr) {
    operator=(Create());
  }
  ASSERT(m_data.parr);

  ArrayData *newarr = m_data.parr->prepend(v, (m_data.parr->getCount() > 1));
  if (newarr) {
    setPtr(newarr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// output functions

void Array::serialize(VariableSerializer *serializer) const {
  if (m_data.parr) {
    m_data.parr->serialize(serializer);
  } else {
    serializer->writeNull();
  }
}

void Array::unserialize(VariableUnserializer *unserializer) {
  std::istream &in = unserializer->in();
  int64 size;
  char sep;
  in >> size >> sep;
  if (sep != ':') {
    throw Exception("Expected ':' but got '%c'", sep);
  }
  in >> sep;
  if (sep != '{') {
    throw Exception("Expected '{' but got '%c'", sep);
  }
  if (size == 0) {
    operator=(Create());
  } else {
    for (int64 i = 0; i < size; i++) {
      Variant key(unserializer->unserializeKey());
      Variant &value = lvalAt(key);
      value.unserialize(unserializer);
    }
  }

  in >> sep;
  if (sep != '}') {
    throw Exception("Expected '}' but got '%c'", sep);
  }
}

void Array::dump() {
  if (m_data.parr) {
    m_data.parr->dump();
  } else {
    printf("(null)\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
// sorting

#define QSORT_STACK_SIZE (sizeof(size_t) * CHAR_BIT)

static void _zend_qsort_swap(void *a, void *b, size_t siz) {
  register char  *tmp_a_char;
  register char  *tmp_b_char;
  register int   *tmp_a_int;
  register int   *tmp_b_int;
  register size_t i;
  int             t_i;
  char            t_c;

  tmp_a_int = (int *) a;
  tmp_b_int = (int *) b;

  for (i = sizeof(int); i <= siz; i += sizeof(int)) {
    t_i = *tmp_a_int;
    *tmp_a_int++ = *tmp_b_int;
    *tmp_b_int++ = t_i;
  }

  tmp_a_char = (char *) tmp_a_int;
  tmp_b_char = (char *) tmp_b_int;

  for (i = i - sizeof(int) + 1; i <= siz; ++i) {
    t_c = *tmp_a_char;
    *tmp_a_char++ = *tmp_b_char;
    *tmp_b_char++ = t_c;
  }
}

typedef int (*compare_func_t)(const void *, const void *, const void *opaque);
static void zend_qsort(void *base, size_t nmemb, size_t siz,
                       compare_func_t compare, void *opaque) {
  void           *begin_stack[QSORT_STACK_SIZE];
  void           *end_stack[QSORT_STACK_SIZE];
  register char  *begin;
  register char  *end;
  register char  *seg1;
  register char  *seg2;
  register char  *seg2p;
  register int    loop;
  uint            offset;

  begin_stack[0] = (char *) base;
  end_stack[0]   = (char *) base + ((nmemb - 1) * siz);

  for (loop = 0; loop >= 0; --loop) {
    begin = (char*)begin_stack[loop];
    end   = (char*)end_stack[loop];

    while (begin < end) {
      offset = (end - begin) >> 1;
      _zend_qsort_swap(begin, begin + (offset - (offset % siz)), siz);

      seg1 = begin + siz;
      seg2 = end;

      while (1) {
        for (; seg1 < seg2 && compare(begin, seg1, opaque) > 0;
             seg1 += siz);

        for (; seg2 >= seg1 && compare(seg2, begin, opaque) > 0;
             seg2 -= siz);

        if (seg1 >= seg2)
          break;

        _zend_qsort_swap(seg1, seg2, siz);

        seg1 += siz;
        seg2 -= siz;
      }

      _zend_qsort_swap(begin, seg2, siz);

      seg2p = seg2;

      if ((seg2p - begin) <= (end - seg2p)) {
        if ((seg2p + siz) < end) {
          begin_stack[loop] = seg2p + siz;
          end_stack[loop++] = end;
        }
        end = seg2p - siz;
      }
      else {
        if ((seg2p - siz) > begin) {
          begin_stack[loop] = begin;
          end_stack[loop++] = seg2p - siz;
        }
        begin = seg2p + siz;
      }
    }
  }
}

static int array_compare_func(const void *n1, const void *n2, const void *op) {
  int index1 = *(int*)n1;
  int index2 = *(int*)n2;
  Array::SortData *opaque = (Array::SortData*)op;
  ssize_t pos1 = opaque->positions[index1];
  ssize_t pos2 = opaque->positions[index2];
  if (opaque->by_key) {
    return opaque->cmp_func((*opaque->array)->getKey(pos1),
                            (*opaque->array)->getKey(pos2),
                            opaque->data);
  }
  return opaque->cmp_func((*opaque->array)->getValue(pos1),
                          (*opaque->array)->getValue(pos2),
                          opaque->data);
}

static int multi_compare_func(const void *n1, const void *n2, const void *op) {
  int index1 = *(int*)n1;
  int index2 = *(int*)n2;
  const std::vector<Array::SortData> *opaques =
    (const std::vector<Array::SortData> *)op;
  for (unsigned int i = 0; i < opaques->size(); i++) {
    const Array::SortData *opaque = &opaques->at(i);
    ssize_t pos1 = opaque->positions[index1];
    ssize_t pos2 = opaque->positions[index2];
    int result;
    if (opaque->by_key) {
      result = opaque->cmp_func((*opaque->array)->getKey(pos1),
                                (*opaque->array)->getKey(pos2),
                                opaque->data);
    } else {
      result = opaque->cmp_func((*opaque->array)->getValue(pos1),
                                (*opaque->array)->getValue(pos2),
                                opaque->data);
    }
    if (result != 0) return result;
  }
  return 0;
}

static void _sort(vector<int> &indices, CArrRef source, Array::SortData &opaque,
                  Array::PFUNC_CMP cmp_func, bool by_key,
                  const void *data /* = NULL */) {
  ASSERT(cmp_func);

  int count = source.size();
  if (count == 0) {
    return;
  }
  indices.reserve(count);
  for (int i = 0; i < count; i++) {
    indices.push_back(i);
  }

  opaque.array = &source;
  opaque.by_key = by_key;
  opaque.cmp_func = cmp_func;
  opaque.data = data;
  opaque.positions.reserve(count);
  for (ssize_t pos = source->iter_begin(); pos != ArrayData::invalid_index;
       pos = source->iter_advance(pos)) {
    opaque.positions.push_back(pos);
  }
  zend_qsort(&indices[0], count, sizeof(int), array_compare_func, &opaque);
}

void Array::sort(PFUNC_CMP cmp_func, bool by_key, bool renumber,
                 const void *data /* = NULL */) {
  Array sorted = Array::Create();
  SortData opaque;
  vector<int> indices;
  _sort(indices, *this, opaque, cmp_func, by_key, data);
  int count = size();
  for (int i = 0; i < count; i++) {
    ssize_t pos = opaque.positions[indices[i]];
    if (renumber) {
      sorted.append(m_data.parr->getValue(pos));
    } else {
      sorted.set(m_data.parr->getKey(pos), m_data.parr->getValue(pos));
    }
  }
  operator=(sorted);
}

bool Array::MultiSort(std::vector<SortData> &data, bool renumber) {
  ASSERT(!data.empty());

  int count = -1;
  for (unsigned int k = 0; k < data.size(); k++) {
    SortData &opaque = data[k];

    ASSERT(opaque.array);
    ASSERT(opaque.cmp_func);
    int size = opaque.array->size();
    if (count == -1) {
      count = size;
    } else if (count != size) {
      throw_invalid_argument("arrays: (inconsistent sizes)");
      return false;
    }

    opaque.positions.reserve(size);
    CArrRef arr = *opaque.array;
    if (!arr.empty()) {
      for (ssize_t pos = arr->iter_begin(); pos != ArrayData::invalid_index;
           pos = arr->iter_advance(pos)) {
        opaque.positions.push_back(pos);
      }
    }
  }
  if (count == 0) {
    return true;
  }

  int *indices = (int *)malloc(sizeof(int) * count);
  for (int i = 0; i < count; i++) {
    indices[i] = i;
  }

  zend_qsort(indices, count, sizeof(int), multi_compare_func, (void *)&data);

  for (unsigned int k = 0; k < data.size(); k++) {
    SortData &opaque = data[k];
    CArrRef arr = *opaque.array;

    Array sorted;
    for (int i = 0; i < count; i++) {
      ssize_t pos = opaque.positions[indices[i]];
      Variant k(arr->getKey(pos));
      if (renumber && k.isInteger()) {
        sorted.append(arr->getValue(pos));
      } else {
        sorted.set(k, arr->getValue(pos));
      }
    }
    *opaque.original = sorted;
  }

  free(indices);
  return true;
}

int Array::SortRegularAscending(CVarRef v1, CVarRef v2, const void *data) {
  if (v1.less(v2)) return -1;
  if (v1.equal(v2)) return 0;
  return 1;
}
int Array::SortRegularDescending(CVarRef v1, CVarRef v2, const void *data) {
  if (v1.less(v2)) return 1;
  if (v1.equal(v2)) return 0;
  return -1;
}

int Array::SortNumericAscending(CVarRef v1, CVarRef v2, const void *data) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return -1;
  if (d1 == d2) return 0;
  return 1;
}
int Array::SortNumericDescending(CVarRef v1, CVarRef v2, const void *data) {
  double d1 = v1.toDouble();
  double d2 = v2.toDouble();
  if (d1 < d2) return 1;
  if (d1 == d2) return 0;
  return -1;
}

int Array::SortStringAscending(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return strcmp(s1.data(), s2.data());
}
int Array::SortStringDescending(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return strcmp(s2.data(), s1.data());
}

int Array::SortLocaleStringAscending(CVarRef v1, CVarRef v2,
                                     const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s1.data(), s2.data());
}

int Array::SortLocaleStringDescending(CVarRef v1, CVarRef v2,
                                      const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();

  return strcoll(s2.data(), s1.data());
}

int Array::SortNatural(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 0);
}

int Array::SortNaturalCase(CVarRef v1, CVarRef v2, const void *data) {
  String s1 = v1.toString();
  String s2 = v2.toString();
  return string_natural_cmp(s1.data(), s1.size(), s2.data(), s2.size(), 1);
}

///////////////////////////////////////////////////////////////////////////////
}
