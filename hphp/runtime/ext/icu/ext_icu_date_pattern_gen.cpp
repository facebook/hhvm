#include "hphp/runtime/ext/icu/ext_icu_date_pattern_gen.h"
#include "hphp/runtime/ext/icu/ext_icu_iterator.h"

namespace HPHP::Intl {
//////////////////////////////////////////////////////////////////////////////
// Internal Resource Data

const StaticString s_IntlDatePatternGenerator("IntlDatePatternGenerator");

#define GENERATOR_GET(dest, src) \
  auto dest = IntlDatePatternGenerator::Get(src); \
  if (!dest) { \
    data->throwException("Invalid date pattern generator"); \
  }

#define ICU_ERR_CHECK(data, ec) \
  if (U_FAILURE(ec)) { \
    data->setError(ec); \
    data->throwException("Date pattern generator error: %d (%s)", \
                         error, u_errorName(error)); \
  }

#define ICU_ERR_CHECK_MSG(data, ec, msg) \
  if (U_FAILURE(ec)) { \
    data->setError(ec, msg); \
    data->throwException(#msg ": %d (%s)", ec, u_errorName(ec)); \
  }

static UDateTimePatternField cast_ptn_field(IntlError *errorHandler,
                                            int64_t fieldInt) {
  // The pattern fields are in an enum and should be less than the field count
  if ((fieldInt < 0) || (fieldInt >= UDATPG_FIELD_COUNT)) {
    const char *msg = "Invalid value: %ld for pattern field";
    errorHandler->setError(U_ILLEGAL_ARGUMENT_ERROR, msg, fieldInt);
    errorHandler->throwException(msg, fieldInt);
  }

  return static_cast<UDateTimePatternField>(fieldInt);
}

//////////////////////////////////////////////////////////////////////////////
// class IntlDatePatternGenerator

static Object HHVM_STATIC_METHOD(IntlDatePatternGenerator, createInstance,
                                 const String& locale) {
  if (locale.empty()) {
    s_intl_error->throwException("No locale provided");
  }

  auto loc = icu::Locale::createFromName(locale.c_str());
  UErrorCode error = U_ZERO_ERROR;
  auto generator = std::unique_ptr<icu::DateTimePatternGenerator>(
    icu::DateTimePatternGenerator::createInstance(loc, error));
  ICU_ERR_CHECK_MSG(s_intl_error, error,
                    "Error creating ICU DateTimePatternGenerator object");

  return IntlDatePatternGenerator::newInstance(std::move(generator));
}

static Object HHVM_STATIC_METHOD(IntlDatePatternGenerator,
                                 createEmptyInstance) {
  UErrorCode error = U_ZERO_ERROR;
  auto generator = std::unique_ptr<icu::DateTimePatternGenerator>(
    icu::DateTimePatternGenerator::createEmptyInstance(error));
  ICU_ERR_CHECK_MSG(s_intl_error, error,
                    "Error creating ICU DateTimePatternGenerator object");

  return IntlDatePatternGenerator::newInstance(std::move(generator));
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          getSkeleton,
                          const String& pattern) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString ptn(u16(pattern, error));
  ICU_ERR_CHECK(data, error);

  icu::UnicodeString ret = data->generator().getSkeleton(ptn, error);
  ICU_ERR_CHECK_MSG(data, error, "Error getting skeleton");

  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          getBaseSkeleton,
                          const String& pattern) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString ptn(u16(pattern, error));
  ICU_ERR_CHECK(data, error);

  icu::UnicodeString ret = data->generator().getBaseSkeleton(ptn, error);
  ICU_ERR_CHECK_MSG(data, error, "Error getting base skeleton");

  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static int64_t HHVM_METHOD(IntlDatePatternGenerator,
                           addPattern,
                           const String& pattern,
                           bool override) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString ptn(u16(pattern, error));
  icu::UnicodeString conflictingPtn;
  ICU_ERR_CHECK(data, error);

  int64_t ret;
  ret = data->generator().addPattern(ptn, override, conflictingPtn, error);
  ICU_ERR_CHECK_MSG(data, error, "Error adding pattern");

  return ret;
}

static void HHVM_METHOD(IntlDatePatternGenerator,
                        setAppendItemFormat,
                        int64_t fieldInt,
                        const String& format) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString fmt(u16(format, error));
  ICU_ERR_CHECK(data, error);

  data->generator().setAppendItemFormat(cast_ptn_field(data, fieldInt), fmt);
  ICU_ERR_CHECK_MSG(data, error, "Error setting append item format");
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          getAppendItemFormat,
                          int64_t fieldInt) {
  GENERATOR_GET(data, this_);

  icu::UnicodeString ret;
  ret = data->generator().getAppendItemFormat(cast_ptn_field(data, fieldInt));

  UErrorCode error = U_ZERO_ERROR;
  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static void HHVM_METHOD(IntlDatePatternGenerator,
                        setAppendItemName,
                        int64_t fieldInt,
                        const String& name) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString value(u16(name, error));
  ICU_ERR_CHECK(data, error);

  data->generator().setAppendItemName(cast_ptn_field(data, fieldInt), value);
  ICU_ERR_CHECK_MSG(data, error, "Error setting append item name");
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          getAppendItemName,
                          int64_t fieldInt) {
  GENERATOR_GET(data, this_);

  icu::UnicodeString ret;
  ret = data->generator().getAppendItemName(cast_ptn_field(data, fieldInt));

  UErrorCode error = U_ZERO_ERROR;
  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static void HHVM_METHOD(IntlDatePatternGenerator,
                        setDateTimeFormat,
                        const String& dateTimeFormat) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString fmt(u16(dateTimeFormat, error));
  ICU_ERR_CHECK(data, error);

  data->generator().setDateTimeFormat(fmt);
  ICU_ERR_CHECK_MSG(data, error, "Error setting date time format");
}

static String HHVM_METHOD(IntlDatePatternGenerator, getDateTimeFormat) {
  GENERATOR_GET(data, this_);

  icu::UnicodeString ret = data->generator().getDateTimeFormat();

  UErrorCode error = U_ZERO_ERROR;
  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          getBestPattern,
                          const String& skeleton) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString skel(u16(skeleton, error));
  ICU_ERR_CHECK(data, error);

  icu::UnicodeString ret = data->generator().getBestPattern(skel, error);
  ICU_ERR_CHECK_MSG(data, error, "Error getting best pattern");

  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          replaceFieldTypes,
                          const String& pattern,
                          const String& skeleton) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString ptn(u16(pattern, error));
  ICU_ERR_CHECK(data, error);

  icu::UnicodeString skel(u16(skeleton, error));
  ICU_ERR_CHECK(data, error);

  icu::UnicodeString ret;
  ret = data->generator().replaceFieldTypes(ptn, skel, error);
  ICU_ERR_CHECK_MSG(data, error, "Error replacing field types");

  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static Object HHVM_METHOD(IntlDatePatternGenerator, getSkeletons) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::StringEnumeration *se = data->generator().getSkeletons(error);
  ICU_ERR_CHECK_MSG(data, error, "Error getting skeletons");

  return IntlIterator::newInstance(se);
}

static String HHVM_METHOD(IntlDatePatternGenerator,
                          getPatternForSkeleton,
                          const String& skeleton) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString skel(u16(skeleton, error));
  ICU_ERR_CHECK(data, error);

  icu::UnicodeString ret = data->generator().getPatternForSkeleton(skel);

  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static Object HHVM_METHOD(IntlDatePatternGenerator, getBaseSkeletons) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::StringEnumeration *se = data->generator().getBaseSkeletons(error);
  ICU_ERR_CHECK_MSG(data, error, "Error getting base skeletons");

  return IntlIterator::newInstance(se);
}

static void HHVM_METHOD(IntlDatePatternGenerator,
                        setDecimal,
                        const String& decimal) {
  GENERATOR_GET(data, this_);

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString dec(u16(decimal, error));
  ICU_ERR_CHECK(data, error);

  data->generator().setDecimal(dec);
}

static String HHVM_METHOD(IntlDatePatternGenerator, getDecimal) {
  GENERATOR_GET(data, this_);

  icu::UnicodeString ret = data->generator().getDecimal();

  UErrorCode error = U_ZERO_ERROR;
  String out(u8(ret, error));
  ICU_ERR_CHECK(data, error);

  return out;
}

static int64_t HHVM_METHOD(IntlDatePatternGenerator, getErrorCode) {
  GENERATOR_GET(data, this_);
  return data->getErrorCode();
}

static String HHVM_METHOD(IntlDatePatternGenerator, getErrorMessage) {
  GENERATOR_GET(data, this_);
  return data->getErrorMessage();
}

//////////////////////////////////////////////////////////////////////////////

#define UDATPG_CONST_FIELD(nm) \
HHVM_RCC_INT(IntlDatePatternGenerator, nm##_PATTERN_FIELD, UDATPG_##nm##_FIELD);

#define UDATPG_CONST(nm) \
  HHVM_RCC_INT(IntlDatePatternGenerator, PATTERN_##nm, UDATPG_##nm);

void IntlExtension::initDatePatternGenerator() {
  // UDateTimePatternField
  UDATPG_CONST_FIELD(ERA);
  UDATPG_CONST_FIELD(YEAR);
  UDATPG_CONST_FIELD(QUARTER);
  UDATPG_CONST_FIELD(MONTH);
  UDATPG_CONST_FIELD(WEEK_OF_YEAR);
  UDATPG_CONST_FIELD(WEEK_OF_MONTH);
  UDATPG_CONST_FIELD(WEEKDAY);
  UDATPG_CONST_FIELD(DAY_OF_YEAR);
  UDATPG_CONST_FIELD(DAY_OF_WEEK_IN_MONTH);
  UDATPG_CONST_FIELD(DAY);
  UDATPG_CONST_FIELD(DAYPERIOD);
  UDATPG_CONST_FIELD(HOUR);
  UDATPG_CONST_FIELD(MINUTE);
  UDATPG_CONST_FIELD(SECOND);
  UDATPG_CONST_FIELD(FRACTIONAL_SECOND);
  UDATPG_CONST_FIELD(ZONE);
  UDATPG_CONST(FIELD_COUNT);

  // UDateTimePatternConflict
  UDATPG_CONST(NO_CONFLICT);
  UDATPG_CONST(BASE_CONFLICT);
  UDATPG_CONST(CONFLICT);
  UDATPG_CONST(CONFLICT_COUNT);

  HHVM_STATIC_ME(IntlDatePatternGenerator, createInstance);
  HHVM_STATIC_ME(IntlDatePatternGenerator, createEmptyInstance);
  HHVM_ME(IntlDatePatternGenerator, getSkeleton);
  HHVM_ME(IntlDatePatternGenerator, getBaseSkeleton);
  HHVM_ME(IntlDatePatternGenerator, addPattern);
  HHVM_ME(IntlDatePatternGenerator, setAppendItemFormat);
  HHVM_ME(IntlDatePatternGenerator, getAppendItemFormat);
  HHVM_ME(IntlDatePatternGenerator, setAppendItemName);
  HHVM_ME(IntlDatePatternGenerator, getAppendItemName);
  HHVM_ME(IntlDatePatternGenerator, setDateTimeFormat);
  HHVM_ME(IntlDatePatternGenerator, getDateTimeFormat);
  HHVM_ME(IntlDatePatternGenerator, getBestPattern);
  HHVM_ME(IntlDatePatternGenerator, replaceFieldTypes);
  HHVM_ME(IntlDatePatternGenerator, getSkeletons);
  HHVM_ME(IntlDatePatternGenerator, getPatternForSkeleton);
  HHVM_ME(IntlDatePatternGenerator, getBaseSkeletons);
  HHVM_ME(IntlDatePatternGenerator, setDecimal);
  HHVM_ME(IntlDatePatternGenerator, getDecimal);
  HHVM_ME(IntlDatePatternGenerator, getErrorCode);
  HHVM_ME(IntlDatePatternGenerator, getErrorMessage);

  Native::registerNativeDataInfo<IntlDatePatternGenerator>();
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
