#include "hphp/runtime/ext/icu/ext_icu_uspoof.h"

namespace HPHP { namespace Intl {
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
                                                    VRefParam issuesFound) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  int32_t ret = uspoof_checkUTF8(data->checker(),
                                 text.c_str(), text.size(),
                                 nullptr, &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not check [%s] for security issues,"
                         "error %d (%s)",
                         text.c_str(), error, u_errorName(error));
  }
  issuesFound = ret;
  return ret != 0;
}

static bool HHVM_METHOD(SpoofChecker, areConfusable, const String& s1,
                                                     const String& s2,
                                                     VRefParam issuesFound) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  int32_t ret = uspoof_areConfusableUTF8(data->checker(),
                                         s1.c_str(), s1.size(),
                                         s2.c_str(), s2.size(),
                                         &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not check [%s] and [%s] for confusability,"
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
    throw data->getException("Could not set allowed locales to [%s],"
                         " error %d (%s)",
                         locs.c_str(), error, u_errorName(error));
  }
}

static void HHVM_METHOD(SpoofChecker, setChecks, int64_t checks) {
  FETCH_SPOOF(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  uspoof_setChecks(data->checker(), checks, &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not set spoof checks to %d, error %d (%s)",
                         checks, error, u_errorName(error));
  }
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initUSpoof() {

#define SPOOFC(v) Native::registerClassConstant<KindOfInt64> \
                    (s_SpoofChecker.get(), makeStaticString(#v), USPOOF_##v)

  SPOOFC(SINGLE_SCRIPT_CONFUSABLE);
  SPOOFC(MIXED_SCRIPT_CONFUSABLE);
  SPOOFC(WHOLE_SCRIPT_CONFUSABLE);
  SPOOFC(ANY_CASE);
  SPOOFC(SINGLE_SCRIPT);
  SPOOFC(INVISIBLE);
  SPOOFC(CHAR_LIMIT);

#undef SPOOFC

  HHVM_ME(SpoofChecker, isSuspicious);
  HHVM_ME(SpoofChecker, areConfusable);
  HHVM_ME(SpoofChecker, setAllowedLocales);
  HHVM_ME(SpoofChecker, setChecks);

  Native::registerNativeDataInfo<SpoofChecker>(s_SpoofChecker.get());

  loadSystemlib("icu_uspoof");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
