#ifndef incl_HPHP_EXT_COLLECTIONS_MAP_H
#define incl_HPHP_EXT_COLLECTIONS_MAP_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct BaseVector;

namespace collections {
struct MapIterator;
void deepCopy(tv_lval);
}

/**
 * BaseMap is a hash-table implementation with int and string keys only.
 * It doesn't represent any PHP-land class; that job is delegated to its
 * c_-prefixed child classes.
 */
struct BaseMap : HashCollection {
protected:
  // BaseMap is an abstract class, with no additional member needing
  // initialization.
  using HashCollection::HashCollection;
  ~BaseMap();

public:
  // init(), used by Map::__construct()
  // expects an iterable of key=>value
  void init(const Variant& t) {
    assertx(m_size == 0);
    addAllImpl(t);
  }
  // addAllPairs(), used by Map::addAll()
  // expects an iterable of Pair objects
  void addAllPairs(const Variant& iterable);

  TypedValue* at(int64_t key) const      { return atImpl<true>(key);  }
  TypedValue* at(StringData* key) const  { return atImpl<true>(key);  }
  TypedValue* get(int64_t key) const     { return atImpl<false>(key); }
  TypedValue* get(StringData* key) const { return atImpl<false>(key); }

  Variant firstValue();
  Variant firstKey();
  Variant lastValue();
  Variant lastKey();

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, TMap*>::type
  static Clone(ObjectData* obj);

  /*
   * Append `v' to the Map and incref it if it's refcounted.
   */
  void add(TypedValue v);
  void add(const Variant& v) { add(*v.asTypedValue()); }

  /*
   * Add `k' => `v' to the Map, increffing each if it's refcounted.
   */
  void set(int64_t k, TypedValue v);
  void set(StringData* k, TypedValue v);
  void set(int64_t k, const Variant& v) { set(k, *v.asTypedValue()); }
  void set(StringData* k, const Variant& v) { set(k, *v.asTypedValue()); }
  void set(TypedValue k, TypedValue v) {
    if (k.m_type == KindOfInt64) {
      set(k.m_data.num, v);
    } else if (isStringType(k.m_type)) {
      set(k.m_data.pstr, v);
    } else {
      throwBadKeyType();
    }
  }
  void set(const Variant& k, const Variant& v) {
    set(*k.asTypedValue(), *v.asTypedValue());
  }

  /*
   * Add `k` => `v` to the Map without inc-ref-ing the value.
   */
  void setMove(int64_t k, TypedValue v);
  void setMove(StringData* k, TypedValue v);

  Variant pop();
  Variant popFront();

  Array toPHPArray();

public:
  template <IntishCast intishCast = IntishCast::None>
  static Array ToArray(const ObjectData* obj) {
    check_collection_cast_to_array();
    return const_cast<BaseMap*>(
      static_cast<const BaseMap*>(obj)
    )->toPHPArrayImpl<intishCast>();
  }

  static bool ToBool(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    auto map = static_cast<BaseMap*>(obj);
    if (key->m_type == KindOfInt64) {
      return throwOnMiss ? map->at(key->m_data.num)
                         : map->get(key->m_data.num);
    }
    if (isStringType(key->m_type)) {
      return throwOnMiss ? map->at(key->m_data.pstr)
                         : map->get(key->m_data.pstr);
    }
    throwBadKeyType();
    return nullptr;
  }
  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  [[noreturn]] static void throwBadKeyType();

protected:
  Variant php_at(const Variant& key) const {
    if (key.isInteger()) {
      return tvAsCVarRef(atImpl<true>(key.toInt64()));
    }
    if (key.isString()) {
      return tvAsCVarRef(atImpl<true>(key.getStringData()));
    }
    throwBadKeyType();
  }
  bool php_containsKey(const Variant& key) const {
    DataType t = key.getType();
    if (t == KindOfInt64) {
      return contains(key.toInt64());
    }
    if (isStringType(t)) {
      return contains(key.getStringData());
    }
    BaseMap::throwBadKeyType();
  }
  Variant php_get(const Variant& key) const {
    TypedValue *tv;
    if (key.isInteger()) {
      tv = atImpl<false>(key.toInt64());
    } else if (key.isString()) {
      tv = atImpl<false>(key.getStringData());
    } else {
      throwBadKeyType();
    }
    if (tv) return tvAsCVarRef(tv);
    return init_null_variant;
  }

  template<bool throwOnError>
  TypedValue* atImpl(int64_t key) const {
    auto p = find(key, hash_int64(key));
    if (UNLIKELY(p == Empty)) {
      if (throwOnError) {
        collections::throwUndef(key);
      } else {
        return nullptr;
      }
    }
    return const_cast<TypedValue*>(
      static_cast<const TypedValue*>(&(data()[p].data))
    );
  }
  template<bool throwOnError>
  TypedValue* atImpl(StringData* key) const {
    auto p = find(key, key->hash());
    if (UNLIKELY(p == Empty)) {
      if (throwOnError) {
        collections::throwUndef(key);
      } else {
        return nullptr;
      }
    }
    return const_cast<TypedValue*>(
      static_cast<const TypedValue*>(&(data()[p].data))
    );
  }
  Object getIterator();

  void addAllImpl(const Variant& iterable);
  void setAllImpl(const Variant& iterable);

  // Set `k` to `v` in the Map. Do not inc-ref `v`. Do not check for mutation.
  void setImpl(int64_t k, TypedValue v);
  void setImpl(StringData* k, TypedValue v);

  // setRaw() assigns a value to the specified key in this Map, but doesn't
  // check for an immutable buffer, so it's only safe to use in some cases.
  // If you're not sure, use set() instead.
  void setRaw(int64_t k, TypedValue v);
  void setRaw(StringData* key, TypedValue v);
  void setRaw(int64_t k, const Variant& v)     { setRaw(k, *v.asTypedValue()); }
  void setRaw(StringData* k, const Variant& v) { setRaw(k, *v.asTypedValue()); }

  void setRaw(TypedValue k, TypedValue v) {
    if (k.m_type == KindOfInt64) {
      setRaw(k.m_data.num, v);
    } else if (isStringType(k.m_type)) {
      setRaw(k.m_data.pstr, v);
    } else {
      throwBadKeyType();
    }
  }
  void setRaw(const Variant& k, const Variant& v) {
    setRaw(*k.asTypedValue(), *v.asTypedValue());
  }

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_differenceByKey(const Variant& it);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_zip(const Variant& iterable);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_take(const Variant& n);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_skip(const Variant& n);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_slice(const Variant& start, const Variant& len);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_concat(const Variant& iterable);

  template<class TMap>
  static typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  FromItems(const Class*, const Variant& iterable);

  template<class TMap>
  static typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  FromArray(const Class*, const Variant& arr);

  template<class TVector>
  Object php_values() {
    auto target = req::make<TVector>();
    int64_t sz = m_size;
    target->reserve(sz);
    assertx(target->canMutateBuffer());
    target->setSize(sz);
    int64_t out = 0;
    auto* eLimit = elmLimit();
    for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit), ++out) {
      tvDup(e->data, target->dataAt(out));
    }
    return Object{std::move(target)};
  }

  template<class TVector>
  Object php_keys() {
    auto vec = req::make<TVector>();
    vec->reserve(m_size);
    assertx(vec->canMutateBuffer());
    auto* e = firstElm();
    auto* eLimit = elmLimit();
    int64_t j = 0;
    for (; e != eLimit; e = nextElm(e, eLimit), vec->incSize(), ++j) {
      if (e->hasIntKey()) {
        tvCopy(make_tv<KindOfInt64>(e->ikey), vec->dataAt(j));
      } else {
        assertx(e->hasStrKey());
        tvDup(make_tv<KindOfString>(e->skey), vec->dataAt(j));
      }
    }
    return Object{std::move(vec)};
  }

private:
  friend void collections::deepCopy(tv_lval);

  friend struct collections::CollectionsExtension;
  friend struct collections::MapIterator;
  friend struct c_Vector;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct c_AwaitAllWaitHandle;
  friend struct c_GenMapWaitHandle;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(BaseMap, m_size)
                  == collections::FAST_SIZE_OFFSET, "");
  }

};

/////////////////////////////////////////////////////////////////////////////

struct c_Map : BaseMap {
  DECLARE_COLLECTIONS_CLASS(Map);

  explicit c_Map()
    : BaseMap(c_Map::classof(), HeaderKind::Map) { }
  explicit c_Map(ArrayData* arr)
    : BaseMap(c_Map::classof(), HeaderKind::Map, arr) { }
  explicit c_Map(uint32_t cap)
    : BaseMap(c_Map::classof(), HeaderKind::Map, cap) { }

  void setAll(const Variant& t) {
    setAllImpl(t);
  }

  void clear();
  static c_Map* Clone(ObjectData* obj);
  Object getImmutableCopy();
 protected:
  friend struct collections::CollectionsExtension;
  Object php_add(const Variant& pair) {
    add(pair);
    return Object{this};
  }
  Object php_addAll(const Variant& it) {
    addAllPairs(it);
    return Object{this};
  }
  Object php_clear() {
    clear();
    return Object{this};
  }
  Object php_removeKey(const Variant& key) {
    DataType t = key.getType();
    if (t == KindOfInt64) {
      remove(key.toInt64());
    } else if (isStringType(t)) {
      remove(key.getStringData());
    } else {
      throwBadKeyType();
    }
    return Object{this};
  }
  void php_reserve(int64_t cap) {
    if (cap < 0) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Parameter sz must be a non-negative integer"
      );
    }
    reserve(cap);
  }
  Object php_set(const Variant& key, const Variant& value) {
    set(key, value);
    return Object{this};
  }
  Object php_setAll(const Variant& it) {
    setAll(it);
    return Object{this};
  }
};

/////////////////////////////////////////////////////////////////////////////

struct c_ImmMap : BaseMap {
  DECLARE_COLLECTIONS_CLASS(ImmMap)

 public:
  explicit c_ImmMap()
    : BaseMap(c_ImmMap::classof(), HeaderKind::ImmMap) { }
  explicit c_ImmMap(ArrayData* arr)
    : BaseMap(c_ImmMap::classof(), HeaderKind::ImmMap, arr) { }
  explicit c_ImmMap(uint32_t cap)
    : BaseMap(c_ImmMap::classof(), HeaderKind::ImmMap, cap) { }

  static c_ImmMap* Clone(ObjectData* obj);

 public:
  friend struct BaseMap;
  friend struct c_Map;
};

namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_MapIterator;

struct MapIterator {
  MapIterator() {}
  MapIterator(const MapIterator& src) = delete;
  MapIterator& operator=(const MapIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    return *this;
  }
  ~MapIterator() {}

  static Object newInstance() {
    static Class* cls = Unit::lookupClass(s_MapIterator.get());
    assertx(cls);
    return Object{cls};
  }

  void setMap(BaseMap* mp) {
    m_obj = mp;
    m_pos = mp->iter_begin();
  }

  Variant current() const {
    auto const mp = m_obj.get();
    if (!mp->iter_valid(m_pos)) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(mp->iter_value(m_pos));
  }

  Variant key() const {
    auto const mp = m_obj.get();
    if (!mp->iter_valid(m_pos)) {
      throw_iterator_not_valid();
    }
    return mp->iter_key(m_pos);
  }

  bool valid() const {
    return m_obj->iter_valid(m_pos);
  }

  void next() {
    auto const mp = m_obj.get();
    m_pos = mp->iter_next(m_pos);
  }

  void rewind() {
    auto const mp = m_obj.get();
    m_pos = mp->iter_begin();
  }

 private:
  req::ptr<BaseMap> m_obj;
  uint32_t m_pos{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
