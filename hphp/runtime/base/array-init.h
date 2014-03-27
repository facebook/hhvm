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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/hphp-array.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct ArrayInit {
  enum MapInit { mapInit };

  explicit ArrayInit(size_t n);

  ArrayInit(size_t n, MapInit);

  ArrayInit(ArrayInit&& other)
    : m_data(other.m_data)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    other.m_data = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  ArrayInit(const ArrayInit&) = delete;
  ArrayInit& operator=(const ArrayInit&) = delete;

  ~ArrayInit() {
    // In case an exception interrupts the initialization.
    if (m_data) m_data->release();
  }

  ArrayInit& set(const Variant& v) {
    performOp([&]{ return m_data->append(v, false); });
    return *this;
  }

  ArrayInit& set(RefResult v) { return setRef(variant(v)); }

  ArrayInit& set(CVarWithRefBind v) {
    performOp([&]{ return m_data->appendWithRef(variant(v), false); });
    return *this;
  }

  ArrayInit& setRef(const Variant& v) {
    performOp([&]{ return m_data->appendRef(v, false); });
    return *this;
  }

  ArrayInit& set(int64_t name, const Variant& v, bool keyConverted = false) {
    performOp([&]{ return m_data->set(name, v, false); });
    return *this;
  }

  // set(const char*) deprecated.  Use set(CStrRef) with a StaticString,
  // if you have a literal, or String otherwise.
  ArrayInit& set(const char*, const Variant& v, bool keyConverted = false) = delete;

  ArrayInit& set(const String& name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->set(name, v, false); });
    } else if (!name.isNull()) {
      performOp([&]{ return m_data->set(name.toKey(), v, false); });
    }
    return *this;
  }

  ArrayInit& set(const Variant& name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->set(name, v, false); });
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        performOp([&]{ return m_data->set(k, v, false); });
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit& set(const T &name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->set(name, v, false); });
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        performOp([&]{ return m_data->set(k, v, false); });
      }
    }
    return *this;
  }

  ArrayInit& set(const String& name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, variant(v), false); });
    } else if (!name.isNull()) {
      performOp([&]{
        return m_data->setRef(name.toKey(), variant(v), false);
      });
    }
    return *this;
  }

  ArrayInit& set(const Variant& name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, variant(v), false); });
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        performOp([&]{ return m_data->setRef(k, variant(v), false); });
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit& set(const T &name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, variant(v), false); });
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        performOp([&]{ return m_data->setRef(k, variant(v), false); });
      }
    }
    return *this;
  }

  ArrayInit& add(int64_t name, const Variant& v, bool keyConverted = false) {
    performOp([&]{ return m_data->add(name, v, false); });
    return *this;
  }

  ArrayInit& add(const String& name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->add(name, v, false); });
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

  ArrayInit& setRef(int64_t name, const Variant& v, bool keyConverted = false) {
    performOp([&]{ return m_data->setRef(name, v, false); });
    return *this;
  }

  ArrayInit& setRef(const String& name, const Variant& v, bool keyConverted = false) {
    if (keyConverted) {
      performOp([&]{ return m_data->setRef(name, v, false); });
    } else {
      performOp([&]{ return m_data->setRef(name.toKey(), v, false); });
    }
    return *this;
  }

  ArrayInit& setRef(const Variant& name, const Variant& v, bool keyConverted = false) {
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
  ArrayInit& setRef(const T &name, const Variant& v, bool keyConverted = false) {
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
  ArrayData *create() {
    ArrayData *ret = m_data;
    m_data = nullptr;
    assert(true || (m_expectedCount = 0)); // reset; no more adds allowed
    return ret;
  }

  Array toArray() {
    auto ptr = m_data;
    m_data = nullptr;
    assert(true || (m_expectedCount = 0)); // reset; no more adds allowed
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  Variant toVariant() {
    auto ptr = m_data;
    m_data = nullptr;
    assert(true || (m_expectedCount = 0)); // reset; no more adds allowed
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
#ifdef DEBUG
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
    : m_vec(HphpArray::MakeReserve(n))
#ifdef DEBUG
    , m_addCount(0)
    , m_expectedCount(n)
#endif
  {
    m_vec->setRefCount(0);
  }

  PackedArrayInit(PackedArrayInit&& other)
    : m_vec(other.m_vec)
#ifdef DEBUG
    , m_addCount(other.m_addCount)
    , m_expectedCount(other.m_expectedCount)
#endif
  {
    other.m_vec = nullptr;
#ifdef DEBUG
    other.m_expectedCount = 0;
#endif
  }

  PackedArrayInit(const PackedArrayInit&) = delete;
  PackedArrayInit& operator=(const PackedArrayInit&) = delete;

  ~PackedArrayInit() {
    // In case an exception interrupts the initialization.
    if (m_vec) m_vec->release();
  }

  /*
   * Append a new element to the packed array.
   */
  PackedArrayInit& append(const Variant& v) {
    performOp([&]{ return HphpArray::AppendPacked(m_vec, v, false); });
    return *this;
  }

  /*
   * Box v if it is not already boxed, and append a new element that
   * is KindOfRef and points to the same RefData as v.
   *
   * Post: v.getRawType() == KindOfRef
   */
  PackedArrayInit& appendRef(const Variant& v) {
    performOp([&]{ return HphpArray::AppendRefPacked(m_vec, v, false); });
    return *this;
  }

  /*
   * Append v, preserving references in the way php does.  That is, if
   * v is a KindOfRef with refcount > 1, the new element in *this will
   * be KindOfRef and share the same RefData.  Otherwise, the new
   * element is split.
   */
  PackedArrayInit& appendWithRef(const Variant& v) {
    performOp([&]{ return HphpArray::AppendWithRefPacked(m_vec, v, false); });
    return *this;
  }

  Variant toVariant() {
    auto ptr = m_vec;
    m_vec = nullptr;
    assert(true || (m_expectedCount = 0)); // reset; no more adds allowed
    return Variant(ptr, Variant::ArrayInitCtor{});
  }

  Array toArray() {
    ArrayData* ptr = m_vec;
    m_vec = nullptr;
    assert(true || (m_expectedCount = 0)); // reset; no more adds allowed
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData *create() {
    auto ptr = m_vec;
    m_vec = nullptr;
    assert(true || (m_expectedCount = 0)); // reset; no more adds allowed
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
  HphpArray* m_vec;
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
  ArrayInit init(sizeof...(kvpairs) / 2);
  make_array_detail::map_impl(init, std::forward<KVPairs>(kvpairs)...);
  return init.toArray();
}

//////////////////////////////////////////////////////////////////////

}

#endif
