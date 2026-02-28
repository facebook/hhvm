




#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ucnv.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

struct IntlUConverter : IntlError, SystemLib::ClassLoader<"UConverter"> {
  IntlUConverter() {}
  IntlUConverter(const IntlUConverter& src) = delete;
  IntlUConverter& operator=(const IntlUConverter& src) {
    IntlError::operator =(src);
    char *buffer[U_CNV_SAFECLONE_BUFFERSIZE];
    int32_t bufferSize = U_CNV_SAFECLONE_BUFFERSIZE;
    UErrorCode error = U_ZERO_ERROR;
    if (m_src) {
      m_src = ucnv_safeClone(m_src, buffer, &bufferSize, &error);
    }
    if (m_dest) {
      m_dest = ucnv_safeClone(m_dest, buffer, &bufferSize, &error);
    }
    return *this;
  }
  ~IntlUConverter() { }

  bool isValid() const { return true; }

  static IntlUConverter* Get(ObjectData* obj) {
    return GetData<IntlUConverter>(obj, className());
  }

  UConverter* src() const { return m_src; }
  UConverter* dest() const { return m_dest; }
  bool setSrc(UConverter* src) {
    if (m_src) {
      ucnv_close(m_src);
    }
    return (m_src = src);
  }
  bool setDest(UConverter* dest) {
    if (m_dest) {
      ucnv_close(m_dest);
    }
    return (m_dest = dest);
  }

  void failure(UErrorCode error, const char* funcname) {
    setError(error, "%s() returned error %ld: %s",
             funcname, (long)error, u_errorName(error));
  }

  static void Failure(UErrorCode error, const char* funcname) {
    s_intl_error->setError(error, "%s() returned error %ld: %s",
                           funcname, (long)error, u_errorName(error));
  }

private:
  UConverter *m_src{nullptr}, *m_dest{nullptr};
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl

