#include "hphp/runtime/ext/icu/ext_icu_timezone.h"
#include "hphp/runtime/ext/icu/ext_icu_iterator.h"

#include <unicode/locid.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
const StaticString s_IntlTimeZone("IntlTimeZone");

IntlTimeZone *IntlTimeZone::Get(Object obj) {
  return GetResData<IntlTimeZone>(obj, s_IntlTimeZone.get());
}

Object IntlTimeZone::wrap() {
  return WrapResData(s_IntlTimeZone.get());
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

#define ULOC_DEFAULT(loc) (loc.empty() ? Intl::GetDefaultLocale() : loc)

//////////////////////////////////////////////////////////////////////////////
// class IntlTimeZone

static Variant HHVM_STATIC_METHOD(IntlTimeZone, countEquivalentIDs,
                                  const String& zoneId) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->set(error, "intltz_count_equivalent_ids: could not convert "
                             "time zone id to UTF-16");
    return false;
  }
  return icu::TimeZone::countEquivalentIDs(id);
}

static Object HHVM_STATIC_METHOD(IntlTimeZone, createDefault) {
  return (NEWOBJ(IntlTimeZone)(icu::TimeZone::createDefault()))->wrap();
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, createEnumeration,
                                  CVarRef countryRawOffset) {
  icu::StringEnumeration *se = nullptr;

  if (countryRawOffset.isNull()) {
    se = icu::TimeZone::createEnumeration();
  } else if (countryRawOffset.isNumeric(true)) {
    se = icu::TimeZone::createEnumeration((int32_t)countryRawOffset.toInt64());
  } else if (countryRawOffset.isString() || countryRawOffset.isObject()) {
    se = icu::TimeZone::createEnumeration(countryRawOffset.toString().c_str());
  } else {
    s_intl_error->set(U_ILLEGAL_ARGUMENT_ERROR,
                      "intltz_create_enumeration: invalid argument type");
    return false;
  }
  return (NEWOBJ(IntlIterator)(se))->wrap();
}

static Object HHVM_STATIC_METHOD(IntlTimeZone, createTimeZone,
                                 const String& zoneId) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->set(error, "intltz_count_equivalent_ids: could not convert "
                             "time zone id to UTF-16");
    return null_object;
  }
  return (NEWOBJ(IntlTimeZone)(icu::TimeZone::createTimeZone(id)))->wrap();
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, getCanonicalID,
                                  const String& zoneId,
                                  VRefParam isSystemID) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, zoneId, error)) {
    s_intl_error->set(error, "intltz_get_canonical_id: could not convert "
                             "time zone id to UTF-16");
    return false;
  }

  icu::UnicodeString result;
  UBool system;
  error = U_ZERO_ERROR;
  icu::TimeZone::getCanonicalID(id, result, system, error);
  if (U_FAILURE(error)) {
    s_intl_error->set(error, "intltz_get_canonical_id: "
                             "error obtaining canonical ID");
    return false;
  }

  isSystemID = (bool)system;
  error = U_ZERO_ERROR;
  String ret(u8(result, error));
  if (U_FAILURE(error)) {
    s_intl_error->set(error, "intltz_get_canonical_id: could not convert "
                             "time zone id to UTF-8");
    return false;
  }
  return ret;
}

static Variant HHVM_METHOD(IntlTimeZone, getDisplayName,
                           bool isDaylight, int64_t style,
                           const String& locale) {
  if (!IntlTimeZone::isValidStyle(style)) {
    s_intl_error->set(U_ILLEGAL_ARGUMENT_ERROR,
                      "intltz_get_display_name: wrong display type");
    return false;
  }
  TZ_GET(data, this_, false);
  icu::UnicodeString result;
  data->timezone()->getDisplayName((UBool)isDaylight,
                                   (icu::TimeZone::EDisplayType)style,
                                    icu::Locale::createFromName(
                                     ULOC_DEFAULT(locale).c_str()),
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
    s_intl_error->set(error, "intltz_get_canonical_id: could not convert "
                             "time zone id to UTF-16");
    return false;
  }

  auto result = icu::TimeZone::getEquivalentID(id, (int32_t)index);
  error = U_ZERO_ERROR;
  String ret(u8(result, error));
  if (U_FAILURE(error)) {
    s_intl_error->set(error, "intltz_get_equivalent_id: "
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
  TZ_GET(data, this_, null_string);
  return data->getErrorMessage();
}

static Object HHVM_STATIC_METHOD(IntlTimeZone, getGMT) {
  return (NEWOBJ(IntlTimeZone)(
    const_cast<icu::TimeZone*>(icu::TimeZone::getGMT()), false))->wrap();
}

static Variant HHVM_METHOD(IntlTimeZone, getID) {
  TZ_GET(data, this_, false);
  icu::UnicodeString id;
  data->timezone()->getID(id);
  UErrorCode error = U_ZERO_ERROR;
  String ret(u8(id, error));
  if (U_FAILURE(error)) {
    s_intl_error->set(error, "intltz_get_id: Could not convert id to UTF-8");
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
    s_intl_error->set(error, "intltz_get_tz_data_version: "
                             "Error obtaining time zone data version");
    return false;
  }
  return String(tzdv, CopyString);
}

static bool HHVM_METHOD(IntlTimeZone, hasSameRules, CObjRef otherTimeZone) {
  TZ_GET(obj1, this_, false);
  TZ_GET(obj2, otherTimeZone, false);
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
                                  CVarRef offset) {
  if (zoneType != UCAL_ZONE_TYPE_ANY &&
      zoneType != UCAL_ZONE_TYPE_CANONICAL &&
      zoneType != UCAL_ZONE_TYPE_CANONICAL_LOCATION) {
    s_intl_error->set(U_ILLEGAL_ARGUMENT_ERROR,
                      "intltz_create_time_zone_id_enumeration: bad zone type");
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
    s_intl_error->set(error, "intltz_create_time_zone_id_enumeration: "
                             "Error obtaining time zone id enumeration");
    return false;
  }
  return (NEWOBJ(IntlIterator)(se))->wrap();
}

static Variant HHVM_STATIC_METHOD(IntlTimeZone, getRegion,
                                  const String& str) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString id;
  if (!ustring_from_char(id, str, error)) {
    s_intl_error->set(error, "intltz_get_region: could not convert "
                             "time zone id to UTF-16");
    return false;
  }

  char outbuf[3];
  error = U_ZERO_ERROR;
  int32_t len = icu::TimeZone::getRegion(id, outbuf, sizeof(outbuf), error);
  if (U_FAILURE(error)) {
    s_intl_error->set(error, "intltz_get_region: Error obtaining region");
    return false;
  }
  return String(outbuf, len, CopyString);
}
#endif // ICU 4.8

//////////////////////////////////////////////////////////////////////////////
// ICU >= 4.9

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 49
static Variant HHVM_STATIC_METHOD(IntlTimeZone, getUnknown) {
  return (NEWOBJ(IntlTimeZone)(
    const_cast<icu::TimeZone*>(&icu::TimeZone::getUnknown()), false))->wrap();
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

  loadSystemlib("icu_timezone");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
