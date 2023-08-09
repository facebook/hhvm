#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ucsdet.h>
#include <unicode/utypes.h>

#include <memory>

namespace HPHP::Intl {
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

  const String& text() const { return m_text; }
  const String& declaredEncoding() const { return m_declaredEncoding; }

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
                            const std::shared_ptr<UCharsetDetector>& det,
                            const String& text,
                            const String& declaredEncoding) {
    Object ret{SystemLib::classLoad(s_EncodingMatch.get(), c_EncodingMatch)};
    auto const data = Native::data<EncodingMatch>(ret);
    data->m_match = const_cast<UCharsetMatch*>(match);
    data->m_encodingDetector = det;
    data->m_text = text;
    data->m_declaredEncoding = declaredEncoding;
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
  // m_match is owned by m_encodingDetector
  std::shared_ptr<UCharsetDetector> m_encodingDetector;
  UCharsetMatch* m_match{nullptr};
  // m_text and m_declaredEncoding are used by m_encodingDetector
  String m_text;
  String m_declaredEncoding;
  static Class* c_EncodingMatch;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl

