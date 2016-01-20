#ifndef incl_HPHP_EXT_COLLECTIONS_PAIR_H
#define incl_HPHP_EXT_COLLECTIONS_PAIR_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

class Header;

namespace collections {
void deepCopy(TypedValue*);
class PairIterator;
}

class c_Pair : public ObjectData {
 public:
  DECLARE_COLLECTIONS_CLASS(Pair);

  enum class NoInit {};

  explicit c_Pair(Class* cls = c_Pair::classof())
    : ObjectData(cls, collections::objectFlags, HeaderKind::Pair)
    , m_size(2)
  {
    tvWriteNull(&elm0);
    tvWriteNull(&elm1);
  }
  explicit c_Pair(NoInit, Class* cls = c_Pair::classof())
    : ObjectData(cls, collections::objectFlags, HeaderKind::Pair)
    , m_size(0) {}
  ~c_Pair();

  int64_t size() const {
    assertx(isFullyConstructed());
    return 2;
  }

  void reserve(int64_t sz) const { assertx(sz == 2); }

  /**
   * Most methods that operate on Pairs can safely assume that all Pairs have
   * two elements that have been initialized. However, methods that deal with
   * initializing and destructing Pairs needs to handle intermediate states
   * where one or both of the elements is uninitialized.
   */
  bool isFullyConstructed() const {
    return m_size == 2;
  }

  TypedValue* at(int64_t key) const {
    assertx(isFullyConstructed());
    if (UNLIKELY(uint64_t(key) >= uint64_t(2))) {
      collections::throwOOB(key);
      return nullptr;
    }
    return const_cast<TypedValue*>(&getElms()[key]);
  }

  TypedValue* get(int64_t key) const {
    assertx(isFullyConstructed());
    if (uint64_t(key) >= uint64_t(2)) {
      return nullptr;
    }
    return const_cast<TypedValue*>(&getElms()[key]);
  }

  bool contains(int64_t key) const {
    assertx(isFullyConstructed());
    return (uint64_t(key) < uint64_t(2));
  }

  int64_t linearSearch(const Variant& value) const;

  /* === ObjectData helpers === */

  static c_Pair* Clone(ObjectData* obj);
  static bool ToBool(const ObjectData* obj) {
    assertx(obj->getVMClass() == c_Pair::classof());
    assertx(static_cast<const c_Pair*>(obj)->isFullyConstructed());
    return true;
  }
  static Array ToArray(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
    auto pair = static_cast<c_Pair*>(obj);
    assertx(pair->isFullyConstructed());
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? pair->at(key->m_data.num)
                         : pair->get(key->m_data.num);
    }
    throwBadKeyType();
    return nullptr;
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  TypedValue* initForUnserialize() {
    m_size = 2;
    elm0.m_type = KindOfNull;
    elm1.m_type = KindOfNull;
    return getElms();
  }
  void initAdd(const TypedValue* val) {
    assertx(!isFullyConstructed());
    assertx(val->m_type != KindOfRef);
    cellDup(*val, getElms()[m_size]);
    ++m_size;
  }
  void initAdd(const Variant& val) {
    initAdd(val.asCell());
  }

  static constexpr uint32_t dataOffset() { return offsetof(c_Pair, elm0); }

 private:
  Variant php_at(const Variant& key) const {
    assertx(isFullyConstructed());
    auto* k = key.asCell();
    if (k->m_type == KindOfInt64) {
      return Variant(tvAsCVarRef(at(k->m_data.num)), Variant::CellDup());
    }
    throwBadKeyType();
  }
  Variant php_get(const Variant& key) const {
    assertx(isFullyConstructed());
    auto* k = key.asCell();
    if (k->m_type == KindOfInt64) {
      if (auto tv = get(k->m_data.num)) {
        return Variant(tvAsCVarRef(tv), Variant::CellDup());
      } else {
        return init_null_variant;
      }
    }
    throwBadKeyType();
  }

  Array toArrayImpl() const;
  Object getIterator();
  int getVersion() const { return 0; }

  ATTRIBUTE_NORETURN static void throwBadKeyType();

  TypedValue* getElms() { return &elm0; }
  const TypedValue* getElms() const { return &elm0; }

  template<class F> friend void scanHeader(const Header*, F& mark);
  template<class F> void scan(F& mark) const {
    if (m_size >= 1) mark(elm0);
    if (m_size >= 2) mark(elm1);
  }

#ifndef USE_LOWPTR
  // Add 4 bytes here to keep m_size aligned the same way as in BaseVector and
  // HashCollection.
  UNUSED uint32_t dummy;
#endif
  uint32_t m_size;

  TypedValue elm0;
  TypedValue elm1;

  friend void collections::deepCopy(TypedValue*);
  friend class collections::PairIterator;
  friend class collections::CollectionsExtension;
  friend class c_Vector;
  friend class BaseVector;
  friend class BaseMap;
  friend class ArrayIter;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(c_Pair, m_size) ==
                  collections::FAST_SIZE_OFFSET, "");
  }
};

namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_PairIterator, s_HH_Pair;

struct PairIterator {
  PairIterator() {}
  PairIterator(const PairIterator& src) = delete;
  PairIterator& operator=(const PairIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    return *this;
  }
  ~PairIterator() {}

  static Object newInstance() {
    static Class* cls = Unit::lookupClass(s_PairIterator.get());
    assertx(cls);
    return Object{cls};
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
#endif
