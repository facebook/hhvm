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

#ifndef incl_HPHP_BASE_VECTOR_H_
#define incl_HPHP_BASE_VECTOR_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void throwOOB(int64_t key);

///////////////////////////////////////////////////////////////////////////////

// BaseVector encapsulates functionality that is common to both c_Vector and
// c_FrozenVector. It doesn't map to any PHP-land class.

class BaseVector : public ExtObjectData {

protected:

  // ConstCollection
  bool isempty();
  int64_t count();
  Object items();

  // ConstIndexAccess
  bool containskey(CVarRef key);
  Variant at(CVarRef key);
  Variant get(CVarRef key);

  // KeyedIterable
  Object getiterator();
  void map(BaseVector* bvec, CVarRef callback);
  void mapwithkey(BaseVector* bvec, CVarRef callback);
  void filter(BaseVector* bvec, CVarRef callback);
  void filterwithkey(BaseVector* bvec, CVarRef callback);
  void zip(BaseVector* bvec, CVarRef iterable);
  void kvzip(BaseVector* bvec);
  void keys(BaseVector* bvec);

  // Others
  void construct(CVarRef iterable = null_variant);
  Object lazy();
  Array toarray();
  Array tokeysarray();
  Array tovaluesarray();
  int64_t linearsearch(CVarRef search_value);

  template<typename T>
  static Object slice(const char* vecType, CVarRef vec, CVarRef offset,
                      CVarRef len = uninit_null()) {

    std::string notVecMsg = std::string("vec must be an instance of ") +
                            std::string(vecType);

    if (!vec.isObject()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(notVecMsg));
      throw e;
    }
    ObjectData* obj = vec.getObjectData();
    if (obj->getVMClass() != T::classof()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(notVecMsg));
      throw e;
    }
    if (!offset.isInteger()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Parameter offset must be an integer"));
      throw e;
    }
    if (!len.isNull() && !len.isInteger()) {
      Object e(SystemLib::AllocInvalidArgumentExceptionObject(
        "Parameter len must be null or an integer"));
      throw e;
    }
    T* target;
    Object ret = target = NEWOBJ(T)();
    auto v = static_cast<T*>(obj);
    int64_t sz = v->m_size;
    int64_t startPos = offset.toInt64();
    if (UNLIKELY(uint64_t(startPos) >= uint64_t(sz))) {
      if (startPos >= 0) {
        return ret;
      }
      startPos += sz;
      if (startPos < 0) {
        startPos = 0;
      }
    }
    int64_t endPos;
    if (len.isInteger()) {
      int64_t intLen = len.toInt64();
      if (LIKELY(intLen > 0)) {
        endPos = startPos + intLen;
        if (endPos > sz) {
          endPos = sz;
        }
      } else {
        if (intLen == 0) {
          return ret;
        }
        endPos = sz + intLen;
        if (endPos <= startPos) {
          return ret;
        }
      }
    } else {
      endPos = sz;
    }
    assert(startPos < endPos);
    uint targetSize = endPos - startPos;
    TypedValue* data;
    target->m_capacity = target->m_size = targetSize;
    target->m_data = data =
      (TypedValue*)smart_malloc(targetSize * sizeof(TypedValue));
    for (uint i = 0; i < targetSize; ++i, ++startPos) {
      cellDup(v->m_data[startPos], data[i]);
    }
    return ret;
  }

  template<typename T>
  static T* Clone(ObjectData* obj) {
    auto thiz = static_cast<T*>(obj);
    auto target = static_cast<T*>(obj->cloneImpl());
    uint sz = thiz->m_size;
    if (!sz) {
      return target;
    }
    TypedValue* data;
    target->m_capacity = target->m_size = sz;
    target->m_data = data = (TypedValue*)smart_malloc(sz * sizeof(TypedValue));
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

  static bool OffsetIsset(ObjectData* obj, TypedValue* key);
  static bool OffsetEmpty(ObjectData* obj, TypedValue* key);
  static bool OffsetContains(ObjectData* obj, TypedValue* key);
  static TypedValue* OffsetGet(ObjectData* obj, TypedValue* key);
  static bool Equals(const ObjectData* obj1, const ObjectData* obj2);

  Array toArrayImpl() const;
  void init(CVarRef t);

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

  static size_t frozenCopyOffset() {
    return offsetof(BaseVector, m_frozenCopy);
  }

protected:

  explicit BaseVector(Class* cls);
  /*virtual*/ ~BaseVector();

  void grow();

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

  /**
   * Should be called by any operation that mutates the vector, since
   * we might need to to trigger COW.
   */
  void mutate() {
    if (!m_frozenCopy.isNull()) cow();
  }

  /**
   * Copy-On-Write the buffer and reset the frozen copy.
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
  // A pointer to a FrozenVector which with it shares the buffer.
  Object m_frozenCopy;

private:

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(
      offsetof(BaseVector, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }

  // Friends

  friend class c_VectorIterator;

  template<typename TVector>
  friend ObjectData* collectionDeepCopyBaseVector(TVector* vec);

  friend void collectionReserve(ObjectData* obj, int64_t sz);
  friend void collectionInitAppend(ObjectData* obj, TypedValue* val);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_BASE_VECTOR_H_ */
