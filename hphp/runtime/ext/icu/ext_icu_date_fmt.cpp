/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/ext/icu/ext_icu_date_fmt.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/ext/icu/ext_icu_timezone.h"
#include "hphp/runtime/ext/icu/ext_icu_calendar.h"
#include <math.h>

namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////
// Internal Resource Data

const StaticString
  s_IntlDateFormatter("IntlDateFormatter"),
  s_tm_year("tm_year"),
  s_tm_mon("tm_mon"),
  s_tm_mday("tm_mday"),
  s_tm_hour("tm_hour"),
  s_tm_min("tm_min"),
  s_tm_sec("tm_sec"),
  s_tm_wday("tm_wday"),
  s_tm_yday("tm_yday"),
  s_tm_isdst("tm_isdst");

Class* IntlDateFormatter::c_IntlDateFormatter = nullptr;

void IntlDateFormatter::setDateFormatter(const String& locale,
                                         int64_t datetype, int64_t timetype,
                                         const Variant& timezone, const Variant& calendar,
                                         const String& pattern) {
  auto loc = icu::Locale::createFromName(locale.c_str());
  int64_t calType = UCAL_GREGORIAN;
  bool calOwned = false;
  const icu::Calendar *cal = IntlCalendar::ParseArg(calendar, loc,
                                                    "datefm_create",
                                                    s_intl_error.get(),
                                                    calType, calOwned);
  if (!cal) { return; }
  SCOPE_EXIT { if (cal && calOwned) delete cal; };

  const icu::TimeZone *tz = IntlTimeZone::ParseArg(timezone,
                                                   "datefmt_create",
                                                   s_intl_error.get());
  if (!tz) { return; }
  SCOPE_EXIT { if (tz && !isValid()) delete tz; };

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString pat(u16(pattern, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "datefmt_create: "
                                  "error converting pattern to UTF-16");
    return;
  }

  error = U_ZERO_ERROR;
  if (m_date_fmt) {
    udat_close(m_date_fmt);
  }
  m_date_fmt = udat_open(pat.isEmpty()?(UDateFormatStyle)timetype:UDAT_IGNORE,
                         pat.isEmpty()?(UDateFormatStyle)datetype:UDAT_IGNORE,
                         locale.c_str(), nullptr, 0,
                         pat.getBuffer(), pat.length(),
                         &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "datefmt_create: date "
                                  "formatter creation failed");
    return;
  }

  icu::DateFormat *obj = datefmtObject();
  if (calOwned) {
    obj->adoptCalendar(const_cast<icu::Calendar*>(cal));
    calOwned = false;
  } else {
    obj->setCalendar(*cal);
  }
  if (tz) {
    obj->setTimeZone(*tz);
  }

  m_date_type = datetype;
  m_time_type = timetype;
  m_calendar = calType;
}

void IntlDateFormatter::setDateFormatter(const IntlDateFormatter *orig) {
  if (!orig || !orig->datefmt()) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "Cannot clone unconstructed IntlDateFormatter");
    throw getException("%s", s_intl_error->getErrorMessage(false).c_str());
  }
  if (m_date_fmt) {
    udat_close(m_date_fmt);
  }
  UErrorCode error = U_ZERO_ERROR;
  m_date_fmt = udat_clone(orig->datefmt(), &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "datefmt_clone: date formatter clone failed");
    throw getException("%s", s_intl_error->getErrorMessage().c_str());
  }
}

int64_t IntlDateFormatter::getArrayElemInt(const Array& arr,
                                           const StaticString &key) {
  if (!arr.exists(key)) {
    return 0;
  }

  auto val = arr[key];
  if (!val.isInteger()) {
    setError(U_ILLEGAL_ARGUMENT_ERROR,
             "datefmt_format: parameter array contains "
             "a non-integer element for key '%s'", key.c_str());
    return 0;
  }
  return val.toInt64();
}

double IntlDateFormatter::getTimestamp(const Variant& arg) {
  if (!arg.isArray()) {
    return VariantToMilliseconds(arg);
  }

  auto arr = arg.toArray();
  clearError();
  auto year = getArrayElemInt(arr, s_tm_year) + 1900,
       month = getArrayElemInt(arr, s_tm_mon),
       mday = getArrayElemInt(arr, s_tm_mday),
       hour = getArrayElemInt(arr, s_tm_hour),
       minute = getArrayElemInt(arr, s_tm_min),
       second = getArrayElemInt(arr, s_tm_sec);
  if (U_FAILURE(getErrorCode())) {
    // error already set
    return NAN;
  }

  UErrorCode error = U_ZERO_ERROR;
  auto pcal = ucal_clone(udat_getCalendar(datefmt()), &error);
  if (U_FAILURE(error)) {
    setError(error, "datefmt_format: error cloning calendar");
    return NAN;
  }
  error = U_ZERO_ERROR;
  ucal_setDateTime(pcal, year, month, mday, hour, minute, second, &error);
  double result = ucal_getMillis(pcal, &error);
  if (U_FAILURE(error)) {
    setError(error, "datefmt_format: error fetching milliseconds");
  }
  ucal_close(pcal);
  return result;
}

#define DATFMT_GET(dest, src, def) \
  auto dest = IntlDateFormatter::Get(src); \
  if (!dest) { \
    return def; \
  }

#define DATFMT_CHECK(ov, ec, fail) \
  if (U_FAILURE(ec)) { \
    ov->setError(ec); \
    return fail; \
  }

//////////////////////////////////////////////////////////////////////////////
// class IntlDateFormatter

static void HHVM_METHOD(IntlDateFormatter, __construct,
                        const String& locale,
                        int64_t datetype, int64_t timetype,
                        const Variant& timezone /*= null */,
                        const Variant& calendar /*= null */,
                        const String& pattern /*= empty_string */) {
  auto data = Native::data<IntlDateFormatter>(this_.get());
  data->setDateFormatter(locale, datetype, timetype,
                         timezone, calendar, pattern);
}

static String HHVM_METHOD(IntlDateFormatter, format, const Variant& value) {
  DATFMT_GET(data, this_, null_string);
  double ts = data->getTimestamp(value);
  if (ts == NAN) {
    return null_string;
  }
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = udat_format(data->datefmt(), ts, nullptr, 0, nullptr, &error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    data->setError(error);
    return null_string;
  }
  error = U_ZERO_ERROR;
  icu::UnicodeString ret;
  auto *buffer = ret.getBuffer(len + 1);
  udat_format(data->datefmt(), ts, buffer, len + 1, nullptr, &error);
  if (U_FAILURE(error)) {
    data->setError(error);
    return null_string;
  }
  ret.releaseBuffer(len);
  String out(u8(ret, error));
  if (U_FAILURE(error)) {
    data->setError(error);
    return null_string;
  }
  return out;
}

static String HHVM_STATIC_METHOD(IntlDateFormatter, formatObject,
                                 const Object& object, const Variant& format,
                                 const String& locale) {
  // TODO: Need IntlCalendar implemented first
  throw NotImplementedException("IntlDateFormatter::formatObject");
}

static int64_t HHVM_METHOD(IntlDateFormatter, getCalendar) {
  DATFMT_GET(data, this_, 0);
  return data->calendar();
}

static int64_t HHVM_METHOD(IntlDateFormatter, getDateType) {
  DATFMT_GET(data, this_, 0);
  return data->dateType();
}

static int64_t HHVM_METHOD(IntlDateFormatter, getErrorCode) {
  DATFMT_GET(data, this_, 0);
  return data->getErrorCode();
}

static String HHVM_METHOD(IntlDateFormatter, getErrorMessage) {
  DATFMT_GET(data, this_, null_string);
  return data->getErrorMessage();
}

static String HHVM_METHOD(IntlDateFormatter, getLocale, const Variant& which) {
  ULocDataLocaleType whichloc = ULOC_ACTUAL_LOCALE;
  if (!which.isNull()) whichloc = (ULocDataLocaleType)which.toInt64();

  DATFMT_GET(data, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  const char *loc = udat_getLocaleByType(data->datefmt(), whichloc, &error);
  if (U_FAILURE(error)) {
    data->setError(error);
    return null_string;
  }
  return String(loc, CopyString);
}

static String HHVM_METHOD(IntlDateFormatter, getPattern) {
  DATFMT_GET(data, this_, null_string);
  UErrorCode error = U_ZERO_ERROR;
  int32_t len = udat_toPattern(data->datefmt(), false, nullptr, 0, &error);
  icu::UnicodeString tmp;
  auto buf = tmp.getBuffer(len + 1);
  error = U_ZERO_ERROR;
  udat_toPattern(data->datefmt(), false, buf, len + 1, &error);
  if (U_FAILURE(error)) {
    data->setError(error, "Error getting formatter pattern");
    return null_string;
  }
  tmp.releaseBuffer(len);
  String ret(u8(tmp, error));
  if (U_FAILURE(error)) {
    data->setError(error);
    return null_string;
  }
  return ret;
}

static int64_t HHVM_METHOD(IntlDateFormatter, getTimeType) {
  DATFMT_GET(data, this_, 0);
  return data->timeType();
}

static String HHVM_METHOD(IntlDateFormatter, getTimeZoneId) {
  DATFMT_GET(data, this_, 0);
  UnicodeString id;
  data->datefmtObject()->getTimeZone().getID(id);
  UErrorCode error = U_ZERO_ERROR;
  String ret(u8(id, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Could not convert time zone id to UTF-8");
    return null_string;
  }
  return ret;
}

static Object HHVM_METHOD(IntlDateFormatter, getCalendarObject) {
  // TODO: Need IntlCalendar implemented first
  throw NotImplementedException("IntlDateFormatter::getCalendarObject");
}

static Object HHVM_METHOD(IntlDateFormatter, getTimeZone) {
  DATFMT_GET(data, this_, null_object);
  const icu::TimeZone& tz = data->datefmtObject()->getTimeZone();
  auto ntz = tz.clone();
  if (!ntz) {
    data->setError(U_MEMORY_ALLOCATION_ERROR,
                   "datefmt_get_timezone: Out of memory "
                   "when cloning time zone");
    return null_object;
  }
  return IntlTimeZone::newInstance(ntz, true);
}

static bool HHVM_METHOD(IntlDateFormatter, isLenient) {
  DATFMT_GET(data, this_, false);
  return udat_isLenient(data->datefmt());
}

static void add_to_localtime_arr(Array &ret, const UCalendar *cal,
                                 UCalendarDateFields calfield,
                                 const String& name, UErrorCode &error,
                                 int64_t delta = 0) {
  if (U_FAILURE(error)) { return; }
  long calendar_field_val = ucal_get(cal, calfield, &error);
  if (U_FAILURE(error)) { return; }
  ret.set(name, calendar_field_val + delta);
}

static Variant HHVM_METHOD(IntlDateFormatter, localtime,
                           const String& value, VRefParam position) {
  DATFMT_GET(data, this_, uninit_null());
  int32_t parse_pos = -1;
  if (!position.isNull()) {
    parse_pos = position.toInt64();
    if (parse_pos > value.size()) {
      return false;
    }
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uValue(u16(value, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Error converting timezone to UTF-16");
    return false;
  }

  error = U_ZERO_ERROR;
  UCalendar *cal = const_cast<UCalendar*>(udat_getCalendar(data->datefmt()));
  udat_parseCalendar(data->datefmt(), cal,
                     uValue.getBuffer(), uValue.length(),
                     &parse_pos, &error);

  Array ret = Array::Create();
  error = U_ZERO_ERROR;
  add_to_localtime_arr(ret, cal, UCAL_SECOND, s_tm_sec, error);
  add_to_localtime_arr(ret, cal, UCAL_MINUTE, s_tm_min, error);
  add_to_localtime_arr(ret, cal, UCAL_HOUR_OF_DAY, s_tm_hour, error);
  add_to_localtime_arr(ret, cal, UCAL_YEAR, s_tm_year, error, -1900);
  add_to_localtime_arr(ret, cal, UCAL_DAY_OF_MONTH, s_tm_mday, error);
  add_to_localtime_arr(ret, cal, UCAL_DAY_OF_WEEK, s_tm_wday, error, -1);
  add_to_localtime_arr(ret, cal, UCAL_DAY_OF_YEAR, s_tm_yday, error);
  add_to_localtime_arr(ret, cal, UCAL_MONTH, s_tm_mon, error);
  if (U_FAILURE(error)) {
    data->setError(error, "Date parsing - localtime failed : "
                          "could not get a field from calendar");
    return false;
  }

  error = U_ZERO_ERROR;
  auto isDST = ucal_inDaylightTime(cal, &error);
  if (U_FAILURE(error)) {
    data->setError(error, "Date parsing - localtime failed : "
                          "while checking if currently in DST.");
    return false;
  }
  ret.set(s_tm_isdst, isDST ? 1 : 0);

  position = (int64_t)parse_pos;
  return ret;
}

static Variant HHVM_METHOD(IntlDateFormatter, parse,
                           const String& value, VRefParam position) {
  DATFMT_GET(data, this_, 0);
  data->clearError();
  int32_t pos = position.toInt64();
  if (pos > value.size()) {
    return false;
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString str(u16(value, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Error converting timezone to UTF-16");
    return false;
  }
  error = U_ZERO_ERROR;
  UDate timestamp = udat_parse(data->datefmt(),
                               str.getBuffer(), str.length(),
                               &pos, &error);
  position = (int64_t)pos;
  if (U_FAILURE(error)) {
    data->setError(error, "Date parsing failed");
    return false;
  }

  double result = (double)timestamp / U_MILLIS_PER_SECOND;
  if ((result > LONG_MAX) || (result < -LONG_MAX)) {
    return (double)((result > 0) ? ceil(result) : floor(result));
  } else {
    return (int64_t)result;
  }
}

static bool HHVM_METHOD(IntlDateFormatter, setCalendar, const Variant& which) {
  // TODO: Need IntlCalendar implemented first
  throw NotImplementedException("IntlDateFormatter::setCalendar");
}

static bool HHVM_METHOD(IntlDateFormatter, setLenient, bool lenient) {
  DATFMT_GET(data, this_, false);
  udat_setLenient(data->datefmt(), (UBool)lenient);
  return true;
}

static bool HHVM_METHOD(IntlDateFormatter, setPattern,
                        const String& pattern) {
  DATFMT_GET(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString pat(u16(pattern, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Error converting pattern to UTF-16");
    return false;
  }
  udat_applyPattern(data->datefmt(), (UBool)false,
                    pat.getBuffer(), pat.length());
  return true;
}

static bool HHVM_METHOD(IntlDateFormatter, setTimeZone, const Variant& zone) {
  DATFMT_GET(data, this_, false);
  icu::TimeZone *tz = IntlTimeZone::ParseArg(zone, "datefmt_set_timezone",
                                             data);
  if (tz == nullptr) {
    return false;
  }
  data->datefmtObject()->adoptTimeZone(tz);
  return true;
}

//////////////////////////////////////////////////////////////////////////////

#define UDAT_CONST(nm) Native::registerClassConstant<KindOfInt64>( \
                       s_IntlDateFormatter.get(), \
                       makeStaticString(#nm), UDAT_##nm);
#define UCAL_CONST(nm) Native::registerClassConstant<KindOfInt64>( \
                       s_IntlDateFormatter.get(), \
                       makeStaticString(#nm), UCAL_##nm);

void IntlExtension::initDateFormatter() {
  UDAT_CONST(FULL);
  UDAT_CONST(LONG);
  UDAT_CONST(MEDIUM);
  UDAT_CONST(SHORT);
  UDAT_CONST(NONE);

  UCAL_CONST(GREGORIAN);
  UCAL_CONST(TRADITIONAL);

  HHVM_ME(IntlDateFormatter, __construct);
  HHVM_ME(IntlDateFormatter, format);
  HHVM_STATIC_ME(IntlDateFormatter, formatObject);
  HHVM_ME(IntlDateFormatter, getCalendar);
  HHVM_ME(IntlDateFormatter, getDateType);
  HHVM_ME(IntlDateFormatter, getErrorCode);
  HHVM_ME(IntlDateFormatter, getErrorMessage);
  HHVM_ME(IntlDateFormatter, getLocale);
  HHVM_ME(IntlDateFormatter, getPattern);
  HHVM_ME(IntlDateFormatter, getTimeType);
  HHVM_ME(IntlDateFormatter, getTimeZoneId);
  HHVM_ME(IntlDateFormatter, getCalendarObject);
  HHVM_ME(IntlDateFormatter, getTimeZone);
  HHVM_ME(IntlDateFormatter, isLenient);
  HHVM_ME(IntlDateFormatter, localtime);
  HHVM_ME(IntlDateFormatter, parse);
  HHVM_ME(IntlDateFormatter, setCalendar);
  HHVM_ME(IntlDateFormatter, setLenient);
  HHVM_ME(IntlDateFormatter, setPattern);
  HHVM_ME(IntlDateFormatter, setTimeZone);

  Native::registerNativeDataInfo<IntlDateFormatter>(s_IntlDateFormatter.get());

  loadSystemlib("icu_date_fmt");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
