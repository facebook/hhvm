#ifndef incl_HPHP_EXT_COLLECTIONS_SET_H
#define incl_HPHP_EXT_COLLECTIONS_SET_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_SetIterator;

struct SetIterator {
  SetIterator() {}
  SetIterator(const SetIterator& src) = delete;
  SetIterator& operator=(const SetIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    m_version = src.m_version;
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
    m_version = mp->getVersion();
  }

  Variant current() const {
    auto st = m_obj.get();
    if (UNLIKELY(m_version != st->getVersion())) {
      throw_collection_modified();
    }
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
    if (UNLIKELY(m_version != st->getVersion())) {
      throw_collection_modified();
    }
    m_pos = st->iter_next(m_pos);
  }

  void rewind() {
    auto st = m_obj.get();
    if (UNLIKELY(m_version != st->getVersion())) {
      throw_collection_modified();
    }
    m_pos = st->iter_begin();
  }

 private:
  req::ptr<BaseSet> m_obj;
  uint32_t m_pos{0};
  int32_t  m_version{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
