#include "hphp/runtime/ext/icu/ext_icu_uspoof.h"

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////
// class SpoofChecker

const StaticString s_SpoofChecker("SpoofChecker");

#define FETCH_SPOOF(dest, src) \
  auto dest = SpoofChecker::Get(src); \
  if (!dest) { \
    SystemLib::throwExceptionObject(           \
      "Call to invalid SpoofChecker Object"); \
  }

static bool HHVM_METHOD(SpoofChecker, isSuspicious, const String& text,
                                                    int64_t& issuesFound) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  int32_t ret = uspoof_checkUTF8(data->checker(),
                                 text.c_str(), text.size(),
                                 nullptr, &error);
  if (U_FAILURE(error)) {
    data->throwException("Could not check [%s] for security issues,"
                         "error %d (%s)",
                         text.c_str(), error, u_errorName(error));
  }
  issuesFound = ret;
  return ret != 0;
}

static bool HHVM_METHOD(SpoofChecker, areConfusable, const String& s1,
                                                     const String& s2,
                                                     int64_t& issuesFound) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  int32_t ret = uspoof_areConfusableUTF8(data->checker(),
                                         s1.c_str(), s1.size(),
                                         s2.c_str(), s2.size(),
                                         &error);
  if (U_FAILURE(error)) {
    data->throwException("Could not check [%s] and [%s] for confusability,"
                         " error %d (%s)",
                         s1.c_str(), s2.c_str(), error, u_errorName(error));
  }
  issuesFound = ret;
  return ret != 0;
}

static void HHVM_METHOD(SpoofChecker, setAllowedLocales, const String& locs) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  uspoof_setAllowedLocales(data->checker(), locs.c_str(), &error);
  if (U_FAILURE(error)) {
    data->throwException("Could not set allowed locales to [%s],"
                         " error %d (%s)",
                         locs.c_str(), error, u_errorName(error));
  }
}

static void HHVM_METHOD(SpoofChecker, setChecks, int64_t checks) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  uspoof_setChecks(data->checker(), checks, &error);
  if (U_FAILURE(error)) {
    data->throwException("Could not set spoof checks to %d, error %d (%s)",
                         checks, error, u_errorName(error));
  }
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::registerNativeUSpoof() {
  HHVM_RCC_INT(SpoofChecker, SINGLE_SCRIPT_CONFUSABLE,
               USPOOF_SINGLE_SCRIPT_CONFUSABLE);
  HHVM_RCC_INT(SpoofChecker, MIXED_SCRIPT_CONFUSABLE,
               USPOOF_MIXED_SCRIPT_CONFUSABLE);
  HHVM_RCC_INT(SpoofChecker, WHOLE_SCRIPT_CONFUSABLE,
               USPOOF_WHOLE_SCRIPT_CONFUSABLE);
  HHVM_RCC_INT(SpoofChecker, ANY_CASE, USPOOF_ANY_CASE);
  HHVM_RCC_INT(SpoofChecker, SINGLE_SCRIPT, USPOOF_SINGLE_SCRIPT);
  HHVM_RCC_INT(SpoofChecker, INVISIBLE, USPOOF_INVISIBLE);
  HHVM_RCC_INT(SpoofChecker, CHAR_LIMIT, USPOOF_CHAR_LIMIT);

  HHVM_ME(SpoofChecker, isSuspicious);
  HHVM_ME(SpoofChecker, areConfusable);
  HHVM_ME(SpoofChecker, setAllowedLocales);
  HHVM_ME(SpoofChecker, setChecks);

  Native::registerNativeDataInfo<SpoofChecker>(s_SpoofChecker.get());
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
