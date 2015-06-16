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
#include "hphp/runtime/ext/icu/ext_icu_timezone.h"
#include "hphp/runtime/ext/icu/ext_icu_iterator.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include <unicode/locid.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
const StaticString s_IntlTimeZone("IntlTimeZone");

Class* IntlTimeZone::c_IntlTimeZone = nullptr;

static bool ustring_from_char(icu::UnicodeString& ret,
                              const String& str,
                              UErrorCode &error) {
  error = U_ZERO_ERROR;
  ret = u16(str, error, U_SENTINEL);
  if (U_FAILURE(error)) {
    ret.setToBogus();
    return false;
  }
  return true;
}

icu::TimeZone* IntlTimeZone::ParseArg(const Variant& arg,
                                      const String& funcname,
                                      IntlError* err) {
  String tzstr;

  if (arg.isNull()) {
    tzstr = f_date_default_timezone_get();
  } else if (arg.isObject()) {
    auto objarg = arg.toObject();
    auto cls = objarg->getVMClass();
    auto IntlTimeZone_Class = Unit::lookupClass(s_IntlTimeZone.get());
    if (IntlTimeZone_Class &&
        ((cls == IntlTimeZone_Class) || cls->classof(IntlTimeZone_Class))) {
      return IntlTimeZone::Get(objarg.get())->timezone()->clone();
    }
    if (objarg.instanceof(DateTimeZoneData::getClass())) {
      auto* dtz = Native::data<DateTimeZoneData>(objarg);
      tzstr = dtz->getName();
    } else {
      tzstr = arg.toString();
    }
  } else {
    tzstr = arg.toString();
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, tzstr, error)) {
    err->setError(error, "%s: Time zone identifier given is not a "
                         "valid UTF-8 string", funcname.c_str());
    return nullptr;
  }
  auto ret = icu::TimeZone::createTimeZone(id);
  if (!ret) {
    err->setError(U_MEMORY_ALLOCATION_ERROR,
                  "%s: could not create time zone", funcname.c_str());
    return nullptr;
  }
  icu::UnicodeString gottenId;
  if (ret->getID(gottenId) != id) {
    err->setError(U_ILLEGAL_ARGUMENT_ERROR,
                  "%s: no such time zone: '%s'",
                  funcname.c_str(), arg.toString().c_str());
    delete ret;
    return nullptr;
  }
  return ret;
}

#define TZ_GET(dest, src, def) \
  auto dest = IntlTimeZone::Get(src); \
  if (!dest) { \
    return def; \
  }

#define TZ_CHECK(ov, ec, fail) \
  if (U_FAILURE(ec)) { \
    ov->setError(ec); \
    return fail; \
  }

//////////////////////////////////////////////////////////////////////////////
// class IntlTimeZone

static Variant HHVM_STATIC_METHOD(IntlTimeZone, countEquivalentIDs,
                                  const String& zoneId) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->setError(error, "intltz_count_equivalent_ids: could not "
                                  "convert time zone id to UTF-16");
    return false;
  }
  return icu::TimeZone::countEquivalentIDs(id);
}

static Object HHVM_STATIC_METHOD(IntlTimeZone, createDefault) {
  return IntlTimeZone::newInstance(icu::TimeZone::createDefault());
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, createEnumeration,
                                  const Variant& countryRawOffset) {
  icu::StringEnumeration *se = nullptr;

  if (countryRawOffset.isNull()) {
    se = icu::TimeZone::createEnumeration();
  } else if (countryRawOffset.isNumeric(true)) {
    se = icu::TimeZone::createEnumeration((int32_t)countryRawOffset.toInt64());
  } else if (countryRawOffset.isString() || countryRawOffset.isObject()) {
    se = icu::TimeZone::createEnumeration(countryRawOffset.toString().c_str());
  } else {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "intltz_create_enumeration: invalid argument type");
    return false;
  }
  return IntlIterator::newInstance(se);
}

static Object HHVM_STATIC_METHOD(IntlTimeZone, createTimeZone,
                                 const String& zoneId) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->setError(error, "intltz_count_equivalent_ids: could not "
                                  "convert time zone id to UTF-16");
    return Object();
  }
  return IntlTimeZone::newInstance(icu::TimeZone::createTimeZone(id));
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, getCanonicalID,
                                  const String& zoneId,
                                  VRefParam isSystemID) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->setError(error, "intltz_get_canonical_id: could not convert "
                                  "time zone id to UTF-16");
    return false;
  }

  icu::UnicodeString result;
  UBool system;
  error = U_ZERO_ERROR;
  icu::TimeZone::getCanonicalID(id, result, system, error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "intltz_get_canonical_id: "
                                  "error obtaining canonical ID");
    return false;
  }

  isSystemID = (bool)system;
  error = U_ZERO_ERROR;
  String ret(u8(result, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "intltz_get_canonical_id: could not convert "
                                  "time zone id to UTF-8");
    return false;
  }
  return ret;
}

static Variant HHVM_METHOD(IntlTimeZone, getDisplayName,
                           bool isDaylight, int64_t style,
                           const String& locale) {
  if (!IntlTimeZone::isValidStyle(style)) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "intltz_get_display_name: wrong display type");
    return false;
  }
  TZ_GET(data, this_, false);
  icu::UnicodeString result;
  data->timezone()->getDisplayName((UBool)isDaylight,
                                   (icu::TimeZone::EDisplayType)style,
                                    icu::Locale::createFromName(
                                     localeOrDefault(locale).c_str()),
                                   result);
  UErrorCode error = U_ZERO_ERROR;
  String ret(u8(result, error));
  TZ_CHECK(data, error, false);
  return ret;
}

static int64_t HHVM_METHOD(IntlTimeZone, getDSTSavings) {
  TZ_GET(data, this_, -1);
  return data->timezone()->getDSTSavings();
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, getEquivalentID,
                                  const String& zoneId, int64_t index) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->setError(error, "intltz_get_canonical_id: could not convert "
                                  "time zone id to UTF-16");
    return false;
  }

  auto result = icu::TimeZone::getEquivalentID(id, (int32_t)index);
  error = U_ZERO_ERROR;
  String ret(u8(result, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "intltz_get_equivalent_id: "
                                  "could not convert resulting time zone id "
                                  "to UTF-16");
    return false;
  }
  return ret;
}

static int64_t HHVM_METHOD(IntlTimeZone, getErrorCode) {
  TZ_GET(data, this_, 0);
  return data->getErrorCode();
}

static String HHVM_METHOD(IntlTimeZone, getErrorMessage) {
  TZ_GET(data, this_, String());
  return data->getErrorMessage();
}

static Object HHVM_STATIC_METHOD(IntlTimeZone, getGMT) {
  return IntlTimeZone::newInstance(
    const_cast<icu::TimeZone*>(icu::TimeZone::getGMT()), false);
}

static Variant HHVM_METHOD(IntlTimeZone, getID) {
  TZ_GET(data, this_, false);
  icu::UnicodeString id;
  data->timezone()->getID(id);
  UErrorCode error = U_ZERO_ERROR;
  String ret(u8(id, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error,
                           "intltz_get_id: Could not convert id to UTF-8");
    return false;
  }
  return ret;
}

static bool HHVM_METHOD(IntlTimeZone, getOffset,
                        double date, bool local,
                        VRefParam rawOffset, VRefParam dstOffset) {
  TZ_GET(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  int32_t rawOff, dstOff;
  data->timezone()->getOffset(date, (UBool)local, rawOff, dstOff, error);
  if (U_FAILURE(error)) {
    data->setError(error, "intltz_get_offset: error obtaining offset");
    return false;
  }
  rawOffset = rawOff;
  dstOffset = dstOff;
  return true;
}

static Variant HHVM_METHOD(IntlTimeZone, getRawOffset) {
  TZ_GET(data, this_, false);
  return data->timezone()->getRawOffset();
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, getTZDataVersion) {
  UErrorCode error = U_ZERO_ERROR;
  const char *tzdv = icu::TimeZone::getTZDataVersion(error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "intltz_get_tz_data_version: "
                                  "Error obtaining time zone data version");
    return false;
  }
  return String(tzdv, CopyString);
}

static bool HHVM_METHOD(IntlTimeZone, hasSameRules, const Object& otherTimeZone) {
  TZ_GET(obj1, this_, false);
  TZ_GET(obj2, otherTimeZone.get(), false);
  return obj1->timezone()->hasSameRules(*obj2->timezone());
}

static bool HHVM_METHOD(IntlTimeZone, useDaylightTime) {
  TZ_GET(data, this_, false);
  return data->timezone()->useDaylightTime();
}

//////////////////////////////////////////////////////////////////////////////
// ICU >= 4.8

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 48
static Variant HHVM_STATIC_METHOD(IntlTimeZone, createTimeZoneIDEnumeration,
                                  int64_t zoneType,
                                  const String& region,
                                  const Variant& offset) {
  if (zoneType != UCAL_ZONE_TYPE_ANY &&
      zoneType != UCAL_ZONE_TYPE_CANONICAL &&
      zoneType != UCAL_ZONE_TYPE_CANONICAL_LOCATION) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "intltz_create_time_zone_id_enumeration: "
                           "bad zone type");
    return false;
  }

  int32_t *pofs = nullptr;
  int32_t   ofs = 0;
  if (offset.isInitialized()) {
    ofs = offset.toInt64();
    pofs = &ofs;
  }

  UErrorCode error = U_ZERO_ERROR;
  auto se = icu::TimeZone::createTimeZoneIDEnumeration(
                                 (USystemTimeZoneType)zoneType,
                                 region.c_str(), pofs, error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "intltz_create_time_zone_id_enumeration: "
                                  "Error obtaining time zone id enumeration");
    return false;
  }
  return IntlIterator::newInstance(se);
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, getRegion,
                                  const String& str) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, str, error)) {
    s_intl_error->setError(error, "intltz_get_region: could not convert "
                                  "time zone id to UTF-16");
    return false;
  }

  char outbuf[3];
  error = U_ZERO_ERROR;
  int32_t len = icu::TimeZone::getRegion(id, outbuf, sizeof(outbuf), error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "intltz_get_region: Error obtaining region");
    return false;
  }
  return String(outbuf, len, CopyString);
}
#endif // ICU 4.8

//////////////////////////////////////////////////////////////////////////////
// ICU >= 4.9

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 49
static Object HHVM_STATIC_METHOD(IntlTimeZone, getUnknown) {
  return IntlTimeZone::newInstance(
    const_cast<icu::TimeZone*>(&icu::TimeZone::getUnknown()), false);
}
#endif // ICU 4.9

//////////////////////////////////////////////////////////////////////////////

#define DISP_CONST(v) Native::registerClassConstant<KindOfInt64>( \
                      s_IntlTimeZone.get(), makeStaticString("DISPLAY_" #v), \
                      icu::TimeZone::v)

#define CAL_CONST(v) Native::registerClassConstant<KindOfInt64>( \
                      s_IntlTimeZone.get(), makeStaticString(#v), \
                      UCAL_ZONE_ ## v)

void IntlExtension::initTimeZone() {
  DISP_CONST(SHORT);
  DISP_CONST(LONG);
#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 44
  DISP_CONST(SHORT_GENERIC);
  DISP_CONST(LONG_GENERIC);
  DISP_CONST(SHORT_GMT);
  DISP_CONST(LONG_GMT);
  DISP_CONST(SHORT_COMMONLY_USED);
  DISP_CONST(GENERIC_LOCATION);
#endif // ICU 4.4

  HHVM_STATIC_ME(IntlTimeZone, countEquivalentIDs);
  HHVM_STATIC_ME(IntlTimeZone, createDefault);
  HHVM_STATIC_ME(IntlTimeZone, createEnumeration);
  HHVM_STATIC_ME(IntlTimeZone, createTimeZone);
  HHVM_STATIC_ME(IntlTimeZone, getCanonicalID);
  HHVM_ME(IntlTimeZone, getDisplayName);
  HHVM_ME(IntlTimeZone, getDSTSavings);
  HHVM_STATIC_ME(IntlTimeZone, getEquivalentID);
  HHVM_ME(IntlTimeZone, getErrorCode);
  HHVM_ME(IntlTimeZone, getErrorMessage);
  HHVM_STATIC_ME(IntlTimeZone, getGMT);
  HHVM_ME(IntlTimeZone, getID);
  HHVM_ME(IntlTimeZone, getOffset);
  HHVM_ME(IntlTimeZone, getRawOffset);
  HHVM_STATIC_ME(IntlTimeZone, getTZDataVersion);
  HHVM_ME(IntlTimeZone, hasSameRules);
  HHVM_ME(IntlTimeZone, useDaylightTime);

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 48
  CAL_CONST(TYPE_ANY);
  CAL_CONST(TYPE_CANONICAL);
  CAL_CONST(TYPE_CANONICAL_LOCATION);
  HHVM_STATIC_ME(IntlTimeZone, createTimeZoneIDEnumeration);
  HHVM_STATIC_ME(IntlTimeZone, getRegion);
#endif // ICU 4.8

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 49
 HHVM_STATIC_ME(IntlTimeZone, getUnknown);
#endif // ICU 4.9

  Native::registerNativeDataInfo<IntlTimeZone>(s_IntlTimeZone.get());

  loadSystemlib("icu_timezone");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
