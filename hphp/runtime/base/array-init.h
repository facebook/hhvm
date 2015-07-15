/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef NDEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    other.m_data = nullptr;
#ifndef NDEBUG
    other.m_expectedCount = 0;
#endif
  }

  ArrayInit(const ArrayInit&) = delete;
  ArrayInit& operator=(const ArrayInit&) = delete;

  ~ArrayInit() {
    // Use non-specialized release call so array instrumentation can track
    // its destruction XXX rfc: keep this? what was it before?
    assert(!m_data || m_data->hasExactlyOneRef());
    if (m_data) m_data->release();
  }

  ArrayInit& set(const Variant& v) = delete;

  ArrayInit& append(const Variant& v) {
    performOp([&]{
      return MixedArray::Append(m_data, v, false);
    });
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

  ArrayInit& set(const Variant& name, const Variant& v,
                 bool keyConverted) = delete;
  ArrayInit& set(const Variant& name, const Variant& v) {
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
  ArrayInit& setKeyUnconverted(const Variant& name, const Variant& v) {
    VarNR k(name.toKey());
    // XXX: the old semantics of ArrayInit used to check if k.isNull
    // and do nothing, but that's not php semantics so we're not doing
    // that anymore.
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

  ArrayInit& add(const String& name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{
        return MixedArray::AddStr(m_data, name.get(),
          v.asInitCellTmp(), false);
      });
    } else if (!name.isNull()) {
      performOp([&]{ return m_data->add(name.toKey(), v, false); });
    }
    return *this;
  }

  ArrayInit& add(const Variant& name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->add(name, v, false); });
    } else {
      VarNR k(name.toKey());
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
      VarNR k(Variant(name).toKey());
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
      performOp([&]{ return m_data->setRef(name.toKey(), v, false); });
    }
    return *this;
  }

  ArrayInit& setRef(const Variant& name,
                    Variant& v,
                    bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, v, false); });
    } else {
      Variant key(name.toKey());
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
      VarNR key(Variant(name).toKey());
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
    auto const ret = m_data;
    m_data = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return ret;
  }

  Array toArray() {
    assert(m_data->hasExactlyOneRef());
    auto ptr = m_data;
    m_data = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  Variant toVariant() {
    assert(m_data->hasExactlyOneRef());
    auto ptr = m_data;
    m_data = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, Variant::ArrayInitCtor{});
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
#ifndef NDEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

//////////////////////////////////////////////////////////////////////

/*
 * Initializer for a vector-shaped array.
 */
class PackedArrayInit {
public:
  explicit PackedArrayInit(size_t n)
    : m_vec(MixedArray::MakeReserve(n))
#ifndef NDEBUG
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
      check_request_surprise_unlikely();
    }
    m_vec = MixedArray::MakeReserve(n);
#ifndef NDEBUG
    m_addCount = 0;
    m_expectedCount = n;
#endif
    assert(m_vec->hasExactlyOneRef());
    check_request_surprise_unlikely();
  }

  PackedArrayInit(PackedArrayInit&& other) noexcept
    : m_vec(other.m_vec)
#ifndef NDEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    other.m_vec = nullptr;
#ifndef NDEBUG
    other.m_expectedCount = 0;
#endif
  }

  PackedArrayInit(const PackedArrayInit&) = delete;
  PackedArrayInit& operator=(const PackedArrayInit&) = delete;

  ~PackedArrayInit() {
    // In case an exception interrupts the initialization.
    assert(!m_vec || m_vec->hasExactlyOneRef());
    if (m_vec) m_vec->release();
  }

  /*
   * Append a new element to the packed array.
   */
  PackedArrayInit& append(const Variant& v) {
    performOp([&]{ return PackedArray::Append(m_vec, v, false); });
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
    auto ptr = m_vec;
    m_vec = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Variant(ptr, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    assert(m_vec->hasExactlyOneRef());
    ArrayData* ptr = m_vec;
    m_vec = nullptr;
#ifndef NDEBUG
    m_expectedCount = 0; // reset; no more adds allowed
#endif
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData *create() {
    assert(m_vec->hasExactlyOneRef());
    auto ptr = m_vec;
    m_vec = nullptr;
#ifndef NDEBUG
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
#ifndef NDEBUG
  size_t m_addCount;
  size_t m_expectedCount;
#endif
};

//////////////////////////////////////////////////////////////////////

namespace make_array_detail {

  inline void packed_impl(PackedArrayInit&) {}

  template<class Val, class... Vals>
  void packed_impl(PackedArrayInit& init, Val&& val, Vals&&... vals) {
    init.append(std::forward<Val>(val));
    packed_impl(init, std::forward<Vals>(vals)...);
  }

  inline String init_key(const char* s) { return String(s); }
  inline int64_t init_key(int k) { return k; }
  inline int64_t init_key(int64_t k) { return k; }
  inline const String& init_key(const String& k) { return k; }

  inline void map_impl(ArrayInit&) {}

  template<class Key, class Val, class... KVPairs>
  void map_impl(ArrayInit& init, Key&& key, Val&& val, KVPairs&&... kvpairs) {
    init.set(init_key(std::forward<Key>(key)), std::forward<Val>(val));
    map_impl(init, std::forward<KVPairs>(kvpairs)...);
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

//////////////////////////////////////////////////////////////////////

}

#endif
