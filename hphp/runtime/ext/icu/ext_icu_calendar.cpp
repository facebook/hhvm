/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/icu/ext_icu_calendar.h"
#include "hphp/runtime/ext/icu/ext_icu_timezone.h"
#include "hphp/runtime/ext/icu/ext_icu_iterator.h"

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_IntlCalendar("IntlCalendar"),
  s_IntlGregorianCalendar("IntlGregorianCalendar");

const icu::Calendar*
IntlCalendar::ParseArg(const Variant& cal, const icu::Locale &locale,
                       const String &funcname, IntlError* err,
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
      err->setError(U_ILLEGAL_ARGUMENT_ERROR,
                    "%s: invalid value for calendar type; it must be "
                    "one of IntlDateFormatter::TRADITIONAL (locale's default "
                    "calendar) or IntlDateFormatter::GREGORIAN. "
                    "Alternatively, it can be an IntlCalendar object",
                    funcname.c_str());
      return nullptr;
    }
    ret = (calType == UCAL_TRADITIONAL)
        ? (icu::Calendar::createInstance(locale, error))
        : (new icu::GregorianCalendar(locale, error));
    calOwned = true;
  } else if (cal.isObject()) {
    auto const data = IntlCalendar::Get(cal.toObject().get());
    if (!data) {
      // ::Get raises errors
      return nullptr;
    }
    calOwned = false;
    return data->calendar();
  } else {
    err->setError(U_ILLEGAL_ARGUMENT_ERROR,
                  "%s: Invalid calendar argument; should be an integer "
                  "or an IntlCalendar instance", funcname.c_str());
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

  err->setError(error, "%s: Failure instantiating calendar", funcname.c_str());
  return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// Methods

#define CAL_FETCH(dest, src, def) \
  auto dest = IntlCalendar::Get(src); \
  if (!dest) { \
    return def; \
  }

#define CAL_CHECK_FIELD(field, func) \
  if ((field < 0) || (field > UCAL_FIELD_COUNT)) { \
    data->setError(U_ILLEGAL_ARGUMENT_ERROR, \
                   "%s: invalid field", func); \
    return false; \
  }

#define GCAL_FETCH(dest, src, def) \
  auto dest = IntlCalendar::Get(src); \
  if (!dest || !dest->gcal()) { \
    return def; \
  }

static bool HHVM_METHOD(IntlCalendar, add, int64_t field, int64_t amount) {
  CAL_FETCH(data, this_, false);
  CAL_CHECK_FIELD(field, "intlcal_add");
  if ((amount < INT32_MIN) || (amount > INT32_MAX)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_add: amount out of bounds");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  data->calendar()->add((UCalendarDateFields)field, (int32_t)amount, error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_add: Call to underlying method failed");
    return false;
  }
  return true;
}

static bool intlcal_compare(ObjectData* this_, const Object& that_,
  UBool (icu::Calendar::*func)(const icu::Calendar&, UErrorCode&) const) {
  CAL_FETCH(obj1, this_, false);
  CAL_FETCH(obj2, that_.get(), false);
  UErrorCode error = U_ZERO_ERROR;
  UBool res = (obj1->calendar()->*func)(*obj2->calendar(), error);
  if (U_FAILURE(error)) {
    obj1->setError(error, "intlcal_before/after: Error calling ICU method");
    return false;
  }
  return res;
}

static bool HHVM_METHOD(IntlCalendar, after, const Object& other) {
  return intlcal_compare(this_, other, &icu::Calendar::after);
}

static bool HHVM_METHOD(IntlCalendar, before, const Object& other) {
  return intlcal_compare(this_, other, &icu::Calendar::before);
}

static bool HHVM_METHOD(IntlCalendar, clear, const Variant& field) {
  CAL_FETCH(data, this_, false);
  if (field.isNull()) {
    data->calendar()->clear();
  } else {
    data->calendar()->clear((UCalendarDateFields)field.toInt64());
  }
  return true;
}

static Object HHVM_STATIC_METHOD(IntlCalendar, createInstance,
                                 const Variant& timeZone, const String& locale) {
  icu::TimeZone *tz =
    IntlTimeZone::ParseArg(timeZone, "intlcal_create_instance",
                           s_intl_error.get());
  if (!tz) {
    return Object();
  }
  String loc = localeOrDefault(locale);
  UErrorCode error = U_ZERO_ERROR;
  icu::Calendar *cal =
    icu::Calendar::createInstance(tz, icu::Locale::createFromName(loc.c_str()),
                                  error);
  if (!cal) {
    delete tz;
    s_intl_error->setError(error, "Error creating ICU Calendar object");
    return Object();
  }
  return IntlCalendar::newInstance(cal);
}

static bool HHVM_METHOD(IntlCalendar, equals, const Object& other) {
  return intlcal_compare(this_, other, &icu::Calendar::equals);
}

static Variant HHVM_METHOD(IntlCalendar, fieldDifference,
                           const Variant& when, int64_t field) {
  CAL_FETCH(data, this_, false);
  CAL_CHECK_FIELD(field, "intlcal_field_difference");
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret = data->calendar()->fieldDifference(
    (UDate)when.toDouble(), (UCalendarDateFields)field, error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_field_difference: "
                          "Call to ICU method has failed");
    return false;
  }
  return ret;
}

static Variant intlcal_field_method(const Object& obj, int64_t field,
       int32_t (icu::Calendar::*func)(UCalendarDateFields, UErrorCode&) const,
       const char *method_name) {
  CAL_FETCH(data, obj.get(), false);
  CAL_CHECK_FIELD(field, method_name);
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret = (data->calendar()->*func)((UCalendarDateFields)field, error);
  if (U_FAILURE(error)) {
    data->setError(error, "Call to ICU method has failed");
    return false;
  }
  return ret;
}

static Variant intlcal_field_method(const Object& obj, int64_t field,
       int32_t (icu::Calendar::*func)(UCalendarDateFields) const,
       const char *method_name) {
  CAL_FETCH(data, obj.get(), false);
  CAL_CHECK_FIELD(field, method_name);
  return (data->calendar()->*func)((UCalendarDateFields)field);
}

static Variant HHVM_METHOD(IntlCalendar, get, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                              &icu::Calendar::get, "intlcal_get");
}

static Variant HHVM_METHOD(IntlCalendar, getActualMaximum, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                              &icu::Calendar::getActualMaximum,
                              "intlcal_get_actual_maximum");
}

static Variant HHVM_METHOD(IntlCalendar, getActualMinimum, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                              &icu::Calendar::getActualMinimum,
                              "intlcal_get_actual_minimum");
}

static Array HHVM_STATIC_METHOD(IntlCalendar, getAvailableLocales) {
  int32_t count;
  const icu::Locale *availLocales = icu::Calendar::getAvailableLocales(count);
  VecInit ret(count);
  for (int i = 0; i < count; ++i) {
    ret.append(String(availLocales[i].getName(), CopyString));
  }
  return ret.toArray();
}

static int64_t HHVM_METHOD(IntlCalendar, getErrorCode) {
  CAL_FETCH(data, this_, 0);
  return data->getErrorCode();
}

static String HHVM_METHOD(IntlCalendar, getErrorMessage) {
  CAL_FETCH(data, this_, String());
  return data->getErrorMessage();
}

static Variant HHVM_METHOD(IntlCalendar, getFirstDayOfWeek) {
  CAL_FETCH(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret = data->calendar()->getFirstDayOfWeek(error);
  if (U_FAILURE(error)) {
    data->setError(error, "Call to ICU method has failed");
    return false;
  }
  return ret;
}

static Variant HHVM_METHOD(IntlCalendar, getGreatestMinimum, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                                &icu::Calendar::getGreatestMinimum,
                                "intlcal_get_greatest_minimum");
}

static Variant HHVM_METHOD(IntlCalendar, getLeastMaximum, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                                &icu::Calendar::getLeastMaximum,
                                "intlcal_get_least_maximum");
}

static Variant HHVM_METHOD(IntlCalendar, getLocale, int64_t localeType) {
  CAL_FETCH(data, this_, false);
  if (localeType != ULOC_ACTUAL_LOCALE && localeType != ULOC_VALID_LOCALE) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_get_locale: invalid locale type");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  icu::Locale locale =
    data->calendar()->getLocale((ULocDataLocaleType)localeType, error);
  if (U_FAILURE(error)) {
    data->setError(error,
                   "intlcal_get_locale: Call to ICU method has failed");
    return false;
  }
  return String(locale.getName(), CopyString);
}

static Variant HHVM_METHOD(IntlCalendar, getMaximum, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                                &icu::Calendar::getMaximum,
                                "intlcal_get_maximum");
}

static Variant HHVM_METHOD(IntlCalendar, getMinimalDaysInFirstWeek) {
  CAL_FETCH(data, this_, false);
  uint64_t ret = data->calendar()->getMinimalDaysInFirstWeek();
  return ret;
}

static Variant HHVM_METHOD(IntlCalendar, getMinimum, int64_t field) {
  return intlcal_field_method(Object{this_}, field,
                                &icu::Calendar::getMinimum,
                                "intlcal_get_maximum");
}

static double HHVM_STATIC_METHOD(IntlCalendar, getNow) {
  return icu::Calendar::getNow();
}

static Variant HHVM_METHOD(IntlCalendar, getTime) {
  CAL_FETCH(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  UDate ret = data->calendar()->getTime(error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_get_time: error calling "
                          "ICU Calendar::getTime");
    return false;
  }
  return (double)ret;
}

static Object HHVM_METHOD(IntlCalendar, getTimezone) {
  CAL_FETCH(data, this_, Object());
  return IntlTimeZone::newInstance(
    data->calendar()->getTimeZone().clone());
}

static Variant HHVM_METHOD(IntlCalendar, getType) {
  CAL_FETCH(data, this_, false);
  return String(data->calendar()->getType(), CopyString);
}

static bool HHVM_METHOD(IntlCalendar, inDaylightTime) {
  CAL_FETCH(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  UBool ret = data->calendar()->inDaylightTime(error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_in_daylight_time: "
                          "Error calling ICU method");
    return false;
  }
  return ret;
}

static bool HHVM_METHOD(IntlCalendar, isEquivalentTo, const Object& other) {
  CAL_FETCH(obj1, this_, false);
  CAL_FETCH(obj2, other.get(), false);
  return obj1->calendar()->isEquivalentTo(*obj2->calendar());
}

static bool HHVM_METHOD(IntlCalendar, isLenient) {
  CAL_FETCH(data, this_, false);
  return data->calendar()->isLenient();
}

static bool HHVM_METHOD(IntlCalendar, isSet, int64_t field) {
  CAL_FETCH(data, this_, false);
  CAL_CHECK_FIELD(field, "intlcal_is_set");
  return data->calendar()->isSet((UCalendarDateFields)field);
}

static bool HHVM_METHOD(IntlCalendar, roll, int64_t field, const Variant& value) {
  CAL_FETCH(data, this_, false);
  CAL_CHECK_FIELD(field, "intlcal_roll");
  UErrorCode error = U_ZERO_ERROR;
  if (value.isBoolean()) {
    data->calendar()->roll((UCalendarDateFields)field,
                           (UBool)value.toBoolean(), error);
  } else {
    data->calendar()->roll((UCalendarDateFields)field,
                           (int32_t)value.toInt64(), error);
  }
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_roll: Error calling ICU Calendar::roll");
    return false;
  }
  return true;
}

static bool HHVM_METHOD(IntlCalendar, set, int64_t yearOrField,
                        int64_t monthOrValue, const Variant& dayOfMonth,
                        const Variant& hour, const Variant& minute,
                        const Variant& second) {
  CAL_FETCH(data, this_, false);

  if (dayOfMonth.isNull()) {
    // we only got 2 args, so the first 2 args are a field and a value
    CAL_CHECK_FIELD(yearOrField, "intcal_set");
    data->calendar()->set((UCalendarDateFields)yearOrField,
                          (int32_t)monthOrValue);
    return true;
  }

  auto outofrange = [&]() {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_set: at least one of the arguments has an "
                   "absolute value that is too large");
    return false;
  };
  if ((yearOrField < INT32_MIN) || (yearOrField > INT32_MAX)) {
    return outofrange();
  }
  auto y = (int32_t)yearOrField;
  if ((monthOrValue < INT32_MIN) || (monthOrValue > INT32_MAX)) {
    return outofrange();
  }
  auto m = (int32_t)monthOrValue;

  assertx(dayOfMonth.isInteger());
  if ((dayOfMonth.asInt64Val() < INT32_MIN) ||
      (dayOfMonth.asInt64Val() > INT32_MAX)) {
    return outofrange();
  }
  auto d = (int32_t)dayOfMonth.asInt64Val();

  if (hour.isNull()) {
    data->calendar()->set(y, m, d);
    return true;
  }
  assertx(hour.isInteger());
  if ((hour.asInt64Val() < INT32_MIN) || (hour.asInt64Val() > INT32_MAX)) {
    return outofrange();
  }
  auto h = (int32_t)hour.asInt64Val();

  if (minute.isNull()) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR, "intlcal_set: bad arguments");
    return false;
  }
  assertx(minute.isInteger());
  if ((minute.asInt64Val() < INT32_MIN) || (minute.asInt64Val() > INT32_MAX)) {
    return outofrange();
  }
  auto i = (int32_t)minute.asInt64Val();

  if (second.isNull()) {
    data->calendar()->set(y, m, d, h, i);
    return true;
  }
  assertx(second.isInteger());
  if ((second.asInt64Val() < INT32_MIN) || (second.asInt64Val() > INT32_MAX)) {
    return outofrange();
  }
  auto s = (int32_t)second.asInt64Val();

  data->calendar()->set(y, m, d, h, i, s);
  return true;
}

static bool HHVM_METHOD(IntlCalendar, setFirstDayOfWeek, int64_t dow) {
  CAL_FETCH(data, this_, false);
  if ((dow < UCAL_SUNDAY) || (dow > UCAL_SATURDAY)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_set_first_day_of_week: invalid day of week");
    return false;
  }
  data->calendar()->setFirstDayOfWeek((UCalendarDaysOfWeek)dow);
  return true;
}

static bool HHVM_METHOD(IntlCalendar, setLenient, bool isLenient) {
  CAL_FETCH(data, this_, false);
  data->calendar()->setLenient((UBool)isLenient);
  return true;
}

static bool HHVM_METHOD(IntlCalendar, setMinimalDaysInFirstWeek,
                        int64_t days) {
  CAL_FETCH(data, this_, false);
  if ((days < 1) || (days > 7)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_set_minimal_days_in_first_week: "
                   "invalid number of days; must be between 1 and 7");
    return false;
  }
  data->calendar()->setMinimalDaysInFirstWeek((uint8_t)days);
  return true;
}

static bool HHVM_METHOD(IntlCalendar, setTime, const Variant& date) {
  CAL_FETCH(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  data->calendar()->setTime((UDate)date.toDouble(), error);
  if (U_FAILURE(error)) {
    data->setError(error, "Call to underlying method failed");
    return false;
  }
  return true;
}

static bool HHVM_METHOD(IntlCalendar, setTimezone, const Variant& timeZone) {
  CAL_FETCH(data, this_, false);
  auto tz = IntlTimeZone::ParseArg(timeZone, "intlcal_set_time_zone",
                                   data);
  if (!tz) {
    // error already set
    return false;
  }
  data->calendar()->adoptTimeZone(tz);
  return true;
}

#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 402
static Variant HHVM_STATIC_METHOD(IntlCalendar, getKeywordValuesForLocale,
                                 const String& key, const String& locale,
                                 bool common) {
  UErrorCode error = U_ZERO_ERROR;
  UEnumeration *uenum = ucal_getKeywordValuesForLocale(key.c_str(),
                                   localeOrDefault(locale).c_str(),
                                   common, &error);
  if (U_FAILURE(error)) {
    if (uenum) { uenum_close(uenum); }
    s_intl_error->setError(error, "intlcal_get_keyword_values_for_locale: "
                                  "error calling underlying method");
    return false;
  }
  return IntlIterator::newInstance(new BugStringCharEnumeration(uenum));
}
#endif // ICU 4.2

#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 404
static Variant HHVM_METHOD(IntlCalendar, getDayOfWeekType, int64_t dow) {
  CAL_FETCH(data, this_, false);
  if ((dow < UCAL_SUNDAY) || (dow > UCAL_SATURDAY)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_get_day_of_week_type: invalid day of week");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret = data->calendar()->getDayOfWeekType(
                                  (UCalendarDaysOfWeek)dow, error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_get_day_of_week_type: "
                          "Call to ICU method has failed");
    return false;
  }
  return ret;
}

static Variant HHVM_METHOD(IntlCalendar, getWeekendTransition,
                           int64_t dow) {
  CAL_FETCH(data, this_, false);
  if ((dow < UCAL_SUNDAY) || (dow > UCAL_SATURDAY)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_get_weekend_transition: "
                   "invalid day of week");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret =
    data->calendar()->getWeekendTransition((UCalendarDaysOfWeek)dow, error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_get_weekend_transition: "
                          "Error calling ICU method");
    return false;
  }
  return ret;
}

static bool HHVM_METHOD(IntlCalendar, isWeekend, const Variant& date) {
  CAL_FETCH(data, this_, false);
  if (date.isNull()) {
    return data->calendar()->isWeekend();
  }
  UErrorCode error = U_ZERO_ERROR;
  bool ret = data->calendar()->isWeekend((UDate)date.toDouble(), error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlcal_is_weekend: Error calling ICU method");
    return false;
  }
  return ret;
}
#endif // ICU 4.4

#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 409
static int64_t HHVM_METHOD(IntlCalendar, getRepeatedWallTimeOption) {
  CAL_FETCH(data, this_, 0);
  return (int64_t)data->calendar()->getRepeatedWallTimeOption();
}

static int64_t HHVM_METHOD(IntlCalendar, getSkippedWallTimeOption) {
  CAL_FETCH(data, this_, 0);
  return (int64_t)data->calendar()->getSkippedWallTimeOption();
}

static bool HHVM_METHOD(IntlCalendar, setRepeatedWallTimeOption,
                        int64_t option) {
  CAL_FETCH(data, this_, false);
  if ((option != UCAL_WALLTIME_FIRST) && (option != UCAL_WALLTIME_LAST)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_set_repeated_wall_time_option: invalid option");
    return false;
  }
  data->calendar()->setRepeatedWallTimeOption((UCalendarWallTimeOption)option);
  return true;
}

static bool HHVM_METHOD(IntlCalendar, setSkippedWallTimeOption,
                        int64_t option) {
  CAL_FETCH(data, this_, false);
  if ((option != UCAL_WALLTIME_FIRST) && (option != UCAL_WALLTIME_LAST)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlcal_set_repeated_wall_time_option: invalid option");
    return false;
  }
  data->calendar()->setSkippedWallTimeOption((UCalendarWallTimeOption)option);
  return true;
}
#endif // ICU 4.9

/////////////////////////////////////////////////////////////////////////////
// IntlGregorianCalendar

static void HHVM_METHOD(IntlGregorianCalendar, __construct,
                        const Variant& yearOrTz, const Variant& monthOrLocale,
                        const Variant& day, const Variant& hour,
                        const Variant& minute, const Variant& second) {

  icu::GregorianCalendar *gcal = nullptr;
  SCOPE_EXIT { if (gcal) { delete gcal; } };
  icu::TimeZone *tz = nullptr;
  SCOPE_EXIT { if (tz) { delete tz; } };
  auto success = [&]() {
    assertx(gcal);
    // tz should be owned by gcal, so we should have cleared our local copy
    assertx(!tz);
    Native::data<IntlCalendar>(this_)->setCalendar(gcal);
    gcal = nullptr;
  };

  if (day.isNull()) {
    // we have 2 args, tz and locale
    tz = IntlTimeZone::ParseArg(yearOrTz, "intlgregcal_create_instance",
                                s_intl_error.get());
    auto error = U_ZERO_ERROR;
    gcal = new icu::GregorianCalendar(
      tz,
      icu::Locale::createFromName(
        localeOrDefault(monthOrLocale.toString()).c_str()
      ),
      error
    );
    if (gcal) tz = nullptr; // gcal owns tz now
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "intlgregcal_create_instance: error "
                             "creating ICU GregorianCalendar from time zone "
                             "and locale");
      return;
    }
    return success();
  }

  auto arg_ok = true;
  auto coerce_arg = [&](const Variant& v) {
    int64_t arg = v.toInt64();
    if ((arg < INT32_MIN) || (arg > INT32_MAX)) {
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                             "intlgregcal_create_instance: at least one of "
                             "the arguments has an absolute value that is "
                             "too large");
      arg_ok = false;
      return 0;
    }
    return (int32_t)arg;
  };

  auto y = coerce_arg(yearOrTz);
  if (!arg_ok) return;
  auto m = coerce_arg(monthOrLocale);
  if (!arg_ok) return;
  auto d = coerce_arg(day);
  if (!arg_ok) return;

  auto error = U_ZERO_ERROR;
  auto finish = [&]() {
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "intlgregcal_create_instance: error "
                             "creating ICU GregorianCalendar from date");
      return;
    }
    assertx(gcal);
    tz = IntlTimeZone::ParseArg(uninit_null(), "intlgregcal_create_instance",
                                s_intl_error.get());
    if (!tz) {
      // error already set
      return;
    }
    gcal->adoptTimeZone(tz);
    tz = nullptr; // gcal owns tz now
    return success();
  };

  if (hour.isNull()) {
    gcal = new icu::GregorianCalendar(y, m, d, error);
    return finish();
  }
  auto h = coerce_arg(hour);
  if (!arg_ok) return;

  if (minute.isNull()) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "intlgregcal_create_instance: no variant with "
                           "4 arguments (excluding trailing NULLs)");
    return;
  }
  auto i = coerce_arg(minute);
  if (!arg_ok) return;

  if (second.isNull()) {
    gcal = new icu::GregorianCalendar(y, m, d, h, i, error);
    return finish();
  }
  auto s = coerce_arg(second);
  if (!arg_ok) return;

  gcal = new icu::GregorianCalendar(y, m, d, h, i, s, error);
  return finish();
}

static bool HHVM_METHOD(IntlGregorianCalendar, isLeapYear, int64_t year) {
  GCAL_FETCH(data, this_, false);
  if ((year < INT32_MIN) || (year > INT32_MAX)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "intlgregcal_is_leap_year: year out of bounds");
    return false;
  }
  return (bool)data->gcal()->isLeapYear((int32_t)year);
}

static double HHVM_METHOD(IntlGregorianCalendar, getGregorianChange) {
  GCAL_FETCH(data, this_, 0.0);
  return (double)data->gcal()->getGregorianChange();
}

static bool HHVM_METHOD(IntlGregorianCalendar, setGregorianChange,
                        double change) {
  GCAL_FETCH(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  data->gcal()->setGregorianChange(change, error);
  if (U_FAILURE(error)) {
    data->setError(error, "intlgregcal_set_gregorian_change: error "
                          "calling ICU method");
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// Extension

void IntlExtension::registerNativeCalendar() {
  HHVM_RCC_INT(IntlCalendar, FIELD_ERA, UCAL_ERA);
  HHVM_RCC_INT(IntlCalendar, FIELD_YEAR, UCAL_YEAR);
  HHVM_RCC_INT(IntlCalendar, FIELD_MONTH, UCAL_MONTH);
  HHVM_RCC_INT(IntlCalendar, FIELD_WEEK_OF_YEAR, UCAL_WEEK_OF_YEAR);
  HHVM_RCC_INT(IntlCalendar, FIELD_WEEK_OF_MONTH, UCAL_WEEK_OF_MONTH);
  HHVM_RCC_INT(IntlCalendar, FIELD_DATE, UCAL_DATE);
  HHVM_RCC_INT(IntlCalendar, FIELD_DAY_OF_YEAR, UCAL_DAY_OF_YEAR);
  HHVM_RCC_INT(IntlCalendar, FIELD_DAY_OF_WEEK, UCAL_DAY_OF_WEEK);
  HHVM_RCC_INT(IntlCalendar, FIELD_DAY_OF_WEEK_IN_MONTH,
               UCAL_DAY_OF_WEEK_IN_MONTH);
  HHVM_RCC_INT(IntlCalendar, FIELD_AM_PM, UCAL_AM_PM);
  HHVM_RCC_INT(IntlCalendar, FIELD_HOUR, UCAL_HOUR);
  HHVM_RCC_INT(IntlCalendar, FIELD_HOUR_OF_DAY, UCAL_HOUR_OF_DAY);
  HHVM_RCC_INT(IntlCalendar, FIELD_MINUTE, UCAL_MINUTE);
  HHVM_RCC_INT(IntlCalendar, FIELD_SECOND, UCAL_SECOND);
  HHVM_RCC_INT(IntlCalendar, FIELD_MILLISECOND, UCAL_MILLISECOND);
  HHVM_RCC_INT(IntlCalendar, FIELD_ZONE_OFFSET, UCAL_ZONE_OFFSET);
  HHVM_RCC_INT(IntlCalendar, FIELD_DST_OFFSET, UCAL_DST_OFFSET);
  HHVM_RCC_INT(IntlCalendar, FIELD_YEAR_WOY, UCAL_YEAR_WOY);
  HHVM_RCC_INT(IntlCalendar, FIELD_DOW_LOCAL, UCAL_DOW_LOCAL);
  HHVM_RCC_INT(IntlCalendar, FIELD_EXTENDED_YEAR, UCAL_EXTENDED_YEAR);
  HHVM_RCC_INT(IntlCalendar, FIELD_JULIAN_DAY, UCAL_JULIAN_DAY);
  HHVM_RCC_INT(IntlCalendar, FIELD_MILLISECONDS_IN_DAY,
               UCAL_MILLISECONDS_IN_DAY);
  HHVM_RCC_INT(IntlCalendar, FIELD_IS_LEAP_MONTH, UCAL_IS_LEAP_MONTH);
  HHVM_RCC_INT(IntlCalendar, FIELD_FIELD_COUNT, UCAL_FIELD_COUNT);
  HHVM_RCC_INT(IntlCalendar, FIELD_DAY_OF_MONTH, UCAL_DAY_OF_MONTH);

  HHVM_RCC_INT(IntlCalendar, DOW_SUNDAY, UCAL_SUNDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_MONDAY, UCAL_MONDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_TUESDAY, UCAL_TUESDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_WEDNESDAY, UCAL_WEDNESDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_THURSDAY, UCAL_THURSDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_FRIDAY, UCAL_FRIDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_SATURDAY, UCAL_SATURDAY);

#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 404
  HHVM_RCC_INT(IntlCalendar, DOW_TYPE_WEEKDAY, UCAL_WEEKDAY);
  HHVM_RCC_INT(IntlCalendar, DOW_TYPE_WEEKEND, UCAL_WEEKEND);
  HHVM_RCC_INT(IntlCalendar, DOW_TYPE_WEEKEND_CEASE, UCAL_WEEKEND_CEASE);

  // Not a typo: Zend defines OFFSET as ONSET
  HHVM_RCC_INT(IntlCalendar, DOW_TYPE_WEEKEND_OFFSET, UCAL_WEEKEND_ONSET);
#endif

#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 409
  HHVM_RCC_INT(IntlCalendar, WALLTIME_FIRST, UCAL_WALLTIME_FIRST);
  HHVM_RCC_INT(IntlCalendar, WALLTIME_LAST, UCAL_WALLTIME_LAST);
  HHVM_RCC_INT(IntlCalendar, WALLTIME_NEXT_VALID, UCAL_WALLTIME_NEXT_VALID);
#endif

  HHVM_ME(IntlCalendar, add);
  HHVM_ME(IntlCalendar, after);
  HHVM_ME(IntlCalendar, before);
  HHVM_ME(IntlCalendar, clear);
  HHVM_STATIC_ME(IntlCalendar, createInstance);
  HHVM_ME(IntlCalendar, equals);
  HHVM_ME(IntlCalendar, fieldDifference);
  HHVM_ME(IntlCalendar, get);
  HHVM_ME(IntlCalendar, getActualMaximum);
  HHVM_ME(IntlCalendar, getActualMinimum);
  HHVM_STATIC_ME(IntlCalendar, getAvailableLocales);
  HHVM_ME(IntlCalendar, getErrorCode);
  HHVM_ME(IntlCalendar, getErrorMessage);
  HHVM_ME(IntlCalendar, getFirstDayOfWeek);
  HHVM_ME(IntlCalendar, getGreatestMinimum);
  HHVM_ME(IntlCalendar, getLeastMaximum);
  HHVM_ME(IntlCalendar, getLocale);
  HHVM_ME(IntlCalendar, getMaximum);
  HHVM_ME(IntlCalendar, getMinimalDaysInFirstWeek);
  HHVM_ME(IntlCalendar, getMinimum);
  HHVM_STATIC_ME(IntlCalendar, getNow);
  HHVM_ME(IntlCalendar, getTime);
  HHVM_ME(IntlCalendar, getTimezone);
  HHVM_ME(IntlCalendar, getType);
  HHVM_ME(IntlCalendar, inDaylightTime);
  HHVM_ME(IntlCalendar, isEquivalentTo);
  HHVM_ME(IntlCalendar, isLenient);
  HHVM_ME(IntlCalendar, isSet);
  HHVM_ME(IntlCalendar, roll);
  HHVM_ME(IntlCalendar, set);
  HHVM_ME(IntlCalendar, setFirstDayOfWeek);
  HHVM_ME(IntlCalendar, setLenient);
  HHVM_ME(IntlCalendar, setMinimalDaysInFirstWeek);
  HHVM_ME(IntlCalendar, setTime);
  HHVM_ME(IntlCalendar, setTimezone);

#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 402
  HHVM_STATIC_ME(IntlCalendar, getKeywordValuesForLocale);
#endif
#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 404
  HHVM_ME(IntlCalendar, getDayOfWeekType);
  HHVM_ME(IntlCalendar, getWeekendTransition);
  HHVM_ME(IntlCalendar, isWeekend);
#endif
#if ((U_ICU_VERSION_MAJOR_NUM * 100) + U_ICU_VERSION_MINOR_NUM) >= 409
  HHVM_ME(IntlCalendar, getRepeatedWallTimeOption);
  HHVM_ME(IntlCalendar, getSkippedWallTimeOption);
  HHVM_ME(IntlCalendar, setRepeatedWallTimeOption);
  HHVM_ME(IntlCalendar, setSkippedWallTimeOption);
#endif

  HHVM_ME(IntlGregorianCalendar, __construct);
  HHVM_ME(IntlGregorianCalendar, isLeapYear);
  HHVM_ME(IntlGregorianCalendar, getGregorianChange);
  HHVM_ME(IntlGregorianCalendar, setGregorianChange);

  Native::registerNativeDataInfo<IntlCalendar>();
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
