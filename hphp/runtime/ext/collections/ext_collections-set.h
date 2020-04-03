#ifndef incl_HPHP_EXT_COLLECTIONS_SET_H
#define incl_HPHP_EXT_COLLECTIONS_SET_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// BaseSet

struct BaseVector;

namespace collections {
struct SetIterator;
}

/**
 * BaseSet is a hash-table implementation of the Set ADT. It doesn't represent
 * any PHP-land class. That job is delegated to its c_-prefixed child classes.
 */
struct BaseSet : HashCollection {
  void addAllKeysOf(TypedValue container);
  void addAll(const Variant& t);

  void init(const Variant& t) {
    assertx(m_size == 0);
    addAll(t);
  }

protected:
  template<bool raw> void addImpl(int64_t k);
  template<bool raw> void addImpl(StringData* k);

  void addRaw(int64_t k);
  void addRaw(StringData* k);
  void addRaw(TypedValue tv) {
    if (tv.m_type == KindOfInt64) {
      addRaw(tv.m_data.num);
    } else if (isStringType(tv.m_type)) {
      addRaw(tv.m_data.pstr);
    } else {
      throwBadValueType();
    }
  }
  void addRaw(const Variant& v) { addRaw(*v.asTypedValue()); }

public:
  /*
   * Append an element to the Set, increffing it if it's refcounted.
   */
  void add(int64_t k);
  void add(StringData* k);
  void add(TypedValue tv) {
    if (tv.m_type == KindOfInt64) {
      add(tv.m_data.num);
    } else if (isStringType(tv.m_type)) {
      add(tv.m_data.pstr);
    } else {
      throwBadValueType();
    }
  }
  void add(const Variant& v) { add(*v.asTypedValue()); }

  /*
   * Prepend an element to the Set, increffing it if it's refcounted.
   */
  void addFront(int64_t k);
  void addFront(StringData* k);
  void addFront(TypedValue tv) {
    if (tv.m_type == KindOfInt64) {
      addFront(tv.m_data.num);
    } else if (isStringType(tv.m_type)) {
      addFront(tv.m_data.pstr);
    } else {
      throwBadValueType();
    }
  }

  Variant firstValue();
  Variant lastValue();
  Variant pop();
  Variant popFront();

  Array toPHPArray();

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, TSet*>::type
  static Clone(ObjectData* obj);

  template <IntishCast intishCast = IntishCast::None>
  static Array ToArray(const ObjectData* obj) {
    check_collection_cast_to_array();
    return const_cast<BaseSet*>(
      static_cast<const BaseSet*>(obj)
    )->toPHPArrayImpl<intishCast>();
  }

  static bool ToBool(const ObjectData* obj);

  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    auto set = static_cast<BaseSet*>(obj);
    ssize_t p;
    if (key->m_type == KindOfInt64) {
      p = set->find(key->m_data.num, hash_int64(key->m_data.num));
    } else if (isStringType(key->m_type)) {
      p = set->find(key->m_data.pstr, key->m_data.pstr->hash());
    } else {
      BaseSet::throwBadValueType();
    }
    if (LIKELY(p != Empty)) {
      return reinterpret_cast<TypedValue*>(&set->data()[p].data);
    }
    if (!throwOnMiss) {
      return nullptr;
    }
    if (key->m_type == KindOfInt64) {
      collections::throwUndef(key->m_data.num);
    } else {
      assertx(isStringType(key->m_type));
      collections::throwUndef(key->m_data.pstr);
    }
  }
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

protected:
  template<class TVector>
  Object php_values() {
    auto vec = req::make<TVector>();
    vec->init(VarNR(this));
    return Object{std::move(vec)};
  }

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_zip(const Variant& iterable);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_take(const Variant& n);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_skip(const Variant& n);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_slice(const Variant& start, const Variant& len);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_concat(const Variant& iterable);

  template<class TSet>
  static typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  fromItems(const Class*, const Variant& iterable) {
    auto set = req::make<TSet>();
    set->addAll(iterable);
    return Object(std::move(set));
  }

  template<class TSet>
  static typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  fromKeysOf(const Class*, const Variant& container) {
    if (container.isNull()) {
      return Object(req::make<TSet>());
    }
    auto const& cellContainer = container_as_tv(container);
    auto target = req::make<TSet>();
    target->addAllKeysOf(cellContainer);
    return Object(std::move(target));
  }

  template<class TSet>
  static typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  fromArray(const Class*, const Variant& arr) {
    if (!arr.isArray()) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Parameter arr must be an array");
    }
    auto set = req::make<TSet>();
    ArrayData* ad = arr.getArrayData();
    auto oldCap = set->cap();
    set->reserve(ad->size()); // presume minimum collisions ...
    ssize_t pos_limit = ad->iter_end();
    for (ssize_t pos = ad->iter_begin(); pos != pos_limit;
         pos = ad->iter_advance(pos)) {
      set->addRaw(ad->nvGetVal(pos));
    }
    set->shrinkIfCapacityTooHigh(oldCap); // ... and shrink if we were wrong
    return Object(std::move(set));
  }

  Object getIterator();
  bool php_contains(const Variant& key) {
    DataType t = key.getType();
    if (t == KindOfInt64) {
      return contains(key.toInt64());
    }
    if (isStringType(t)) {
      return contains(key.getStringData());
    }
    throwBadValueType();
  }

public:
  [[noreturn]] static void throwNoMutableIndexAccess();
  [[noreturn]] static void throwBadValueType();

protected:
  // BaseSet is an abstract class with no additional member needing
  // initialization.
  using HashCollection::HashCollection;

  ~BaseSet();

private:

  friend struct collections::CollectionsExtension;
  friend struct collections::SetIterator;
  friend struct c_Vector;
  friend struct c_Set;
  friend struct c_Map;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(BaseSet, m_size) ==
                  collections::FAST_SIZE_OFFSET, "");
  }
};

/////////////////////////////////////////////////////////////////////////////
// c_Set

struct c_Set : BaseSet {
  DECLARE_COLLECTIONS_CLASS(Set)

 public:
  // PHP-land methods.
  explicit c_Set()
    : BaseSet(c_Set::classof(), HeaderKind::Set) { }
  explicit c_Set(ArrayData* arr)
    : BaseSet(c_Set::classof(), HeaderKind::Set, arr) { }
  explicit c_Set(uint32_t cap)
    : BaseSet(c_Set::classof(), HeaderKind::Set, cap) { }

  void clear();
  static c_Set* Clone(ObjectData* obj);

 protected:
  friend struct collections::CollectionsExtension;

  Object getImmutableCopy();
  Object php_add(const Variant& val) {
    add(val);
    return Object{this};
  }
  Object php_addAll(const Variant& it) {
    addAll(it);
    return Object{this};
  }
  Object php_addAllKeysOf(const Variant& container) {
    if (!container.isNull()) {
      auto const& containerCell = container_as_tv(container);
      addAllKeysOf(containerCell);
    }
    return Object{this};
  }
  Object php_clear() {
    clear();
    return Object{this};
  }
  Object php_remove(const Variant& key) {
    DataType t = key.getType();
    if (t == KindOfInt64) {
      remove(key.toInt64());
    } else if (isStringType(t)) {
      remove(key.getStringData());
    } else {
      throwBadValueType();
    }
    return Object{this};
  }
  Object php_removeAll(const Variant& it) {
    size_t sz;
    ArrayIter iter = getArrayIterHelper(it, sz);
    for (; iter; ++iter) {
      Variant v = iter.second();
      if (v.isInteger()) {
        remove(v.toInt64());
      } else if (v.isString()) {
        remove(v.getStringData());
      } else {
        throwBadValueType();
      }
    }
    return Object{this};
  }
  void php_reserve(int64_t cap) {
    if (UNLIKELY(cap < 0)) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Parameter sz must be a non-negative integer"
      );
    }
    reserve(cap);
  }
};

///////////////////////////////////////////////////////////////////////////////
// class ImmSet

struct c_ImmSet : BaseSet {
  DECLARE_COLLECTIONS_CLASS(ImmSet)

  explicit c_ImmSet()
    : BaseSet(c_ImmSet::classof(), HeaderKind::ImmSet) { }
  explicit c_ImmSet(ArrayData* arr)
    : BaseSet(c_ImmSet::classof(), HeaderKind::ImmSet, arr) { }
  explicit c_ImmSet(uint32_t cap)
    : BaseSet(c_ImmSet::classof(), HeaderKind::ImmSet, cap) { }

  static c_ImmSet* Clone(ObjectData* obj);
};

namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_SetIterator;

struct SetIterator {
  SetIterator() {}
  SetIterator(const SetIterator& src) = delete;
  SetIterator& operator=(const SetIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    return *this;
  }
  ~SetIterator() {}

  static Object newInstance() {
    static Class* cls = Unit::lookupClass(s_SetIterator.get());
    assertx(cls);
    return Object{cls};
  }

  void setSet(BaseSet* mp) {
    m_obj = mp;
    m_pos = mp->iter_begin();
  }

  Variant current() const {
    auto st = m_obj.get();
    if (!st->iter_valid(m_pos)) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(st->iter_value(m_pos));
  }

  Variant key() const { return current(); }

  bool valid() const {
    return m_obj->iter_valid(m_pos);
  }

  void next() {
    auto st = m_obj.get();
    m_pos = st->iter_next(m_pos);
  }

  void rewind() {
    auto st = m_obj.get();
    m_pos = st->iter_begin();
  }

 private:
  req::ptr<BaseSet> m_obj;
  uint32_t m_pos{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
