#ifndef incl_HPHP_EXT_COLLECTIONS_MAP_H
#define incl_HPHP_EXT_COLLECTIONS_MAP_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP { namespace collections {
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
