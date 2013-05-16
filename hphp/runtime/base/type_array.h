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

#ifndef incl_HPHP_ARRAY_H_
#define incl_HPHP_ARRAY_H_

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'type_array.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include <boost/static_assert.hpp>

#include "hphp/runtime/base/util/smart_ptr.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/array/array_data.h"
#include "hphp/runtime/base/type_string.h"
#include "hphp/runtime/base/hphp_value.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline int64_t ToKey(double d) {
  return d > std::numeric_limits<uint64_t>::max() ? 0u : (uint64_t)d;
}

// forward declaration
class ArrayIter;

/**
 * Array type wrapping around ArrayData to implement reference
 * counting, copy-on-write and ArrayData escalation.
 *
 * "Escalation" happens when an underlying ArrayData cannot handle an operation
 * and instead it needs to "upgrade" itself to be a more general (but slower)
 * type of ArrayData to accomplish the task. This "upgrade" is called
 * escalation.
 */
class Array : protected SmartPtr<ArrayData> {
 typedef SmartPtr<ArrayData> ArrayBase;

 public:
  /**
   * Create an empty array or an array with one element. Note these are
   * different than those copying constructors that also take one value.
   */
  static Array Create() { return ArrayData::Create();}
  static Array Create(CVarRef value) { return ArrayData::Create(value);}
  static Array Create(CVarRef key, CVarRef value);

 public:
  Array() {}
  ~Array();

  ArrayData* get() const { return m_px; }
  void reset() { ArrayBase::reset(); }

  // Deliberately doesn't throw_null_pointer_exception as a perf
  // optimization.
  ArrayData *operator->() const {
    return m_px;
  }

  /**
   * Constructors. Those that take "arr" or "var" are copy constructors, taking
   * array value from the parameter, and they are NOT constructing an array
   * with that single value (then one should use Array::Create() functions).
   */
  /* implicit */ Array(ArrayData *data) : ArrayBase(data) { }
  /* implicit */ Array(CArrRef arr) : ArrayBase(arr.m_px) { }

  /*
   * Special constructor for use from ArrayInit that creates an Array
   * without a null check.
   */
  enum class ArrayInitCtor { Tag };
  explicit Array(ArrayData* ad, ArrayInitCtor)
    : ArrayBase(ad, ArrayBase::NonNull::Tag)
  {}

  // Move ctor
  Array(Array&& src) : ArrayBase(std::move(src)) {
    static_assert(sizeof(Array) == sizeof(ArrayBase), "Fix this.");
  }
  // Move assign
  Array& operator=(Array&& src) {
    static_assert(sizeof(Array) == sizeof(ArrayBase), "Fix this.");
    ArrayBase::operator=(std::move(src));
    return *this;
  }
  /**
   * Informational
   */
  bool empty() const {
    return m_px == nullptr || m_px->empty();
  }
  ssize_t size() const {
    return m_px ? m_px->size() : 0;
  }
  ssize_t length() const {
    return m_px ? m_px->size() : 0;
  }
  bool isNull() const {
    return m_px == nullptr;
  }
  bool valueExists(CVarRef search_value, bool strict = false) const;
  Variant key(CVarRef search_value, bool strict = false) const;

  /**
   * Note that search_value == null_variant will return all keys,
   * while search_value == init_null_variant will return only keys
   * for pairs where the value is a PHP NULL. (or null-like if strict is false)
   */
  Array keys(CVarRef search_value = null_variant, bool strict = false) const;
  Array values() const;

  /**
   * Operators
   */
  Array &operator =  (ArrayData *data);
  Array &operator =  (CArrRef v);
  Array &operator =  (CVarRef v);
  Array  operator +  (ArrayData *data) const;
  Array  operator +  (CArrRef v) const;
  Array  operator +  (CVarRef v) const;
  Array &operator += (ArrayData *data);
  Array &operator += (CArrRef v);
  Array &operator += (CVarRef v);

  // Move assignment
  Array &operator =  (Variant&& v);

  /**
   * Returns the entries that have keys and/or values that are not present in
   * specified array. Keys and values can be compared by user supplied
   * functions and key_data or value_data will be passed into PFUNC_CMP as
   * "data" parameter. Otherwise, equal() will be called for comparisons. If
   * both by_key and by_value, both keys and values have to match to be
   * excluded.
   */
  typedef int (*PFUNC_CMP)(CVarRef v1, CVarRef v2, const void *data);
  Array diff(CVarRef array, bool by_key, bool by_value,
             PFUNC_CMP key_cmp_function = nullptr,
             const void *key_data = nullptr,
             PFUNC_CMP value_cmp_function = nullptr,
             const void *value_data = nullptr) const;

  /**
   * Returns the entries that have keys and/or values that are present in
   * specified array. Keys and values can be compared by user supplied
   * functions and key_data or value_data will be passed into PFUNC_CMP as
   * "data" parameter. Otherwise, equal() will be called for comparisons. If
   * both by_key and by_value, both keys and values have to match to be
   * included.
   */
  Array intersect(CVarRef array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function = nullptr,
                  const void *key_data = nullptr,
                  PFUNC_CMP value_cmp_function = nullptr,
                  const void *value_data = nullptr) const;

  /**
   * Iterator functions. See array_iterator.h for end() and next().
   */
  ArrayIter begin(CStrRef context = null_string) const;

  void escalate();

  /**
   * Manipulations
   *
   * Merge: This is different from operator "+", where existing key's values
   * are NOT modified. This function will actually override with new values.
   * When merging a vector with a vector, new elements are always appended, and
   * this is also different from operator "+", where existing numeric indices
   * are not modified.
   *
   * Slice: Taking a slice. When "preserve_keys" is true, a vector will turn
   * into numerically keyed map.
   */
  Array &merge(CArrRef arr);
  Array slice(int offset, int length, bool preserve_keys) const;

  /**
   * Sorting.
   */
  static int SortRegularAscending(CVarRef v1, CVarRef v2, const void *data);
  static int SortNumericAscending(CVarRef v1, CVarRef v2, const void *data);
  static int SortStringAscending(CVarRef v1, CVarRef v2, const void *data);
  static int SortLocaleStringAscending(CVarRef v1, CVarRef v2,
                                       const void *data);

  static int SortRegularDescending(CVarRef v1, CVarRef v2, const void *data);
  static int SortNumericDescending(CVarRef v1, CVarRef v2, const void *data);
  static int SortStringDescending(CVarRef v1, CVarRef v2, const void *data);
  static int SortLocaleStringDescending(CVarRef v1, CVarRef v2,
                                        const void *data);

  static int SortNatural(CVarRef v1, CVarRef v2, const void *data);
  static int SortNaturalCase(CVarRef v1, CVarRef v2, const void *data);

  void sort(PFUNC_CMP cmp_func, bool by_key, bool renumber,
            const void *data = nullptr);

  /**
   * Sort multiple arrays at once similar to how ORDER BY clause works in SQL.
   */
  struct SortData {
    Variant    *original;
    const Array      *array;
    bool        by_key;
    PFUNC_CMP   cmp_func;
    const void *data;
    std::vector<ssize_t> positions;
  };
  static bool MultiSort(std::vector<SortData> &data, bool renumber);

  static void SortImpl(std::vector<int> &indices, CArrRef source,
                       Array::SortData &opaque,
                       Array::PFUNC_CMP cmp_func,
                       bool by_key, const void *data = nullptr);

  /**
   * Type conversions
   */
  bool   toBoolean() const { return  m_px && !m_px->empty();}
  char   toByte   () const { return (m_px && !m_px->empty()) ? 1 : 0;}
  short  toInt16  () const { return (m_px && !m_px->empty()) ? 1 : 0;}
  int    toInt32  () const { return (m_px && !m_px->empty()) ? 1 : 0;}
  int64_t  toInt64  () const { return (m_px && !m_px->empty()) ? 1 : 0;}
  double toDouble () const { return (m_px && !m_px->empty()) ? 1.0 : 0.0;}
  String toString () const { return m_px ? "Array" : "";}
  Object toObject () const;

  /**
   * Comparisons
   */
  bool same (CArrRef v2) const;
  bool same (CObjRef v2) const;
  bool equal(CArrRef v2) const;
  bool equal(CObjRef v2) const;
  bool less (CArrRef v2, bool flip = false) const;
  bool less (CObjRef v2) const;
  bool less (CVarRef v2) const;
  bool more (CArrRef v2, bool flip = true) const;
  bool more (CObjRef v2) const;
  bool more (CVarRef v2) const;

  /**
   * Offset
   */
  Variant rvalAt(int     key, ACCESSPARAMS_DECL) const;
  Variant rvalAt(int64_t   key, ACCESSPARAMS_DECL) const;
  Variant rvalAt(double  key, ACCESSPARAMS_DECL) const;
  Variant rvalAt(CStrRef key, ACCESSPARAMS_DECL) const;
  Variant rvalAt(CVarRef key, ACCESSPARAMS_DECL) const;

  /**
   * To get offset for temporary usage
   */
  CVarRef rvalAtRef(int     key, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(int64_t   key, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(double  key, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(CVarRef key, ACCESSPARAMS_DECL) const;
  CVarRef rvalAtRef(CStrRef key, ACCESSPARAMS_DECL) const;

  const Variant operator[](int     key) const;
  const Variant operator[](int64_t   key) const;
  const Variant operator[](double  key) const;
  const Variant operator[](CStrRef key) const;
  const Variant operator[](CVarRef key) const;
  const Variant operator[](const char*) const = delete; // use CStrRef

  Variant &lval(int64_t key) {
    if (!m_px) ArrayBase::operator=(ArrayData::Create());
    Variant *ret = nullptr;
    ArrayData *escalated = m_px->lval(key, ret, m_px->getCount() > 1);
    if (escalated != m_px) ArrayBase::operator=(escalated);
    assert(ret);
    return *ret;
  }

  Variant &lval(CStrRef key) {
    if (!m_px) ArrayBase::operator=(ArrayData::Create());
    Variant *ret = nullptr;
    ArrayData *escalated = m_px->lval(key, ret, m_px->getCount() > 1);
    if (escalated != m_px) ArrayBase::operator=(escalated);
    assert(ret);
    return *ret;
  }

  Variant *lvalPtr(CStrRef key, bool forWrite, bool create);

  Variant &lvalAt();

  Variant &lvalAt(int     key, ACCESSPARAMS_DECL) {
    return lvalAtImpl(key, flags);
  }
  Variant &lvalAt(int64_t   key, ACCESSPARAMS_DECL) {
    return lvalAtImpl(key, flags);
  }
  Variant &lvalAt(double  key, ACCESSPARAMS_DECL) {
    return lvalAtImpl(ToKey(key), flags);
  }
  Variant &lvalAt(CStrRef key, ACCESSPARAMS_DECL);
  Variant &lvalAt(CVarRef key, ACCESSPARAMS_DECL);

  // defined in type_variant.h
  template<typename T>
  CVarRef setImpl(const T &key, CVarRef v);
  template<typename T>
  CVarRef setRefImpl(const T &key, CVarRef v);

  CVarRef set(int     key, CVarRef v) {
    return set((int64_t)key, v);
  }
  CVarRef set(int64_t   key, CVarRef v);
  CVarRef set(double  key, CVarRef v) {
    return set(ToKey(key), v);
  }

  CVarRef set(CStrRef key, CVarRef v, bool isKey = false);
  CVarRef set(CVarRef key, CVarRef v, bool isKey = false);

  CVarRef set(char    key, RefResult v) { return setRef(key,variant(v)); }
  CVarRef set(short   key, RefResult v) { return setRef(key,variant(v)); }
  CVarRef set(int     key, RefResult v) { return setRef(key,variant(v)); }
  CVarRef set(int64_t   key, RefResult v) { return setRef(key,variant(v)); }
  CVarRef set(double  key, RefResult v) { return setRef(key,variant(v)); }

  CVarRef set(CStrRef key, RefResult v, bool isKey = false) {
    return setRef(key,variant(v), isKey);
  }
  CVarRef set(CVarRef key, RefResult v, bool isKey = false) {
    return setRef(key,variant(v), isKey);
  }

  CVarRef setRef(int     key, CVarRef v) {
    return setRef((int64_t)key, v);
  }
  CVarRef setRef(int64_t   key, CVarRef v);
  CVarRef setRef(double  key, CVarRef v) {
    return setRef(ToKey(key), v);
  }

  CVarRef setRef(CStrRef key, CVarRef v, bool isKey = false);
  CVarRef setRef(CVarRef key, CVarRef v, bool isKey = false);

  // defined in type_variant.h
  template<typename T>
  CVarRef addImpl(const T &key, CVarRef v);

  CVarRef add(int     key, CVarRef v) {
    return add((int64_t)key, v);
  }
  CVarRef add(int64_t   key, CVarRef v);
  CVarRef add(double  key, CVarRef v) {
    return add(ToKey(key), v);
  }

  CVarRef add(CStrRef key, CVarRef v, bool isKey = false);
  CVarRef add(CVarRef key, CVarRef v, bool isKey = false);

  // defined in type_variant.h
  template<typename T>
  Variant &addLvalImpl(const T &key) {
    if (!m_px) ArrayBase::operator=(ArrayData::Create());
    Variant *ret = nullptr;
    ArrayData *escalated = m_px->addLval(key, ret, m_px->getCount() > 1);
    if (escalated != m_px) ArrayBase::operator=(escalated);
    assert(ret);
    return *ret;
  }

  Variant &addLval(int     key) {
    return addLvalImpl((int64_t)key);
  }
  Variant &addLval(int64_t   key) {
    return addLvalImpl(key);
  }
  Variant &addLval(double  key) {
    return addLvalImpl(ToKey(key));
  }

  Variant &addLval(CStrRef key, bool isKey = false);
  Variant &addLval(CVarRef key, bool isKey = false);

  /**
   * Membership functions.
   */
  template<typename T>
  bool existsImpl(const T &key) const {
    if (m_px) return m_px->exists(key);
    return false;
  }
  bool exists(char    key) const {
    return existsImpl((int64_t)key);
  }
  bool exists(short   key) const {
    return existsImpl((int64_t)key);
  }
  bool exists(int     key) const {
    return existsImpl((int64_t)key);
  }
  bool exists(int64_t   key) const {
    return existsImpl(key);
  }
  bool exists(double  key) const {
    return existsImpl(ToKey(key));
  }
  bool exists(CStrRef key, bool isKey = false) const;
  bool exists(CVarRef key, bool isKey = false) const;

  template<typename T>
  void removeImpl(const T &key) {
    if (m_px) {
      ArrayData *escalated = m_px->remove(key, (m_px->getCount() > 1));
      if (escalated != m_px) ArrayBase::operator=(escalated);
    }
  }
  void remove(char    key) {
    removeImpl((int64_t)key);
  }
  void remove(short   key) {
    removeImpl((int64_t)key);
  }
  void remove(int     key) {
    removeImpl((int64_t)key);
  }
  void remove(int64_t   key) {
    removeImpl(key);
  }
  void remove(double  key) {
    removeImpl(ToKey(key));
  }
  void remove(CStrRef key, bool isString = false);
  void remove(CVarRef key);

  void removeAll();
  void clear() { removeAll();}

  CVarRef append(CVarRef v);
  CVarRef append(RefResult v) { return appendRef(variant(v)); }
  CVarRef appendRef(CVarRef v);
  CVarRef appendWithRef(CVarRef v);
  Variant appendOpEqual(int op, CVarRef v);
  Variant pop();
  Variant dequeue();
  void prepend(CVarRef v);

  /**
   * Input/Output
   */
  void serialize(VariableSerializer *serializer,
                 bool isObject = false) const;
  void unserialize(VariableUnserializer *uns);

  // Transfer ownership of our reference to this ArrayData.
  ArrayData* detach() {
    ArrayData* ret = m_px;
    m_px = nullptr;
    return ret;
  }

  void dump();

  ArrayData *getArrayData() const {
    return m_px;
  }

  void setEvalScalar() const;

 private:
  // helpers
  bool compare(CArrRef v2) const;
  static int CompareAsStrings(CVarRef v1, CVarRef v2, const void *data);
  Array &mergeImpl(ArrayData *data, ArrayData::ArrayOp op);
  Array diffImpl(CArrRef array, bool by_key, bool by_value, bool match,
                 PFUNC_CMP key_cmp_function, const void *key_data,
                 PFUNC_CMP value_cmp_function, const void *value_data) const;

  template<typename T>
  Variant &lvalAtImpl(const T &key, ACCESSPARAMS_DECL) {
    if (!m_px) ArrayBase::operator=(ArrayData::Create());
    Variant *ret = nullptr;
    ArrayData *escalated =
      m_px->lval(key, ret, m_px->getCount() > 1,
                 flags & AccessFlags::CheckExist);
    if (escalated != m_px) ArrayBase::operator=(escalated);
    assert(ret);
    return *ret;
  }

  static void compileTimeAssertions() {
    BOOST_STATIC_ASSERT((offsetof(Array, m_px) == kExpectedMPxOffset));
  }
};

///////////////////////////////////////////////////////////////////////////////
// ArrNR

struct ArrNR {
  explicit ArrNR(ArrayData *data = 0) {
    m_px = data;
  }
  ArrNR(const ArrNR &a) {
    m_px = a.m_px;
  }

  operator CArrRef() const { return asArray(); }

  Array& asArray() {
    return *reinterpret_cast<Array*>(this); // XXX
  }

  const Array& asArray() const {
    return const_cast<ArrNR*>(this)->asArray();
  }

  ArrayData* get() const { return m_px; }

protected:
  ArrayData *m_px;
  static void compileTimeAssertions() {
    BOOST_STATIC_ASSERT((offsetof(ArrNR, m_px) == kExpectedMPxOffset));
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_H_
