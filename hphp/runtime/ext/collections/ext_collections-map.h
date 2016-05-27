#ifndef incl_HPHP_EXT_COLLECTIONS_MAP_H
#define incl_HPHP_EXT_COLLECTIONS_MAP_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

namespace collections {
struct MapIterator;
void deepCopy(TypedValue*);
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
    assert(m_size == 0);
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

  void add(const TypedValue* val);
  void add(const Variant& val) { add(val.asCell()); }

  void set(int64_t k, const TypedValue* data);
  void set(StringData* key, const TypedValue* data);
  void set(int64_t k, const Variant& data) {
    set(k, data.asCell());
  }
  void set(StringData* key, const Variant& data) {
    set(key, data.asCell());
  }
  void set(const TypedValue* key, const TypedValue* data) {
    assert(key->m_type != KindOfRef);
    if (key->m_type == KindOfInt64) {
      set(key->m_data.num, data);
    } else if (isStringType(key->m_type)) {
      set(key->m_data.pstr, data);
    } else {
      throwBadKeyType();
    }
  }
  void set(const Variant& key, const Variant& data) {
    set(key.asCell(), data.asCell());
  }

  Variant pop();
  Variant popFront();

 public:
  static Array ToArray(const ObjectData* obj);
  static bool ToBool(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key) {
    assertx(key->m_type != KindOfRef);
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
  static bool OffsetEmpty(ObjectData* obj, const TypedValue* key);
  static bool OffsetContains(ObjectData* obj, const TypedValue* key);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  enum EqualityFlavor { OrderMatters, OrderIrrelevant };

  static bool Equals(EqualityFlavor eq,
                     const ObjectData* obj1, const ObjectData* obj2);

  [[noreturn]] static void throwBadKeyType();

  static bool instanceof(const ObjectData*);

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
    auto p = find(key, hashint(key));
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

  template<bool raw>
  void setImpl(int64_t k, const TypedValue* val);
  template<bool raw>
  void setImpl(StringData* key, const TypedValue* data);

  // setRaw() assigns a value to the specified key in this Map, but doesn't
  // check for an immutable buffer and doesn't increment m_version, so it's
  // only safe to use in some cases. If you're not sure, use set() instead.
  void setRaw(int64_t k, const TypedValue* data);
  void setRaw(StringData* key, const TypedValue* data);
  void setRaw(int64_t k, const Variant& data) {
    setRaw(k, data.asCell());
  }
  void setRaw(StringData* key, const Variant& data) {
    setRaw(key, data.asCell());
  }
  void setRaw(const TypedValue* key, const TypedValue* data) {
    assert(key->m_type != KindOfRef);
    if (key->m_type == KindOfInt64) {
      setRaw(key->m_data.num, data);
    } else if (isStringType(key->m_type)) {
      setRaw(key->m_data.pstr, data);
    } else {
      throwBadKeyType();
    }
  }
  void setRaw(const Variant& key, const Variant& data) {
    setRaw(key.asCell(), data.asCell());
  }

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_differenceByKey(const Variant& it);

  template<bool useKey>
  Object php_retain(const Variant& callback);

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
  php_skipWhile(const Variant& fn);

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
    assert(target->canMutateBuffer());
    target->setSize(sz);
    auto* out = target->data();
    auto* eLimit = elmLimit();
    for (auto* e = firstElm(); e != eLimit; e = nextElm(e, eLimit), ++out) {
      cellDup(e->data, *out);
    }
    return Object{std::move(target)};
  }

  template<class TVector>
  Object php_keys() {
    auto vec = req::make<TVector>();
    vec->reserve(m_size);
    assert(vec->canMutateBuffer());
    auto* e = firstElm();
    auto* eLimit = elmLimit();
    ssize_t j = 0;
    for (; e != eLimit; e = nextElm(e, eLimit), vec->incSize(), ++j) {
      if (e->hasIntKey()) {
        vec->data()[j].m_data.num = e->ikey;
        vec->data()[j].m_type = KindOfInt64;
      } else {
        assert(e->hasStrKey());
        cellDup(make_tv<KindOfString>(e->skey), vec->data()[j]);
      }
    }
    return Object{std::move(vec)};
  }

 private:
  friend void collections::deepCopy(TypedValue*);

  friend struct collections::CollectionsExtension;
  friend struct collections::MapIterator;
  friend struct c_Vector;
  friend struct c_Map;
  friend struct c_ImmMap;
  friend struct ArrayIter;
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

  explicit c_Map(Class* cls = c_Map::classof())
    : BaseMap(cls, HeaderKind::Map) { }
  explicit c_Map(Class* cls, ArrayData* arr)
    : BaseMap(cls, HeaderKind::Map, arr) { }
  explicit c_Map(Class* cls, uint32_t cap)
    : BaseMap(cls, HeaderKind::Map, cap) { }
  explicit c_Map(uint32_t cap, Class* cls = c_Map::classof())
    : c_Map(cls, cap) { }

  void addAll(const Variant& t) {
    addAllImpl(t);
  }
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
  explicit c_ImmMap(Class* cls = c_ImmMap::classof())
    : BaseMap(cls, HeaderKind::ImmMap) { }
  explicit c_ImmMap(Class* cls, ArrayData* arr)
    : BaseMap(cls, HeaderKind::ImmMap, arr) { }
  explicit c_ImmMap(Class* cls, uint32_t cap)
    : BaseMap(cls, HeaderKind::ImmMap, cap) { }
  explicit c_ImmMap(uint32_t cap, Class* cls = c_ImmMap::classof())
    : c_ImmMap(cls, cap) { }

  static c_ImmMap* Clone(ObjectData* obj);

 public:
  friend struct BaseMap;
  friend struct c_Map;
};

inline bool BaseMap::instanceof(const ObjectData* obj) {
  return c_Map::instanceof(obj) ||
         c_ImmMap::instanceof(obj);
}

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
    m_version = src.m_version;
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
    m_version = mp->getVersion();
  }

  Variant current() const {
    auto const mp = m_obj.get();
    if (UNLIKELY(m_version != mp->getVersion())) {
      throw_collection_modified();
    }
    if (!mp->iter_valid(m_pos)) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(mp->iter_value(m_pos));
  }

  Variant key() const {
    auto const mp = m_obj.get();
    if (UNLIKELY(m_version != mp->getVersion())) {
      throw_collection_modified();
    }
    if (!mp->iter_valid(m_pos)) {
      throw_iterator_not_valid();
    }
    return mp->iter_key(m_pos);
  }

  bool valid() const {
    return m_obj->iter_valid(m_pos);
  }

  void next()   {
    auto const mp = m_obj.get();
    if (UNLIKELY(m_version != mp->getVersion())) {
      throw_collection_modified();
    }
    m_pos = mp->iter_next(m_pos);
  }

  void rewind() {
    auto const mp = m_obj.get();
    if (UNLIKELY(m_version != mp->getVersion())) {
      throw_collection_modified();
    }
    m_pos = mp->iter_begin();
  }

 private:
  req::ptr<BaseMap> m_obj;
  uint32_t m_pos{0};
  int32_t  m_version{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
