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

  // Helpers

  Array toArrayImpl() const;
  void reserve(int64_t sz);
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

  void add(TypedValue* val) {
    assert(val->m_type != KindOfRef);

    ++m_version;
    if (m_capacity <= m_size) {
      grow();
    }

    cellDup(*val, m_data[m_size]);
    ++m_size;
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

protected:

  // We don't want anybody instantiating the class, hence the protected
  // constructor/destructor.
  explicit BaseVector(Class* cls);
  virtual ~BaseVector();
  void grow();

  static void throwBadKeyType() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

  static void Unserialize(const char* vectorType, ObjectData* obj,
                          VariableUnserializer* uns, int64_t sz, char type);

  // Properties
  uint m_size;
  TypedValue* m_data;
  uint m_capacity;
  int32_t m_version;

private:

  void freeData();

  friend class c_VectorIterator;

  static void compileTimeAssertions() {
    // For performance, all native collection classes have their m_size field
    // at the same offset.
    static_assert(
      offsetof(BaseVector, m_size) == FAST_COLLECTION_SIZE_OFFSET, "");
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_BASE_VECTOR_H_ */
