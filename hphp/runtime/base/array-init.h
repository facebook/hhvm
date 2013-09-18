/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

class ArrayInit {
public:
  enum MapInit { mapInit };

  explicit ArrayInit(ssize_t n);

  ArrayInit(ssize_t n, MapInit);

  ArrayInit(ArrayInit&& other) : m_data(other.m_data) {
    other.m_data = nullptr;
  }

  ArrayInit(const ArrayInit&) = delete;
  ArrayInit& operator=(const ArrayInit&) = delete;

  ~ArrayInit() {
    // In case an exception interrupts the initialization.
    if (m_data) m_data->release();
  }

  ArrayInit &set(CVarRef v) {
    m_data->append(v, false);
    return *this;
  }

  ArrayInit &set(RefResult v) {
    setRef(variant(v));
    return *this;
  }

  ArrayInit &set(CVarWithRefBind v) {
    m_data->appendWithRef(variant(v), false);
    return *this;
  }

  ArrayInit &setRef(CVarRef v) {
    m_data->appendRef(v, false);
    return *this;
  }

  ArrayInit &set(int64_t name, CVarRef v, bool keyConverted = false) {
    m_data->set(name, v, false);
    return *this;
  }

  // set(const char*) deprecated.  Use set(CStrRef) with a StaticString,
  // if you have a literal, or String otherwise.
  ArrayInit &set(const char*, CVarRef v, bool keyConverted = false) = delete;

  ArrayInit &set(CStrRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else if (!name.isNull()) {
      m_data->set(name.toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &set(CVarRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        m_data->set(k, v, false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &set(const T &name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        m_data->set(k, v, false);
      }
    }
    return *this;
  }

  ArrayInit &set(CStrRef name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else if (!name.isNull()) {
      m_data->setRef(name.toKey(), variant(v), false);
    }
    return *this;
  }

  ArrayInit &set(CVarRef name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        m_data->setRef(k, variant(v), false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &set(const T &name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        m_data->setRef(k, variant(v), false);
      }
    }
    return *this;
  }

  ArrayInit &add(int64_t name, CVarRef v, bool keyConverted = false) {
    m_data->add(name, v, false);
    return *this;
  }

  ArrayInit &add(CStrRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->add(name, v, false);
    } else if (!name.isNull()) {
      m_data->add(name.toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &add(CVarRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->add(name, v, false);
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        m_data->add(k, v, false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &add(const T &name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->add(name, v, false);
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        m_data->add(k, v, false);
      }
    }
    return *this;
  }

  ArrayInit &setRef(int64_t name, CVarRef v, bool keyConverted = false) {
    m_data->setRef(name, v, false);
    return *this;
  }

  ArrayInit &setRef(CStrRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      m_data->setRef(name.toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &setRef(CVarRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      Variant key(name.toKey());
      if (!key.isNull()) {
        m_data->setRef(key, v, false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &setRef(const T &name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      VarNR key(Variant(name).toKey());
      if (!key.isNull()) {
        m_data->setRef(key, v, false);
      }
    }
    return *this;
  }

  // Prefer toArray() in new code---it can save a null check when the
  // compiler can't prove m_data hasn't changed.
  ArrayData *create() {
    ArrayData *ret = m_data;
    m_data = nullptr;
    return ret;
  }

  Array toArray() {
    auto ptr = m_data;
    m_data = nullptr;
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  Variant toVariant() {
    auto ptr = m_data;
    m_data = nullptr;
    return Variant(ptr, Variant::ArrayInitCtor::Tag);
  }

private:
  ArrayData* m_data;
};

//////////////////////////////////////////////////////////////////////

/*
 * Initializer for a vector-shaped array.
 */
class PackedArrayInit {
public:
  explicit PackedArrayInit(size_t n) : m_vec(HphpArray::MakeReserve(n)) {
    m_vec->setRefCount(0);
  }

  PackedArrayInit(PackedArrayInit&& other) : m_vec(other.m_vec) {
    other.m_vec = nullptr;
  }

  PackedArrayInit(const PackedArrayInit&) = delete;
  PackedArrayInit& operator=(const PackedArrayInit&) = delete;

  ~PackedArrayInit() {
    // In case an exception interrupts the initialization.
    if (m_vec) m_vec->release();
  }

  PackedArrayInit& add(CVarRef v) {
    m_vec->nvAppend(v.asTypedValue());
    return *this;
  }

  PackedArrayInit& add(CVarWithRefBind v) {
    HphpArray::AppendWithRefPacked(m_vec, variant(v), false);
    return *this;
  }

  PackedArrayInit& add(RefResult v) {
    HphpArray::AppendRefPacked(m_vec, variant(v), false);
    return *this;
  }

  Variant toVariant() {
    auto ptr = m_vec;
    m_vec = nullptr;
    return Variant(ptr, Variant::ArrayInitCtor::Tag);
  }

  Array toArray() {
    ArrayData* ptr = m_vec;
    m_vec = nullptr;
    return Array(ptr, Array::ArrayInitCtor::Tag);
  }

  ArrayData *create() {
    auto ptr = m_vec;
    m_vec = nullptr;
    return ptr;
  }

private:
  HphpArray* m_vec;
};

//////////////////////////////////////////////////////////////////////

namespace make_array_detail {

  inline void packed_impl(PackedArrayInit&) {}

  template<class Val, class... Vals>
  void packed_impl(PackedArrayInit& init, Val&& val, Vals&&... vals) {
    init.add(std::forward<Val>(val));
    packed_impl(init, std::forward<Vals>(vals)...);
  }

  inline String init_key(const char* s) { return String(s); }
  inline int64_t init_key(int k) { return k; }
  inline int64_t init_key(int64_t k) { return k; }
  inline CStrRef init_key(CStrRef k) { return k; }

  inline void map_impl(ArrayInit&) {}

  template<class Key, class Val, class... KVPairs>
  void map_impl(ArrayInit& init, Key&& key, Val&& val, KVPairs&&... kvpairs) {
    init.set(init_key(std::forward<Key>(key)), std::forward<Val>(val));
    map_impl(init, std::forward<KVPairs>(kvpairs)...);
  }

}

/*
 * Helper for creating packed arrays (vector-like).
 *
 * Usage:
 *
 *   auto newArray = make_packed_array(1, 2, 3, 4);
 *
 */
template<class... Vals>
Array make_packed_array(Vals&&... vals) {
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
