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

#pragma once

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/tv-variant.h"
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
 * Array type wrapping ArrayData to implement reference counting, copy-on-write
 * and ArrayData escalation.
 *
 * "Escalation" happens when an underlying ArrayData cannot handle an operation
 * and instead it needs to "upgrade" itself to be a more general (but slower)
 * type of ArrayData to accomplish the task.
 */
struct Array {
private:
  using Ptr = req::ptr<ArrayData>;
  using NoIncRef = Ptr::NoIncRef;
  using NonNull = Ptr::NonNull;

  using Flags = AccessFlags;

public:
  /*
   * Create an empty array of a given type.
   */
  static Array CreateVec() {
    return Array(ArrayData::CreateVec(), NoIncRef{});
  }
  static Array CreateDict() {
    return Array(ArrayData::CreateDict(), NoIncRef{});
  }
  static Array CreateKeyset() {
    return Array(ArrayData::CreateKeyset(), NoIncRef{});
  }

  /////////////////////////////////////////////////////////////////////////////

  Array() {}
  ~Array();

  /*
   * Take ownership of `ad'.
   */
  static ALWAYS_INLINE Array attach(ArrayData* ad) {
    return Array(ad, NoIncRef{});
  }

  /*
   * Transfer ownership of our underlying ArrayData.
   */
  ArrayData* detach() { return m_arr.detach(); }

  /*
   * Get or null out the underlying ArrayData pointer.
   */
  ArrayData* get() const { return m_arr.get(); }
  void reset(ArrayData* arr = nullptr) { m_arr.reset(arr); }

  /*
   * Delegate to the underlying ArrayData.
   *
   * Deliberately doesn't throw_null_pointer_exception as a perf optimization.
   */
  ArrayData* operator->() const { return m_arr.get(); }

  /*
   * "Copy" constructors.
   */
  explicit Array(ArrayData* data) : m_arr(data) { }
  Array(const Array& arr) : m_arr(arr.m_arr) { }

  /*
   * Special constructor for use from ArrayInit that creates an Array without a
   * null check and without an inc-ref.
   */
  enum class ArrayInitCtor { Tag };
  explicit Array(ArrayData* ad, ArrayInitCtor)
    : m_arr(ad, NoIncRef{})
  {}

  /*
   * Move constructor.
   */
  Array(Array&& src) noexcept : m_arr(std::move(src.m_arr)) { }

  /*
   * Assignment.
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

  /*
   * Move assignment.
   */
  Array& operator=(Array&& src) {
    m_arr = std::move(src.m_arr);
    return *this;
  }
  Array& operator=(Variant&& v);

  #define COPY_BODY(meth, def)                                          \
    if (!m_arr) return def;                                             \
    auto new_arr = m_arr->meth;                                         \
    return new_arr != m_arr ? Array{new_arr, NoIncRef{}} : Array{*this};

  /*
   * Make a copy of this array.
   *
   * Like the underlying ArrayData::copy operation, the returned Array may
   * point to the same underlying array as the original, or a new one.
   */
  Array toVec() const { COPY_BODY(toVec(true), CreateVec()) }
  Array toDict() const { COPY_BODY(toDict(true), CreateDict()) }
  Array toKeyset() const { COPY_BODY(toKeyset(true), CreateKeyset()) }
  Array toDictIntishCast() const { COPY_BODY(toDictIntishCast(true), Array{}) }

  #undef COPY_BODY

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Nullability.
   */
  bool isNull() const { return !m_arr; }

  /*
   * Size.
   */
  bool empty() const { return !m_arr || m_arr->empty(); }
  ssize_t size() const { return m_arr ? m_arr->size() : 0; }
  ssize_t length() const { return m_arr ? m_arr->size() : 0; }

  /*
   * Array kind.
   */
  bool isVec() const { return m_arr && m_arr->isVecType(); }
  bool isDict() const { return m_arr && m_arr->isDictType(); }
  bool isKeyset() const { return m_arr && m_arr->isKeysetType(); }

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Start iterator.
   *
   * See array-iterator.h for end() and next().
   */
  ArrayIter begin(const String& context = null_string) const;

  /*
   * Converts `k' to a valid key for this array kind.
   */
  template <IntishCast IC = IntishCast::None>
  TypedValue convertKey(TypedValue k) const;
  template <IntishCast IC = IntishCast::None>
  TypedValue convertKey(const Variant& k) const;

  /*
   * Convert the underlying ArrayData to a static copy of itself.
   */
  void setEvalScalar() const;

  /////////////////////////////////////////////////////////////////////////////
  // PHP operations.

  /*
   * PHP array union operator.
   */
  Array  operator+(ArrayData* data) const = delete;
  Array  operator+(const Array& v) const = delete;
  Array  operator+(const Variant& v) const = delete;
  Array& operator+=(ArrayData* data) = delete;
  Array& operator+=(const Array& v) = delete;
  Array& operator+=(const Variant& v) = delete;

  /*
   * Comparison function for array operations.
   */
  using PFUNC_CMP = int (*)(const Variant& v1, const Variant& v2,
                            const void* data);

  /*
   * Return the entries that have keys and/or values that are (intersect), or
   * are not (diff) present in `array'.
   *
   * Keys and values can be compared by user supplied functions and `key_data'
   * or `value_data' will be passed into the corresponding `cmp_function' as
   * the `data' parameter.  Otherwise, equal() will be called for comparisons.
   * If both `by_key' and `by_value' are true, both keys and values have to
   * match to be included (intersect) or excluded (diff).
   */
  Array diff(const Variant& array, bool by_key, bool by_value,
             PFUNC_CMP key_cmp_function = nullptr,
             const void* key_data = nullptr,
             PFUNC_CMP value_cmp_function = nullptr,
             const void* value_data = nullptr) const;
  Array intersect(const Variant& array, bool by_key, bool by_value,
                  PFUNC_CMP key_cmp_function = nullptr,
                  const void* key_data = nullptr,
                  PFUNC_CMP value_cmp_function = nullptr,
                  const void* value_data = nullptr) const;

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
    Variant* original;
    const Array*   array;
    bool           by_key;
    PFUNC_CMP      cmp_func;
    const void*    data;
    std::vector<ssize_t> positions;
  };
  static bool MultiSort(std::vector<SortData>& data);

  static void SortImpl(std::vector<int>& indices, const Array& source,
                       Array::SortData& opaque,
                       Array::PFUNC_CMP cmp_func,
                       bool by_key, const void* data = nullptr);

  /*
   * Type conversions.
   */
  bool toBoolean() const { return m_arr && !m_arr->empty(); }
  int8_t  toByte()  const { return toBoolean() ? 1 : 0; }
  int16_t toInt16() const { return toBoolean() ? 1 : 0; }
  int32_t toInt32() const { return toBoolean() ? 1 : 0; }
  int64_t toInt64() const { return toBoolean() ? 1 : 0; }
  double toDouble() const { return toBoolean() ? 1.0 : 0.0; }
  String toString() const;

  /*
   * Enable the legacy behavior bit on this array
   */
  void setLegacyArray(bool);

  /////////////////////////////////////////////////////////////////////////////
  // Element lookup/lval.

#define FOR_EACH_KEY_TYPE(...)    \
  C(TypedValue, __VA_ARGS__)            \
  I(int, __VA_ARGS__)             \
  I(int64_t, __VA_ARGS__)         \
  V(const String&, __VA_ARGS__)   \
  V(const Variant&, __VA_ARGS__)  \
  D(double, __VA_ARGS__)

  /*
   * Get a refcounted copy of the element at `key'.
   */
  Variant operator[](TypedValue key) const;
  Variant operator[](int key) const;
  Variant operator[](int64_t key) const;
  Variant operator[](const String& key) const;
  Variant operator[](const Variant& key) const;
  Variant operator[](double key) const = delete;
  Variant operator[](const char*) const = delete;

#define C(key_t, name, ret_t, cns) \
  ret_t name(key_t, Flags = Flags::None) cns;
#define V C
#define I V
#define D(key_t, name, ret_t, cns) \
  ret_t name(key_t, Flags = Flags::None) cns = delete;

  /*
   * Get the value of the the element at `key', or an Uninit TypedValue if
   * this key is not present in the array. Does not inc-ref the element.
   */
  FOR_EACH_KEY_TYPE(lookup, TypedValue, const)

  /*
   * Get an lval to the element at `key'.
   *
   * This is ArrayData::lval, with COW and escalation handled internally.
   *
   * lvalForce() has the legacy lval() behavior---if the key is not present,
   * it writes null, then returns the lval.
   */
  FOR_EACH_KEY_TYPE(lval, tv_lval, )
  FOR_EACH_KEY_TYPE(lvalForce, tv_lval, )

#undef D
#undef I
#undef V
#undef C

  /////////////////////////////////////////////////////////////////////////////
  // Element access and mutation.

#define C(key_t, ret_t, name, cns)  ret_t name(key_t, bool isKey = false) cns;
#define V C
#define I(key_t, ret_t, name, cns)  ret_t name(key_t) cns;
#define D(key_t, ret_t, name, cns)  ret_t name(key_t) cns = delete;

  /*
   * Membership.
   */
  FOR_EACH_KEY_TYPE(bool, exists, const)

  /*
   * Remove an element.
   */
  FOR_EACH_KEY_TYPE(void, remove, )

#undef D
#undef I
#undef V
#undef C

#define C(key_t, name, value_t) \
  void name(key_t k, value_t v, bool isKey = false);
#define V C
#define I(key_t, name, value_t) void name(key_t k, value_t v);
#define D(key_t, name, value_t) void name(key_t k, value_t v) = delete;

  /*
   * Set an element to `v'.
   */
  FOR_EACH_KEY_TYPE(set, TypedValue)

#undef D
#undef I
#undef V
#undef C

#define C(key_t, name, value_t)
#define V(key_t, name, value_t) \
  void name(key_t k, value_t v, bool isKey = false);
#define I(key_t, name, value_t) void name(key_t k, value_t v);
#define D(key_t, name, value_t) void name(key_t k, value_t v) = delete;

  /*
   * Variant overloads.
   */
  FOR_EACH_KEY_TYPE(set, const Variant&)

#undef D
#undef I
#undef V
#undef C

  /*
   * Append an element, with semantics like set().
   */
  void append(TypedValue v);
  void append(const Variant& v);

  /*
   * Remove all elements.
   */
  void clear() { operator=(CreateDict()); }

  /*
   * Stack-like function - the inverse of append().
   */
  Variant pop();

#undef FOR_EACH_KEY_TYPE

  /////////////////////////////////////////////////////////////////////////////

private:
  Array(ArrayData* ad, NoIncRef) : m_arr(ad, NoIncRef{}) {}

  Array diffImpl(const Array& array, bool by_key, bool by_value, bool match,
                 PFUNC_CMP key_cmp_function, const void* key_data,
                 PFUNC_CMP value_cmp_function, const void* value_data) const;

  template<typename T> TypedValue lookupImpl(const T& key, Flags) const;
  template<typename T> tv_lval lvalImpl(const T& key, Flags);
  template<typename T> tv_lval lvalForceImpl(const T& key, Flags);

  template<typename T> bool existsImpl(const T& key) const;
  template<typename T> void removeImpl(const T& key);
  template<typename T> void setImpl(const T& key, TypedValue v);

  static void compileTimeAssertions();

  /////////////////////////////////////////////////////////////////////////////

private:
  Ptr m_arr;
};

///////////////////////////////////////////////////////////////////////////////

struct ArrNR {
  explicit ArrNR(ArrayData* data = nullptr) { m_px = data; }
  explicit ArrNR(const ArrayData *ad) : m_px(const_cast<ArrayData*>(ad)) {}

  ArrNR(const ArrNR& a) { m_px = a.m_px; }

  ~ArrNR() {
    if (debug) {
      m_px = reinterpret_cast<ArrayData*>(0xdeadbeeffaceb004);
    }
  }

  ArrayData* get() const { return m_px; }

  operator const Array&() const { return asArray(); }

  Array& asArray() {
    return *reinterpret_cast<Array*>(this); // XXX
  }
  const Array& asArray() const {
    return const_cast<ArrNR*>(this)->asArray();
  }

private:
  static void compileTimeAssertions();

private:
  ArrayData* m_px;
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

ALWAYS_INLINE Array empty_vec_array() {
  return Array::attach(ArrayData::CreateVec());
}

ALWAYS_INLINE Array empty_dict_array() {
  return Array::attach(ArrayData::CreateDict());
}

ALWAYS_INLINE Array empty_keyset() {
  return Array::attach(ArrayData::CreateKeyset());
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Type-pun a referenced ArrayData* as an Array&.
 *
 * This lets us take advantage of Array's CoW and escalation machinery to
 * possibly mutate `ad', without the overhead or behavioral change of
 * refcounting.
 *
 * asArrRef() unconditionally removes Persistent bits from the referenced type.
 */
ALWAYS_INLINE Array& asArrRef(tv_lval tv) {
  assertx(tvIsArrayLike(tv));
  tv.type() = tv.val().parr->toDataType();
  return *reinterpret_cast<Array*>(&val(tv).parr);
}

ALWAYS_INLINE const Array& asCArrRef(tv_rval tv) {
  assertx(tvIsArrayLike(tv));
  return *reinterpret_cast<const Array*>(&val(tv).parr);
}

///////////////////////////////////////////////////////////////////////////////
}
