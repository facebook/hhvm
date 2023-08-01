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

#include <boost/variant.hpp>
#include <type_traits>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/match.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// SetInPlace helpers that assume that the input array has a mixed layout.
// These helpers work for both mixed PHP arrays and dicts.
namespace arr_init {
inline ArrayData* SetInPlace(ArrayData* ad, int64_t k, TypedValue v) {
  return VanillaDict::SetIntInPlace(ad, k, tvToInit(v));
}
inline ArrayData* SetInPlace(ArrayData* ad, StringData* k, TypedValue v) {
  return VanillaDict::SetStrInPlace(ad, k, tvToInit(v));
}
inline ArrayData* SetInPlace(ArrayData* ad, const String& k, TypedValue v) {
  return VanillaDict::SetStrInPlace(ad, k.get(), tvToInit(v));
}
inline ArrayData* SetInPlace(ArrayData* ad, TypedValue k, TypedValue v) {
  if (isIntType(k.m_type)) {
    return VanillaDict::SetIntInPlace(ad, k.m_data.num, tvToInit(v));
  } else {
    assertx(isStringType(k.m_type));
    return VanillaDict::SetStrInPlace(ad, k.m_data.pstr, tvToInit(v));
  }
}
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Flag indicating whether an array allocation should be pre-checked for OOM.
 */
enum class CheckAllocation {};

/*
 * Base class for ArrayInits specialized on array kind.
 *
 * Takes two template parameters:
 *  - TArray is a bag of static class functions, such as VanillaDict.  See the
 *    `detail' namespace below for the requirements.
 *  - DT is the DataType for the arrays created by the ArrayInit. If DT is
 *    the sentinel KindOfUninit, we won't make assumptions about the type.
 */
template<typename TArray, DataType DT>
struct ArrayInitBase {
  explicit ArrayInitBase(size_t n)
    : m_arr(TArray::MakeReserve(n))
#ifndef NDEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assertx(m_arr->hasExactlyOneRef());
  }

  ArrayInitBase(ArrayInitBase&& other) noexcept
    : m_arr(other.m_arr)
#ifndef NDEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assertx(!m_arr || DT == KindOfUninit || m_arr->toDataType() == DT);
    other.m_arr = nullptr;
#ifndef NDEBUG
    other.m_expectedCount = 0;
#endif
  }

  ArrayInitBase(const ArrayInitBase&) = delete;
  ArrayInitBase& operator=(const ArrayInitBase&) = delete;

  ~ArrayInitBase() {
    // In case an exception interrupts the initialization.
    assertx(!m_arr || DT == KindOfUninit ||
            (m_arr->hasExactlyOneRef() && m_arr->toDataType() == DT));
    if (m_arr) TArray::Release(m_arr);
  }

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Finish routines.
   *
   * These all invalidate the ArrayInit and return the initialized array.
   */
  Variant toVariant() {
    if (DT == KindOfUninit) {
      return Array(create(), Array::ArrayInitCtor::Tag);
    }
    return Variant(create(), DT, Variant::ArrayInitCtor{});
  }
  Array toArray() {
    return Array(create(), Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(DT == KindOfUninit || m_arr->toDataType() == DT);
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
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
    : m_arr{nullptr}
#ifndef NDEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {}

  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved initializations.
    assertx(newp == m_arr);
    // You cannot add/set more times than you reserved with ArrayInit.
    assertx(++m_addCount <= m_expectedCount);
  }

protected:
  ArrayData* m_arr;
#ifndef NDEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Dummy VanillaDict-like bags of statics for Hack arrays.
 */
namespace detail {

struct Vec {
  static constexpr auto MakeReserve = &VanillaVec::MakeReserveVec;
  static constexpr auto Release = VanillaVec::Release;
};

struct Dict {
  static constexpr auto MakeReserve = &VanillaDict::MakeReserveDict;
  static constexpr auto Release = VanillaDict::Release;
};

}

///////////////////////////////////////////////////////////////////////////////


struct DictInit : ArrayInitBase<detail::Dict, KindOfDict> {
  using ArrayInitBase<detail::Dict, KindOfDict>::ArrayInitBase;

  /*
   * For large array allocations, consider passing CheckAllocation, which will
   * throw if the allocation would OOM the request.
   */
  DictInit(size_t n, CheckAllocation);

  /////////////////////////////////////////////////////////////////////////////

  DictInit& append(TypedValue tv) {
    performOp([&]{ return VanillaDict::AppendMove(m_arr, tvToInit(tv)); });
    tvIncRefGen(tv);
    return *this;
  }
  DictInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  DictInit& set(int64_t name, TypedValue tv) {
    performOp([&]{ return arr_init::SetInPlace(m_arr, name, tv); });
    return *this;
  }
  DictInit& set(StringData* name, TypedValue tv) {
    performOp([&]{ return arr_init::SetInPlace(m_arr, name, tv); });
    return *this;
  }
  DictInit& set(const String& name, TypedValue tv) {
    performOp([&]{ return arr_init::SetInPlace(m_arr, name, tv); });
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
      assertx(isIntType(name.m_type) || isStringType(name.m_type));
      return arr_init::SetInPlace(m_arr, name, v);
    });
    return *this;
  }
  DictInit& setValidKey(const Variant& name, const Variant& v) {
    return setValidKey(*name.asTypedValue(), *v.asTypedValue());
  }

  template <IntishCast IC>
  DictInit& setUnknownKey(TypedValue name, const Variant& v) {
    auto const k = tvToKey<IC>(name, m_arr);
    if (UNLIKELY(tvIsNull(k))) return *this;
    performOp([&]{ return arr_init::SetInPlace(m_arr, k, v.asInitTVTmp()); });
    return *this;
  }

  DictInit& setLegacyArray() {
    m_arr->setLegacyArrayInPlace(true);
    return *this;
  }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Initializer for a Hack vector array.
 */

struct VecInit final : ArrayInitBase<detail::Vec, KindOfVec> {
  using ArrayInitBase<detail::Vec, KindOfVec>::ArrayInitBase;
  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws: RequestMemoryExceededException if allocating would OOM.
   */
  VecInit(size_t n, CheckAllocation) : ArrayInitBase(n, CheckAllocation{})
  {
    auto allocsz = sizeof(ArrayData) + sizeof(TypedValue) * n;
    if (UNLIKELY(allocsz > kMaxSmallSize)) {
      // If they're asking to check allocation then also check that the size
      // doesn't exceed our max capacity. The error message is a bit misleading
      // (OOM) but it's better than a crash later on.
      if (UNLIKELY(!VanillaVec::checkCapacity(n))) {
        setSurpriseFlag(MemExceededFlag);
        RID().setRequestOOMFlag();
        check_non_safepoint_surprise();
      }
      if (UNLIKELY(tl_heap->preAllocOOM(allocsz))) {
        check_non_safepoint_surprise();
      }
    }
    m_arr = detail::Vec::MakeReserve(n);
    assertx(m_arr->hasExactlyOneRef());
    check_non_safepoint_surprise();
  }

  VecInit& append(TypedValue tv) {
    this->performOp([&]{
      return VanillaVec::AppendInPlace(m_arr, tvToInit(tv));
    });
    return *this;
  }
  VecInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  VecInit& setLegacyArray(bool legacy) {
    m_arr->setLegacyArrayInPlace(legacy);
    return *this;
  }

};

///////////////////////////////////////////////////////////////////////////////

/*
 * Initializer for a Hack keyset.
 */
struct KeysetInit : ArrayInitBase<VanillaKeyset, KindOfKeyset> {
  using ArrayInitBase<VanillaKeyset, KindOfKeyset>::ArrayInitBase;

  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws RequestMemoryExceededException if allocating would OOM.
   */
  KeysetInit(size_t n, CheckAllocation);

  KeysetInit& add(int64_t v) {
    performOp([&]{ return VanillaKeyset::AddToSetInPlace(m_arr, v); });
    return *this;
  }
  KeysetInit& add(StringData* v) {
    performOp([&]{ return VanillaKeyset::AddToSetInPlace(m_arr, v); });
    return *this;
  }
  KeysetInit& add(TypedValue tv) {
    performOp([&]{ return VanillaKeyset::AppendMove(m_arr, tvToInit(tv)); });
    tvIncRefGen(tv);
    return *this;
  }
  KeysetInit& add(const Variant& v) {
    return add(*v.asTypedValue());
  }
};

///////////////////////////////////////////////////////////////////////////////

namespace make_array_detail {

  inline void vec_impl(VecInit&) {}

  template<class Val, class... Vals>
  void vec_impl(VecInit& init, Val&& val, Vals&&... vals) {
    init.append(Variant(std::forward<Val>(val)));
    vec_impl(init, std::forward<Vals>(vals)...);
  }

  inline String init_key(const char* s) { return String(s); }
  inline int64_t init_key(int k) { return k; }
  inline int64_t init_key(int64_t k) { return k; }
  inline const String& init_key(const String& k) { return k; }
  inline const String init_key(StringData* k) { return String{k}; }

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
 * Helper for creating Hack vec arrays (vector-like).
 *
 * Usage:
 *
 *   auto newArray = make_vec_array(1, 2, 3, 4);
 */
template<class... Vals>
Array make_vec_array(Vals&&... vals) {
  static_assert(sizeof...(vals), "use Array::CreateVec() instead");
  VecInit init(sizeof...(vals));
  make_array_detail::vec_impl(init, std::forward<Vals>(vals)...);
  return init.toArray();
}

/*
 * Helper for creating Hack dictionaries.
 *
 * Usage:
 *
 *   auto newArray = make_dict_array(1, 2, 3, 4);
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
