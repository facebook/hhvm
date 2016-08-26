/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

//////////////////////////////////////////////////////////////////////

/*
 * Flag indicating whether this allocation should be pre-checked for OOM.
 */
enum class CheckAllocation {};

struct ArrayInit {
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
  ArrayInit(size_t n, Map);
  ArrayInit(size_t n, Map, CheckAllocation);

  ArrayInit(ArrayInit&& other) noexcept
    : m_data(other.m_data)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assert(!m_data || m_data->isPHPArray());
    other.m_data = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  ArrayInit(const ArrayInit&) = delete;
  ArrayInit& operator=(const ArrayInit&) = delete;

  ~ArrayInit() {
    // Use non-specialized release call so array instrumentation can track
    // its destruction XXX rfc: keep this? what was it before?
    assert(!m_data || (m_data->hasExactlyOneRef() && m_data->isPHPArray()));
    if (m_data) m_data->release();
  }

  ArrayInit& set(const Variant& v) = delete;

  ArrayInit& append(const Variant& v) {
    auto const cell = LIKELY(v.getType() != KindOfUninit)
      ? *v.asCell()
      : make_tv<KindOfNull>();
    performOp([&]{ return MixedArray::Append(m_data, cell, false); });
    return *this;
  }

  ArrayInit& setRef(Variant& v) = delete;
  ArrayInit& appendRef(Variant& v) {
    performOp([&]{ return MixedArray::AppendRef(m_data, v, false); });
    return *this;
  }

  ArrayInit& set(int64_t name, const Variant& v,
                 bool keyConverted) = delete;
  ArrayInit& set(int64_t name, const Variant& v) {
    performOp([&]{
      return MixedArray::SetInt(m_data, name, v.asInitCellTmp(), false);
    });
    return *this;
  }

  // set(const char*) deprecated.  Use set(CStrRef) with a
  // StaticString, if you have a literal, or String otherwise.  Also
  // don't try to pass a bool.
  ArrayInit& set(const char*, const Variant& v, bool keyConverted) = delete;
  ArrayInit& set(const String& name, const Variant& v,
                 bool keyConverted) = delete;

  ArrayInit& set(const String& name, const Variant& v) {
    performOp([&]{
      return MixedArray::SetStr(m_data, name.get(), v.asInitCellTmp(), false);
    });
    return *this;
  }

  ArrayInit& set(const Variant& name, const Variant& v) = delete;
  ArrayInit& set(const Variant& name, const Variant& v,
                 bool keyConverted) = delete;
  ArrayInit& setValidKey(const Variant& name, const Variant& v) {
    performOp([&]{ return m_data->set(name, v, false); });
    return *this;
  }

  /*
   * This function is deprecated and exists for backward compatibility
   * with the ArrayInit api.  Generally you should be able to figure
   * out if your key is a pure string (not-integer-like) or not when
   * using ArrayInit, and if not you should probably use toKey
   * yourself.
   */
  ArrayInit& setUnknownKey(const Variant& name, const Variant& v) {
    VarNR k(name.toKey(m_data));
    if (UNLIKELY(k.isNull())) return *this;
    performOp([&]{ return m_data->set(k, v, false); });
    return *this;
  }

  template<class T>
  ArrayInit& set(const T& name, const Variant& v, bool) = delete;

  template<class T>
  ArrayInit& set(const T& name, const Variant& v) {
    performOp([&]{ return m_data->set(name, v, false); });
    return *this;
  }

  ArrayInit& add(int64_t name, const Variant& v, bool keyConverted = false) {
    performOp([&]{
      return MixedArray::AddInt(m_data, name, v.asInitCellTmp(), false);
    });
    return *this;
  }

  ArrayInit& add(const String& name, const Variant& v,
                 bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{
        return MixedArray::AddStr(m_data, name.get(),
          v.asInitCellTmp(), false);
      });
    } else if (!name.isNull()) {
      performOp([&]{ return m_data->add(VarNR::MakeKey(name), v, false); });
    }
    return *this;
  }

  ArrayInit& add(const Variant& name, const Variant& v,
                 bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->add(name, v, false); });
    } else {
      VarNR k(name.toKey(m_data));
      if (!k.isNull()) {
        performOp([&]{ return m_data->add(k, v, false); });
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit& add(const T &name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->add(name, v, false); });
    } else {
      VarNR k(Variant(name).toKey(m_data));
      if (!k.isNull()) {
        performOp([&]{ return m_data->add(k, v, false); });
      }
    }
    return *this;
  }

  ArrayInit& setRef(int64_t name,
                    Variant& v,
                    bool keyConverted = false) {
    performOp([&]{ return MixedArray::SetRefInt(m_data, name, v, false); });
    return *this;
  }

  ArrayInit& setRef(const String& name,
                    Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{
        return MixedArray::SetRefStr(m_data, name.get(), v, false);
      });
    } else {
      performOp([&]{ return m_data->setRef(VarNR::MakeKey(name), v, false); });
    }
    return *this;
  }

  ArrayInit& setRef(const Variant& name,
                    Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, v, false); });
    } else {
      Variant key(name.toKey(m_data));
      if (!key.isNull()) {
        performOp([&]{ return m_data->setRef(key, v, false); });
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit& setRef(const T &name,
                    Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, v, false); });
    } else {
      VarNR key(Variant(name).toKey(m_data));
      if (!key.isNull()) {
        performOp([&]{ return m_data->setRef(key, v, false); });
      }
    }
    return *this;
  }

  // Prefer toArray() in new code---it can save a null check when the
  // compiler can't prove m_data hasn't changed.
  ArrayData* create() {
    assert(m_data->hasExactlyOneRef());
    assert(m_data->isPHPArray());
    auto const ret = m_data;
    m_data = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ret;
  }

  Array toArray() {
    assert(m_data->hasExactlyOneRef());
    assert(m_data->isPHPArray());
    auto ptr = m_data;
    m_data = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  Variant toVariant() {
    assert(m_data->hasExactlyOneRef());
    assert(m_data->isPHPArray());
    auto ptr = m_data;
    m_data = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, KindOfArray, Variant::ArrayInitCtor{});
  }

private:
  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved
    // initializations.
    assert(newp == m_data);
    // You cannot add/set more times than you reserved with ArrayInit.
    assert(++m_addCount <= m_expectedCount);
  }

private:
  ArrayData* m_data;
#ifdef DEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

struct DictInit {
  /*
   * For large array allocations, consider passing CheckAllocation, which will
   * throw if the allocation would OOM the request.
   */
  explicit DictInit(size_t n);
  DictInit(size_t n, CheckAllocation);

  DictInit(DictInit&& other) noexcept
    : m_data(other.m_data)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assert(!m_data || m_data->isDict());
    other.m_data = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  DictInit(const DictInit&) = delete;
  DictInit& operator=(const DictInit&) = delete;

  ~DictInit() {
    assert(!m_data || (m_data->hasExactlyOneRef() && m_data->isDict()));
    if (m_data) MixedArray::Release(m_data);
  }

  DictInit& append(const Variant& v) {
    auto const cell = LIKELY(v.getType() != KindOfUninit)
      ? *v.asCell()
      : make_tv<KindOfNull>();
    performOp([&]{ return MixedArray::AppendDict(m_data, cell, false); });
    return *this;
  }

  DictInit& set(int64_t name, const Variant& v) {
    performOp([&]{
      return MixedArray::SetIntDict(m_data, name, v.asInitCellTmp(), false);
    });
    return *this;
  }

  DictInit& set(StringData* name, const Variant& v) {
    performOp([&]{
      return MixedArray::SetStrDict(m_data, name, v.asInitCellTmp(), false);
    });
    return *this;
  }

  /*
   * set(const char*) deprecated.  Use set(CStrRef) with a
   * StaticString, if you have a literal, or String otherwise.
   */
  DictInit& set(const char*, const Variant& v) = delete;

  DictInit& set(const String& name, const Variant& v) {
    performOp([&]{
      return MixedArray::SetStrDict(m_data, name.get(),
                                    v.asInitCellTmp(), false);
    });
    return *this;
  }

  DictInit& set(const Variant& name, const Variant& v) = delete;
  DictInit& setValidKey(const Variant& name, const Variant& v) {
    assert(name.isInteger() || name.isString());
    performOp(
      [&]{
        auto const cell = name.asCell();
        return isIntType(cell->m_type)
          ? MixedArray::SetIntDict(m_data, cell->m_data.num,
                                   v.asInitCellTmp(), false)
          : MixedArray::SetStrDict(m_data, cell->m_data.pstr,
                                   v.asInitCellTmp(), false);
      }
    );
    return *this;
  }

  ArrayData* create() {
    assert(m_data->hasExactlyOneRef());
    assert(m_data->isDict());
    auto const ret = m_data;
    m_data = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ret;
  }

  Array toArray() {
    assert(m_data->hasExactlyOneRef());
    assert(m_data->isDict());
    auto ptr = m_data;
    m_data = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  Variant toVariant() {
    assert(m_data->hasExactlyOneRef());
    assert(m_data->isDict());
    auto ptr = m_data;
    m_data = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, KindOfDict, Variant::ArrayInitCtor{});
  }

private:
  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved
    // initializations.
    assert(newp == m_data);
    // You cannot add/set more times than you reserved with DictInit.
    assert(++m_addCount <= m_expectedCount);
  }

private:
  ArrayData* m_data;
#ifdef DEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};
//////////////////////////////////////////////////////////////////////

/*
 * Initializer for a vector-shaped array.
 */
struct PackedArrayInit {
  explicit PackedArrayInit(size_t n)
    : m_vec(PackedArray::MakeReserve(n))
#ifdef DEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assert(m_vec->hasExactlyOneRef());
  }

  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws RequestMemoryExceededException if allocating would OOM.
   */
  PackedArrayInit(size_t n, CheckAllocation) {
    auto allocsz = sizeof(ArrayData) + sizeof(TypedValue) * n;
    if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
      check_non_safepoint_surprise();
    }
    m_vec = PackedArray::MakeReserve(n);
#ifdef DEBUG
    m_addCount = 0;
    m_expectedCount = n;
#endif
    assert(m_vec->hasExactlyOneRef());
    check_non_safepoint_surprise();
  }

  PackedArrayInit(PackedArrayInit&& other) noexcept
    : m_vec(other.m_vec)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assert(!m_vec || m_vec->isPHPArray());
    other.m_vec = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  PackedArrayInit(const PackedArrayInit&) = delete;
  PackedArrayInit& operator=(const PackedArrayInit&) = delete;

  ~PackedArrayInit() {
    // In case an exception interrupts the initialization.
    assert(!m_vec || (m_vec->hasExactlyOneRef() && m_vec->isPHPArray()));
    if (m_vec) m_vec->release();
  }

  /*
   * Append a new element to the packed array.
   */
  PackedArrayInit& append(const Variant& v) {
    auto const cell = LIKELY(v.getType() != KindOfUninit)
      ? *v.asCell()
      : make_tv<KindOfNull>();
    performOp([&]{ return PackedArray::Append(m_vec, cell, false); });
    return *this;
  }

  /*
   * Box v if it is not already boxed, and append a new element that
   * is KindOfRef and points to the same RefData as v.
   *
   * Post: v.getRawType() == KindOfRef
   */
  PackedArrayInit& appendRef(Variant& v) {
    performOp([&]{ return PackedArray::AppendRef(m_vec, v, false); });
    return *this;
  }

  /*
   * Append v, preserving references in the way php does.  That is, if
   * v is a KindOfRef with refcount > 1, the new element in *this will
   * be KindOfRef and share the same RefData.  Otherwise, the new
   * element is split.
   */
  PackedArrayInit& appendWithRef(const Variant& v) {
    performOp([&]{ return PackedArray::AppendWithRef(m_vec, v, false); });
    return *this;
  }

  Variant toVariant() {
    assert(m_vec->hasExactlyOneRef());
    assert(m_vec->isPHPArray());
    auto ptr = m_vec;
    m_vec = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, KindOfArray, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assert(m_vec->hasExactlyOneRef());
    assert(m_vec->isPHPArray());
    ArrayData* ptr = m_vec;
    m_vec = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assert(m_vec->hasExactlyOneRef());
    assert(m_vec->isPHPArray());
    auto ptr = m_vec;
    m_vec = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ptr;
  }

private:
  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved
    // initializations.
    assert(newp == m_vec);
    // You cannot add/set more times than you reserved with ArrayInit.
    assert(++m_addCount <= m_expectedCount);
  }

private:
  ArrayData* m_vec;
#ifdef DEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

//////////////////////////////////////////////////////////////////////

/*
 * Initializer for a Hack vector array.
 */
struct VecArrayInit {
  explicit VecArrayInit(size_t n)
    : m_vec(PackedArray::MakeReserveVec(n))
#ifdef DEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assert(m_vec->hasExactlyOneRef());
  }

  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws RequestMemoryExceededException if allocating would OOM.
   */
  VecArrayInit(size_t n, CheckAllocation) {
    auto allocsz = sizeof(ArrayData) + sizeof(TypedValue) * n;
    if (UNLIKELY(allocsz > kMaxSmallSize && MM().preAllocOOM(allocsz))) {
      check_non_safepoint_surprise();
    }
    m_vec = PackedArray::MakeReserveVec(n);
#ifdef DEBUG
    m_addCount = 0;
    m_expectedCount = n;
#endif
    assert(m_vec->hasExactlyOneRef());
    check_non_safepoint_surprise();
  }

  VecArrayInit(VecArrayInit&& other) noexcept
    : m_vec(other.m_vec)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assert(!m_vec || m_vec->isVecArray());
    other.m_vec = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  VecArrayInit(const VecArrayInit&) = delete;
  VecArrayInit& operator=(const VecArrayInit&) = delete;

  ~VecArrayInit() {
    // In case an exception interrupts the initialization.
    assert(!m_vec || (m_vec->hasExactlyOneRef() && m_vec->isVecArray()));
    if (m_vec) PackedArray::Release(m_vec);
  }

  /*
   * Append a new element to the vec array.
   */
  VecArrayInit& append(const Variant& v) {
    auto const cell = LIKELY(v.getType() != KindOfUninit)
      ? *v.asCell()
      : make_tv<KindOfNull>();
    performOp([&]{ return PackedArray::AppendVec(m_vec, cell, false); });
    return *this;
  }

  Variant toVariant() {
    assert(m_vec->hasExactlyOneRef());
    assert(m_vec->isVecArray());
    auto ptr = m_vec;
    m_vec = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, KindOfVec, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assert(m_vec->hasExactlyOneRef());
    assert(m_vec->isVecArray());
    ArrayData* ptr = m_vec;
    m_vec = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assert(m_vec->hasExactlyOneRef());
    assert(m_vec->isVecArray());
    auto ptr = m_vec;
    m_vec = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ptr;
  }

private:
  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved
    // initializations.
    assert(newp == m_vec);
    // You cannot add/set more times than you reserved with ArrayInit.
    assert(++m_addCount <= m_expectedCount);
  }

private:
  ArrayData* m_vec;
#ifdef DEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

//////////////////////////////////////////////////////////////////////

/*
 * Initializer for a Hack keyset.
 */
struct KeysetInit {
  explicit KeysetInit(size_t n)
    : m_keyset(SetArray::MakeReserveSet(n))
#ifdef DEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    assert(m_keyset->hasExactlyOneRef());
  }

  /*
   * Before allocating, check if the allocation would cause the request to OOM.
   *
   * @throws RequestMemoryExceededException if allocating would OOM.
   */
  KeysetInit(size_t n, CheckAllocation);

  KeysetInit(KeysetInit&& other) noexcept
    : m_keyset(other.m_keyset)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    assert(!m_keyset || m_keyset->isKeyset());
    other.m_keyset = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  KeysetInit(const KeysetInit&) = delete;
  KeysetInit& operator=(const KeysetInit&) = delete;

  ~KeysetInit() {
    // In case an exception interrupts the initialization.
    assert(!m_keyset || (m_keyset->hasExactlyOneRef() && m_keyset->isKeyset()));
    if (m_keyset) SetArray::Release(m_keyset);
  }

  /*
   * Add a new element to the keyset.
   */
  KeysetInit& add(int64_t v) {
    performOp([&]{ return SetArray::AddToSet(m_keyset, v, false); });
    return *this;
  }
  KeysetInit& add(StringData* v) {
    performOp([&]{ return SetArray::AddToSet(m_keyset, v, false); });
    return *this;
  }
  KeysetInit& add(const Variant& v) {
    performOp([&]{
      return SetArray::Append(m_keyset, v.asInitCellTmp(), false);
    });
    return *this;
  }

  Variant toVariant() {
    assert(m_keyset->hasExactlyOneRef());
    assert(m_keyset->isKeyset());
    auto ptr = m_keyset;
    m_keyset = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, KindOfKeyset, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assert(m_keyset->hasExactlyOneRef());
    assert(m_keyset->isKeyset());
    ArrayData* ptr = m_keyset;
    m_keyset = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData* create() {
    assert(m_keyset->hasExactlyOneRef());
    assert(m_keyset->isKeyset());
    auto ptr = m_keyset;
    m_keyset = nullptr;
#ifdef DEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ptr;
  }

private:
  template<class Operation>
  ALWAYS_INLINE void performOp(Operation oper) {
    DEBUG_ONLY auto newp = oper();
    // Array escalation must not happen during these reserved
    // initializations.
    assert(newp == m_keyset);
    // You cannot add/set more times than you reserved with ArrayInit.
    assert(++m_addCount <= m_expectedCount);
  }

private:
  ArrayData* m_keyset;
#ifdef DEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

}

#endif
