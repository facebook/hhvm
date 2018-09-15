#ifndef incl_HPHP_ICU_UCSDET_H
#define incl_HPHP_ICU_UCSDET_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ucsdet.h>
#include <unicode/utypes.h>

#include <memory>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_EncodingDetector;

struct EncodingDetector : IntlError {
  EncodingDetector() = default;
  EncodingDetector(const EncodingDetector&) = delete;
  EncodingDetector& operator=(const EncodingDetector& /*src*/) = delete;
  ~EncodingDetector() = default;

  bool isValid() const { return true; }

  static EncodingDetector* Get(ObjectData* obj) {
    return GetData<EncodingDetector>(obj, s_EncodingDetector);
  }

  std::shared_ptr<UCharsetDetector> detector();

  void setText(const String& text) {
    m_text = text;
  }

  void setDeclaredEncoding(const String& encoding) {
    m_declaredEncoding = encoding;
  }

 private:
  String m_text;
  String m_declaredEncoding;
};

/////////////////////////////////////////////////////////////////////////////

extern const StaticString s_EncodingMatch;

struct EncodingMatch : IntlError {
  EncodingMatch() {}
  EncodingMatch(const EncodingMatch&) = delete;
  EncodingMatch& operator=(const EncodingMatch& src) {
    IntlError::operator =(src);
    m_match = src.m_match;
    return *this;
  }
  ~EncodingMatch() = default;

  static Object newInstance(const UCharsetMatch* match,
                            const std::shared_ptr<UCharsetDetector>& det) {
    if (UNLIKELY(!c_EncodingMatch)) {
      c_EncodingMatch = Unit::lookupClass(s_EncodingMatch.get());
      assertx(c_EncodingMatch);
    }
    Object ret{c_EncodingMatch};
    auto const data = Native::data<EncodingMatch>(ret);
    data->m_match = const_cast<UCharsetMatch*>(match);
    data->m_encodingDetector = det;
    return ret;
  }

  static EncodingMatch* Get(ObjectData* obj) {
    return GetData<EncodingMatch>(obj, s_EncodingMatch);
  }

  bool isValid() const {
    return m_match;
  }

  UCharsetMatch* match() const { return m_match; }

 private:
  std::shared_ptr<UCharsetDetector> m_encodingDetector;
  UCharsetMatch* m_match{nullptr};
  static Class* c_EncodingMatch;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_UCSDET_H
