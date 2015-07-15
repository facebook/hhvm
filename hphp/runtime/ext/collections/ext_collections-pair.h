#ifndef incl_HPHP_EXT_COLLECTIONS_PAIR_H
#define incl_HPHP_EXT_COLLECTIONS_PAIR_H

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_PairIterator;

struct PairIterator {
  PairIterator() {}
  PairIterator(const PairIterator& src) = delete;
  PairIterator& operator=(const PairIterator& src) {
    m_obj = src.m_obj;
    m_pos = src.m_pos;
    return *this;
  }
  ~PairIterator() {}

  static Object newInstance() {
    static Class* cls = Unit::lookupClass(s_PairIterator.get());
    assertx(cls);
    return Object{cls};
  }

  void setPair(c_Pair* pr) {
    m_obj = pr;
    m_pos = 0;
  }

  Variant current() const {
    auto pair = m_obj.get();
    if (!pair->contains(m_pos)) {
      throw_iterator_not_valid();
    }
    return tvAsCVarRef(&pair->getElms()[m_pos]);
  }

  int64_t key() const {
    auto pair = m_obj.get();
    if (!pair->contains(m_pos)) {
      throw_iterator_not_valid();
    }
    return m_pos;
  }

  bool valid() const {
    static_assert(std::is_unsigned<decltype(m_pos)>::value,
                  "m_pos should be unsigned");
    return m_obj && (m_pos < 2);
  }

  void next()   { ++m_pos;   }
  void rewind() { m_pos = 0; }

 private:
  req::ptr<c_Pair> m_obj;
  uint32_t m_pos{0};
};

/////////////////////////////////////////////////////////////////////////////
}}
#endif
