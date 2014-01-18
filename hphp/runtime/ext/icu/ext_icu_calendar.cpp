#include "hphp/runtime/ext/icu/ext_icu_calendar.h"

#include <unicode/gregocal.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

const icu::Calendar*
IntlCalendar::ParseArg(CVarRef cal, const icu::Locale &locale,
                       const String &funcname, intl_error &err,
                       int64_t &calType, bool &calOwned) {
  icu::Calendar *ret = nullptr;
  UErrorCode error = U_ZERO_ERROR;
  if (cal.isNull()) {
    ret = new icu::GregorianCalendar(locale, error);
    calType = UCAL_GREGORIAN;
    calOwned = true;
  } else if (cal.isInteger()) {
    calType = cal.toInt64();
    if (calType != UCAL_GREGORIAN && calType != UCAL_TRADITIONAL) {
      err.code = U_ILLEGAL_ARGUMENT_ERROR;
      err.custom_error_message = funcname +
        String(": invalid value for calendar type; it must be "
               "one of IntlDateFormatter::TRADITIONAL (locale's default "
               "calendar) or IntlDateFormatter::GREGORIAN. "
               "Alternatively, it can be an IntlCalendar object",
               CopyString);
      return nullptr;
    }
    ret = (calType == UCAL_TRADITIONAL)
        ? (icu::Calendar::createInstance(locale, error))
        : (new icu::GregorianCalendar(locale, error));
    calOwned = true;
  } else if (cal.isObject()) {
    throw NotImplementedException("IntlCalendar");
  } else {
    err.code = U_ILLEGAL_ARGUMENT_ERROR;
    err.custom_error_message = funcname +
      String(": Invalid calendar argument; should be an integer "
             "or an IntlCalendar instance", CopyString);
    return nullptr;
  }
  if (ret) {
    if (U_SUCCESS(error)) {
      return ret;
    }
    delete ret;
  }

  if (!U_FAILURE(error)) {
    error = U_MEMORY_ALLOCATION_ERROR;
  }

  err.code = error;
  err.custom_error_message = funcname +
    String(": Failure instantiating calendar", CopyString);
  return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
