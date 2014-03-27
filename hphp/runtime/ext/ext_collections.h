/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"
#include <limits>
#include "hphp/system/systemlib.h"

#define DECLARE_COLLECTION_MAGIC_METHODS()           \
  String t___tostring();                             \
  Variant t___get(Variant name);                     \
  Variant t___set(Variant name, Variant value);      \
  bool t___isset(Variant name);                      \
  Variant t___unset(Variant name)

#define DECLARE_ITERABLE_MATERIALIZE_METHODS()       \
  Object t_tovector();                               \
  Object t_toimmvector();                          \
  Object t_toset();                                  \
  Object t_toimmset()

#define DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS()  \
  DECLARE_ITERABLE_MATERIALIZE_METHODS();            \
  Object t_tomap();                                  \
  Object t_toimmmap()

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * All native collection class have their m_size field at the same
 * offset in the object.
 */
constexpr ptrdiff_t FAST_COLLECTION_SIZE_OFFSET = 20;
inline size_t getCollectionSize(const ObjectData* od) {
  assert(od->isCollection());
  return *reinterpret_cast<const uint32_t*>(
    reinterpret_cast<const char*>(od) + FAST_COLLECTION_SIZE_OFFSET
  );
}

/**
 * Called by the JIT on an emitVectorSet().
 */
void triggerCow(c_Vector* vec);
ArrayIter getArrayIterHelper(const Variant& v, size_t& sz);
TypedValue* cvarToCell(const Variant* v);

using ExtCollectionObjectData = ExtObjectDataFlags<
  ObjectData::IsCppBuiltin |
  ObjectData::IsCollection |
  ObjectData::UseGet |
  ObjectData::UseSet |
  ObjectData::UseIsset |
  ObjectData::UseUnset |
  ObjectData::CallToImpl | // not used for the always-truthy c_Pair
  ObjectData::HasClone>;

void throwOOB(int64_t key) ATTRIBUTE_NORETURN;

///////////////////////////////////////////////////////////////////////////////
// class BaseVector: encapsulates functionality that is common to both
// c_Vector and c_ImmVector. It doesn't map to any PHP-land class.

class BaseVector : public ExtCollectionObjectData {

 protected:

  // ConstCollection
  bool isempty();
  int64_t count();
  Object items();

  // ConstIndexAccess
  bool containskey(const Variant& key);
  Variant at(const Variant& key);
  Variant get(const Variant& key);

  // KeyedIterable
  Object getiterator();
  template<class TVector, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_map(const Variant& callback, MakeArgs);

  template<class TVector, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_filter(const Variant& callback, MakeArgs);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_take(const Variant& n);

  template<class TVector, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_skip(const Variant& n);

  template<class TVector, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, Object>::type
  php_skipWhile(const Variant& fn);

  void zip(BaseVector* bvec, const Variant& iterable);
  void kvzip(BaseVector* bvec);
  void keys(BaseVector* bvec);

  // Others
  void construct(const Variant& iterable = null_variant);
  Object lazy();
  Array toarray();
  Array tokeysarray();
  Array tovaluesarray();
  int64_t linearsearch(const Variant& search_value);

  template<class TVector>
  typename std::enable_if<
    std::is_base_of<BaseVector, TVector>::value, TVector*>::type
  static Clone(ObjectData* obj) {
    auto thiz = static_cast<TVector*>(obj);
    auto target = static_cast<TVector*>(obj->cloneImpl());
    uint sz = thiz->m_size;
    if (!sz) {
      return target;
    }
    TypedValue* data;
    target->m_capacity = target->m_size = sz;
    target->m_data = data =
      (TypedValue*)MM().objMallocLogged(sz * sizeof(TypedValue));
    for (int i = 0; i < sz; ++i) {
      cellDup(thiz->m_data[i], data[i]);
    }
    return target;
  }

 public:

  static Array ToArray(const ObjectData* obj) {
    check_collection_cast_to_array();
    return static_cast<const BaseVector*>(obj)->toArrayImpl();
  }

  static bool ToBool(const ObjectData* obj) {
    return static_cast<const BaseVector*>(obj)->toBoolImpl();
  }

  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  Array toArrayImpl() const;
  void init(const Variant& t);

  // Try to get the compiler to inline these.

  TypedValue* at(int64_t key) {
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return nullptr;
    }
    return &m_data[key];
  }

  TypedValue* get(int64_t key) {
    if ((uint64_t)key >= (uint64_t)m_size) {
      return nullptr;
    }
    return &m_data[key];
  }

  bool contains(int64_t key) const {
    return ((uint64_t)key < (uint64_t)m_size);
  }

  int getVersion() const {
    return m_version;
  }

  int64_t size() const {
    return m_size;
  }

  bool toBoolImpl() const {
    return (m_size != 0);
  }

  void reserve(int64_t sz);

  static size_t sizeOffset() { return offsetof(BaseVector, m_size); }
  static size_t dataOffset() { return offsetof(BaseVector, m_data); }

  static size_t immCopyOffset() {
    return offsetof(BaseVector, m_immCopy);
  }

  void addFront(TypedValue* val);

  Variant popFront();

 protected:

  explicit BaseVector(Class* cls);
  /*virtual*/ ~BaseVector();

  void grow();

  static constexpr uint64_t MaxCapacity() {
    return std::numeric_limits<decltype(m_capacity)>::max();
  }

  void add(TypedValue* val) {
    assert(val->m_type != KindOfRef);

    ++m_version;
    mutate();
    if (m_capacity <= m_size) {
      grow();
    }

    cellDup(*val, m_data[m_size]);
    ++m_size;
  }

 public:
  bool hasImmutableBuffer() const {
    return !m_immCopy.isNull();
  }

  /**
   * Should be called by any operation that mutates the vector, since
   * we might need to to trigger COW.
   */
  void mutate() {
    if (hasImmutableBuffer()) cow();
  }

 protected:
  /**
   * Copy-On-Write the buffer and reset the immutable copy.
   */
  void cow();

  static void throwBadKeyType() ATTRIBUTE_NORETURN;

  static void Unserialize(const char* vectorType, ObjectData* obj,
                          VariableUnserializer* uns, int64_t sz, char type);

  // Properties
  uint m_size;
  TypedValue* m_data;
  uint m_capacity;
  int32_t m_version;
  // A pointer to a ImmVector which with it shares the buffer.
  Object m_immCopy;

 private:

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(
      offsetof(BaseVector, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }

  // Friends

  friend class c_VectorIterator;
  friend class c_Pair;

  template<class TVector>
  friend ObjectData* collectionDeepCopyBaseVector(TVector* vec);

  friend void collectionReserve(ObjectData* obj, int64_t sz);
  friend void collectionInitAppend(ObjectData* obj, TypedValue* val);
};

///////////////////////////////////////////////////////////////////////////////
// class Vector

FORWARD_DECLARE_CLASS(Vector);
class c_Vector : public BaseVector {
 public:
  DECLARE_CLASS_NO_SWEEP(Vector)

 public:

  explicit c_Vector(Class* cls = c_Vector::classof());

  void t___construct(const Variant& iterable = null_variant);
  Object t_add(const Variant& val);
  Object t_addall(const Variant& val);
  Object t_append(const Variant& val); // deprecated
  Variant t_pop();
  void t_resize(const Variant& sz, const Variant& value);
  void t_reserve(const Variant& sz);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_values();
  Object t_lazy();
  Object t_kvzip();
  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);
  Object t_set(const Variant& key, const Variant& value);
  Object t_setall(const Variant& iterable);
  bool t_contains(const Variant& key); // deprecated
  bool t_containskey(const Variant& key);
  Object t_removekey(const Variant& key);
  Array t_toarray();
  DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  void t_reverse();
  void t_splice(const Variant& offset, const Variant& len = uninit_null(),
                const Variant& replacement = uninit_null());
  int64_t t_linearsearch(const Variant& search_value);
  void t_shuffle();
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& fn);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  DECLARE_COLLECTION_MAGIC_METHODS();
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromarray(const Variant& arr); // deprecated
  static Object ti_slice(const Variant& vec, const Variant& offset,
                         const Variant& len = uninit_null());
  static Object t_slice(const Variant& vec, const Variant& offset,
                        const Variant& len = uninit_null()) {
    return ti_slice(vec, offset, len);
  }
  Object t_immutable();

  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;

  void set(int64_t key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    if (UNLIKELY((uint64_t)key >= (uint64_t)m_size)) {
      throwOOB(key);
      return;
    }
    mutate();
    tvRefcountedIncRef(val);
    TypedValue* tv = &m_data[key];
    tvRefcountedDecRef(tv);
    tvCopy(*val, *tv);
  }

  void resize(int64_t sz, TypedValue* val);

  enum SortFlavor { IntegerSort, StringSort, GenericSort };

  void sort(int sort_flags, bool ascending);
  bool usort(const Variant& cmp_function);

  static c_Vector* Clone(ObjectData* obj) {
    return BaseVector::Clone<c_Vector>(obj);
  }

  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        TypedValue* val);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type) {
    BaseVector::Unserialize("Vector", obj, uns, sz, type);
  }

 private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc);

  Object getImmutableCopy();
  int64_t checkRequestedCapacity(const Variant& sz);

  // Friends
  friend void collectionAppend(ObjectData* obj, TypedValue* val);
  friend void triggerCow(c_Vector* vec);

  friend class BaseMap;
  friend class c_Pair;
  friend class ArrayIter;
};

///////////////////////////////////////////////////////////////////////////////
// class VectorIterator

FORWARD_DECLARE_CLASS(VectorIterator);
class c_VectorIterator : public ExtObjectData {
 public:
  DECLARE_CLASS_NO_SWEEP(VectorIterator)

 public:
  explicit c_VectorIterator(Class* cls = c_VectorIterator::classof());
  ~c_VectorIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<BaseVector> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class BaseVector;
};

///////////////////////////////////////////////////////////////////////////////
// class ImmVector

FORWARD_DECLARE_CLASS(ImmVector);
class c_ImmVector : public BaseVector {
public:
  DECLARE_CLASS_NO_SWEEP(ImmVector)

public:
  // The methods that implement the ConstVector interface simply forward
  // invocations to the implementations in BaseVector. Unfortunately, we need
  // to explicitly declare them so that the code automatically generated from
  // the IDL can link against them.

  // ConstCollection
  bool t_isempty();
  int64_t t_count();
  Object t_items();

  // ConstIndexAccess
  bool t_containskey(const Variant& key);
  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);

  // KeyedIterable
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& fn);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_kvzip();
  Object t_keys();

  // Others
  void t___construct(const Variant& iterable = null_variant);
  Object t_lazy();
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  int64_t t_linearsearch(const Variant& search_value);
  Object t_values();

  static Object ti_slice(const Variant& vec, const Variant& offset,
                         const Variant& len = uninit_null());

  Object t_immutable();

  static c_ImmVector* Clone(ObjectData* obj) {
    return BaseVector::Clone<c_ImmVector>(obj);
  }

  DECLARE_COLLECTION_MAGIC_METHODS();

  DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS();

public:

  explicit c_ImmVector(Class* cls = c_ImmVector::classof());

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type) {
    BaseVector::Unserialize("ImmVector", obj, uns, sz, type);
  }

  friend class c_Vector;
  friend class c_Pair;
};

///////////////////////////////////////////////////////////////////////////////
// class BaseMap

/**
 * BaseMap is a hash-table implementation with int and string keys only.
 * It doesn't represent any PHP-land class; that job is delegated to its
 * c_-prefixed child classes.
 */
class BaseMap : public ExtCollectionObjectData {

 public:
  struct Elm {
    /* The key is either a string pointer or an int value, and the m_aux
     * field in data is used to discriminate the key type. m_aux = 0 means
     * int, nonzero values contain 32 bits of a string's hashcode. It is
     * critical that when we return &data to clients, that they not read
     * or write the m_aux field! */
    union {
      int64_t ikey;
      StringData* skey;
    };
    /* We store values here, but also some information local to this array:
     * data.m_aux.u_hash contains either 0 (for an int key) or a string
     * hashcode; the high bit is the int/string key descriminator.
     * data.m_type == KindOfInvalid if this is an empty slot in the
     * Map (e.g. after an element is removed). */
    TypedValueAux data;
    bool hasStrKey() const {
      return data.hash() != 0;
    }
    bool hasIntKey() const {
      return data.hash() == 0;
    }
    int32_t hash() const {
      return data.hash();
    }
    void setStaticKey(StringData* k, strhash_t h) {
      assert(k->isStatic());
      skey = k;
      data.hash() = h | STRHASH_MSB;
    }
    void setStrKey(StringData* k, strhash_t h) {
      skey = k;
      data.hash() = h | STRHASH_MSB;
      k->incRefCount();
    }
    void setIntKey(int64_t k) {
      ikey = k;
      data.hash() = 0;
    }
    static constexpr size_t dataOff() {
      return offsetof(Elm, data);
    }
  };

 protected:
  uint32_t m_size;          // Number of values
  union {
    struct {
      uint32_t m_used;      // Number of currently used Elm slots (values or
                            // tombstones)
      uint32_t m_cap;       // Maximum number of Elm slots we can use without
                            // having to grow
    };
    uint64_t m_capAndUsed;
  };
  union {
    struct {
      uint32_t m_tableMask; // Bitmask used when indexing into the hash table
      int32_t m_version;    // Version number; used to keep track if the
                            // collection has been modified during iteration
    };
    uint64_t m_maskAndVersion;
  };
  Elm* m_data;              // Elm store.
  int32_t* m_hash;          // Hash table.

 public:

  static const int32_t Empty      = -1;
  static const int32_t Tombstone  = -2;

  // Load factor scaler. If C is the power-of-2 hashtable capacity and L is
  // LoadScale, then we can accommodate up to C-C/L elements before needing
  // to grow. So LoadScale=2 gives a maximum load factor of 0.5, LoadScale=4
  // gives a maximum load factor of 0.75, and LoadScale=8 gives a maximum load
  // factor of 0.875 load factor. We use powers of 2 so that we can use bit-
  // shifting and bit-masking instructions instead of multiply and divide
  // instructions.
  static const uint LoadScale = 4;

  // If a Map has a hash table allocated, the smallest hash table size we
  // support is 4 (2^2). Note that the hash table size is always the hash
  // mask plus 1.
  static const uint32_t SmallLgTableSize = 2;
  static const uint32_t SmallMask = (size_t(1) << SmallLgTableSize) - 1;
  static const uint32_t SmallSize = SmallMask - SmallMask / LoadScale;

  // The largest hash table size we support is 4294967296 (2^32).
  static const uint32_t MaxLgTableSize = 32;
  static const uint32_t MaxMask = (size_t(1) << MaxLgTableSize) - 1;
  static const uint32_t MaxSize = MaxMask - MaxMask / LoadScale;

  // Map can only guarantee that it won't throw "cannot add element"
  // exceptions if m_size <= MaxSize / 2. Therefore, we only allow
  // reserve() to make room for up to MaxSize / 2 elements.
  static const uint32_t MaxReserveSize = MaxSize / 2;

 private:
  void deleteElms();
  void freeData();

 protected:
  Elm* data() const { return (Elm*)m_data; }
  int32_t* hashTab() const { return (int32_t*)m_hash; }
  uint32_t iterLimit() const { return m_used; }

  // We use this funny-looking helper to make g++ use lea and shl
  // instructions instead of imul when indexing into m_data
  Elm* fetchElm(Elm* data, intptr_t pos) const {
    assert(sizeof(Elm) == 24);
    assert(sizeof(int64_t) == 8);
    intptr_t index = 3 * pos;
    int64_t* ptr = (int64_t*)data;
    return (Elm*)(&ptr[index]);
  }

  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_NORETURN;
  static void throwTooLarge() ATTRIBUTE_NORETURN;
  static void throwReserveTooLarge() ATTRIBUTE_NORETURN;
  static int32_t* warnUnbalanced(size_t n, int32_t* ei);

 public:

  TypedValue* at(int64_t key) const;
  TypedValue* at(StringData* key) const;
  TypedValue* get(int64_t key) const;
  TypedValue* get(StringData* key) const;
  void set(int64_t key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    update(key, val);
  }
  void set(StringData* key, TypedValue* val) {
    assert(val->m_type != KindOfRef);
    update(key, val);
  }
  void add(TypedValue* val);
  Variant pop();
  Variant popFront();
  void remove(int64_t key);
  void remove(StringData* key);
  bool contains(int64_t key) const;
  bool contains(StringData* key) const;
  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }
  bool toBoolImpl() const {
    return (m_size != 0);
  }

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, TMap*>::type
  static Clone(ObjectData* obj);

  // template<class TMap>
  // typename std::enable_if<
  //   std::is_base_of<BaseMap, TMap>::value, TMap*>::type
  // static DeepCopy(TMap* mp);

  static Array ToArray(const ObjectData* obj);
  static bool ToBool(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key);
  static void OffsetSet(ObjectData* obj, const TypedValue* key,
                        TypedValue* val);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static void OffsetUnset(ObjectData* obj, const TypedValue* key);

  enum EqualityFlavor { OrderMatters, OrderIrrelevant };

  static bool Equals(EqualityFlavor eq,
                     const ObjectData* obj1, const ObjectData* obj2);

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  static uint sizeOffset() {
    return offsetof(BaseMap, m_size);
  }

  static bool validPos(ssize_t pos) {
    return pos >= 0;
    static_assert(ssize_t(Empty) == ssize_t(-1), "");
  }

  static bool validPos(int32_t pos) {
    return pos >= 0;
    static_assert(Empty == -1, "");
  }

  static bool isTombstone(DataType t) {
    assert(IS_REAL_TYPE(t) || t == KindOfInvalid);
    return t < KindOfUninit;
    static_assert(KindOfUninit == 0 && KindOfInvalid < 0, "");
  }

  bool isTombstone(ssize_t pos) const {
    assert(size_t(pos) <= m_used);
    return isTombstone(data()[pos].data.m_type);
  }

  bool hasTombstones() const { return m_size != m_used; }

  size_t hashSize() const {
    return size_t(m_tableMask) + 1;
  }

  static uint32_t computeMaxElms(uint32_t tableMask) {
    return tableMask - tableMask / LoadScale;
  }

  static void initHash(int32_t* table, size_t tableSize) {
    wordfill(table, Empty, tableSize);
  }

  void init(const Variant& t);

  template <class Hit>
  ssize_t findImpl(size_t h0, Hit) const;
  ssize_t find(int64_t h) const;
  ssize_t find(const StringData* s, strhash_t prehash) const;

  template <class Hit>
  int32_t* findForInsertImpl(size_t h0, Hit) const;
  int32_t* findForInsert(int64_t h) const;
  int32_t* findForInsert(const StringData* s, strhash_t prehash) const;

  int32_t* findForNewInsert(size_t h0) const;
  int32_t* findForNewInsert(int32_t* table, size_t mask, size_t h0) const;

  void update(int64_t h, TypedValue* data);
  void update(StringData* key, TypedValue* data);

  void erase(int32_t* pos);
  void eraseNoCompact(int32_t* pos);

  bool isFull() { return m_used == m_cap; }
  bool isDensityTooLow() const { return (m_size < m_used / 2); }

  // This method will grow or compact as needed to make room to add
  // one new element.
  void makeRoom();

  // This method will grow or compact as needed in preparation for
  // repeatedly adding new elements until m_size >= sz.
  void reserve(int64_t sz);

  void grow(uint32_t newCap, uint32_t newMask);
  void compactIfNecessary();

  BaseMap::Elm& allocElm(int32_t* ei) {
    assert(ei && !validPos(*ei) && m_size <= m_used && m_used < m_cap);
    size_t i = m_used;
    (*ei) = i;
    m_used = i + 1;
    ++m_size;
    return data()[i];
  }

 public:
  ssize_t iter_begin() const {
    Elm* p = data();
    auto* pLimit = fetchElm(data(), iterLimit());
    for (; p != pLimit; ++p) {
      if (LIKELY(!isTombstone(p->data.m_type))) return ssize_t(p);
    }
    return 0;
  }

  ssize_t iter_next(ssize_t pos) const {
    if (!iter_valid(pos)) return pos;
    auto* p = (Elm*)pos;
    auto* pLimit = fetchElm(data(), iterLimit());
    for (++p; p != pLimit; ++p) {
      if (LIKELY(!isTombstone(p->data.m_type))) return ssize_t(p);
    }
    return 0;
  }

  ssize_t iter_prev(ssize_t pos) const {
    if (!iter_valid(pos)) return pos;
    auto* p = (Elm*)pos;
    auto* pLimit = fetchElm(data(), -1);
    for (--p; p != pLimit; --p) {
      if (LIKELY(!isTombstone(p->data.m_type))) return ssize_t(p);
    }
    return 0;
  }

  Variant iter_key(ssize_t pos) const {
    assert(iter_valid(pos));
    auto* p = (Elm*)pos;
    if (p->hasStrKey()) {
      return p->skey;
    }
    return (int64_t)p->ikey;
  }

  TypedValue* iter_value(ssize_t pos) const {
    assert(iter_valid(pos));
    auto* p = (Elm*)pos;
    return &p->data;
  }

  bool iter_valid(ssize_t pos) const {
    return pos != 0;
  }

  enum SortFlavor { IntegerSort, StringSort, GenericSort };

 protected: // BaseMap is an abstract class
  explicit BaseMap(Class* cls);
  ~BaseMap();

 private:
  template <typename AccessorT>
  SortFlavor preSort(const AccessorT& acc, bool checkTypes);
  void postSort();

 public:
  void asort(int sort_flags, bool ascending);
  void ksort(int sort_flags, bool ascending);
  bool uasort(const Variant& cmp_function);
  bool uksort(const Variant& cmp_function);

  static void throwBadKeyType() ATTRIBUTE_NORETURN;

 private:

  template<class TMap>
  typename std::enable_if<
   std::is_base_of<BaseMap, TMap>::value, ObjectData*>::type
  friend collectionDeepCopyBaseMap(TMap* vec);

  friend class c_MapIterator;
  friend class c_Vector;
  friend class c_ImmMap;
  friend class ArrayIter;
  friend class c_GenMapWaitHandle;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(BaseMap, m_size)
                  == FAST_COLLECTION_SIZE_OFFSET, "");
  }

 protected: // implementations of the API accessible from user PHP code

  void php_construct(const Variant& iterable = null_variant);
  Object php_add(const Variant& val);
  Object php_addAll(const Variant& val);
  Object php_clear();
  bool php_isEmpty() const { return !toBoolImpl(); }
  Object php_items() {
    return SystemLib::AllocLazyKVZipIterableObject(this);
  }
  Object php_keys() const;
  Object php_lazy() {
    return SystemLib::AllocLazyKeyedIterableViewObject(this);
  }
  Object php_kvzip() const;
  Variant php_at(const Variant& key) const;
  Variant php_get(const Variant& key) const;
  Object php_set(const Variant& key, const Variant& value);
  Object php_setAll(const Variant& iterable);
  Object php_put(const Variant& key, const Variant& value); // deprecated
  bool php_contains(const Variant& key) const;
  Object php_remove(const Variant& key);
  Array php_toArray() const;
  Array php_toKeysArray() const;
  Array php_toValuesArray() const;
  Object php_values() const;

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_differenceByKey(const Variant& it);

  Object php_getIterator();

  template<class TMap, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_map(const Variant& callback, MakeArgs) const;

  template<class TMap, class MakeArgs>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_filter(const Variant& callback, MakeArgs) const;

  template<class MakeArgs>
  Object php_retain(const Variant& callback, MakeArgs);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_zip(const Variant& iterable) const;

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_take(const Variant& n);

  template<class TMap, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_skip(const Variant& n);

  template<class TMap, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  php_skipWhile(const Variant& fn);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  static php_mapFromIterable(const Variant& iterable);

  template<class TMap>
  typename std::enable_if<
    std::is_base_of<BaseMap, TMap>::value, Object>::type
  static php_mapFromArray(const Variant& arr);
};

///////////////////////////////////////////////////////////////////////////////
// class Map

FORWARD_DECLARE_CLASS(Map);
class c_Map : public BaseMap {

 public:
  DECLARE_CLASS_NO_SWEEP(Map)

 public:
  explicit c_Map(Class* cls = c_Map::classof());

  static c_Map* Clone(ObjectData* obj);

 public: // PHP API - No inlines (required by .idl.json linking)
  void t___construct(const Variant& iterable = null_variant);
  Object t_add(const Variant& val);
  Object t_addall(const Variant& val);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_lazy();
  Object t_kvzip(); // const
  Variant t_at(const Variant& key); // const
  Variant t_get(const Variant& key); // const
  Object t_set(const Variant& key, const Variant& value);
  Object t_setall(const Variant& iterable);
  bool t_contains(const Variant& key); // const
  bool t_containskey(const Variant& key); // const
  Object t_remove(const Variant& key);
  Object t_removekey(const Variant& key);
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS();
  Object t_values();
  Object t_differencebykey(const Variant& it);
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_retain(const Variant& callback);
  Object t_retainwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  DECLARE_COLLECTION_MAGIC_METHODS();
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromarray(const Variant& arr); // deprecated
  Object t_immutable();
};

///////////////////////////////////////////////////////////////////////////////
// class ImmMap

FORWARD_DECLARE_CLASS(ImmMap);
class c_ImmMap : public BaseMap {

 public:
  DECLARE_CLASS_NO_SWEEP(ImmMap)

  public:
  explicit c_ImmMap(Class* cls = c_ImmMap::classof());

  static c_ImmMap* Clone(ObjectData* obj);

 public: // PHP API - No inlines (required by .idl.json linking)
  void t___construct(const Variant& iterable = null_variant);
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_lazy();
  Object t_kvzip();
  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);
  bool t_contains(const Variant& key);
  bool t_containskey(const Variant& key);
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS();
  Object t_values();
  Object t_differencebykey(const Variant& it);
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  DECLARE_COLLECTION_MAGIC_METHODS();
  static Object ti_fromitems(const Variant& iterable);
  Object t_immutable();
};

///////////////////////////////////////////////////////////////////////////////
// class MapIterator

FORWARD_DECLARE_CLASS(MapIterator);
class c_MapIterator : public ExtObjectData {
 public:
  DECLARE_CLASS_NO_SWEEP(MapIterator)

 public:
  explicit c_MapIterator(Class* cls = c_MapIterator::classof());
  ~c_MapIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<BaseMap> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class BaseMap;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * BaseSet is a hash-table implementation of the Set ADT. It doesn't represent
 * any PHP-land class. That job is delegated to its child classes.
 *
 * BaseSet uses a power of two for the table size and quadratic probing to
 * resolve hash collisions, similar to the Map class. See the comments
 * in the Map class for more details on how the hash table works and how
 * we decide when to grow or shrink the table.
 */
class BaseSet : public ExtCollectionObjectData {

 public:
  struct Elm {
    // data.m_type == KindOfInvalid if this is an empty slot in the
    // Set (e.g. after a value is deleted).
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

    inline int32_t hash() const { return data.hash(); }

    static constexpr size_t dataOff() {
      return offsetof(Elm, data);
    }
  };

 protected:
  uint32_t m_size;          // Number of values
  union {
    struct {
      uint32_t m_used;      // Number of currently used Elm slots (values or
                            // tombstones)
      uint32_t m_cap;       // Maximum number of Elm slots we can use without
                            // having to grow
    };
    uint64_t m_capAndUsed;
  };
  union {
    struct {
      uint32_t m_tableMask; // Bitmask used when indexing into the hash table
      int32_t m_version;    // Version number; used to keep track if the
                            // collection has been modified during iteration
    };
    uint64_t m_maskAndVersion;
  };
  Elm* m_data;              // Elm store.
  int32_t* m_hash;          // Hash table.

 public:

  static const int32_t Empty      = -1;
  static const int32_t Tombstone  = -2;

  static const uint LoadScale = 4;

  static const uint32_t SmallLgTableSize = 2;
  static const uint32_t SmallMask = (size_t(1) << SmallLgTableSize) - 1;
  static const uint32_t SmallSize = SmallMask - SmallMask / LoadScale;

  static const uint32_t MaxLgTableSize = 32;
  static const uint32_t MaxMask = (size_t(1) << MaxLgTableSize) - 1;
  static const uint32_t MaxSize = MaxMask - MaxMask / LoadScale;

  static const uint32_t MaxReserveSize = MaxSize / 2;

 private:
  void deleteElms();
  void freeData();

 protected:
  Elm* data() const { return (Elm*)m_data; }
  int32_t* hashTab() const { return (int32_t*)m_hash; }
  uint32_t iterLimit() const { return m_used; }

  static void throwTooLarge() ATTRIBUTE_NORETURN;
  static void throwReserveTooLarge() ATTRIBUTE_NORETURN;
  static int32_t* warnUnbalanced(size_t n, int32_t* ei);

 public:

  int getVersion() const {
    return m_version;
  }
  int64_t size() const {
    return m_size;
  }

  static uint sizeOffset() {
    return offsetof(BaseSet, m_size);
  }

  static bool validPos(ssize_t pos) {
    return pos >= 0;
    static_assert(ssize_t(Empty) == ssize_t(-1), "");
  }

  static bool validPos(int32_t pos) {
    return pos >= 0;
    static_assert(Empty == -1, "");
  }

  static bool isTombstone(DataType t) {
    assert(IS_REAL_TYPE(t) || t == KindOfInvalid);
    return t < KindOfUninit;
    static_assert(KindOfUninit == 0 && KindOfInvalid < 0, "");
  }

  bool isTombstone(ssize_t pos) const {
    assert(size_t(pos) <= m_used);
    return isTombstone(data()[pos].data.m_type);
  }

  bool hasTombstones() const { return m_size != m_used; }

  size_t hashSize() const {
    return size_t(m_tableMask) + 1;
  }

  static uint32_t computeMaxElms(uint32_t tableMask) {
    return tableMask - tableMask / LoadScale;
  }

  static void initHash(int32_t* table, size_t tableSize) {
    wordfill(table, Empty, tableSize);
  }

  template <class Hit>
  ssize_t findImpl(size_t h0, Hit) const;
  ssize_t find(int64_t h) const;
  ssize_t find(const StringData* s, strhash_t prehash) const;

  template <class Hit>
  int32_t* findForInsertImpl(size_t h0, Hit) const;
  int32_t* findForInsert(int64_t h) const;
  int32_t* findForInsert(const StringData* s, strhash_t prehash) const;

  int32_t* findForNewInsert(size_t h0) const;
  int32_t* findForNewInsert(int32_t* table, size_t mask, size_t h0) const;

  void erase(int32_t* pos);

  bool isFull() { return m_used == m_cap; }
  bool isDensityTooLow() const { return (m_size < m_used / 2); }

  // This method will grow or compact as needed to make room to add
  // one new element.
  void makeRoom();

  // This method will grow or compact as needed in preparation for
  // repeatedly adding new elements until m_size >= sz.
  void reserve(int64_t sz);

  void grow(uint32_t newCap, uint32_t newMask);
  void compact();

  BaseSet::Elm& allocElm(int32_t* ei) {
    assert(ei && !validPos(*ei) && m_size <= m_used && m_used < m_cap);
    size_t i = m_used;
    (*ei) = i;
    m_used = i + 1;
    ++m_size;
    return data()[i];
  }

  BaseSet::Elm& allocElmFront(int32_t* ei);

 public:
  ssize_t iter_begin() const {
    Elm* p = data();
    auto* pLimit = data() + iterLimit();
    for (; p != pLimit; ++p) {
      if (LIKELY(!isTombstone(p->data.m_type))) return ssize_t(p);
    }
    return 0;
  }

  ssize_t iter_next(ssize_t pos) const {
    if (!iter_valid(pos)) return pos;
    auto* p = (Elm*)pos;
    auto* pLimit = data() + iterLimit();
    for (++p; p != pLimit; ++p) {
      if (LIKELY(!isTombstone(p->data.m_type))) return ssize_t(p);
    }
    return 0;
  }

  ssize_t iter_prev(ssize_t pos) const {
    if (!iter_valid(pos)) return pos;
    auto* p = (Elm*)pos;
    auto* pFirst = data();
    for (--p; p >= pFirst; --p) {
      if (LIKELY(!isTombstone(p->data.m_type))) return ssize_t(p);
    }
    return 0;
  }

  Variant iter_key(ssize_t pos) const {
    return tvAsCVarRef(iter_value(pos));
  }

  TypedValue* iter_value(ssize_t pos) const {
    assert(iter_valid(pos));
    auto* p = (Elm*)pos;
    return &p->data;
  }

  bool iter_valid(ssize_t pos) const {
    return pos != 0;
  }

 public:
  void init(const Variant& t);

  void add(TypedValue* val) {
    if (val->m_type == KindOfInt64) {
      add(val->m_data.num);
    } else if (IS_STRING_TYPE(val->m_type)) {
      add(val->m_data.pstr);
    } else {
      throwBadValueType();
    }
  }

  void add(int64_t h);
  void add(StringData* key);

  void addFront(TypedValue* val) {
    if (val->m_type == KindOfInt64) {
      addFront(val->m_data.num);
    } else if (IS_STRING_TYPE(val->m_type)) {
      addFront(val->m_data.pstr);
    } else {
      throwBadValueType();
    }
  }
  void addFront(int64_t h);
  void addFront(StringData* key);

  Variant pop();
  Variant popFront();

  void remove(int64_t key) {
    ++m_version;
    auto* p = findForInsert(key);
    if (validPos(*p)) {
      erase(p);
    }
  }

  void remove(StringData* key) {
    ++m_version;
    auto* p = findForInsert(key, key->hash());
    if (validPos(*p)) {
      erase(p);
    }
  }

  bool contains(int64_t key) const;
  bool contains(StringData* key) const;

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, TSet*>::type
  static Clone(ObjectData* obj);

  // Static methods
  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;
  static void throwOOB(StringData* key) ATTRIBUTE_NORETURN;
  static void throwNoIndexAccess() ATTRIBUTE_NORETURN;

  static Array ToArray(const ObjectData* obj);
  static bool ToBool(const ObjectData* obj);

  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  static void Unserialize(const char* setType, ObjectData* obj,
                          VariableUnserializer* uns, int64_t sz, char type);

protected:
  // PHP-land methods exported by child classes.

  void    php_construct(const Variant& iterable = null_variant);

  Object  php_add(const Variant& val) {
    TypedValue* tv = cvarToCell(&val);
    add(tv);
    return this;
  }

  Object  php_addAll(const Variant& val);

  Object  php_clear();

  bool    php_isEmpty() { return !toBoolImpl(); }
  int64_t php_count() { return m_size; }
  Object  php_items() { return SystemLib::AllocLazyIterableViewObject(this); }

  template<class TVector>
  Object  php_values() {
    TVector* vec;
    Object o = vec = NEWOBJ(TVector)();
    vec->init(VarNR(this));
    return o;
  }

  Object  php_lazy() { return SystemLib::AllocLazyIterableViewObject(this); }
  bool    php_contains(const Variant& key);
  Object  php_remove(const Variant& key);
  Array   php_toArray() { return toArrayImpl(); }
  Array   php_toKeysArray() { return php_toValuesArray(); }
  Array   php_toValuesArray();
  Object  php_getIterator();

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_map(const Variant& callback);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_filter(const Variant& callback);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_zip(const Variant& iterable);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_take(const Variant& n);

  template<class TSet, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_takeWhile(const Variant& fn);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_skip(const Variant& n);

  template<class TSet, bool checkVersion>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  php_skipWhile(const Variant& fn);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromItems(const Variant& iterable);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromArray(const Variant& arr);

  template<class TSet>
  typename std::enable_if<
    std::is_base_of<BaseSet, TSet>::value, Object>::type
  static php_fromArrays(int _argc, const Array& _argv = null_array);

protected:
  // BaseSet is an abstract class.

  explicit BaseSet(Class* cls);
  /* virtual */ ~BaseSet();

private:
  // Helpers

  /**
   * Raises a warning if the set contains an int and a string with the same
   * numeric value: e.g. Set {'123', 123}. It's a no-op otherwise.
   */
  void warnOnStrIntDup() const;

  Array toArrayImpl() const;
  bool toBoolImpl() const { return (m_size != 0); }

  static void throwBadValueType() ATTRIBUTE_NORETURN;

 private:

  friend class c_SetIterator;
  friend class c_Vector;
  friend class c_Set;
  friend class c_Map;
  friend class ArrayIter;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(BaseSet, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }
};

///////////////////////////////////////////////////////////////////////////////
// class Set

FORWARD_DECLARE_CLASS(Set);
class c_Set : public BaseSet {

 public:
  DECLARE_CLASS_NO_SWEEP(Set)

 public:
  // PHP-land methods.

  explicit c_Set(Class* cls = c_Set::classof());
  void t___construct(const Variant& iterable = null_variant);
  Object t_add(const Variant& val);
  Object t_addall(const Variant& val);
  Object t_clear();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_values();
  Object t_lazy();
  bool t_contains(const Variant& key);
  Object t_remove(const Variant& key);
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  DECLARE_ITERABLE_MATERIALIZE_METHODS();
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  Object t_removeall(const Variant& iterable);
  Object t_difference(const Variant& iterable);
  DECLARE_COLLECTION_MAGIC_METHODS();
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromarray(const Variant& arr); // deprecated
  static Object ti_fromarrays(int _argc, const Array& _argv = null_array);
  Object t_immutable();

 public:

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  static c_Set* Clone(ObjectData* obj);
};

///////////////////////////////////////////////////////////////////////////////
// class ImmSet

FORWARD_DECLARE_CLASS(ImmSet);
class c_ImmSet : public BaseSet {

 public:
  DECLARE_CLASS_NO_SWEEP(ImmSet)

 public:
  // PHP-land methods.

  void t___construct(const Variant& iterable = null_variant);

  // API
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_values();
  Object t_lazy();
  bool t_contains(const Variant& key);
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);

  // Materialization methods.
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();

  DECLARE_ITERABLE_MATERIALIZE_METHODS();

  DECLARE_COLLECTION_MAGIC_METHODS();

  // Static methods.
  static Object ti_fromitems(const Variant& iterable);
  static Object ti_fromarrays(int _argc, const Array& _argv = null_array);

  Object t_immutable();

 public:
  explicit c_ImmSet(Class* cls = c_ImmSet::classof());

  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  static c_ImmSet* Clone(ObjectData* obj);
};

///////////////////////////////////////////////////////////////////////////////
// class SetIterator

FORWARD_DECLARE_CLASS(SetIterator);
class c_SetIterator : public ExtObjectData {
 public:
  DECLARE_CLASS_NO_SWEEP(SetIterator)

 public:
  explicit c_SetIterator(Class* cls = c_SetIterator::classof());
  ~c_SetIterator();
  void t___construct();
  Variant t_current();
  Variant t_key();
  bool t_valid();
  void t_next();
  void t_rewind();

 private:
  SmartPtr<BaseSet> m_obj;
  ssize_t m_pos;
  int32_t m_version;

  friend class BaseSet;
};

///////////////////////////////////////////////////////////////////////////////
// class Pair

FORWARD_DECLARE_CLASS(Pair);
class c_Pair : public ExtObjectDataFlags<ObjectData::IsCollection|
                                         ObjectData::UseGet|
                                         ObjectData::UseSet|
                                         ObjectData::UseIsset|
                                         ObjectData::UseUnset|
                                         ObjectData::HasClone> {
 public:
  DECLARE_CLASS_NO_SWEEP(Pair)

 public:
  explicit c_Pair(Class* cls = c_Pair::classof());
  ~c_Pair();
  void t___construct();
  bool t_isempty();
  int64_t t_count();
  Object t_items();
  Object t_keys();
  Object t_values();
  Object t_lazy();
  Object t_kvzip();
  Variant t_at(const Variant& key);
  Variant t_get(const Variant& key);
  bool t_containskey(const Variant& key);
  Array t_toarray();
  Array t_tokeysarray();
  Array t_tovaluesarray();
  DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS();
  Object t_getiterator();
  Object t_map(const Variant& callback);
  Object t_mapwithkey(const Variant& callback);
  Object t_filter(const Variant& callback);
  Object t_filterwithkey(const Variant& callback);
  Object t_zip(const Variant& iterable);
  Object t_take(const Variant& n);
  Object t_takewhile(const Variant& callback);
  Object t_skip(const Variant& n);
  Object t_skipwhile(const Variant& fn);
  DECLARE_COLLECTION_MAGIC_METHODS();
  Object t_immutable();

  static void throwOOB(int64_t key) ATTRIBUTE_NORETURN;

  /**
   * Most methods that operate on Pairs can safely assume that all Pairs have
   * two elements that have been initialized. However, methods that deal with
   * initializing and destructing Pairs needs to handle intermediate states
   * where one or both of the elements is uninitialized.
   */
  bool isFullyConstructed() const {
    return m_size == 2;
  }

  TypedValue* at(int64_t key) {
    assert(isFullyConstructed());
    if (UNLIKELY(uint64_t(key) >= uint64_t(2))) {
      throwOOB(key);
      return NULL;
    }
    return &getElms()[key];
  }
  TypedValue* get(int64_t key) {
    assert(isFullyConstructed());
    if (uint64_t(key) >= uint64_t(2)) {
      return NULL;
    }
    return &getElms()[key];
  }
  void initAdd(TypedValue* val) {
    assert(!isFullyConstructed());
    assert(val->m_type != KindOfRef);
    cellDup(*val, getElms()[m_size]);
    ++m_size;
  }
  bool contains(int64_t key) const {
    assert(isFullyConstructed());
    return (uint64_t(key) < uint64_t(2));
  }

  Array toArrayImpl() const;

  static c_Pair* Clone(ObjectData* obj);
  static Array ToArray(const ObjectData* obj);
  template <bool throwOnMiss>
  static TypedValue* OffsetAt(ObjectData* obj, const TypedValue* key);
  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);
  static void Unserialize(ObjectData* obj, VariableUnserializer* uns,
                          int64_t sz, char type);

  int64_t size() const {
    assert(isFullyConstructed());
    return 2;
  }

  static uint dataOffset() { return offsetof(c_Pair, elm0); }

 private:
  static void throwBadKeyType() ATTRIBUTE_NORETURN;

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
  friend class BaseVector;
  friend class BaseMap;
  friend class ArrayIter;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(offsetof(c_Pair, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }
};

///////////////////////////////////////////////////////////////////////////////
// class PairIterator

FORWARD_DECLARE_CLASS(PairIterator);
class c_PairIterator : public ExtObjectData {
 public:
  DECLARE_CLASS_NO_SWEEP(PairIterator)

 public:
  explicit c_PairIterator(Class* cls = c_PairIterator::classof());
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

///////////////////////////////////////////////////////////////////////////////

TypedValue* collectionAt(ObjectData* obj, const TypedValue* key);
TypedValue* collectionAtLval(ObjectData* obj, const TypedValue* key);
TypedValue* collectionAtRw(ObjectData* obj, const TypedValue* key);
TypedValue* collectionGet(ObjectData* obj, TypedValue* key);
void collectionSet(ObjectData* obj, const TypedValue* key, TypedValue* val);
// used for collection literal syntax only
void collectionInitSet(ObjectData* obj, TypedValue* key, TypedValue* val);
bool collectionIsset(ObjectData* obj, TypedValue* key);
bool collectionEmpty(ObjectData* obj, TypedValue* key);
void collectionUnset(ObjectData* obj, const TypedValue* key);
void collectionAppend(ObjectData* obj, TypedValue* val);
// used for collection literal syntax only
void collectionInitAppend(ObjectData* obj, TypedValue* val);
bool collectionContains(ObjectData* obj, const Variant& offset);
void collectionReserve(ObjectData* obj, int64_t sz);
void collectionUnserialize(ObjectData* obj, VariableUnserializer* uns,
                           int64_t sz, char type);
bool collectionEquals(const ObjectData* obj1, const ObjectData* obj2);
void collectionDeepCopyTV(TypedValue* tv);
ArrayData* collectionDeepCopyArray(ArrayData* arr);
ObjectData* collectionDeepCopyVector(c_Vector* vec);
ObjectData* collectionDeepCopyImmVector(c_ImmVector* vec);
ObjectData* collectionDeepCopyMap(c_Map* mp);
ObjectData* collectionDeepCopyImmMap(c_ImmMap* mp);
ObjectData* collectionDeepCopySet(c_Set* mp);
ObjectData* collectionDeepCopyImmSet(c_ImmSet* st);
ObjectData* collectionDeepCopyPair(c_Pair* pair);

ObjectData* newCollectionHelper(uint32_t type, uint32_t size);

///////////////////////////////////////////////////////////////////////////////

inline TypedValue* cvarToCell(const Variant* v) {
  return const_cast<TypedValue*>(v->asCell());
}

inline bool isOptimizableCollectionClass(const Class* klass) {
  return klass == c_Vector::classof() || klass == c_Map::classof() ||
    klass == c_Pair::classof();
}

void collectionSerialize(ObjectData* obj, VariableSerializer* serializer);

///////////////////////////////////////////////////////////////////////////////

}

#undef DECLARE_COLLECTION_MAGIC_METHODS
#undef DECLARE_ITERABLE_MATERIALIZE_METHODS
#undef DECLARE_KEYEDITERABLE_MATERIALIZE_METHODS

#endif // incl_HPHP_EXT_COLLECTION_H_
