#ifndef incl_HPHP_EXT_COLLECTIONS_VECTOR_H
#define incl_HPHP_EXT_COLLECTIONS_VECTOR_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_VectorIterator;

struct VectorIterator {
  VectorIterator() {}
  VectorIterator(const VectorIterator& src) = delete;
  VectorIterator& operator=(const VectorIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    m_version = src.m_version;
    return *this;
  }
  ~VectorIterator() {}

  static Object newInstance() {
    static Class* cls = Unit::lookupClass(s_VectorIterator.get());
    assertx(cls);
    return Object{cls};
  }

  void setVector(BaseVector* vec) {
    m_obj = vec;
    m_pos = 0;
    m_version = vec->getVersion();
  }

  Variant current() const {
    auto vec = m_obj.get();
    if (UNLIKELY(m_version != vec->getVersion())) {
      throw_collection_modified();
    }
    if (m_pos >= vec->m_size) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(&vec->m_data[m_pos]);
  }

  int64_t key() const {
    auto vec = m_obj.get();
    if (m_pos >= vec->m_size) {
      throw_iterator_not_valid();
    }
    return m_pos;
  }

  bool valid() const {
    auto vec = m_obj.get();
    return vec && (m_pos < vec->m_size);
  }

  void next()   { ++m_pos;   }
  void rewind() { m_pos = 0; }

 private:
  req::ptr<BaseVector> m_obj;
  uint32_t m_pos{0};
  int32_t  m_version{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
