#ifndef incl_HPHP_ICU_UCSDET_H
#define incl_HPHP_ICU_UCSDET_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ucsdet.h>
#include <unicode/utypes.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_EncodingDetector;

class EncodingDetector : public IntlError {
 public:
  EncodingDetector() {
    UErrorCode error = U_ZERO_ERROR;
    m_encodingDetector = ucsdet_open(&error);
    if (U_FAILURE(error)) {
      throw getException("Could not open spoof checker, error %d (%s)",
                     error, u_errorName(error));
    }
  }
  EncodingDetector(const EncodingDetector&) = delete;
  EncodingDetector& operator=(const EncodingDetector& src) {
    throw getException("EncodingDetector may not be cloned.");
    not_reached();
  }
  ~EncodingDetector() {
    ucsdet_close(m_encodingDetector);
  }

  static EncodingDetector* Get(ObjectData* obj) {
    return GetData<EncodingDetector>(obj, s_EncodingDetector);
  }

  bool isValid() const {
    return m_encodingDetector;
  }

  UCharsetDetector* detector() const { return m_encodingDetector; }

  void setText(const std::string& text) {
    m_text = text;
    UErrorCode error = U_ZERO_ERROR;
    ucsdet_setText(m_encodingDetector, m_text.c_str(), m_text.size(), &error);
    if (U_FAILURE(error)) {
      throw getException("Could not set encoding detector text to "
                     "[%s], error %d (%s)",
                     m_text.c_str(), error, u_errorName(error));
    }
  }

  void setDeclaredEncoding(const std::string& encoding) {
    m_declaredEncoding = encoding;
    UErrorCode error = U_ZERO_ERROR;
    ucsdet_setDeclaredEncoding(m_encodingDetector,
                               m_declaredEncoding.c_str(),
                               m_declaredEncoding.size(), &error);
    if (U_FAILURE(error)) {
      throw getException("Could not set encoding detector declaredEncoding to "
                     "[%s], error %d (%s)",
                     m_text.c_str(), error, u_errorName(error));
    }
  }

 private:
  UCharsetDetector* m_encodingDetector;
  std::string m_text;
  std::string m_declaredEncoding;
};

/////////////////////////////////////////////////////////////////////////////

extern const StaticString s_EncodingMatch;

class EncodingMatch : public IntlError {
 public:
  EncodingMatch() {}
  EncodingMatch(const EncodingMatch&) = delete;
  EncodingMatch& operator=(const EncodingMatch& src) {
    *this = src;
    return *this;
  }
  ~EncodingMatch() {}

  static Object newInstance(const UCharsetMatch* match) {
    if (UNLIKELY(!c_EncodingMatch)) {
      c_EncodingMatch = Unit::lookupClass(s_EncodingMatch.get());
      assert(c_EncodingMatch);
    }
    Object ret{c_EncodingMatch};
    Native::data<EncodingMatch>(ret)->m_match =
      const_cast<UCharsetMatch*>(match);
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
  UCharsetMatch* m_match{nullptr};
  static Class* c_EncodingMatch;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_UCSDET_H
