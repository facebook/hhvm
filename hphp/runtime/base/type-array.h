/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/types.h"

#include <algorithm>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ArrayIter;
struct VariableUnserializer;

////////////////////////////////////////////////////////////////////////////////

enum class AccessFlags {
  None     = 0,
  Error    = 1,
  Key      = 2,
  ErrorKey = Error | Key,
};

inline AccessFlags operator&(AccessFlags a, AccessFlags b) {
  return static_cast<AccessFlags>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr bool any(AccessFlags a) {
  return a != AccessFlags::None;
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Array type wrapping around ArrayData to implement reference
 * counting, copy-on-write and ArrayData escalation.
 *
 * "Escalation" happens when an underlying ArrayData cannot handle an operation
 * and instead it needs to "upgrade" itself to be a more general (but slower)
 * type of ArrayData to accomplish the task. This "upgrade" is called
 * escalation.
 */
struct Array {
private:
  using Ptr = req::ptr<ArrayData>;
  using NoIncRef = Ptr::NoIncRef;
  using NonNull = Ptr::NonNull;

  using Flags = AccessFlags;

  Ptr m_arr;

  Array(ArrayData* ad, NoIncRef) : m_arr(ad, NoIncRef{}) {}

public:
  /*
   * Create an empty array or an array with one element. Note these are
   * different than those copying constructors that also take one value.
   */
  static Array Create() {
    return Array(ArrayData::Create(), NoIncRef{});
  }

  static Array CreateVec() {
    return Array(ArrayData::CreateVec(), NoIncRef{});
  }

  static Array CreateDict() {
    return Array(ArrayData::CreateDict(), NoIncRef{});
  }

  static Array CreateKeyset() {
    return Array(ArrayData::CreateKeyset(), NoIncRef{});
  }

  static Array Create(const Variant& value) {
    return Array(ArrayData::Create(value), NoIncRef{});
  }

  static Array Create(const Variant& key, const Variant& value);

public:
  Array() {}
  ~Array();

  // Take ownership of this ArrayData.
  static ALWAYS_INLINE Array attach(ArrayData* ad) {
    return Array(ad, NoIncRef{});
  }

  // Transfer ownership of our reference to this ArrayData.
  ArrayData* detach() { return m_arr.detach(); }

  ArrayData* get() const { return m_arr.get(); }
  void reset(ArrayData* arr = nullptr) { m_arr.reset(arr); }

  // Deliberately doesn't throw_null_pointer_exception as a perf
  // optimization.
  ArrayData* operator->() const { return m_arr.get(); }

  void escalate();

  #define COPY_BODY(meth, def)                                          \
    if (!m_arr) return def;                                             \
    auto new_arr = m_arr->meth;                                         \
    return new_arr != m_arr ? Array{new_arr, NoIncRef{}} : Array{*this};

  // Make a copy of this array. Like the underlying ArrayData::copy operation,
  // the returned Array may point to the same underlying array as the original,
  // or a new one.
  Array copy() const { COPY_BODY(copy(), Array{}) }
  Array toVec() const { COPY_BODY(toVec(true), CreateVec()) }
  Array toDict() const { COPY_BODY(toDict(true), CreateDict()) }
  Array toKeyset() const { COPY_BODY(toKeyset(true), CreateKeyset()) }
  Array toPHPArray() const { COPY_BODY(toPHPArray(true), Array{}) }

  #undef COPY_BODY

  /*
   * Constructors. Those that take "arr" or "var" are copy constructors, taking
   * array value from the parameter, and they are NOT constructing an array
   * with that single value (then one should use Array::Create() functions).
   */
  explicit Array(ArrayData* data) : m_arr(data) { }
  /* implicit */ Array(const Array& arr) : m_arr(arr.m_arr) { }

  /*
   * Special constructor for use from ArrayInit that creates an Array without a
   * null check and without an inc-ref.
   */
  enum class ArrayInitCtor { Tag };
  explicit Array(ArrayData* ad, ArrayInitCtor)
    : m_arr(ad, NoIncRef{})
  {}

  // Move ctor
  Array(Array&& src) noexcept : m_arr(std::move(src.m_arr)) { }

  // Move assign
  Array& operator=(Array&& src) {
    m_arr = std::move(src.m_arr);
    return *this;
  }

  /*
   * Informational
   */
  bool empty() const {
    return !m_arr || m_arr->empty();
  }
  ssize_t size() const {
    return m_arr ? m_arr->size() : 0;
  }
  ssize_t length() const {
    return m_arr ? m_arr->size() : 0;
  }
  bool isNull() const {
    return !m_arr;
  }
  Array values() const;

  bool isVecArray() const { return m_arr && m_arr->isVecArray(); }
  bool isDict() const { return m_arr && m_arr->isDict(); }
  bool isKeyset() const { return m_arr && m_arr->isKeyset(); }
  bool isHackArray() const { return m_arr && m_arr->isHackArray(); }
  bool isPHPArray() const { return !m_arr || m_arr->isPHPArray(); }

  bool useWeakKeys() const {
    // If array isn't set we may implicitly create a mixed array. We never
    // implicitly create a dict array or vec.
    return !m_arr || m_arr->useWeakKeys();
  }

  /*
   * Converts k to a valid key for this array type
   */
  VarNR convertKey(const Variant& k) const;

  /*
   * Operators
   */
  Array& operator=(ArrayData* data) {
    m_arr = data;
    return *this;
  }
  Array& operator=(const Array& arr) {
    m_arr = arr.m_arr;
    return *this;
  }
  Array& operator=(const Variant& v);
  Array  operator+(ArrayData* data) const;
  Array  operator+(const Array& v) const;
  Array  operator+(const Variant& v) const = delete;
  Array& operator+=(ArrayData* data);
  Array& operator+=(const Array& v);
  Array& operator+=(const Variant& v);

  // Move assignment
  Array& operator=(Variant&& v);

  /*
   * Returns the entries that have keys and/or values that are not present in
   * specified array. Keys and values can be compared by user supplied
   * functions and key_data or value_data will be passed into PFUNC_CMP as
   * "data" parameter. Otherwise, equal() will be called for comparisons. If
   * both by_key and by_value, both keys and values have to match to be
   * excluded.
   */
  typedef int (*PFUNC_CMP)(const Variant& v1, const Variant& v2, const void* data);
  Array diff(const Variant& array, bool by_key, bool by_value,
             PFUNC_CMP key_cmp_function = nullptr,
             const void* key_data = nullptr,
             PFUNC_CMP value_cmp_function = nullptr,
             const void* value_data = nullptr) const;

  /*
   * Returns the entries that have keys and/or values that are present in
   * specified array. Keys and values can be compared by user supplied
   * functions and key_data or value_data will be passed into PFUNC_CMP as
   * "data" parameter. Otherwise, equal() will be called for comparisons. If
   * both by_key and by_value, both keys and values have to match to be
   * included.
   */
  Array intersect(const Variant& array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function = nullptr,
                  const void* key_data = nullptr,
                  PFUNC_CMP value_cmp_function = nullptr,
                  const void* value_data = nullptr) const;

  /*
   * Iterator functions. See array-iterator.h for end() and next().
   */
  ArrayIter begin(const String& context = null_string) const;

  /*
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
  Array& merge(const Array& arr);

  /*
   * Sorting.
   */
  static int SortRegularAscending(const Variant& v1, const Variant& v2,
                                  const void* data);
  static int SortNumericAscending(const Variant& v1, const Variant& v2,
                                  const void* data);
  static int SortStringAscending(const Variant& v1, const Variant& v2,
                                 const void* data);
  static int SortStringAscendingCase(const Variant& v1, const Variant& v2,
                                     const void* data);
  static int SortLocaleStringAscending(const Variant& v1, const Variant& v2,
                                       const void* data);

  static int SortRegularDescending(const Variant& v1, const Variant& v2,
                                   const void* data);
  static int SortNumericDescending(const Variant& v1, const Variant& v2,
                                   const void* data);
  static int SortStringDescending(const Variant& v1, const Variant& v2,
                                  const void* data);
  static int SortStringDescendingCase(const Variant& v1, const Variant& v2,
                                      const void* data);
  static int SortLocaleStringDescending(const Variant& v1, const Variant& v2,
                                        const void* data);

  static int SortNaturalAscending(const Variant& v1, const Variant& v2,
                                  const void* data);
  static int SortNaturalDescending(const Variant& v1, const Variant& v2,
                                   const void* data);
  static int SortNaturalCaseAscending(const Variant& v1, const Variant& v2,
                                      const void* data);
  static int SortNaturalCaseDescending(const Variant& v1, const Variant& v2,
                                       const void* data);

  void sort(PFUNC_CMP cmp_func, bool by_key, bool renumber,
            const void* data = nullptr);

  /*
   * Sort multiple arrays at once similar to how ORDER BY clause works in SQL.
   */
  struct SortData {
    Variant*     original;
    const Array* array;
    bool         by_key;
    PFUNC_CMP    cmp_func;
    const void*  data;
    std::vector<ssize_t> positions;
  };
  static bool MultiSort(std::vector<SortData>& data, bool renumber);

  static void SortImpl(std::vector<int>& indices, const Array& source,
                       Array::SortData& opaque,
                       Array::PFUNC_CMP cmp_func,
                       bool by_key, const void* data = nullptr);

  /*
   * Type conversions
   */
  bool    toBoolean() const { return  m_arr && !m_arr->empty(); }
  char    toByte   () const { return (m_arr && !m_arr->empty()) ? 1 : 0; }
  short   toInt16  () const { return (m_arr && !m_arr->empty()) ? 1 : 0; }
  int     toInt32  () const { return (m_arr && !m_arr->empty()) ? 1 : 0; }
  int64_t toInt64  () const { return (m_arr && !m_arr->empty()) ? 1 : 0; }
  double  toDouble () const { return (m_arr && !m_arr->empty()) ? 1.0 : 0.0; }
  String  toString () const;

  /*
   * Comparisons
   */
  bool same (const Array& v2) const;
  bool same (const Object& v2) const;
  bool equal(const Array& v2) const;
  bool equal(const Object& v2) const;
  bool less (const Array& v2, bool flip = false) const;
  bool less (const Object& v2) const;

  bool less (const Variant& v2) const;
  bool more (const Array& v2, bool flip = true) const;
  bool more (const Object& v2) const;
  bool more (const Variant& v2) const;
  int compare (const Array& v2, bool flip = false) const;

  /*
   * Offset
   */
  Variant rvalAt(int     key, Flags = Flags::None) const;
  Variant rvalAt(int64_t key, Flags = Flags::None) const;
  Variant rvalAt(double  key, Flags = Flags::None) const = delete;
  Variant rvalAt(const String& key, Flags = Flags::None) const;
  Variant rvalAt(const Variant& key, Flags = Flags::None) const;

  /*
   * To get offset for temporary usage
   */
  const Variant& rvalAtRef(int     key, Flags = Flags::None) const;
  const Variant& rvalAtRef(int64_t key, Flags = Flags::None) const;
  const Variant& rvalAtRef(double  key, Flags = Flags::None) const = delete;
  const Variant& rvalAtRef(const Variant& key, Flags = Flags::None) const;
  const Variant& rvalAtRef(const String& key, Flags = Flags::None) const;

  const Variant operator[](int     key) const;
  const Variant operator[](int64_t key) const;
  const Variant operator[](double  key) const = delete;
  const Variant operator[](const String& key) const;
  const Variant operator[](const Variant& key) const;
  const Variant operator[](const char*) const = delete; // use const String&

  /*
   * Get an lval reference to a newly created element, with the intent
   * of reading or writing to it as a Cell.
   */
  Variant& lvalAt();

  /*
   * Get an lval reference to a newly created element, with the intent
   * of using binding assignment with the newly created element.
   */
  Variant& lvalAtRef();

  /*
   * Get an lval reference to an element.
   */
  Variant& lvalAt(int key, Flags flags = Flags::None) {
    return lvalAtImpl(key, flags);
  }
  Variant& lvalAt(int64_t key, Flags flags = Flags::None) {
    return lvalAtImpl(key, flags);
  }
  Variant& lvalAt(double key, Flags = Flags::None) = delete;
  Variant& lvalAt(const String& key, Flags = Flags::None);
  Variant& lvalAt(const Variant& key, Flags = Flags::None);

  Variant& lvalAtRef(int key, Flags flags = Flags::None) {
    return lvalAtRefImpl(key, flags);
  }
  Variant& lvalAtRef(int64_t key, Flags flags = Flags::None) {
    return lvalAtRefImpl(key, flags);
  }
  Variant& lvalAtRef(double key, Flags = Flags::None) = delete;
  Variant& lvalAtRef(const String& key, Flags = Flags::None);
  Variant& lvalAtRef(const Variant& key, Flags = Flags::None);

  /*
   * Set an element to a value.
   */
  void set(int     key, const Variant& v) { set(int64_t(key), v); }
  void set(int64_t key, const Variant& v);
  void set(double  key, const Variant& v) = delete;
  void set(const String& key, const Variant& v, bool isKey = false);
  void set(const Variant& key, const Variant& v, bool isKey = false);

  /*
   * Set an element to a reference.
   */
  void setRef(int     key, Variant& v) { setRef(int64_t(key), v); }
  void setRef(int64_t key, Variant& v);
  void setRef(double  key, Variant& v) = delete;
  void setRef(const String& key, Variant& v, bool isKey = false);
  void setRef(const Variant& key, Variant& v, bool isKey = false);

  void setWithRef(const Variant& key, const Variant& v, bool isKey = false);

  /*
   * Add an element.
   */
  void add(int     key, const Variant& v) { add(int64_t(key), v); }
  void add(int64_t key, const Variant& v);
  void add(double  key, const Variant& v) = delete;
  void add(const String& key, const Variant& v, bool isKey = false);
  void add(const Variant& key, const Variant& v, bool isKey = false);

  /*
   * Membership functions.
   */
  bool exists(char    key) const { return existsImpl((int64_t)key); }
  bool exists(short   key) const { return existsImpl((int64_t)key); }
  bool exists(int     key) const { return existsImpl((int64_t)key); }
  bool exists(int64_t key) const { return existsImpl(key); }
  bool exists(double  key) const = delete;
  bool exists(const String& key, bool isKey = false) const;
  bool exists(const Variant& key, bool isKey = false) const;

  /*
   * Remove an element.
   */
  void remove(char    key) { removeImpl((int64_t)key); }
  void remove(short   key) { removeImpl((int64_t)key); }
  void remove(int     key) { removeImpl((int64_t)key); }
  void remove(int64_t key) { removeImpl(key); }
  void remove(double  key) = delete;
  void remove(const String& key, bool isString = false);
  void remove(const Variant& key);

  /*
   * Remove all elements.
   */
  void clear() { operator=(Create()); }

  /*
   * Append an element.
   */
  const Variant& append(const Variant& v);
  const Variant& appendRef(Variant& v);
  const Variant& appendWithRef(const Variant& v);

  /*
   * Stack/queue-like functions.
   */
  Variant pop();
  Variant dequeue();
  void prepend(const Variant& v);

  void setEvalScalar() const;

 private:
  Array& plusImpl(ArrayData* data);
  Array& mergeImpl(ArrayData* data);
  Array diffImpl(const Array& array, bool by_key, bool by_value, bool match,
                 PFUNC_CMP key_cmp_function, const void* key_data,
                 PFUNC_CMP value_cmp_function, const void* value_data) const;

  template<typename T> void setImpl(const T& key, const Variant& v);
  template<typename T> void setRefImpl(const T& key, Variant& v);
  template<typename T> void addImpl(const T& key, const Variant& v);

  template<typename T>
  bool existsImpl(const T& key) const {
    if (m_arr) return m_arr->exists(key);
    return false;
  }

  template<typename T>
  void removeImpl(const T& key) {
    if (m_arr) {
      ArrayData* escalated = m_arr->remove(key, m_arr->cowCheck());
      if (escalated != m_arr) m_arr = Ptr::attach(escalated);
    }
  }

  template<typename T>
  Variant& lvalAtImpl(const T& key, Flags = Flags::None) {
    if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
    auto const lval = m_arr->lval(key, m_arr->cowCheck());
    if (lval.arr_base() != m_arr) m_arr = Ptr::attach(lval.arr_base());
    assert(lval.tv());
    return reinterpret_cast<Variant&>(*lval.tv());
  }

  template<typename T>
  Variant& lvalAtRefImpl(const T& key, Flags = Flags::None) {
    if (!m_arr) m_arr = Ptr::attach(ArrayData::Create());
    auto const lval = m_arr->lvalRef(key, m_arr->cowCheck());
    if (lval.arr_base() != m_arr) m_arr = Ptr::attach(lval.arr_base());
    assert(lval.tv());
    return reinterpret_cast<Variant&>(*lval.tv());
  }

  static void compileTimeAssertions();
};

///////////////////////////////////////////////////////////////////////////////
// ArrNR

struct ArrNR {
  explicit ArrNR(ArrayData* data = nullptr) {
    m_px = data;
  }

  ArrNR(const ArrNR& a) {
    m_px = a.m_px;
  }

  ~ArrNR() {
    if (debug) {
      m_px = reinterpret_cast<ArrayData*>(0xdeadbeeffaceb004);
    }
  }

  operator const Array&() const { return asArray(); }

  Array& asArray() {
    return *reinterpret_cast<Array*>(this); // XXX
  }

  const Array& asArray() const {
    return const_cast<ArrNR*>(this)->asArray();
  }

  ArrayData* get() const { return m_px; }

protected:
  ArrayData* m_px;

private:
  static void compileTimeAssertions();
};

///////////////////////////////////////////////////////////////////////////////

/*
 * An Array wrapper that doesn't run a destructor unless you
 * explicitly ask it to.
 *
 * This is used for the dynPropTable in ExecutionContext, so that at
 * sweep time we don't run these destructors.
 */
struct ArrayNoDtor {
  /* implicit */ ArrayNoDtor(ArrayData* table) { new (&m_arr) Array(table); }
  ArrayNoDtor() { new (&m_arr) Array(); }
  ArrayNoDtor(ArrayNoDtor&& o) noexcept {
    new (&m_arr) Array(std::move(o.m_arr));
  }
  ~ArrayNoDtor() {}
  Array& arr() { return m_arr; }
  const Array& arr() const { return m_arr; }
  void destroy() { m_arr.~Array(); }
private:
  union { Array m_arr; };
};

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE Array empty_array() {
  return Array::attach(staticEmptyArray());
}

ALWAYS_INLINE Array empty_vec_array() {
  return Array::attach(staticEmptyVecArray());
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
