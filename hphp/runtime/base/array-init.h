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
#ifndef incl_HPHP_ARRAY_INIT_H_
#define incl_HPHP_ARRAY_INIT_H_

#include <type_traits>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Flag indicating whether an array allocation should be pre-checked for OOM.
 */
enum class CheckAllocation {};

/*
 * Base class for ArrayInits specialized on array kind.
 *
 * Takes two template parameters:
 *  - TArray is a bag of static class functions, such as MixedArray.  See the
 *    `detail' namespace below for the requirements.
 *  - DT is the DataType for the arrays created by the ArrayInit.
 */
template<typename TArray, DataType DT>
struct ArrayInitBase {
  explicit ArrayInitBase(size_t n)
    : m_arr(TArray::MakeReserve(n))
#ifdef DEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assert(m_arr->hasExactlyOneRef());
  }

  ArrayInitBase(ArrayInitBase&& other) noexcept
    : m_arr(other.m_arr)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assert(!m_arr || m_arr->toDataType() == DT);
    other.m_arr = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  ArrayInitBase(const ArrayInitBase&) = delete;
  ArrayInitBase& operator=(const ArrayInitBase&) = delete;

  ~ArrayInitBase() {
    // In case an exception interrupts the initialization.
    assert(!m_arr || (m_arr->hasExactlyOneRef() &&
                      m_arr->toDataType() == DT));
    if (m_arr) TArray::Release(m_arr);
  }

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Finish routines.
   *
   * These all invalidate the ArrayInit and return the initialized array.
   */

  Variant toVariant() {
    assert(m_arr->hasExactlyOneRef());
    assert(m_arr->toDataType() == DT);
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, DT, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assert(m_arr->hasExactlyOneRef());
    assert(m_arr->toDataType() == DT);
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assert(m_arr->hasExactlyOneRef());
    assert(m_arr->toDataType() == DT);
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ptr;
  }

  /////////////////////////////////////////////////////////////////////////////

protected:
  /*
   * Checked-allocation constructor.
   *
   * Only used by the constructors of derived classes.
   */
  ArrayInitBase(size_t n, CheckAllocation)
#ifdef DEBUG
    : m_addCount(0)
    , m_expectedCount(n)
#endif
  {}

  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved initializations.
    assert(newp == m_arr);
    // You cannot add/set more times than you reserved with ArrayInit.
    assert(++m_addCount <= m_expectedCount);
  }

protected:
  ArrayData* m_arr;
#ifdef DEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Dummy MixedArray-like bags of statics for Hack arrays.
 */
namespace detail {

struct VecArray {
  static constexpr auto MakeReserve = &PackedArray::MakeReserveVec;
  static constexpr auto Release = PackedArray::ReleaseVec;
};

struct DictArray {
  static constexpr auto MakeReserve = &MixedArray::MakeReserveDict;
  static constexpr auto Release = MixedArray::ReleaseDict;
};

}

///////////////////////////////////////////////////////////////////////////////


/*
 * Initializer for a MixedArray.
 */
struct ArrayInit : ArrayInitBase<MixedArray, KindOfArray> {
  enum class Map {};
  // This is the same as map right now, but is here for documentation
  // so we can find them later.
  using Mixed = Map;

  /*
   * When you create an ArrayInit, you must specify the "kind" of
   * array you are creating, for performance reasons.  "Kinds" that
   * are relevant to know about for extension code:
   *
   *   Packed -- a vector-like array: don't use ArrayInit, use PackedArrayInit
   *   Map    -- you expect only string keys and any value type
   *   Mixed  -- you expect either integer keys, mixed keys
   *
   * Also, generally it's preferable to use make_map_array or
   * make_packed_array when it's easy, since you don't have to get 'n'
   * right in that case.
   *
   * For large array allocations, consider passing CheckAllocation, which will
   * throw if the allocation would OOM the request.
   */
  ArrayInit(size_t n, Map) : ArrayInitBase(n) {}
  ArrayInit(size_t n, Map, CheckAllocation);

  ArrayInit(ArrayInit&& o) noexcept : ArrayInitBase(std::move(o)) {}

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Call append() on the underlying array.
   */
  ArrayInit& append(TypedValue tv) {
    performOp([&]{
      return MixedArray::Append(m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  ArrayInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  /*
   * Call set() on the underlying ArrayData.
   */
  ArrayInit& set(int64_t name, TypedValue tv) {
    performOp([&]{
      return MixedArray::SetInt(m_arr, name, tvToInitCell(tv), false);
    });
    return *this;
  }
  ArrayInit& set(const String& name, TypedValue tv) {
    performOp([&]{
      return MixedArray::SetStr(m_arr, name.get(), tvToInitCell(tv), false);
    });
    return *this;
  }
  template<class T>
  ArrayInit& set(const T& name, TypedValue tv) {
    performOp([&]{ return m_arr->set(name, tvToInitCell(tv), false); });
    return *this;
  }

#define IMPL_SET(KeyType)                           \
  ArrayInit& set(KeyType name, const Variant& v) {  \
    return set(name, *v.asTypedValue());            \
  }

  IMPL_SET(int64_t)
  IMPL_SET(const String&)
  template<typename T> IMPL_SET(const T&)

  ArrayInit& set(const Variant& name, const Variant& v) = delete;

#undef IMPL_SET

  /*
   * Same as set(), but for the deleted double `const Variant&' overload.
   */
  ArrayInit& setValidKey(TypedValue name, TypedValue v) {
    performOp([&]{
      return m_arr->set(tvToInitCell(name), tvToInitCell(v), false);
    });
    return *this;
  }
  ArrayInit& setValidKey(const Variant& name, const Variant& v) {
    return setValidKey(*name.asTypedValue(), *v.asTypedValue());
  }

  /*
   * This function is deprecated and exists for backward compatibility with the
   * ArrayInit API.
   *
   * Generally you should be able to figure out if your key is a pure string
   * (not-integer-like) or not when using ArrayInit, and if not you should
   * probably use toKey yourself.
   */
  ArrayInit& setUnknownKey(const Variant& name, const Variant& v) {
    auto const k = name.toKey(m_arr).tv();
    if (LIKELY(!isNullType(k.m_type))) {
      performOp([&]{ return m_arr->set(k, v.asInitCellTmp(), false); });
    }
    return *this;
  }

  /*
   * Call add() on the underlying array.
   */
  ArrayInit& add(int64_t name, TypedValue tv, bool /*keyConverted*/ = false) {
    performOp([&]{
      return MixedArray::AddInt(m_arr, name, tvToInitCell(tv), false);
    });
    return *this;
  }

  ArrayInit& add(const String& name, TypedValue tv,
                 bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{
        return MixedArray::AddStr(m_arr, name.get(), tvToInitCell(tv), false);
      });
    } else if (!name.isNull()) {
      performOp([&]{
        return m_arr->add(VarNR::MakeKey(name).tv(), tvToInitCell(tv), false);
      });
    }
    return *this;
  }

  ArrayInit& add(const Variant& name, TypedValue tv,
                 bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{
        return m_arr->add(name.asInitCellTmp(), tvToInitCell(tv), false);
      });
    } else {
      auto const k = name.toKey(m_arr).tv();
      if (!isNullType(k.m_type)) {
        performOp([&]{ return m_arr->add(k, tvToInitCell(tv), false); });
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit& add(const T& name, TypedValue tv,
                 bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_arr->add(name, tvToInitCell(tv), false); });
    } else {
      auto const k = Variant(name).toKey(m_arr).tv();
      if (!isNullType(k.m_type)) {
        performOp([&]{ return m_arr->add(k, tvToInitCell(tv), false); });
      }
    }
    return *this;
  }

#define IMPL_ADD(KeyType)                               \
  ArrayInit& add(KeyType name, const Variant& v,        \
                 bool keyConverted = false) {           \
    return add(name, *v.asTypedValue(), keyConverted);  \
  }

  IMPL_ADD(int64_t)
  IMPL_ADD(const String&)
  IMPL_ADD(const Variant&)
  template<typename T> IMPL_ADD(const T&)

#undef IMPL_ADD

  /*
   * Call appendRef() on the underlying array.
   */
  ArrayInit& appendRef(Variant& v) {
    performOp([&]{ return MixedArray::AppendRef(m_arr, v, false); });
    return *this;
  }

  /*
   * Call setRef() on the underlying array.
   */
  ArrayInit& setRef(int64_t name, Variant& v, bool /*keyConverted*/ = false) {
    performOp([&]{ return MixedArray::SetRefInt(m_arr, name, v, false); });
    return *this;
  }

  ArrayInit& setRef(const String& name, Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{
        return MixedArray::SetRefStr(m_arr, name.get(), v, false);
      });
    } else {
      performOp([&]{
        return m_arr->setRef(VarNR::MakeKey(name).tv(), v, false);
      });
    }
    return *this;
  }

  ArrayInit& setRef(TypedValue name, Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_arr->setRef(tvToCell(name), v, false); });
    } else {
      auto const k = tvToKey(name, m_arr);
      if (!isNullType(k.m_type)) {
        performOp([&]{ return m_arr->setRef(k, v, false); });
      }
    }
    return *this;
  }
  ArrayInit& setRef(const Variant& name, Variant& v,
                    bool keyConverted = false) {
    return setRef(*name.asTypedValue(), v, keyConverted);
  }

  template<typename T>
  ArrayInit& setRef(const T& name, Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_arr->setRef(name, v, false); });
    } else {
      auto const k = Variant(name).toKey(m_arr).tv();
      if (!isNullType(k.m_type)) {
        performOp([&]{ return m_arr->setRef(k, v, false); });
      }
    }
    return *this;
  }
};

struct MixedArrayInit : ArrayInit {
  explicit MixedArrayInit(size_t n) : ArrayInit(n, Map{}) {}
  MixedArrayInit(size_t n, CheckAllocation c) : ArrayInit(n, Map{}, c) {}
  MixedArrayInit(MixedArrayInit&& o) noexcept : ArrayInit(std::move(o)) {}
};

///////////////////////////////////////////////////////////////////////////////

struct DictInit : ArrayInitBase<detail::DictArray, KindOfDict> {
  using ArrayInitBase<detail::DictArray, KindOfDict>::ArrayInitBase;

  /*
   * For large array allocations, consider passing CheckAllocation, which will
   * throw if the allocation would OOM the request.
   */
  DictInit(size_t n, CheckAllocation);

  /////////////////////////////////////////////////////////////////////////////

  DictInit& append(TypedValue tv) {
    performOp([&]{
      return MixedArray::AppendDict(m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  DictInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  DictInit& set(int64_t name, TypedValue tv) {
    performOp([&]{
      return MixedArray::SetIntDict(m_arr, name, tvToInitCell(tv), false);
    });
    return *this;
  }
  DictInit& set(StringData* name, TypedValue tv) {
    performOp([&]{
      return MixedArray::SetStrDict(m_arr, name, tvToInitCell(tv), false);
    });
    return *this;
  }
  DictInit& set(const String& name, TypedValue tv) {
    performOp([&]{
      return MixedArray::SetStrDict(m_arr, name.get(), tvToInitCell(tv), false);
    });
    return *this;
  }

#define IMPL_SET(KeyType)                         \
  DictInit& set(KeyType name, const Variant& v) { \
    return set(name, *v.asTypedValue());          \
  }

  IMPL_SET(int64_t)
  IMPL_SET(StringData*)
  IMPL_SET(const String&)

  DictInit& set(const char*, TypedValue tv) = delete;
  DictInit& set(const char*, const Variant& v) = delete;
  DictInit& set(const Variant& name, const Variant& v) = delete;

#undef IMPL_SET

  DictInit& setValidKey(TypedValue name, TypedValue v) {
    performOp([&]{
      auto const k = tvToCell(name);
      assert(isIntType(k.m_type) || isStringType(k.m_type));

      return isIntType(k.m_type)
        ? MixedArray::SetIntDict(m_arr, k.m_data.num, tvToInitCell(v), false)
        : MixedArray::SetStrDict(m_arr, k.m_data.pstr, tvToInitCell(v), false);
    });
    return *this;
  }
  DictInit& setValidKey(const Variant& name, const Variant& v) {
    return setValidKey(*name.asTypedValue(), *v.asTypedValue());
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Base initializer for Packed-layout arrays.
 */
template<typename TArray, DataType DT>
struct PackedArrayInitBase : ArrayInitBase<TArray, DT> {
  using ArrayInitBase<TArray, DT>::ArrayInitBase;

  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws: RequestMemoryExceededException if allocating would OOM.
   */
  PackedArrayInitBase(size_t n, CheckAllocation) :
    ArrayInitBase<TArray, DT>(n, CheckAllocation{})
  {
    auto allocsz = sizeof(ArrayData) + sizeof(TypedValue) * n;
    if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
      check_non_safepoint_surprise();
    }
    this->m_arr = TArray::MakeReserve(n);
    assert(this->m_arr->hasExactlyOneRef());
    check_non_safepoint_surprise();
  }
};

/*
 * Initializer for a PHP vector-shaped array.
 */
struct PackedArrayInit : PackedArrayInitBase<PackedArray, KindOfArray> {
  using PackedArrayInitBase<PackedArray, KindOfArray>::PackedArrayInitBase;

  PackedArrayInit& append(TypedValue tv) {
    performOp([&]{
      return PackedArray::Append(m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  PackedArrayInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  PackedArrayInit& appendRef(Variant& v) {
    performOp([&]{ return PackedArray::AppendRef(m_arr, v, false); });
    return *this;
  }

  PackedArrayInit& appendWithRef(TypedValue v) {
    performOp([&]{ return PackedArray::AppendWithRef(m_arr, v, false); });
    return *this;
  }
  PackedArrayInit& appendWithRef(const Variant& v) {
    return appendWithRef(*v.asTypedValue());
  }
};

/*
 * Initializer for a Hack vector array.
 */
struct VecArrayInit : PackedArrayInitBase<detail::VecArray, KindOfVec> {
  using PackedArrayInitBase<detail::VecArray, KindOfVec>::PackedArrayInitBase;

  VecArrayInit& append(TypedValue tv) {
    performOp([&]{
      return PackedArray::AppendVec(m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  VecArrayInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Initializer for a Hack keyset.
 */
struct KeysetInit : ArrayInitBase<SetArray, KindOfKeyset> {
  using ArrayInitBase<SetArray, KindOfKeyset>::ArrayInitBase;

  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws RequestMemoryExceededException if allocating would OOM.
   */
  KeysetInit(size_t n, CheckAllocation);

  KeysetInit& add(int64_t v) {
    performOp([&]{ return SetArray::AddToSet(m_arr, v, false); });
    return *this;
  }
  KeysetInit& add(StringData* v) {
    performOp([&]{ return SetArray::AddToSet(m_arr, v, false); });
    return *this;
  }
  KeysetInit& add(TypedValue tv) {
    performOp([&]{
      return SetArray::Append(m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  KeysetInit& add(const Variant& v) {
    return add(*v.asTypedValue());
  }
};

///////////////////////////////////////////////////////////////////////////////

namespace make_array_detail {

  inline void packed_impl(PackedArrayInit&) {}

  template<class Val, class... Vals>
  void packed_impl(PackedArrayInit& init, Val&& val, Vals&&... vals) {
    init.append(Variant(std::forward<Val>(val)));
    packed_impl(init, std::forward<Vals>(vals)...);
  }

  inline void vec_impl(VecArrayInit&) {}

  template<class Val, class... Vals>
  void vec_impl(VecArrayInit& init, Val&& val, Vals&&... vals) {
    init.append(Variant(std::forward<Val>(val)));
    vec_impl(init, std::forward<Vals>(vals)...);
  }

  inline String init_key(const char* s) { return String(s); }
  inline int64_t init_key(int k) { return k; }
  inline int64_t init_key(int64_t k) { return k; }
  inline const String& init_key(const String& k) { return k; }
  inline const String init_key(StringData* k) { return String{k}; }

  inline void map_impl(ArrayInit&) {}

  template<class Key, class Val, class... KVPairs>
  void map_impl(ArrayInit& init, Key&& key, Val&& val, KVPairs&&... kvpairs) {
    init.set(init_key(std::forward<Key>(key)), Variant(std::forward<Val>(val)));
    map_impl(init, std::forward<KVPairs>(kvpairs)...);
  }

  inline String dict_init_key(const char* s) { return String(s); }
  inline int64_t dict_init_key(int k) { return k; }
  inline int64_t dict_init_key(int64_t k) { return k; }
  inline StringData* dict_init_key(const String& k) { return k.get(); }
  inline StringData* dict_init_key(StringData* k) { return k; }

  inline void dict_impl(DictInit&) {}

  template<class Key, class Val, class... KVPairs>
  void dict_impl(DictInit& init, Key&& key, Val&& val, KVPairs&&... kvpairs) {
    init.set(dict_init_key(std::forward<Key>(key)),
             Variant(std::forward<Val>(val)));
    dict_impl(init, std::forward<KVPairs>(kvpairs)...);
  }

  inline String keyset_init_key(const char* s) { return String(s); }
  inline int64_t keyset_init_key(int k) { return k; }
  inline int64_t keyset_init_key(int64_t k) { return k; }
  inline StringData* keyset_init_key(const String& k) { return k.get(); }
  inline StringData* keyset_init_key(StringData* k) { return k; }

  inline void keyset_impl(KeysetInit&) {}

  template<class Val, class... Vals>
  void keyset_impl(KeysetInit& init, Val&& val, Vals&&... vals) {
    init.add(keyset_init_key(std::forward<Val>(val)));
    keyset_impl(init, std::forward<Vals>(vals)...);
  }

}

/*
 * Helper for creating packed arrays (vector-like) that don't contain
 * references.
 *
 * Usage:
 *
 *   auto newArray = make_packed_array(1, 2, 3, 4);
 *
 * If you need to deal with references, you currently have to use
 * PackedArrayInit directly.
 */
template<class... Vals>
Array make_packed_array(Vals&&... vals) {
  static_assert(sizeof...(vals), "use Array::Create() instead");
  PackedArrayInit init(sizeof...(vals));
  make_array_detail::packed_impl(init, std::forward<Vals>(vals)...);
  return init.toArray();
}

/*
 * Helper for creating Hack vec arrays (vector-like). Vec arrays can't contain
 * references.
 *
 * Usage:
 *
 *   auto newArray = make_vec_array(1, 2, 3, 4);
 */
template<class... Vals>
Array make_vec_array(Vals&&... vals) {
  static_assert(sizeof...(vals), "use Array::CreateVec() instead");
  VecArrayInit init(sizeof...(vals));
  make_array_detail::vec_impl(init, std::forward<Vals>(vals)...);
  return init.toArray();
}

/*
 * Helper for creating map-like arrays (kMixedKind).  Takes pairs of
 * arguments for the keys and values.
 *
 * Usage:
 *
 *   auto newArray = make_map_array(keyOne, valueOne,
 *                                  otherKey, otherValue);
 *
 */
template<class... KVPairs>
Array make_map_array(KVPairs&&... kvpairs) {
  static_assert(
    sizeof...(kvpairs) % 2 == 0, "make_map_array needs key value pairs");
  ArrayInit init(sizeof...(kvpairs) / 2, ArrayInit::Map{});
  make_array_detail::map_impl(init, std::forward<KVPairs>(kvpairs)...);
  return init.toArray();
}

/*
 * Helper for creating Hack dictionaries.
 *
 * Usage:
 *
 *   auto newArray = make_keyset_array(1, 2, 3, 4);
 */
template<class... KVPairs>
Array make_dict_array(KVPairs&&... kvpairs) {
  static_assert(sizeof...(kvpairs), "use Array::CreateDict() instead");
  static_assert(
    sizeof...(kvpairs) % 2 == 0, "make_dict_array needs key value pairs");
  DictInit init(sizeof...(kvpairs) / 2);
  make_array_detail::dict_impl(init, std::forward<KVPairs>(kvpairs)...);
  return init.toArray();
}

/*
 * Helper for creating Hack keysets.
 *
 * Usage:
 *
 *   auto newArray = make_keyset_array(1, 2, 3, 4);
 */
template<class... Vals>
Array make_keyset_array(Vals&&... vals) {
  static_assert(sizeof...(vals), "use Array::CreateKeyset() instead");
  KeysetInit init(sizeof...(vals));
  make_array_detail::keyset_impl(init, std::forward<Vals>(vals)...);
  return init.toArray();
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
