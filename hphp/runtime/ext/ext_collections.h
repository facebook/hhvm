/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_COLLECTION_H_
#define incl_HPHP_EXT_COLLECTION_H_

#include "hphp/runtime/base/base_includes.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// class Vector

FORWARD_DECLARE_CLASS_BUILTIN(Vector);
class c_Vector : public ExtObjectDataFlags<ObjectData::VectorAttrInit|
                                           ObjectData::UseGet|
                                           ObjectData::UseSet|
                                           ObjectData::UseIsset|
                                           ObjectData::UseUnset> {
 public:
  DECLARE_CLASS(Vector, Vector, ObjectData)

 public:
  explicit c_Vector(Class* cls = c_Vector::s_cls);
  ~c_Vector();
  void freeData();
  void t___construct(CVarRef iterable = null_variant);
  Object t_add(CVarRef val);
  Object t_addall(CVarRef val);
  Object t_append(CVarRef val); // deprecated
  Variant t_pop();
  void t_resize(CVarRef sz, CVarRef value);
  void t_reserve(CVarRef sz);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_view();
  Object t_kvzip();
  Variant t_at(CVarRef key);
  Variant t_get(CVarRef key);
  Object t_set(CVarRef key, CVarRef value);
  Object t_setall(CVarRef iterable);
  Object t_put(CVarRef key, CVarRef value); // deprecated
  bool t_contains(CVarRef key); // deprecated
  bool t_containskey(CVarRef key);
  Object t_removekey(CVarRef key);
  Array t_toarray();
  void t_sort(CVarRef col = uninit_null()); // deprecated
  void t_reverse();
  void t_splice(CVarRef offset, CVarRef len = uninit_null(),
                CVarRef replacement = uninit_null());
  int64_t t_linearsearch(CVarRef search_value);
  void t_shuffle();
  Object t_getiterator();
  Object t_map(CVarRef callback);
  Object t_filter(CVarRef callback);
  Object t_zip(CVarRef iterable);
  String t___tostring();
  Variant t___get(Variant name);
  Variant t___set(Variant name, Variant value);
  bool t___isset(Variant name);
  Variant t___unset(Variant name);
  static Object ti_fromitems(CVarRef iterable);
  static Object ti_fromarray(CVarRef arr); // deprecated
  static Object ti_fromvector(CVarRef vec); // deprecated
  static Variant ti_slice(CVarRef vec, CVarRef offset,
                          CVarRef len = uninit_null());
  static Variant t_slice(CVarRef vec, CVarRef offset,
                         CVarRef len = uninit_null()) {
    return ti_slice(vec, offset, len);
  }

  static void throwOOB(int64_t key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  TypedValue* at(int64_t key) {
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return NULL;
    }
    return &m_data[key];
  }
  TypedValue* get(int64_t key) {
    if ((uint64_t)key >= (uint64_t)m_size) {
      return NULL;
    }
    return &m_data[key];
  }
  void set(int64_t key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return;
    }
    tvRefcountedIncRef(val);
    TypedValue* tv = &m_data[key];
    tvRefcountedDecRef(tv);
    tv->m_data.num = val->m_data.num;
    tv->m_type = val->m_type;
  }
  void add(TypedValue* val) {
    assert(val->m_type != KindOfRef);
    ++m_version;
    if (m_capacity <= m_size) {
      grow();
    }
    tvRefcountedIncRef(val);
    TypedValue* tv = &m_data[m_size];
    tv->m_data.num = val->m_data.num;
    tv->m_type = val->m_type;
    ++m_size;
  }
  void resize(int64_t sz, TypedValue* val);
  bool contains(int64_t key) {
    return ((uint64_t)key < (uint64_t)m_size);
  }
  void reserve(int64_t sz);
  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }


  Array toArrayImpl() const;

  Array o_toArray() const;
  c_Vector* clone();

  enum SortFlavor { IntegerSort, StringSort, GenericSort };

 private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc);

 public:
  void sort(int sort_flags, bool ascending);
  void usort(CVarRef cmp_function);

  static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  static void OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static void OffsetUnset(ObjectData* obj, TypedValue* key);
  static void OffsetAppend(ObjectData* obj, TypedValue* val);
  static bool Equals(ObjectData* obj1, ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

 private:
  int64_t iterInit(TypedValue* valOut);
  int64_t iterInitK(TypedValue* valOut, TypedValue* keyOut);
  int64_t iterNext(ssize_t pos, TypedValue* valOut);
  int64_t iterNextK(ssize_t pos, TypedValue* valOut, TypedValue* keyOut);

  void grow();
  static void throwBadKeyType() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  TypedValue* m_data;
  uint m_size;
  uint m_capacity;
  int32_t m_version;

  friend class c_VectorIterator;
  friend class c_Map;
  friend class c_StableMap;
  friend class c_Pair;
  friend class ArrayIter;
  friend ObjectData* collectionDeepCopyVector(c_Vector* vec);
};

///////////////////////////////////////////////////////////////////////////////
// class VectorIterator

FORWARD_DECLARE_CLASS_BUILTIN(VectorIterator);
class c_VectorIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(VectorIterator, VectorIterator, ObjectData)

 public:
  explicit c_VectorIterator(Class* cls = c_VectorIterator::s_cls);
  ~c_VectorIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<c_Vector> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class c_Vector;
};

///////////////////////////////////////////////////////////////////////////////
// class Map

FORWARD_DECLARE_CLASS_BUILTIN(Map);
class c_Map : public ExtObjectDataFlags<ObjectData::MapAttrInit|
                                        ObjectData::UseGet|
                                        ObjectData::UseSet|
                                        ObjectData::UseIsset|
                                        ObjectData::UseUnset> {
 public:
  DECLARE_CLASS(Map, Map, ObjectData)

 public:
  explicit c_Map(Class* cls = c_Map::s_cls);
  ~c_Map();
  void freeData();
  void t___construct(CVarRef iterable = null_variant);
  Object t_add(CVarRef val);
  Object t_addall(CVarRef val);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_view();
  Object t_kvzip();
  Variant t_at(CVarRef key);
  Variant t_get(CVarRef key);
  Object t_set(CVarRef key, CVarRef value);
  Object t_setall(CVarRef iterable);
  Object t_put(CVarRef key, CVarRef value); // deprecated
  bool t_contains(CVarRef key);
  bool t_containskey(CVarRef key);
  Object t_remove(CVarRef key);
  Object t_removekey(CVarRef key);
  Object t_discard(CVarRef key); // deprecated
  Array t_toarray();
  Array t_copyasarray(); // deprecated
  Array t_tokeysarray(); // deprecated
  Object t_values(); // deprecated
  Array t_tovaluesarray(); // deprecated
  Object t_updatefromarray(CVarRef arr);
  Object t_updatefromiterable(CVarRef it);
  Object t_differencebykey(CVarRef it);
  Object t_getiterator();
  Object t_map(CVarRef callback);
  Object t_filter(CVarRef callback);
  Object t_zip(CVarRef iterable);
  String t___tostring();
  Variant t___get(Variant name);
  Variant t___set(Variant name, Variant value);
  bool t___isset(Variant name);
  Variant t___unset(Variant name);
  static Object ti_fromitems(CVarRef iterable);
  static Object ti_fromarray(CVarRef arr); // deprecated
  static Object ti_fromiterable(CVarRef vec); // deprecated

  static void throwOOB(int64_t key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  TypedValue* at(int64_t key) {
    Bucket* p = find(key);
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  TypedValue* get(int64_t key) {
    Bucket* p = find(key);
    if (p) return &p->data;
    return NULL;
  }
  TypedValue* at(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  TypedValue* get(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (p) return &p->data;
    return NULL;
  }
  void set(int64_t key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    update(key, val);
  }
  void set(StringData* key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    update(key, val);
  }
  void add(TypedValue* val);
  void remove(int64_t key) {
    ++m_version;
    erase(find(key));
  }
  void remove(StringData* key) {
    ++m_version;
    erase(find(key->data(), key->size(), key->hash()));
  }
  bool contains(int64_t key) {
    return find(key);
  }
  bool contains(StringData* key) {
    return find(key->data(), key->size(), key->hash());
  }
  void reserve(int64_t sz) {
    if (int64_t(m_load) + sz - int64_t(m_size) >= computeMaxLoad()) {
      adjustCapacityImpl(sz);
    }
  }
  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }
  Array toArrayImpl() const;

  Array o_toArray() const;
  c_Map* clone();

  static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  static void OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static void OffsetUnset(ObjectData* obj, TypedValue* key);
  static void OffsetAppend(ObjectData* obj, TypedValue* val);
  static bool Equals(ObjectData* obj1, ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  static const int32_t KindOfTombstone = -1;

  struct Bucket {
    /**
     * Buckets are 24 bytes and we allocate Buckets continguously in memory
     * without any padding, so some Buckets span multiple cache lines. We
     * access data.m_aux, data.m_type, and ikey/skey during hash lookup, so we
     * intentionally put the data field first so that the accessed fields are
     * all next to each other, which means that they will be on the same cache
     * line for 87.5% of the buckets.
     *
     * The key is either a string pointer or an int value, and the
     * m_aux.u_hash field in data is used to discriminate the key type.
     * u_hash = 0 means int, nonzero values contain 31 bits of a string's
     * hashcode. It is critical that when we return &data to clients, that
     * they not read or write the m_aux field.
     */
    TypedValueAux data;
    union {
      int64_t ikey;
      StringData *skey;
    };
    inline bool hasStrKey() const { return data.hash() != 0; }
    inline bool hasIntKey() const { return data.hash() == 0; }
    inline void setStrKey(StringData* k, strhash_t h) {
      skey = k;
      skey->incRefCount();
      data.hash() = int32_t(h) | 0x80000000;
    }
    inline void setIntKey(int64_t k) {
      ikey = k;
      data.hash() = 0;
    }
    inline int64_t hashKey() const {
      return data.hash() == 0 ? ikey : data.hash();
    }
    inline int32_t hash() const {
      return data.hash();
    }
    bool validValue() const {
      return (intptr_t(data.m_type) > 0);
    }
    bool empty() const {
      return data.m_type == KindOfUninit;
    }
    bool tombstone() const {
      return data.m_type == KindOfTombstone;
    }
    void dump();
  };

 private:
  int64_t iterInit(TypedValue* valOut);
  int64_t iterInitK(TypedValue* valOut, TypedValue* keyOut);
  int64_t iterNext(ssize_t key, TypedValue* valOut);
  int64_t iterNextK(ssize_t key, TypedValue* valOut, TypedValue* keyOut);

  /**
   * Map uses a power of two for the table size and quadratic probing to
   * resolve hash collisions.
   *
   * When an element is removed from the table, a marker called a "tombstone"
   * is left behind in the slot that the element used to occupy. The tombstone
   * will remain in that slot until either (a) the table is resized, or (b) a
   * new element is inserted into that slot.
   *
   * To ensure that hash lookups are efficient, Map keeps the load factor
   * of the table below 75%. If adding a new element causes the load to
   * increase to 75% or greater, we grow the table to lower the load. Note
   * that tombstones count towards load.
   *
   * To ensure that iteration performance is efficient, Map keeps the ratio
   * of # elements / # slots to be at least 18.75%. If removing an element
   * causes the ratio to drop below 18.75%, we shrink the table to increase
   * the ratio.
   *
   * When a Map has never had any removals performed, the load factor is
   * guaranteed to be between 37.5% and 75% (as long as the Map has at least
   * 2 elements).
   */

  Bucket* m_data;
  uint m_size;
  uint m_load;
  uint m_nLastSlot;
  int32_t m_version;

  size_t numSlots() const {
    return m_nLastSlot + 1;
  }

  // The maximum load factor is 75%.
  size_t computeMaxLoad() const {
    size_t n = numSlots();
    return (n - (n >> 2));
  }

  // When the map is not empty, the minimum allowed ratio
  // of # elements / # slots is 18.75%.
  size_t computeMinElements() const {
    size_t n = numSlots();
    return ((n >> 3) + ((n+8) >> 4));
  }

  // We use this funny-looking helper to make g++ use lea and shl
  // instructions instead of imul when indexing into m_data
  Bucket* fetchBucket(Bucket* data, intptr_t slot) const {
    assert(sizeof(Bucket) == 24);
    assert(sizeof(int64_t) == 8);
    assert(slot >= 0 && slot <= m_nLastSlot);
    intptr_t index = slot + (slot<<1);
    int64_t* ptr = (int64_t*)data;
    return (Bucket*)(&ptr[index]);
  }

  Bucket* fetchBucket(intptr_t slot) const {
    return fetchBucket(m_data, slot);
  }

  Bucket* find(int64_t h) const;
  Bucket* find(const char* k, int len, strhash_t prehash) const;
  Bucket* findForInsert(int64_t h) const;
  Bucket* findForInsert(const char* k, int len, strhash_t prehash) const;
  Bucket* findForNewInsert(size_t h0) const;

  bool update(int64_t h, TypedValue* data);
  bool update(StringData* key, TypedValue* data);
  void erase(Bucket* prev);

  void adjustCapacityImpl(int64_t sz);
  void adjustCapacity() {
    adjustCapacityImpl(m_size);
  }

  void deleteBuckets();

  ssize_t iter_begin() const;
  ssize_t iter_next(ssize_t prev) const;
  ssize_t iter_prev(ssize_t prev) const;
  Variant iter_key(ssize_t pos) const;
  TypedValue* iter_value(ssize_t pos) const;

  static void throwBadKeyType() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  friend ObjectData* collectionDeepCopyMap(c_Map* mp);
  friend class c_MapIterator;
  friend class c_Vector;
  friend class c_StableMap;
  friend class ArrayIter;
  friend class c_GenMapWaitHandle;
};

///////////////////////////////////////////////////////////////////////////////
// class MapIterator

FORWARD_DECLARE_CLASS_BUILTIN(MapIterator);
class c_MapIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(MapIterator, MapIterator, ObjectData)

 public:
  explicit c_MapIterator(Class* cls = c_MapIterator::s_cls);
  ~c_MapIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<c_Map> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class c_Map;
};

///////////////////////////////////////////////////////////////////////////////
// class StableMap

FORWARD_DECLARE_CLASS_BUILTIN(StableMap);
class c_StableMap : public ExtObjectDataFlags<ObjectData::StableMapAttrInit|
                                              ObjectData::UseGet|
                                              ObjectData::UseSet|
                                              ObjectData::UseIsset|
                                              ObjectData::UseUnset> {
 public:
  DECLARE_CLASS(StableMap, StableMap, ObjectData)

 public:
  explicit c_StableMap(Class* cls = c_StableMap::s_cls);
  ~c_StableMap();
  void freeData();
  void t___construct(CVarRef iterable = null_variant);
  Object t_add(CVarRef val);
  Object t_addall(CVarRef val);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_view();
  Object t_kvzip();
  Variant t_at(CVarRef key);
  Variant t_get(CVarRef key);
  Object t_set(CVarRef key, CVarRef value);
  Object t_setall(CVarRef iterable);
  Object t_put(CVarRef key, CVarRef value); // deprecated
  bool t_contains(CVarRef key);
  bool t_containskey(CVarRef key);
  Object t_remove(CVarRef key);
  Object t_removekey(CVarRef key);
  Object t_discard(CVarRef key); // deprecated
  Array t_toarray();
  Array t_copyasarray(); // deprecated
  Array t_tokeysarray(); // deprecated
  Object t_values(); // deprecated
  Array t_tovaluesarray(); // deprecated
  Object t_updatefromarray(CVarRef arr);
  Object t_updatefromiterable(CVarRef it);
  Object t_differencebykey(CVarRef it);
  Object t_getiterator();
  Object t_map(CVarRef callback);
  Object t_filter(CVarRef callback);
  Object t_zip(CVarRef iterable);
  String t___tostring();
  Variant t___get(Variant name);
  Variant t___set(Variant name, Variant value);
  bool t___isset(Variant name);
  Variant t___unset(Variant name);
  static Object ti_fromitems(CVarRef iterable);
  static Object ti_fromarray(CVarRef arr); // deprecated
  static Object ti_fromiterable(CVarRef vec); // deprecated

  static void throwOOB(int64_t key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  TypedValue* at(int64_t key) {
    Bucket* p = find(key);
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  TypedValue* at(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  TypedValue* get(int64_t key) {
    Bucket* p = find(key);
    if (p != NULL) return &p->data;
    return NULL;
  }
  TypedValue* get(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (p != NULL) return &p->data;
    return NULL;
  }
  void set(int64_t key, TypedValue* val) {
    update(key, val);
  }
  void set(StringData* key, TypedValue* val) {
    update(key, val);
  }
  void add(TypedValue* val);
  void remove(int64_t key) {
    ++m_version;
    erase(findForErase(key));
  }
  void remove(StringData* key) {
    ++m_version;
    erase(findForErase(key->data(), key->size(), key->hash()));
  }
  bool contains(int64_t key) {
    return find(key);
  }
  bool contains(StringData* key) {
    return find(key->data(), key->size(), key->hash());
  }
  void reserve(int64_t sz) {
    if (sz > int64_t(m_nTableSize)) {
      adjustCapacityImpl(sz);
    }
  }
  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }
  Array toArrayImpl() const;

  Array o_toArray() const;
  c_StableMap* clone();

  static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  static void OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static void OffsetUnset(ObjectData* obj, TypedValue* key);
  static void OffsetAppend(ObjectData* obj, TypedValue* val);
  static bool Equals(ObjectData* obj1, ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  struct Bucket {
    Bucket() : ikey(0), pListNext(nullptr), pListLast(nullptr), pNext(nullptr) {
      data.hash() = 0;
    }
    explicit Bucket(TypedValue* tv) : ikey(0), pListNext(nullptr),
        pListLast(nullptr), pNext(nullptr) {
      tvDup(*tv, data);
      data.hash() = 0;
    }
    ~Bucket();
    // set the top bit for string hashes to make sure the hash
    // value is never zero. hash value 0 corresponds to integer key.
    static inline int32_t encodeHash(strhash_t h) {
      return int32_t(h) | 0x80000000;
    }

    /* The key is either a string pointer or an int value, and the m_aux.u_hash
     * field in data is used to discriminate the key type. u_hash = 0 means
     * int, nonzero values contain 31 bits of a string's hashcode.
     * It is critical that when we return &data to clients, that they not
     * read or write the m_aux field! */
    TypedValueAux data;
    union {
      int64_t ikey;
      StringData* skey;
    };
    Bucket* pListNext;
    Bucket* pListLast;
    Bucket* pNext;

    inline bool hasStrKey() const { return data.hash() != 0; }
    inline bool hasIntKey() const { return data.hash() == 0; }
    inline void setStrKey(StringData* k, strhash_t h) {
      skey = k;
      skey->incRefCount();
      data.hash() = encodeHash(h);
    }
    inline void setIntKey(int64_t k) {
      ikey = k;
      data.hash() = 0;
    }
    inline int64_t hashKey() const {
      return data.hash() == 0 ? ikey : data.hash();
    }
    inline int32_t hash() const {
      return data.hash();
    }

    /**
     * Memory allocator methods.
     */
    DECLARE_SMART_ALLOCATION(Bucket);
    void dump();
  };

  enum SortFlavor { IntegerSort, StringSort, GenericSort };

 private:
  template <typename AccessorT>
  SortFlavor preSort(Bucket** buffer, const AccessorT& acc, bool checkTypes);

  void postSort(Bucket** buffer);

 public:
  void asort(int sort_flags, bool ascending);
  void ksort(int sort_flags, bool ascending);
  void uasort(CVarRef cmp_function);
  void uksort(CVarRef cmp_function);

 private:
  int64_t iterInit(TypedValue* valOut);
  int64_t iterInitK(TypedValue* valOut, TypedValue* keyOut);
  int64_t iterNext(ssize_t key, TypedValue* valOut);
  int64_t iterNextK(ssize_t key, TypedValue* valOut, TypedValue* keyOut);

  uint m_size;
  uint m_nTableSize;
  uint m_nTableMask;
  int32_t m_version;
  Bucket* m_pListHead;
  Bucket* m_pListTail;
  Bucket** m_arBuckets;

  Bucket* find(int64_t h) const;
  Bucket* find(const char* k, int len, strhash_t prehash) const;
  Bucket** findForErase(int64_t h) const;
  Bucket** findForErase(const char* k, int len, strhash_t prehash) const;

  bool update(int64_t h, TypedValue* data);
  bool update(StringData* key, TypedValue* data);
  void erase(Bucket** prev);

  void adjustCapacityImpl(int64_t sz);
  void adjustCapacity() {
    adjustCapacityImpl(m_size);
  }

  void deleteBuckets();

  ssize_t iter_begin() const;
  ssize_t iter_next(ssize_t prev) const;
  ssize_t iter_prev(ssize_t prev) const;
  Variant iter_key(ssize_t pos) const;
  TypedValue* iter_value(ssize_t pos) const;

  static void throwBadKeyType() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  friend ObjectData* collectionDeepCopyStableMap(c_StableMap* smp);
  friend class c_StableMapIterator;
  friend class c_Vector;
  friend class c_Map;
  friend class ArrayIter;
};

///////////////////////////////////////////////////////////////////////////////
// class StableMapIterator

FORWARD_DECLARE_CLASS_BUILTIN(StableMapIterator);
class c_StableMapIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(StableMapIterator, StableMapIterator, ObjectData)

 public:
  explicit c_StableMapIterator(Class* cls = c_StableMapIterator::s_cls);
  ~c_StableMapIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<c_StableMap> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class c_StableMap;
};

///////////////////////////////////////////////////////////////////////////////
// class Set

FORWARD_DECLARE_CLASS_BUILTIN(Set);
class c_Set : public ExtObjectDataFlags<ObjectData::SetAttrInit|
                                        ObjectData::UseGet|
                                        ObjectData::UseSet|
                                        ObjectData::UseIsset|
                                        ObjectData::UseUnset> {
 public:
  DECLARE_CLASS(Set, Set, ObjectData)

 public:
  static const int32_t KindOfTombstone = -1;

  explicit c_Set(Class* cls = c_Set::s_cls);
  ~c_Set();
  void freeData();
  void t___construct(CVarRef iterable = null_variant);
  Object t_add(CVarRef val);
  Object t_addall(CVarRef val);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_view();
  bool t_contains(CVarRef key);
  Object t_remove(CVarRef key);
  Object t_discard(CVarRef key);
  Array t_toarray();
  Object t_getiterator();
  Object t_map(CVarRef callback);
  Object t_filter(CVarRef callback);
  Object t_zip(CVarRef iterable);
  Object t_difference(CVarRef iterable);
  Object t_updatefromarrayvalues(CVarRef arr);
  Object t_updatefromiterablevalues(CVarRef iterable);
  String t___tostring();
  Variant t___get(Variant name);
  Variant t___set(Variant name, Variant value);
  bool t___isset(Variant name);
  Variant t___unset(Variant name);
  static Object ti_fromitems(CVarRef iterable);
  static Object ti_fromarray(CVarRef arr);
  static Object ti_fromarrays(int _argc,
                              CArrRef _argv = null_array);
  static Object ti_fromiterablevalues(CVarRef iterable);

  static void throwOOB(int64_t key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;
  static void throwNoIndexAccess() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  void add(TypedValue* val);
  void remove(int64_t key) {
    ++m_version;
    erase(find(key));
  }
  void remove(StringData* key) {
    ++m_version;
    erase(find(key->data(), key->size(), key->hash()));
  }
  bool contains(int64_t key) {
    return find(key);
  }
  bool contains(StringData* key) {
    return find(key->data(), key->size(), key->hash());
  }
  void reserve(int64_t sz) {
    if (int64_t(m_load) + sz - int64_t(m_size) >= computeMaxLoad()) {
      adjustCapacityImpl(sz);
    }
  }
  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }
  Array toArrayImpl() const;

  Array o_toArray() const;
  c_Set* clone();

  static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  static void OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static void OffsetUnset(ObjectData* obj, TypedValue* key);
  static void OffsetAppend(ObjectData* obj, TypedValue* val);
  static bool Equals(ObjectData* obj1, ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  struct Bucket {
    /**
     * Buckets are 16 bytes. We use m_aux for our own nefarious purposes.
     * It is critical that when we return &data to clients, that they not
     * read or write the m_aux field.
     */
    TypedValueAux data;
    inline bool hasStr() const { return IS_STRING_TYPE(data.m_type); }
    inline bool hasInt() const { return data.m_type == KindOfInt64; }
    inline void setStr(StringData* k, strhash_t h) {
      k->incRefCount();
      data.m_data.pstr = k;
      data.m_type = KindOfString;
      data.hash() = int32_t(h) | 0x80000000;
    }
    inline void setInt(int64_t k) {
      data.m_data.num = k;
      data.m_type = KindOfInt64;
      data.hash() = int32_t(k) | 0x80000000;
    }
    inline int32_t hash() const {
      return data.hash();
    }
    bool validValue() const {
      return (intptr_t(data.m_type) > 0);
    }
    bool empty() const {
      return data.m_type == KindOfUninit;
    }
    bool tombstone() const {
      return data.m_type == KindOfTombstone;
    }
    void dump();
  };

 private:
  int64_t iterInit(TypedValue* valOut);
  int64_t iterInitK(TypedValue* valOut, TypedValue* keyOut);
  int64_t iterNext(ssize_t key, TypedValue* valOut);
  int64_t iterNextK(ssize_t key, TypedValue* valOut, TypedValue* keyOut);

  /**
   * Set uses a power of two for the table size and quadratic probing to
   * resolve hash collisions, similar to the Map class. See the comments
   * in the Map class for more details on how the hashtable works and how
   * we decide when to grow or shrink the table.
   */

  Bucket* m_data;
  uint m_size;
  uint m_load;
  uint m_nLastSlot;
  int32_t m_version;

  size_t numSlots() const {
    return m_nLastSlot + 1;
  }

  // The maximum load factor is 75%.
  size_t computeMaxLoad() const {
    size_t n = numSlots();
    return (n - (n >> 2));
  }

  // When the map is not empty, the minimum allowed ratio
  // of # elements / # slots is 18.75%.
  size_t computeMinElements() const {
    size_t n = numSlots();
    return ((n >> 3) + ((n+8) >> 4));
  }

  Bucket* fetchBucket(Bucket* data, intptr_t slot) const {
    return &data[slot];
  }

  Bucket* fetchBucket(intptr_t slot) const {
    return fetchBucket(m_data, slot);
  }

  Bucket* find(int64_t h) const;
  Bucket* find(const char* k, int len, strhash_t prehash) const;
  Bucket* findForInsert(int64_t h) const;
  Bucket* findForInsert(const char* k, int len, strhash_t prehash) const;
  Bucket* findForNewInsert(size_t h0) const;

  void update(int64_t h);
  void update(StringData* key);
  void erase(Bucket* prev);

  void adjustCapacityImpl(int64_t sz);
  void adjustCapacity() {
    adjustCapacityImpl(m_size);
  }

  void deleteBuckets();

  ssize_t iter_begin() const;
  ssize_t iter_next(ssize_t prev) const;
  ssize_t iter_prev(ssize_t prev) const;
  const TypedValue* iter_value(ssize_t pos) const;
  Variant iter_key(ssize_t pos) const { return uninit_null(); }

  static void throwBadValueType() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  friend ObjectData* collectionDeepCopySet(c_Set* st);
  friend class c_SetIterator;
  friend class c_Vector;
  friend class c_Map;
  friend class c_StableMap;
  friend class ArrayIter;
};

///////////////////////////////////////////////////////////////////////////////
// class SetIterator

FORWARD_DECLARE_CLASS_BUILTIN(SetIterator);
class c_SetIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(SetIterator, SetIterator, ObjectData)

 public:
  explicit c_SetIterator(Class* cls = c_SetIterator::s_cls);
  ~c_SetIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<c_Set> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class c_Set;
};

///////////////////////////////////////////////////////////////////////////////
// class Pair

FORWARD_DECLARE_CLASS_BUILTIN(Pair);
class c_Pair : public ExtObjectDataFlags<ObjectData::PairAttrInit|
                                          ObjectData::UseGet|
                                          ObjectData::UseSet|
                                          ObjectData::UseIsset|
                                          ObjectData::UseUnset> {
 public:
  DECLARE_CLASS(Pair, Pair, ObjectData)

 public:
  explicit c_Pair(Class* cls = c_Pair::s_cls);
  ~c_Pair();
  void t___construct();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_view();
  Object t_kvzip();
  Variant t_at(CVarRef key);
  Variant t_get(CVarRef key);
  bool t_containskey(CVarRef key);
  Array t_toarray();
  Object t_getiterator();
  Object t_map(CVarRef callback);
  Object t_filter(CVarRef callback);
  Object t_zip(CVarRef iterable);
  String t___tostring();
  Variant t___get(Variant name);
  Variant t___set(Variant name, Variant value);
  bool t___isset(Variant name);
  Variant t___unset(Variant name);

  static void throwOOB(int64_t key) ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  TypedValue* at(int64_t key) {
    if (UNLIKELY(uint64_t(key) >= uint64_t(2))) {
      throwOOB(key);
      return NULL;
    }
    return &getElms()[key];
  }
  TypedValue* get(int64_t key) {
    if (uint64_t(key) >= uint64_t(2)) {
      return NULL;
    }
    return &getElms()[key];
  }
  void add(TypedValue* val) {
    assert(val->m_type != KindOfRef);
    if (m_size == 2) {
      Object e(SystemLib::AllocRuntimeExceptionObject(
        "Cannot add a new element to a Pair"));
      throw e;
    }
    tvRefcountedIncRef(val);
    TypedValue* tv = &getElms()[m_size];
    tv->m_data.num = val->m_data.num;
    tv->m_type = val->m_type;
    ++m_size;
  }
  void addInt(int64_t val) {
    if (m_size == 2) {
      Object e(SystemLib::AllocRuntimeExceptionObject(
        "Cannot add a new element to a Pair"));
      throw e;
    }
    TypedValue* tv = &getElms()[m_size];
    tv->m_data.num = val;
    tv->m_type = KindOfInt64;
    ++m_size;
  }
  bool contains(int64_t key) {
    return (uint64_t(key) < uint64_t(2));
  }

  Array toArrayImpl() const;

  Array o_toArray() const;
  c_Pair* clone();

  static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  static void OffsetSet(ObjectData* obj, TypedValue* key, TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static void OffsetUnset(ObjectData* obj, TypedValue* key);
  static void OffsetAppend(ObjectData* obj, TypedValue* val);
  static bool Equals(ObjectData* obj1, ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);
  int64_t size() const {
    return 2;
  }

 private:
  int64_t iterInit(TypedValue* valOut);
  int64_t iterInitK(TypedValue* valOut, TypedValue* keyOut);
  int64_t iterNext(ssize_t pos, TypedValue* valOut);
  int64_t iterNextK(ssize_t pos, TypedValue* valOut, TypedValue* keyOut);

  static void throwBadKeyType() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  uint m_size;

  // TODO Can we add something here to make sure elm0 is 16-byte aligned?
  TypedValue elm0;
  TypedValue elm1;

  TypedValue* getElms() const {
    return (TypedValue*)(&elm0);
  }

  friend ObjectData* collectionDeepCopyPair(c_Pair* pair);
  friend class c_PairIterator;
  friend class c_Vector;
  friend class c_Map;
  friend class c_StableMap;
  friend class ArrayIter;
};

///////////////////////////////////////////////////////////////////////////////
// class PairIterator

FORWARD_DECLARE_CLASS_BUILTIN(PairIterator);
class c_PairIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(PairIterator, PairIterator, ObjectData)

 public:
  explicit c_PairIterator(Class* cls = c_PairIterator::s_cls);
  ~c_PairIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<c_Pair> m_obj;
  ssize_t m_pos;

  friend class c_Pair;
};

TypedValue* collectionGet(ObjectData* obj, TypedValue* key);
void collectionSet(ObjectData* obj, TypedValue* key, TypedValue* val);
bool collectionIsset(ObjectData* obj, TypedValue* key);
bool collectionEmpty(ObjectData* obj, TypedValue* key);
void collectionUnset(ObjectData* obj, TypedValue* key);
void collectionAppend(ObjectData* obj, TypedValue* val);
Variant& collectionOffsetGet(ObjectData* obj, int64_t offset);
Variant& collectionOffsetGet(ObjectData* obj, CStrRef offset);
Variant& collectionOffsetGet(ObjectData* obj, CVarRef offset);
void collectionOffsetSet(ObjectData* obj, int64_t offset, CVarRef val);
void collectionOffsetSet(ObjectData* obj, CStrRef offset, CVarRef val);
void collectionOffsetSet(ObjectData* obj, CVarRef offset, CVarRef val);
bool collectionOffsetContains(ObjectData* obj, CVarRef offset);
bool collectionOffsetIsset(ObjectData* obj, CVarRef offset);
bool collectionOffsetEmpty(ObjectData* obj, CVarRef offset);
int64_t collectionSize(ObjectData* obj);
void collectionReserve(ObjectData* obj, int64_t sz);
void collectionUnserialize(ObjectData* obj, VariableUnserializer* uns,
                           int64_t sz, char type);
bool collectionEquals(ObjectData* obj1, ObjectData* obj2);
void collectionDeepCopyTV(TypedValue* tv);
ArrayData* collectionDeepCopyArray(ArrayData* arr);
ObjectData* collectionDeepCopyVector(c_Vector* vec);
ObjectData* collectionDeepCopyMap(c_Map* mp);
ObjectData* collectionDeepCopyStableMap(c_StableMap* smp);
ObjectData* collectionDeepCopySet(c_Set* st);
ObjectData* collectionDeepCopyPair(c_Pair* pair);

///////////////////////////////////////////////////////////////////////////////

inline TypedValue* cvarToCell(const Variant* v) {
  return const_cast<TypedValue*>(tvToCell(v->asTypedValue()));
}

inline Variant& collectionOffsetGet(ObjectData* obj, bool offset) {
  return collectionOffsetGet(obj, Variant(offset));
}

inline Variant& collectionOffsetGet(ObjectData* obj, double offset) {
  return collectionOffsetGet(obj, Variant(offset));
}

inline Variant& collectionOffsetGet(ObjectData* obj, litstr offset) {
  return collectionOffsetGet(obj, Variant(offset));
}

inline void collectionOffsetSet(ObjectData* obj, bool offset, CVarRef val) {
  collectionOffsetSet(obj, Variant(offset), val);
}

inline void collectionOffsetSet(ObjectData* obj, double offset, CVarRef val) {
  collectionOffsetSet(obj, Variant(offset), val);
}

inline void collectionOffsetSet(ObjectData* obj, litstr offset, CVarRef val) {
  collectionOffsetSet(obj, Variant(offset), val);
}

inline void collectionOffsetUnset(ObjectData* obj, CVarRef offset) {
  TypedValue* key = cvarToCell(&offset);
  collectionUnset(obj, key);
}

inline void collectionOffsetAppend(ObjectData* obj, CVarRef val) {
  TypedValue* tv = cvarToCell(&val);
  collectionAppend(obj, tv);
}

inline bool isOptimizableCollectionClass(const Class* klass) {
  return klass == c_Vector::s_cls || klass == c_Map::s_cls ||
         klass == c_StableMap::s_cls || klass == c_Pair::s_cls;
}

inline bool isCollectionClass(const Class* klass) {
  return klass == c_Vector::s_cls || klass == c_Map::s_cls ||
         klass == c_StableMap::s_cls || klass == c_Pair::s_cls ||
         klass == c_Set::s_cls;
}

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer);

///////////////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_EXT_COLLECTION_H_
