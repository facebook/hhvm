#ifndef incl_HPHP_ICU_USPOOF_H
#define incl_HPHP_ICU_USPOOF_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/uspoof.h>
#include <unicode/utypes.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_SpoofChecker;

class SpoofChecker : public IntlError {
 public:
  SpoofChecker() {
    UErrorCode error = U_ZERO_ERROR;
    m_checker = uspoof_open(&error);

    int32_t checks = uspoof_getChecks(m_checker, &error);
    uspoof_setChecks(m_checker,
                     checks & ~USPOOF_SINGLE_SCRIPT,
                     &error);

    if (U_FAILURE(error)) {
      throw getException("Could not open spoof checker, error %d (%s)",
                     error, u_errorName(error));
    }
  }
  SpoofChecker(const SpoofChecker&) = delete;
  SpoofChecker& operator=(const SpoofChecker& src) {
    *this = src;
    UErrorCode error = U_ZERO_ERROR;
    m_checker = uspoof_clone(src.m_checker, &error);
    if (U_FAILURE(error)) {
      throw getException("Could not clone spoof checker, error %d (%s)",
                     error, u_errorName(error));
    }
    return *this;
  }
  ~SpoofChecker() {
    if (m_checker) {
      uspoof_close(m_checker);
    }
  }

  static SpoofChecker* Get(Object obj) {
    return GetData<SpoofChecker>(obj, s_SpoofChecker);
  }

  bool isValid() const {
    return m_checker;
  }

  USpoofChecker* checker() const { return m_checker; }

 private:
  USpoofChecker* m_checker{nullptr};
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_USPOOF_H
