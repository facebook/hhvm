#ifndef incl_HPHP_ICU_RSRC_BUNDLE_H
#define incl_HPHP_ICU_RSRC_BUNDLE_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ures.h>
#include <unicode/resbund.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_ResourceBundle;

class ResourceBundle : public IntlError {
public:
  ResourceBundle() {}
  ResourceBundle(const ResourceBundle&) = delete;
  ResourceBundle& operator=(const ResourceBundle& src) {
    if (src.m_rsrc) {
      m_rsrc = src.m_rsrc->clone();
    }
    return *this;
  }
  ~ResourceBundle() { setResource(nullptr); }

  bool isValid() const {
    return m_rsrc;
  }

  static ResourceBundle* Get(ObjectData* obj) {
    return GetData<ResourceBundle>(obj, s_ResourceBundle);
  }

  static Object newInstance(icu::ResourceBundle* bundle) {
    if (!c_ResourceBundle) {
      c_ResourceBundle = Unit::lookupClass(s_ResourceBundle.get());
      assert(c_ResourceBundle);
    }
    Object obj{c_ResourceBundle};
    auto data = Native::data<ResourceBundle>(obj);
    data->setResource(bundle);
    return obj;
  }

  icu::ResourceBundle* resource() const { return m_rsrc; }
  void setResource(icu::ResourceBundle* rsrc)  {
    if (m_rsrc) {
      delete m_rsrc;
    }
    m_rsrc = rsrc;
    m_isTable = rsrc && (rsrc->getType() == URES_TABLE);
    m_iterIndex = 0;
    m_size = rsrc ? rsrc->getSize() : 0;
  }

  int32_t count() const { return m_size; }
  bool iterValid() const { return isValid() && (m_iterIndex < m_size); }
  void iterRewind() { m_iterIndex = 0; }
  bool iterNext() {
    if (!iterValid()) return false;
    if (++m_iterIndex >= m_size) return false;
    return true;
  }
  icu::ResourceBundle iterCurrent(UErrorCode& error) {
    assert(iterValid());
    return m_rsrc->get(m_iterIndex, error);
  }
  Variant iterKey() {
    if (!iterValid()) return init_null();
    if (m_isTable) {
      UErrorCode error = U_ZERO_ERROR;
      auto key = m_rsrc->get(m_iterIndex, error).getKey();
      return String(key, CopyString);
    }
    return m_iterIndex;
  }


private:
  icu::ResourceBundle* m_rsrc{nullptr};
  bool m_isTable{false};
  int32_t m_iterIndex{0}, m_size{0};

  static Class* c_ResourceBundle;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_RSRC_BUNDLE_H
