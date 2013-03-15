/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_COLLECTION_H__
#define __EXT_COLLECTION_H__

#include <runtime/base/base_includes.h>
#include <system/lib/systemlib.h>

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
  friend class c_VectorIterator;
  friend class c_Map;
  friend class c_StableMap;
  friend class ArrayIter;

  enum SortFlavor { IntegerSort, StringSort, GenericSort };

  public: c_Vector(VM::Class* cls = c_Vector::s_cls);
  public: ~c_Vector();
  public: void freeData();
  public: void t___construct(CVarRef iterable = null_variant);
  public: Variant t___destruct();
  public: Object t_add(CVarRef val);
  public: Object t_append(CVarRef val);
  public: Variant t_pop();
  public: void t_resize(CVarRef sz, CVarRef value);
  public: Object t_clear();
  public: bool t_isempty();
  public: int64_t t_count();
  public: Variant t_at(CVarRef key);
  public: Variant t_get(CVarRef key);
  public: Object t_set(CVarRef key, CVarRef value);
  public: Object t_put(CVarRef key, CVarRef value);
  public: bool t_contains(CVarRef key);
  public: Object t_removeat(CVarRef key);
  public: Array t_toarray();
  public: void t_sort(CVarRef col = uninit_null());
  public: void t_reverse();
  public: void t_splice(CVarRef offset, CVarRef len = uninit_null(),
                        CVarRef replacement = uninit_null());
  public: int64_t t_linearsearch(CVarRef search_value);
  public: void t_shuffle();
  public: Object t_getiterator();
  public: String t___tostring();
  public: Variant t___get(Variant name);
  public: Variant t___set(Variant name, Variant value);
  public: bool t___isset(Variant name);
  public: Variant t___unset(Variant name);
  public: static Object ti_fromarray(const char* cls, CVarRef arr);
  public: static Object t_fromarray(CVarRef arr) {
    return ti_fromarray("vector", arr);
  }
  public: static Object ti_fromvector(const char* cls, CVarRef vec);
  public: static Object t_fromvector(CVarRef vec) {
    return ti_fromvector("vector", vec);
  }
  public: static Variant ti_slice(const char* cls, CVarRef vec, CVarRef offset,
                                  CVarRef len = uninit_null());
  public: static Variant t_slice(CVarRef vec, CVarRef offset,
                                 CVarRef len = uninit_null()) {
    return ti_slice("vector", vec, offset, len);
  }

  public: static void throwOOB(int64_t key) ATTRIBUTE_COLD;

  public: TypedValue* at(int64_t key) {
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return NULL;
    }
    return &m_data[key];
  }
  public: TypedValue* get(int64_t key) {
    if ((uint64_t)key >= (uint64_t)m_size) {
      return NULL;
    }
    return &m_data[key];
  }
  public: void put(int64_t key, TypedValue* val) {
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
  public: void add(TypedValue* val) {
    assert(val->m_type != KindOfRef);
    ++m_versionNumber;
    if (m_capacity <= m_size) {
      grow();
    }
    tvRefcountedIncRef(val);
    TypedValue* tv = &m_data[m_size];
    tv->m_data.num = val->m_data.num;
    tv->m_type = val->m_type;
    ++m_size;
  }
  public: void resize(int64_t sz, TypedValue* val);
  public: bool contains(int64_t key) {
    return ((uint64_t)key < (uint64_t)m_size);
  }
  public: void reserve(int64_t sz);
  public: int getVersionNumber() {
    return m_versionNumber;
  }

  public: Array toArrayImpl() const;

  public: Array o_toArray() const;
  public: ObjectData* clone();

  private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc);

  public: void sort(int sort_flags, bool ascending);
  public: void usort(CVarRef cmp_function);

  public: static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  public: static void OffsetSet(ObjectData* obj, TypedValue* key,
                                TypedValue* val);
  public: static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  public: static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  public: static bool OffsetContains(ObjectData* obj, TypedValue* key);
  public: static void OffsetUnset(ObjectData* obj, TypedValue* key);
  public: static void OffsetAppend(ObjectData* obj, TypedValue* val);
  public: static bool Equals(ObjectData* obj1, ObjectData* obj2);


 private:
  void grow();
  static void throwBadKeyType();

  TypedValue* m_data;
  uint m_size;
  uint m_capacity;
  int m_versionNumber;

  friend ObjectData* collectionDeepCopyVector(c_Vector* vec);
};

///////////////////////////////////////////////////////////////////////////////
// class VectorIterator

FORWARD_DECLARE_CLASS_BUILTIN(VectorIterator);
class c_VectorIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(VectorIterator, VectorIterator, ObjectData)
  friend class c_Vector;

  // need to implement
  public: c_VectorIterator(VM::Class* cls = c_VectorIterator::s_cls);
  public: ~c_VectorIterator();
  public: void t___construct();
  public: Variant t_current();
  public: Variant t_key();
  public: bool t_valid();
  public: void t_next();
  public: void t_rewind();


 private:
  SmartPtr<c_Vector> m_obj;
  ssize_t m_pos;
  int m_versionNumber;
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
  friend class c_MapIterator;
  friend class c_Vector;
  friend class c_StableMap;
  friend class ArrayIter;

  public: static const int32_t KindOfTombstone = -1;

  public: c_Map(VM::Class* cls = c_Map::s_cls);
  public: ~c_Map();
  public: void freeData();
  public: void t___construct(CVarRef iterable = null_variant);
  public: Variant t___destruct();
  public: Object t_clear();
  public: bool t_isempty();
  public: int64_t t_count();
  public: Variant t_at(CVarRef key);
  public: Variant t_get(CVarRef key);
  public: Object t_set(CVarRef key, CVarRef value);
  public: Object t_put(CVarRef key, CVarRef value);
  public: bool t_contains(CVarRef key);
  public: Object t_remove(CVarRef key);
  public: Object t_removeat(CVarRef key);
  public: Object t_discard(CVarRef key);
  public: Array t_toarray();
  public: Array t_copyasarray();
  public: Array t_tokeysarray();
  public: Object t_values();
  public: Array t_tovaluesarray();
  public: Object t_updatefromarray(CVarRef arr);
  public: Object t_updatefromiterable(CVarRef it);
  public: Object t_differencebykey(CVarRef it);
  public: Object t_getiterator();
  public: String t___tostring();
  public: Variant t___get(Variant name);
  public: Variant t___set(Variant name, Variant value);
  public: bool t___isset(Variant name);
  public: Variant t___unset(Variant name);
  public: static Object ti_fromarray(const char* cls, CVarRef arr);
  public: static Object t_fromarray(CVarRef arr) {
    return ti_fromarray("map", arr);
  }
  public: static Object ti_fromiterable(const char* cls, CVarRef vec);
  public: static Object t_fromiterable(CVarRef vec) {
    return ti_fromiterable("map", vec);
  }

  public: static void throwOOB(int64_t key) ATTRIBUTE_COLD;
  public: static void throwOOB(StringData* key) ATTRIBUTE_COLD;

  public: TypedValue* at(int64_t key) {
    Bucket* p = find(key);
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  public: TypedValue* get(int64_t key) {
    Bucket* p = find(key);
    if (p) return &p->data;
    return NULL;
  }
  public: TypedValue* at(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  public: TypedValue* get(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (p) return &p->data;
    return NULL;
  }
  public: void put(int64_t key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    update(key, val);
  }
  public: void put(StringData* key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    update(key, val);
  }
  public: void remove(int64_t key) {
    ++m_versionNumber;
    erase(find(key));
  }
  public: void remove(StringData* key) {
    ++m_versionNumber;
    erase(find(key->data(), key->size(), key->hash()));
  }
  public: bool contains(int64_t key) {
    return find(key);
  }
  public: bool contains(StringData* key) {
    return find(key->data(), key->size(), key->hash());
  }
  public: void reserve(int64_t sz);
  public: int getVersionNumber() {
    return m_versionNumber;
  }
  public: Array toArrayImpl() const;

  public: Array o_toArray() const;
  public: ObjectData* clone();

  public: static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  public: static void OffsetSet(ObjectData* obj, TypedValue* key,
                                TypedValue* val);
  public: static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  public: static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  public: static bool OffsetContains(ObjectData* obj, TypedValue* key);
  public: static void OffsetUnset(ObjectData* obj, TypedValue* key);
  public: static void OffsetAppend(ObjectData* obj, TypedValue* val);
  public: static bool Equals(ObjectData* obj1, ObjectData* obj2);


public:
  class Bucket {
  public:
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

  Bucket*          m_data;
  uint             m_size;
  uint             m_load;
  uint             m_nLastSlot;
  int              m_versionNumber;

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

  void resize();
  void deleteBuckets();

  ssize_t iter_begin() const;
  ssize_t iter_next(ssize_t prev) const;
  ssize_t iter_prev(ssize_t prev) const;
  Variant iter_key(ssize_t pos) const;
  Variant iter_value(ssize_t pos) const;

  static void throwBadKeyType();

  friend ObjectData* collectionDeepCopyMap(c_Map* mp);
};

///////////////////////////////////////////////////////////////////////////////
// class MapIterator

FORWARD_DECLARE_CLASS_BUILTIN(MapIterator);
class c_MapIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(MapIterator, MapIterator, ObjectData)
  friend class c_Map;

  // need to implement
  public: c_MapIterator(VM::Class* cls = c_MapIterator::s_cls);
  public: ~c_MapIterator();
  public: void t___construct();
  public: Variant t_current();
  public: Variant t_key();
  public: bool t_valid();
  public: void t_next();
  public: void t_rewind();


 private:
  SmartPtr<c_Map> m_obj;
  ssize_t m_pos;
  int m_versionNumber;
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
  friend class c_StableMapIterator;
  friend class c_Vector;
  friend class c_Map;
  friend class ArrayIter;

  enum SortFlavor { IntegerSort, StringSort, GenericSort };

  public: c_StableMap(VM::Class* cls = c_StableMap::s_cls);
  public: ~c_StableMap();
  public: void freeData();
  public: void t___construct(CVarRef iterable = null_variant);
  public: Variant t___destruct();
  public: Object t_clear();
  public: bool t_isempty();
  public: int64_t t_count();
  public: Variant t_at(CVarRef key);
  public: Variant t_get(CVarRef key);
  public: Object t_set(CVarRef key, CVarRef value);
  public: Object t_put(CVarRef key, CVarRef value);
  public: bool t_contains(CVarRef key);
  public: Object t_remove(CVarRef key);
  public: Object t_removeat(CVarRef key);
  public: Object t_discard(CVarRef key);
  public: Array t_toarray();
  public: Array t_copyasarray();
  public: Array t_tokeysarray();
  public: Object t_values();
  public: Array t_tovaluesarray();
  public: Object t_updatefromarray(CVarRef arr);
  public: Object t_updatefromiterable(CVarRef it);
  public: Object t_differencebykey(CVarRef it);
  public: Object t_getiterator();
  public: String t___tostring();
  public: Variant t___get(Variant name);
  public: Variant t___set(Variant name, Variant value);
  public: bool t___isset(Variant name);
  public: Variant t___unset(Variant name);
  public: static Object ti_fromarray(const char* cls, CVarRef arr);
  public: static Object t_fromarray(CVarRef arr) {
    return ti_fromarray("map", arr);
  }
  public: static Object ti_fromiterable(const char* cls, CVarRef vec);
  public: static Object t_fromiterable(CVarRef vec) {
    return ti_fromiterable("map", vec);
  }

  public: static void throwOOB(int64_t key) ATTRIBUTE_COLD;
  public: static void throwOOB(StringData* key) ATTRIBUTE_COLD;

  public: TypedValue* at(int64_t key) {
    Bucket* p = find(key);
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  public: TypedValue* at(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (LIKELY(p != NULL)) return &p->data;
    throwOOB(key);
    return NULL;
  }
  public: TypedValue* get(int64_t key) {
    Bucket* p = find(key);
    if (p != NULL) return &p->data;
    return NULL;
  }
  public: TypedValue* get(StringData* key) {
    Bucket* p = find(key->data(), key->size(), key->hash());
    if (p != NULL) return &p->data;
    return NULL;
  }
  public: void put(int64_t key, TypedValue* val) {
    update(key, val);
  }
  public: void put(StringData* key, TypedValue* val) {
    update(key, val);
  }
  public: void remove(int64_t key) {
    ++m_versionNumber;
    erase(findForErase(key));
  }
  public: void remove(StringData* key) {
    ++m_versionNumber;
    erase(findForErase(key->data(), key->size(), key->hash()));
  }
  public: bool contains(int64_t key) {
    return find(key);
  }
  public: bool contains(StringData* key) {
    return find(key->data(), key->size(), key->hash());
  }
  public: void reserve(int64_t sz);
  public: int getVersionNumber() {
    return m_versionNumber;
  }
  public: Array toArrayImpl() const;

  public: Array o_toArray() const;
  public: ObjectData* clone();

  public: static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  public: static void OffsetSet(ObjectData* obj, TypedValue* key,
                                TypedValue* val);
  public: static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  public: static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  public: static bool OffsetContains(ObjectData* obj, TypedValue* key);
  public: static void OffsetUnset(ObjectData* obj, TypedValue* key);
  public: static void OffsetAppend(ObjectData* obj, TypedValue* val);
  public: static bool Equals(ObjectData* obj1, ObjectData* obj2);


public:
  class Bucket {
  public:
    Bucket() : ikey(0), pListNext(nullptr), pListLast(nullptr), pNext(nullptr) {
      data.hash() = 0;
    }
    Bucket(TypedValue* tv) : ikey(0), pListNext(nullptr), pListLast(nullptr),
        pNext(nullptr) {
      tvDup(tv, &data);
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

  private:
  template <typename AccessorT>
  SortFlavor preSort(Bucket** buffer, const AccessorT& acc, bool checkTypes);

  private: void postSort(Bucket** buffer);

  public: void asort(int sort_flags, bool ascending);
  public: void ksort(int sort_flags, bool ascending);
  public: void uasort(CVarRef cmp_function);
  public: void uksort(CVarRef cmp_function);

private:
  uint             m_size;
  uint             m_nTableSize;
  uint             m_nTableMask;
  int              m_versionNumber;
  Bucket*          m_pListHead;
  Bucket*          m_pListTail;
  Bucket**         m_arBuckets;

  Bucket* find(int64_t h) const;
  Bucket* find(const char* k, int len, strhash_t prehash) const;
  Bucket** findForErase(int64_t h) const;
  Bucket** findForErase(const char* k, int len, strhash_t prehash) const;

  bool update(int64_t h, TypedValue* data);
  bool update(StringData* key, TypedValue* data);
  void erase(Bucket** prev);

  void resize();
  void deleteBuckets();

  ssize_t iter_begin() const;
  ssize_t iter_next(ssize_t prev) const;
  ssize_t iter_prev(ssize_t prev) const;
  Variant iter_key(ssize_t pos) const;
  Variant iter_value(ssize_t pos) const;

  static void throwBadKeyType();

  friend ObjectData* collectionDeepCopyStableMap(c_StableMap* smp);
};

///////////////////////////////////////////////////////////////////////////////
// class StableMapIterator

FORWARD_DECLARE_CLASS_BUILTIN(StableMapIterator);
class c_StableMapIterator : public ExtObjectData {
 public:
  DECLARE_CLASS(StableMapIterator, StableMapIterator, ObjectData)
  friend class c_StableMap;

  // need to implement
  public: c_StableMapIterator(VM::Class* cls = c_StableMapIterator::s_cls);
  public: ~c_StableMapIterator();
  public: void t___construct();
  public: Variant t_current();
  public: Variant t_key();
  public: bool t_valid();
  public: void t_next();
  public: void t_rewind();


 private:
  SmartPtr<c_StableMap> m_obj;
  ssize_t m_pos;
  int m_versionNumber;
};

///////////////////////////////////////////////////////////////////////////////

// Helpers for hhvm

inline TypedValue* collectionGet(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return c_Vector::OffsetGet(obj, key);
  } else if (ct == Collection::MapType) {
    return c_Map::OffsetGet(obj, key);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::OffsetGet(obj, key);
  } else {
    assert(false);
    return NULL;
  }
}

inline void collectionSet(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assert(key->m_type != KindOfRef);
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector::OffsetSet(obj, key, val);
  } else if (ct == Collection::MapType) {
    c_Map::OffsetSet(obj, key, val);
  } else if (ct == Collection::StableMapType) {
    c_StableMap::OffsetSet(obj, key, val);
  } else {
    assert(false);
  }
}

inline bool collectionIsset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return c_Vector::OffsetIsset(obj, key);
  } else if (ct == Collection::MapType) {
    return c_Map::OffsetIsset(obj, key);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::OffsetIsset(obj, key);
  } else {
    assert(false);
    return false;
  }
}

inline bool collectionEmpty(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return c_Vector::OffsetEmpty(obj, key);
  } else if (ct == Collection::MapType) {
    return c_Map::OffsetEmpty(obj, key);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::OffsetEmpty(obj, key);
  } else {
    assert(false);
    return true;
  }
}

inline void collectionUnset(ObjectData* obj, TypedValue* key) {
  assert(key->m_type != KindOfRef);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector::OffsetUnset(obj, key);
  } else if (ct == Collection::MapType) {
    c_Map::OffsetUnset(obj, key);
  } else if (ct == Collection::StableMapType) {
    c_StableMap::OffsetUnset(obj, key);
  } else {
    assert(false);
  }
}

inline void collectionAppend(ObjectData* obj, TypedValue* val) {
  assert(val->m_type != KindOfRef);
  assert(val->m_type != KindOfUninit);
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector::OffsetAppend(obj, val);
  } else if (ct == Collection::MapType) {
    c_Map::OffsetAppend(obj, val);
  } else if (ct == Collection::StableMapType) {
    c_StableMap::OffsetAppend(obj, val);
  } else {
    assert(false);
  }
}

// Helpers for hphpc

inline Variant& collectionOffsetGet(ObjectData* obj, int64_t offset) {
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector* vec = static_cast<c_Vector*>(obj);
    return *(Variant*)(vec->at(offset));
  } else if (ct == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    return *(Variant*)(mp->at(offset));
  } else if (ct == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    return *(Variant*)(smp->at(offset));
  } else {
    assert(false);
    return *(Variant*)(NULL);
  }
}

inline Variant& collectionOffsetGet(ObjectData* obj, CStrRef offset) {
  StringData* key = offset.get();
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Only integer keys may be used with Vectors"));
    throw e;
    return *(Variant*)(NULL);
  } else if (ct == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    return *(Variant*)(mp->at(key));
  } else if (ct == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    return *(Variant*)(smp->at(key));
  } else {
    assert(false);
    return *(Variant*)(NULL);
  }
}

inline Variant& collectionOffsetGet(ObjectData* obj, CVarRef offset) {
  TypedValue* key = (TypedValue*)(&offset);
  if (key->m_type == KindOfRef) {
    key = key->m_data.pref->tv();
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return *(Variant*)(c_Vector::OffsetGet(obj, key));
  } else if (ct == Collection::MapType) {
    return *(Variant*)(c_Map::OffsetGet(obj, key));
  } else if (ct == Collection::StableMapType) {
    return *(Variant*)(c_StableMap::OffsetGet(obj, key));
  } else {
    assert(false);
    return *(Variant*)(NULL);
  }
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

inline void collectionOffsetSet(ObjectData* obj, int64_t offset, CVarRef val) {
  TypedValue* tv = (TypedValue*)(&val);
  if (UNLIKELY(tv->m_type == KindOfRef)) {
    tv = tv->m_data.pref->tv();
  }
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector* vec = static_cast<c_Vector*>(obj);
    vec->put(offset, tv);
  } else if (ct == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    mp->put(offset, tv);
  } else if (ct == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    smp->put(offset, tv);
  } else {
    assert(false);
  }
}

inline void collectionOffsetSet(ObjectData* obj, CStrRef offset, CVarRef val) {
  StringData* key = offset.get();
  TypedValue* tv = (TypedValue*)(&val);
  if (UNLIKELY(tv->m_type == KindOfRef)) {
    tv = tv->m_data.pref->tv();
  }
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Only integer keys may be used with Vectors"));
    throw e;
  } else if (ct == Collection::MapType) {
    c_Map* mp = static_cast<c_Map*>(obj);
    mp->put(key, tv);
  } else if (ct == Collection::StableMapType) {
    c_StableMap* smp = static_cast<c_StableMap*>(obj);
    smp->put(key, tv);
  } else {
    assert(false);
  }
}

inline void collectionOffsetSet(ObjectData* obj, CVarRef offset, CVarRef val) {
  TypedValue* key = (TypedValue*)(&offset);
  if (UNLIKELY(key->m_type == KindOfRef)) {
    key = key->m_data.pref->tv();
  }
  TypedValue* tv = (TypedValue*)(&val);
  if (UNLIKELY(tv->m_type == KindOfRef)) {
    tv = tv->m_data.pref->tv();
  }
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    c_Vector::OffsetSet(obj, key, tv);
  } else if (ct == Collection::MapType) {
    c_Map::OffsetSet(obj, key, tv);
  } else if (ct == Collection::StableMapType) {
    c_StableMap::OffsetSet(obj, key, tv);
  } else {
    assert(false);
  }
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

inline bool collectionOffsetContains(ObjectData* obj, CVarRef offset) {
  TypedValue* key = (TypedValue*)(&offset);
  if (key->m_type == KindOfRef) {
    key = key->m_data.pref->tv();
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return c_Vector::OffsetContains(obj, key);
  } else if (ct == Collection::MapType) {
    return c_Map::OffsetContains(obj, key);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::OffsetContains(obj, key);
  } else {
    assert(false);
    return false;
  }
}

inline bool collectionOffsetIsset(ObjectData* obj, CVarRef offset) {
  TypedValue* key = (TypedValue*)(&offset);
  if (key->m_type == KindOfRef) {
    key = key->m_data.pref->tv();
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return c_Vector::OffsetIsset(obj, key);
  } else if (ct == Collection::MapType) {
    return c_Map::OffsetIsset(obj, key);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::OffsetIsset(obj, key);
  } else {
    assert(false);
    return false;
  }
}

inline bool collectionOffsetEmpty(ObjectData* obj, CVarRef offset) {
  TypedValue* key = (TypedValue*)(&offset);
  if (key->m_type == KindOfRef) {
    key = key->m_data.pref->tv();
  }
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return c_Vector::OffsetEmpty(obj, key);
  } else if (ct == Collection::MapType) {
    return c_Map::OffsetEmpty(obj, key);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::OffsetEmpty(obj, key);
  } else {
    assert(false);
    return true;
  }
}

inline void collectionOffsetUnset(ObjectData* obj, CVarRef offset) {
  TypedValue* key = (TypedValue*)(&offset);
  if (UNLIKELY(key->m_type == KindOfRef)) {
    key = key->m_data.pref->tv();
  }
  collectionUnset(obj, key);
}

inline void collectionOffsetAppend(ObjectData* obj, CVarRef val) {
  TypedValue* tv = (TypedValue*)(&val);
  if (UNLIKELY(tv->m_type == KindOfRef)) {
    tv = tv->m_data.pref->tv();
  }
  if (UNLIKELY(tv->m_type == KindOfUninit)) {
    tv = (TypedValue*)(&init_null_variant);
  }
  collectionAppend(obj, tv);
}

inline int64_t collectionSize(ObjectData* obj) {
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    return static_cast<c_Vector*>(obj)->t_count();
  } else if (ct == Collection::MapType) {
    return static_cast<c_Map*>(obj)->t_count();
  } else if (ct == Collection::StableMapType) {
    return static_cast<c_StableMap*>(obj)->t_count();
  } else {
    assert(false);
    return 0;
  }
}

inline void collectionReserve(ObjectData* obj, int64_t sz) {
  int ct = obj->getCollectionType();
  if (ct == Collection::VectorType) {
    static_cast<c_Vector*>(obj)->reserve(sz);
  } else if (ct == Collection::MapType) {
    static_cast<c_Map*>(obj)->reserve(sz);
  } else if (ct == Collection::StableMapType) {
    static_cast<c_StableMap*>(obj)->reserve(sz);
  } else {
    assert(false);
  }
}

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer);
void collectionUnserialize(ObjectData* obj,
                           VariableUnserializer* uns,
                           int64_t sz,
                           char type);

inline bool collectionEquals(ObjectData* obj1, ObjectData* obj2) {
  int ct = obj1->getCollectionType();
  assert(ct == obj2->getCollectionType());
  if (ct == Collection::VectorType) {
    return c_Vector::Equals(obj1, obj2);
  } else if (ct == Collection::MapType) {
    return c_Map::Equals(obj1, obj2);
  } else if (ct == Collection::StableMapType) {
    return c_StableMap::Equals(obj1, obj2);
  }
  assert(false);
  return false;
}

void collectionDeepCopyTV(TypedValue* tv);
ArrayData* collectionDeepCopyArray(ArrayData* arr);
ObjectData* collectionDeepCopyVector(c_Vector* vec);
ObjectData* collectionDeepCopyMap(c_Map* vec);
ObjectData* collectionDeepCopyStableMap(c_StableMap* vec);

class CollectionInit {
public:
  CollectionInit(int cType, ssize_t nElems);
  ~CollectionInit() {
    // In case an exception interrupts the initialization.
    if (m_data) m_data->release();
  }
  CollectionInit &set(CVarRef v) {
    collectionOffsetAppend(m_data, v);
    return *this;
  }
  CollectionInit &set(RefResult v) {
    collectionOffsetAppend(m_data, variant(v));
    return *this;
  }
  CollectionInit &set(CVarWithRefBind v) {
    collectionOffsetAppend(m_data, variant(v));
    return *this;
  }
  CollectionInit &set(int64_t name, CVarRef v) {
    collectionOffsetSet(m_data, name, v);
    return *this;
  }
  CollectionInit &set(litstr name, CVarRef v) {
    collectionOffsetSet(m_data, name, v);
    return *this;
  }
  CollectionInit &set(CStrRef name, CVarRef v) {
    collectionOffsetSet(m_data, name, v);
    return *this;
  }
  CollectionInit &set(CVarRef name, CVarRef v) {
    collectionOffsetSet(m_data, name, v);
    return *this;
  }
  template<typename T>
  CollectionInit &set(const T &name, CVarRef v) {
    collectionOffsetSet(m_data, name, variant(v));
    return *this;
  }
  CollectionInit &set(litstr name, RefResult v) {
    collectionOffsetSet(m_data, name, variant(v));
    return *this;
  }
  CollectionInit &set(CStrRef name, RefResult v) {
    collectionOffsetSet(m_data, name, variant(v));
    return *this;
  }
  CollectionInit &set(CVarRef name, RefResult v) {
    collectionOffsetSet(m_data, name, variant(v));
    return *this;
  }
  template<typename T>
  CollectionInit &set(const T &name, RefResult v) {
    collectionOffsetSet(m_data, name, variant(v));
    return *this;
  }
  ObjectData *create() {
    ObjectData *ret = m_data;
    m_data = NULL;
    return ret;
  }
  operator ObjectData *() { return create(); }
private:
  ObjectData *m_data;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_COLLECTION_H__
