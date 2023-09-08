#pragma once

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct BaseVector;
struct BaseMap;
struct c_Vector;

namespace collections {
void deepCopy(tv_lval);
struct PairIterator;
}

struct c_Pair : c_Collection, SystemLib::ClassLoader<"HH\\Pair"> {
  DECLARE_COLLECTIONS_CLASS_NOCTOR(Pair);

  static ObjectData* instanceCtor(Class* /*cls*/) {
    SystemLib::throwInvalidOperationExceptionObject(
      "Pairs cannot be created using the new operator"
    );
  }

  c_Pair() = delete;
  explicit c_Pair(const TypedValue& e0, const TypedValue& e1)
    : c_Collection(c_Pair::classof(), HeaderKind::Pair)
    , m_size(2)
  {
    tvDup(e0, elm0);
    tvDup(e1, elm1);
  }
  enum class NoIncRef {};
  explicit c_Pair(const TypedValue& e0, const TypedValue& e1, NoIncRef)
    : c_Collection(c_Pair::classof(), HeaderKind::Pair)
    , m_size(2)
  {
    tvCopy(e0, elm0);
    tvCopy(e1, elm1);
  }
  ~c_Pair();

  int64_t size() const {
    return 2;
  }

  TypedValue* at(int64_t key) const {
    if (UNLIKELY(uint64_t(key) >= uint64_t(2))) {
      collections::throwOOB(key);
      return nullptr;
    }
    return const_cast<TypedValue*>(&getElms()[key]);
  }

  TypedValue* get(int64_t key) const {
    if (uint64_t(key) >= uint64_t(2)) {
      return nullptr;
    }
    return const_cast<TypedValue*>(&getElms()[key]);
  }

  bool contains(int64_t key) const {
    return (uint64_t(key) < uint64_t(2));
  }

  int64_t linearSearch(const Variant& value) const;

  /* === ObjectData helpers === */

  static c_Pair* Clone(ObjectData* obj);
  static bool ToBool(const ObjectData* obj) {
    assertx(obj->getVMClass() == c_Pair::classof());
    return true;
  }
  template <IntishCast intishCast = IntishCast::None>
  static Array ToArray(const ObjectData* obj) {
    auto pair = static_cast<const c_Pair*>(obj);
    check_collection_cast_to_array();
    return pair->toPHPArrayImpl();
  }

  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    auto pair = static_cast<c_Pair*>(obj);
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? pair->at(key->m_data.num)
                         : pair->get(key->m_data.num);
    }
    throwBadKeyType();
    return nullptr;
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  static constexpr uint32_t dataOffset() { return offsetof(c_Pair, elm0); }

  void scan(type_scan::Scanner& scanner) const {
    scanner.scan(elm0, 2 * sizeof(elm0));
  }

 private:
  Variant php_at(const Variant& key) const {
    auto* k = key.asTypedValue();
    if (k->m_type == KindOfInt64) {
      return Variant(tvAsCVarRef(at(k->m_data.num)), Variant::TVDup());
    }
    throwBadKeyType();
  }
  Variant php_get(const Variant& key) const {
    auto* k = key.asTypedValue();
    if (k->m_type == KindOfInt64) {
      if (auto tv = get(k->m_data.num)) {
        return Variant(tvAsCVarRef(tv), Variant::TVDup());
      } else {
        return init_null_variant;
      }
    }
    throwBadKeyType();
  }

  Array toPHPArrayImpl() const;

  Object getIterator();

  [[noreturn]] static void throwBadKeyType();

  TypedValue* getElms() { return &elm0; }
  const TypedValue* getElms() const { return &elm0; }

  // make sure we're aligned to 8 bytes, otherwise in non-lowptr
  // builds, m_size will fill a hole in ObjectData, and won't be at
  // collections::FAST_SIZE_OFFSET (see static_assert below).
  alignas(8) uint32_t m_size;

  TypedValue elm0;
  TypedValue elm1;

  friend void collections::deepCopy(tv_lval);
  friend struct collections::PairIterator;
  friend struct collections::CollectionsExtension;
  friend struct c_Vector;
  friend struct BaseVector;
  friend struct BaseMap;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(c_Pair, m_size) ==
                  collections::FAST_SIZE_OFFSET, "");
  }
};

namespace collections {
/////////////////////////////////////////////////////////////////////////////

struct PairIterator : SystemLib::ClassLoader<"PairIterator"> {
  PairIterator() {}
  PairIterator(const PairIterator& src) = delete;
  PairIterator& operator=(const PairIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    return *this;
  }
  ~PairIterator() {}

  static Object newInstance() {
    return Object{ classof() };
  }

  void setPair(c_Pair* pr) {
    m_obj = pr;
    m_pos = 0;
  }

  Variant current() const {
    auto pair = m_obj.get();
    if (!pair->contains(m_pos)) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(&pair->getElms()[m_pos]);
  }

  int64_t key() const {
    auto pair = m_obj.get();
    if (!pair->contains(m_pos)) {
      throw_iterator_not_valid();
    }
    return m_pos;
  }

  bool valid() const {
    static_assert(std::is_unsigned<decltype(m_pos)>::value,
                  "m_pos should be unsigned");
    return m_obj && (m_pos < 2);
  }

  void next()   { ++m_pos;   }
  void rewind() { m_pos = 0; }

 private:
  req::ptr<c_Pair> m_obj;
  uint32_t m_pos{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
