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

#include <boost/variant.hpp>
#include <type_traits>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/util/match.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

#define LV(v) tv_lval{v.asTypedValue()}

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
    assertx(!m_arr || m_arr->toDataType() == DT);
    other.m_arr = nullptr;
#ifndef NDEBUG
    other.m_expectedCount = 0;
#endif
  }

  ArrayInitBase(const ArrayInitBase&) = delete;
  ArrayInitBase& operator=(const ArrayInitBase&) = delete;

  ~ArrayInitBase() {
    // In case an exception interrupts the initialization.
    assertx(!m_arr || (m_arr->hasExactlyOneRef() &&
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
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->toDataType() == DT);
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, DT, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->toDataType() == DT);
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->toDataType() == DT);
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
#ifndef NDEBUG
    : m_addCount(0)
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
 * Dummy MixedArray-like bags of statics for Hack arrays.
 */
namespace detail {

struct VArray {
  static constexpr auto MakeReserve = &PackedArray::MakeReserveVArray;
  static constexpr auto Release = PackedArray::Release;
};

struct DArray {
  static constexpr auto MakeReserve = &MixedArray::MakeReserveDArray;
  static constexpr auto Release = MixedArray::Release;
};

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
template <typename TArray>
struct MixedPHPArrayInitBase : ArrayInitBase<TArray, KindOfArray> {
  enum class Map {};
  // This is the same as map right now, but is here for documentation
  // so we can find them later.
  using Mixed = Map;

  /*
   * When you create an MixedPHPArrayInitBase, you must specify the "kind" of
   * array you are creating, for performance reasons.  "Kinds" that
   * are relevant to know about for extension code:
   *
   *   Packed -- a vector-like array: don't use MixedPHPArrayInitBase,
   *             use PackedArrayInit
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
  MixedPHPArrayInitBase(size_t n, Map)
    : ArrayInitBase<TArray, KindOfArray>(n) {}
  MixedPHPArrayInitBase(size_t n, Map, CheckAllocation);

  MixedPHPArrayInitBase(MixedPHPArrayInitBase&& o) noexcept
    : ArrayInitBase<TArray, KindOfArray>(std::move(o)) {}

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Call append() on the underlying array.
   */
  MixedPHPArrayInitBase& append(TypedValue tv) {
    this->performOp([&]{
      return MixedArray::Append(this->m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  MixedPHPArrayInitBase& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  /*
   * Call set() on the underlying ArrayData.
   */
  MixedPHPArrayInitBase& set(int64_t name, TypedValue tv) {
    this->performOp([&]{
      return MixedArray::SetInt(this->m_arr, name, tvToInitCell(tv), false);
    });
    return *this;
  }
  MixedPHPArrayInitBase& set(const String& name, TypedValue tv) {
    this->performOp([&]{
      return MixedArray::SetStr(
        this->m_arr, name.get(), tvToInitCell(tv), false
      );
    });
    return *this;
  }
  template<class T>
  MixedPHPArrayInitBase& set(const T& name, TypedValue tv) {
    this->performOp([&]{
      return this->m_arr->set(name, tvToInitCell(tv), false);
    });
    return *this;
  }

#define IMPL_SET(KeyType)                           \
  MixedPHPArrayInitBase& set(KeyType name, const Variant& v) {  \
    return set(name, *v.asTypedValue());            \
  }

  IMPL_SET(int64_t)
  IMPL_SET(const String&)
  template<typename T> IMPL_SET(const T&)

  MixedPHPArrayInitBase& set(const Variant& name, const Variant& v) = delete;

#undef IMPL_SET

  /*
   * Same as set(), but for the deleted double `const Variant&' overload.
   */
  MixedPHPArrayInitBase& setValidKey(TypedValue name, TypedValue v) {
    this->performOp([&]{
      return this->m_arr->set(tvToInitCell(name), tvToInitCell(v), false);
    });
    return *this;
  }
  MixedPHPArrayInitBase& setValidKey(const Variant& name, const Variant& v) {
    return setValidKey(*name.asTypedValue(), *v.asTypedValue());
  }

  /*
   * This function is deprecated and exists for backward compatibility with the
   * MixedPHPArrayInitBase API.
   *
   * Generally you should be able to figure out if your key is a pure string
   * (not-integer-like) or not when using MixedPHPArrayInitBase, and if not you
   * should probably use toKey yourself.
   */
  MixedPHPArrayInitBase& setUnknownKey(const Variant& name, const Variant& v) {
    auto const k = name.toKey(this->m_arr).tv();
    if (LIKELY(!isNullType(k.m_type))) {
      this->performOp([&]{
        return this->m_arr->set(k, v.asInitCellTmp(), false);
      });
    }
    return *this;
  }

  /*
   * Call setWithRef() on the underlying array.
   */
  MixedPHPArrayInitBase& setWithRef(int64_t name, TypedValue tv) {
    this->performOp([&]{
      return MixedArray::SetWithRefInt(this->m_arr, name, tv, false);
    });
    return *this;
  }
  MixedPHPArrayInitBase& setWithRef(const String& name, TypedValue tv) {
    this->performOp([&]{
      return MixedArray::SetWithRefStr(this->m_arr, name.get(), tv, false);
    });
    return *this;
  }
  template<class T>
  MixedPHPArrayInitBase& setWithRef(const T& name, TypedValue tv) {
    this->performOp([&]{
      return this->m_arr->setWithRef(name, tv, false);
    });
    return *this;
  }

  /*
   * Call add() on the underlying array.
   */
  MixedPHPArrayInitBase& add(int64_t name, TypedValue tv,
                             bool /*keyConverted*/ = false) {
    this->performOp([&]{
      return MixedArray::AddInt(this->m_arr, name, tvToInitCell(tv), false);
    });
    return *this;
  }

  MixedPHPArrayInitBase& add(const String& name, TypedValue tv,
                             bool keyConverted = false) {
    if (keyConverted) {
      this->performOp([&]{
        return MixedArray::AddStr(
          this->m_arr, name.get(), tvToInitCell(tv), false
        );
      });
    } else if (!name.isNull()) {
      this->performOp([&]{
        return this->m_arr->add(
          VarNR::MakeKey(name).tv(), tvToInitCell(tv), false
        );
      });
    }
    return *this;
  }

  MixedPHPArrayInitBase& add(const Variant& name, TypedValue tv,
                             bool keyConverted = false) {
    if (keyConverted) {
      this->performOp([&]{
        return this->m_arr->add(name.asInitCellTmp(), tvToInitCell(tv), false);
      });
    } else {
      auto const k = name.toKey(this->m_arr).tv();
      if (!isNullType(k.m_type)) {
        this->performOp([&]{
          return this->m_arr->add(k, tvToInitCell(tv), false);
        });
      }
    }
    return *this;
  }

  template<typename T>
  MixedPHPArrayInitBase& add(const T& name, TypedValue tv,
                             bool keyConverted = false) {
    if (keyConverted) {
      this->performOp([&]{
        return this->m_arr->add(name, tvToInitCell(tv), false);
      });
    } else {
      auto const k = Variant(name).toKey(this->m_arr).tv();
      if (!isNullType(k.m_type)) {
        this->performOp([&]{
          return this->m_arr->add(k, tvToInitCell(tv), false);
        });
      }
    }
    return *this;
  }

#define IMPL_ADD(KeyType)                                           \
  MixedPHPArrayInitBase& add(KeyType name, const Variant& v,        \
                             bool keyConverted = false) {           \
    return add(name, *v.asTypedValue(), keyConverted);              \
  }

  IMPL_ADD(int64_t)
  IMPL_ADD(const String&)
  IMPL_ADD(const Variant&)
  template<typename T> IMPL_ADD(const T&)

#undef IMPL_ADD

  /*
   * Call appendRef() on the underlying array.
   */
  MixedPHPArrayInitBase& appendRef(Variant& v) {
    this->performOp([&]{
      return MixedArray::AppendRef(this->m_arr, LV(v), false);
    });
    return *this;
  }

  /*
   * Call setRef() on the underlying array.
   */
  MixedPHPArrayInitBase& setRef(int64_t name, tv_lval v,
                                bool /*keyConverted*/ = false) {
    this->performOp([&]{
      return MixedArray::SetRefInt(this->m_arr, name, v, false);
    });
    return *this;
  }

  MixedPHPArrayInitBase& setRef(const String& name, tv_lval v,
                             bool keyConverted = false) {
    if (keyConverted) {
      this->performOp([&]{
        return MixedArray::SetRefStr(this->m_arr, name.get(), v, false);
      });
    } else {
      this->performOp([&]{
        return this->m_arr->setRef(VarNR::MakeKey(name).tv(), v, false);
      });
    }
    return *this;
  }

  MixedPHPArrayInitBase& setRef(TypedValue name, tv_lval v,
                                bool keyConverted = false) {
    if (keyConverted) {
      this->performOp([&]{
        return this->m_arr->setRef(tvToCell(name), v, false);
      });
    } else {
      auto const k = tvToKey(name, this->m_arr);
      if (!isNullType(k.m_type)) {
        this->performOp([&]{ return this->m_arr->setRef(k, v, false); });
      }
    }
    return *this;
  }
  MixedPHPArrayInitBase& setRef(const Variant& name, tv_lval v,
                                bool keyConverted = false) {
    return setRef(*name.asTypedValue(), v, keyConverted);
  }

  template<typename T>
  MixedPHPArrayInitBase& setRef(const T& name, tv_lval v,
                                bool keyConverted = false) {
    if (keyConverted) {
      this->performOp([&]{ return this->m_arr->setRef(name, v, false); });
    } else {
      auto const k = Variant(name).toKey(this->m_arr).tv();
      if (!isNullType(k.m_type)) {
        this->performOp([&]{ return this->m_arr->setRef(k, v, false); });
      }
    }
    return *this;
  }
};

using ArrayInit = MixedPHPArrayInitBase<MixedArray>;

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
      assertx(isIntType(k.m_type) || isStringType(k.m_type));

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
    if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
      check_non_safepoint_surprise();
    }
    this->m_arr = TArray::MakeReserve(n);
    assertx(this->m_arr->hasExactlyOneRef());
    check_non_safepoint_surprise();
  }
};

/*
 * Initializer for a PHP vector-shaped array.
 */
template <typename TArray>
struct PackedPHPArrayInitBase : PackedArrayInitBase<TArray, KindOfArray> {
  using PackedArrayInitBase<TArray, KindOfArray>::PackedArrayInitBase;

  PackedPHPArrayInitBase& append(TypedValue tv) {
    this->performOp([&]{
      return PackedArray::Append(this->m_arr, tvToInitCell(tv), false);
    });
    return *this;
  }
  PackedPHPArrayInitBase& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  PackedPHPArrayInitBase& appendRef(Variant& v) {
    this->performOp([&]{
      return PackedArray::AppendRef(this->m_arr, LV(v), false);
    });
    return *this;
  }

  PackedPHPArrayInitBase& appendWithRef(TypedValue v) {
    this->performOp([&]{
      return PackedArray::AppendWithRef(this->m_arr, v, false);
    });
    return *this;
  }
  PackedPHPArrayInitBase& appendWithRef(const Variant& v) {
    return appendWithRef(*v.asTypedValue());
  }
};

using PackedArrayInit = PackedPHPArrayInitBase<PackedArray>;

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

struct VArrayInit {
  explicit VArrayInit(size_t n)
    : m_arr(RuntimeOption::EvalHackArrDVArrs
              ? PackedArray::MakeReserveVec(n)
              : PackedArray::MakeReserveVArray(n)
           )
#ifndef NDEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assertx(m_arr->hasExactlyOneRef());
  }

  VArrayInit(VArrayInit&& other) noexcept
    : m_arr(other.m_arr)
#ifndef NDEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assertx(!m_arr || m_arr->isVecOrVArray());
    other.m_arr = nullptr;
#ifndef NDEBUG
    other.m_expectedCount = 0;
#endif
  }

  VArrayInit(const VArrayInit&) = delete;
  VArrayInit& operator=(const VArrayInit&) = delete;

  ~VArrayInit() {
    // In case an exception interrupts the initialization.
    assertx(!m_arr || (m_arr->hasExactlyOneRef() &&
                       m_arr->isVecOrVArray()));
    if (m_arr) m_arr->release();
  }

  VArrayInit& append(TypedValue tv) {
    performOp([&]{ return m_arr->append(tvToInitCell(tv), false); });
    return *this;
  }
  VArrayInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  VArrayInit& appendWithRef(TypedValue v) {
    performOp([&]{ return m_arr->appendWithRef(v, false); });
    return *this;
  }
  VArrayInit& appendWithRef(const Variant& v) {
    return appendWithRef(*v.asTypedValue());
  }

  Variant toVariant() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->isVecOrVArray());
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, ptr->toDataType(), Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->isVecOrVArray());
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->isVecOrVArray());
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ptr;
  }

private:

  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved initializations.
    assertx(newp == m_arr);
    // You cannot add/set more times than you reserved with ArrayInit.
    assertx(++m_addCount <= m_expectedCount);
  }

  ArrayData* m_arr;
#ifndef NDEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

struct DArrayInit {
  explicit DArrayInit(size_t n)
    : m_arr(RuntimeOption::EvalHackArrDVArrs
              ? MixedArray::MakeReserveDict(n)
              : MixedArray::MakeReserveDArray(n)
           )
#ifndef NDEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assertx(m_arr->hasExactlyOneRef());
  }

  DArrayInit(size_t, CheckAllocation);

  DArrayInit(DArrayInit&& other) noexcept
    : m_arr(other.m_arr)
#ifndef NDEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assertx(!m_arr || m_arr->isDictOrDArray());
    other.m_arr = nullptr;
#ifndef NDEBUG
    other.m_expectedCount = 0;
#endif
  }

  DArrayInit(const DArrayInit&) = delete;
  DArrayInit& operator=(const DArrayInit&) = delete;

  ~DArrayInit() {
    // In case an exception interrupts the initialization.
    assertx(!m_arr || (m_arr->hasExactlyOneRef() &&
                      m_arr->isDictOrDArray()));
    if (m_arr) m_arr->release();
  }

  DArrayInit& append(TypedValue tv) {
    performOp([&]{ return m_arr->append(tvToInitCell(tv), false); });
    return *this;
  }
  DArrayInit& append(const Variant& v) {
    return append(*v.asTypedValue());
  }

  /*
   * Call add() on the underlying array.
   */
  DArrayInit& add(int64_t name, TypedValue tv,
                  bool /*keyConverted*/ = false) {
    performOp([&]{ return m_arr->add(name, tvToInitCell(tv), false); });
    return *this;
  }

  DArrayInit& add(const String& name, TypedValue tv,
                  bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_arr->add(name, tvToInitCell(tv), false); });
    } else if (!name.isNull()) {
      performOp(
        [&]{
          return m_arr->add(VarNR::MakeKey(name).tv(), tvToInitCell(tv), false);
        }
      );
    }
    return *this;
  }

  DArrayInit& add(const Variant& name, TypedValue tv,
                  bool keyConverted = false) {
    if (keyConverted) {
      performOp(
        [&]{ return m_arr->add(name.asInitCellTmp(), tvToInitCell(tv), false); }
      );
    } else {
      auto const k = name.toKey(m_arr).tv();
      if (!isNullType(k.m_type)) {
        performOp([&]{ return m_arr->add(k, tvToInitCell(tv), false); });
      }
    }
    return *this;
  }

  template<typename T>
  DArrayInit& add(const T& name, TypedValue tv,
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

#define IMPL_ADD(KeyType)                                           \
  DArrayInit& add(KeyType name, const Variant& v,                   \
                  bool keyConverted = false) {                      \
    return add(name, *v.asTypedValue(), keyConverted);              \
  }

  IMPL_ADD(int64_t)
  IMPL_ADD(const String&)
  IMPL_ADD(const Variant&)
  template<typename T> IMPL_ADD(const T&)
#undef IMPL_ADD

  /*
   * Call set() on the underlying ArrayData.
   */
  DArrayInit& set(int64_t name, TypedValue tv) {
    performOp([&]{ return m_arr->set(name, tvToInitCell(tv), false); });
    return *this;
  }
  DArrayInit& set(const String& name, TypedValue tv) {
    performOp([&]{ return m_arr->set(name, tvToInitCell(tv), false); });
    return *this;
  }
  template<class T>
  DArrayInit& set(const T& name, TypedValue tv) {
    performOp([&]{ return m_arr->set(name, tvToInitCell(tv), false); });
    return *this;
  }

#define IMPL_SET(KeyType)                            \
  DArrayInit& set(KeyType name, const Variant& v) {  \
    return set(name, *v.asTypedValue());             \
  }

  IMPL_SET(int64_t)
  IMPL_SET(const String&)
  template<typename T> IMPL_SET(const T&)

  DArrayInit& set(const Variant& name, const Variant& v) = delete;
#undef IMPL_SET

  DArrayInit& setValidKey(TypedValue name, TypedValue v) {
    performOp(
      [&]{ return m_arr->set(tvToInitCell(name), tvToInitCell(v), false); }
    );
    return *this;
  }
  DArrayInit& setValidKey(const Variant& name, const Variant& v) {
    return setValidKey(*name.asTypedValue(), *v.asTypedValue());
  }

  DArrayInit& setUnknownKey(const Variant& name, const Variant& v) {
    auto const k = name.toKey(m_arr).tv();
    if (LIKELY(!isNullType(k.m_type))) {
      performOp([&]{ return m_arr->set(k, v.asInitCellTmp(), false); });
    }
    return *this;
  }

  Variant toVariant() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->isDictOrDArray());
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, ptr->toDataType(), Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->isDictOrDArray());
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assertx(m_arr->hasExactlyOneRef());
    assertx(m_arr->isDictOrDArray());
    auto const ptr = m_arr;
    m_arr = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ptr;
  }

private:

  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved initializations.
    assertx(newp == m_arr);
    // You cannot add/set more times than you reserved with ArrayInit.
    assertx(++m_addCount <= m_expectedCount);
  }

  ArrayData* m_arr;
#ifndef NDEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
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

  inline void varray_impl(VArrayInit&) {}

  template<class Val, class... Vals>
  void varray_impl(VArrayInit& init, Val&& val, Vals&&... vals) {
    init.append(Variant(std::forward<Val>(val)));
    varray_impl(init, std::forward<Vals>(vals)...);
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

  inline String darray_init_key(const char* s) { return String(s); }
  inline int64_t darray_init_key(int k) { return k; }
  inline int64_t darray_init_key(int64_t k) { return k; }
  inline const String& darray_init_key(const String& k) { return k; }
  inline const String darray_init_key(StringData* k) { return String{k}; }

  inline void darray_impl(DArrayInit&) {}

  template<class Key, class Val, class... KVPairs>
  void darray_impl(DArrayInit& init, Key&& key,
                   Val&& val, KVPairs&&... kvpairs) {
    init.set(darray_init_key(std::forward<Key>(key)),
             Variant(std::forward<Val>(val)));
    darray_impl(init, std::forward<KVPairs>(kvpairs)...);
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
 * Helper for creating packed varrays that don't contain references.
 *
 * Usage:
 *
 *   auto newArray = make_varray(1, 2, 3, 4);
 *
 * If you need to deal with references, you currently have to use
 * VArrayInit directly.
 */
template<class... Vals>
Array make_varray(Vals&&... vals) {
  static_assert(sizeof...(vals), "use Array::CreateVArray() instead");
  VArrayInit init(sizeof...(vals));
  make_array_detail::varray_impl(init, std::forward<Vals>(vals)...);
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
 * Helper for creating darrays.  Takes pairs of arguments for the keys and
 * values.
 *
 * Usage:
 *
 *   auto newArray = make_darray(keyOne, valueOne, otherKey, otherValue);
 *
 */
template<class... KVPairs>
Array make_darray(KVPairs&&... kvpairs) {
  static_assert(sizeof...(kvpairs), "use Array::CreateDArray() instead");
  static_assert(
    sizeof...(kvpairs) % 2 == 0, "make_darray needs key value pairs");
  DArrayInit init(sizeof...(kvpairs) / 2);
  make_array_detail::darray_impl(init, std::forward<KVPairs>(kvpairs)...);
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

#undef LV

///////////////////////////////////////////////////////////////////////////////

template <typename TArray>
MixedPHPArrayInitBase<TArray>::MixedPHPArrayInitBase(size_t n,
                                                     Map,
                                                     CheckAllocation)
  : ArrayInitBase<TArray, KindOfArray>(n, CheckAllocation{})
{
  if (n > std::numeric_limits<int>::max()) {
    tl_heap->forceOOM();
    check_non_safepoint_surprise();
  }
  auto const allocsz = MixedArray::computeAllocBytes(
                         MixedArray::computeScaleFromSize(n)
                       );
  if (UNLIKELY(allocsz > kMaxSmallSize && tl_heap->preAllocOOM(allocsz))) {
    check_non_safepoint_surprise();
  }
  this->m_arr = TArray::MakeReserve(n);
  assertx(this->m_arr->hasExactlyOneRef());
  check_non_safepoint_surprise();
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
