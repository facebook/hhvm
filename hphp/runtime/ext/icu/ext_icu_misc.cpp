#include "hphp/runtime/ext/icu/icu.h"

#include "hphp/runtime/base/array-init.h"

#include <unicode/uidna.h>
#include <unicode/parseerr.h>

namespace HPHP { namespace Intl {

/////////////////////////////////////////////////////////////////////////////
// Global error code/message

static int64_t HHVM_FUNCTION(intl_get_error_code) {
  return s_intl_error->getErrorCode();
}

static String HHVM_FUNCTION(intl_get_error_message) {
  return s_intl_error->getErrorMessage();
}

static String HHVM_FUNCTION(intl_error_name, int64_t errorCode) {
  return String(u_errorName((UErrorCode)errorCode), CopyString);
}

static bool HHVM_FUNCTION(intl_is_failure, int64_t errorCode) {
  return U_FAILURE((UErrorCode)errorCode);
}

/////////////////////////////////////////////////////////////////////////////
// IDNA functions

IMPLEMENT_DEFAULT_EXTENSION_VERSION(idn, NO_EXTENSION_VERSION_YET);

enum IdnVariant {
  INTL_IDNA_VARIANT_2003 = 0,
  INTL_IDNA_VARIANT_UTS46
};

static Variant doIdnTranslate2003(const String& domain, int64_t options,
                                  bool toUtf8) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uDomain(u16(domain, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error);
    return false;
  }
  icu::UnicodeString ret;
  UChar *retBuffer = ret.getBuffer(64);
  int32_t len = 0;
  for(;;) {
    UParseError parseError;
    error = U_ZERO_ERROR;
    if (toUtf8) {
      len = uidna_IDNToUnicode(uDomain.getBuffer(), uDomain.length(),
                               retBuffer, ret.getCapacity(),
                               options, &parseError, &error);
    } else {
      len = uidna_IDNToASCII(uDomain.getBuffer(), uDomain.length(),
                             retBuffer, ret.getCapacity(),
                             options, &parseError, &error);
    }
    if (error != U_BUFFER_OVERFLOW_ERROR) break;
    if (len < ret.getCapacity()) {
      // Bufferoverflow which didn't overflow the buffer???
      error = U_INTERNAL_PROGRAM_ERROR;
      break;
    }
    ret.releaseBuffer(0);
    retBuffer = ret.getBuffer(len);
  }
  ret.releaseBuffer(len);

  if (U_FAILURE(error)) {
    s_intl_error->setError(error);
    return false;
  }

  error = U_ZERO_ERROR;
  String out(u8(ret, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error converting result from Unicode");
    return false;
  }
  return out;
}

#ifdef UIDNA_INFO_INITIALIZER // Indicates ICU >= 4.6
const StaticString
  s_result("result"),
  s_isTransitionalDifferent("isTransitionalDifferent"),
  s_errors("errors");
#endif

static Variant doIdnTranslateUTS46(const String& domain, int64_t options,
                                   VRefParam retInfo, bool toUtf8) {
#ifdef UIDNA_INFO_INITIALIZER
  UErrorCode error = U_ZERO_ERROR;
  UIDNAInfo   info = UIDNA_INFO_INITIALIZER;
  auto idna = uidna_openUTS46(options, &error);
  SCOPE_EXIT{ uidna_close(idna); };
  String result(255, ReserveString); // 255 == max length possible
  int32_t len;
  auto capacity = result.capacity() + 1;
  if (toUtf8) {
    len = uidna_nameToUnicodeUTF8(idna, domain.c_str(), domain.size(),
                                  result.mutableData(), capacity,
                                  &info, &error);
  } else {
    len = uidna_nameToASCII_UTF8(idna, domain.c_str(), domain.size(),
                                 result.mutableData(), capacity,
                                 &info, &error);
  }
  if (len > capacity) {
    s_intl_error->setError(U_INTERNAL_PROGRAM_ERROR);
    return false;
  }
  if (U_FAILURE(error)) {
    s_intl_error->setError(error);
    return false;
  }
  if (info.errors) {
    return false;
  }
  result.setSize(len);

  ArrayInit arr(3, ArrayInit::Map{});
  arr.set(s_result, result);
  arr.set(s_isTransitionalDifferent, info.isTransitionalDifferent);
  arr.set(s_errors, (long)info.errors);
  retInfo.assignIfRef(arr.toVariant());
  return result;

#else
  s_intl_error->setError(U_UNSUPPORTED_ERROR);
  return false;
#endif
}

inline Variant doIdnTranslate(const String& domain, int64_t options,
                              IdnVariant variant, VRefParam info,
                              bool toUtf8) {
  switch (variant) {
    case INTL_IDNA_VARIANT_2003:
      return doIdnTranslate2003(domain, options, toUtf8);
    case INTL_IDNA_VARIANT_UTS46:
      return doIdnTranslateUTS46(domain, options, info, toUtf8);
  }
  return false;
}

static Variant HHVM_FUNCTION(idn_to_ascii, const String& domain,
                                           int64_t options /*= 0 */,
                                           int64_t variant /*= *_2003 */,
                                           VRefParam info /*= null */) {
  return doIdnTranslate(domain, options, (IdnVariant)variant, info, false);
}

static Variant HHVM_FUNCTION(idn_to_utf8, const String& domain,
                                          int64_t options /*= 0 */,
                                          int64_t variant /*= *_2003 */,
                                          VRefParam info /*= null */) {
  return doIdnTranslate(domain, options, (IdnVariant)variant, info, true);
}

/////////////////////////////////////////////////////////////////////////////

void IntlExtension::initMisc() {
  HHVM_FE(intl_get_error_code);
  HHVM_FE(intl_get_error_message);
  HHVM_FE(intl_error_name);
  HHVM_FE(intl_is_failure);

  HHVM_FE(idn_to_ascii);
  HHVM_FE(idn_to_utf8);

  HHVM_RC_INT_SAME(INTL_IDNA_VARIANT_2003);
  HHVM_RC_INT_SAME(INTL_IDNA_VARIANT_UTS46);

#define UHHVM_RC_INT_SAME(cns) HHVM_RC_INT(cns, U ## cns)
  UHHVM_RC_INT_SAME(IDNA_DEFAULT);
  UHHVM_RC_INT_SAME(IDNA_ALLOW_UNASSIGNED);
  UHHVM_RC_INT_SAME(IDNA_USE_STD3_RULES);
#ifdef UIDNA_INFO_INITIALIZER /* ICU 46 */
  UHHVM_RC_INT_SAME(IDNA_CHECK_BIDI);
  UHHVM_RC_INT_SAME(IDNA_CHECK_CONTEXTJ);
  UHHVM_RC_INT_SAME(IDNA_NONTRANSITIONAL_TO_ASCII);
  UHHVM_RC_INT_SAME(IDNA_NONTRANSITIONAL_TO_UNICODE);

  UHHVM_RC_INT_SAME(IDNA_ERROR_EMPTY_LABEL);
  UHHVM_RC_INT_SAME(IDNA_ERROR_LABEL_TOO_LONG);
  UHHVM_RC_INT_SAME(IDNA_ERROR_DOMAIN_NAME_TOO_LONG);
  UHHVM_RC_INT_SAME(IDNA_ERROR_LEADING_HYPHEN);
  UHHVM_RC_INT_SAME(IDNA_ERROR_TRAILING_HYPHEN);
  UHHVM_RC_INT_SAME(IDNA_ERROR_HYPHEN_3_4);
  UHHVM_RC_INT_SAME(IDNA_ERROR_LEADING_COMBINING_MARK);
  UHHVM_RC_INT_SAME(IDNA_ERROR_DISALLOWED);
  UHHVM_RC_INT_SAME(IDNA_ERROR_PUNYCODE);
  UHHVM_RC_INT_SAME(IDNA_ERROR_LABEL_HAS_DOT);
  UHHVM_RC_INT_SAME(IDNA_ERROR_INVALID_ACE_LABEL);
  UHHVM_RC_INT_SAME(IDNA_ERROR_BIDI);
  UHHVM_RC_INT_SAME(IDNA_ERROR_CONTEXTJ);
#endif
#ifdef UIDNA_CHECK_CONTEXTO /* ICU 49 */
  UHHVM_RC_INT_SAME(IDNA_CHECK_CONTEXTO);
  UHHVM_RC_INT_SAME(IDNA_ERROR_CONTEXTO_PUNCTUATION);
  UHHVM_RC_INT_SAME(IDNA_ERROR_CONTEXTO_DIGITS);
#endif
#undef UHHVM_RC_INT_SAME

  /* Constants not found in ICU library */
  HHVM_RC_INT(IDNA_CONTAINS_ACE_PREFIX, 8);
  HHVM_RC_INT(IDNA_CONTAINS_MINUS, 4);
  HHVM_RC_INT(IDNA_CONTAINS_NON_LDH, 3);
  HHVM_RC_INT(IDNA_ICONV_ERROR, 9);
  HHVM_RC_INT(IDNA_INVALID_LENGTH, 5);
  HHVM_RC_INT(IDNA_MALLOC_ERROR, 201);
  HHVM_RC_INT(IDNA_NO_ACE_PREFIX, 6);
  HHVM_RC_INT(IDNA_PUNYCODE_ERROR, 2);
  HHVM_RC_INT(IDNA_ROUNDTRIP_VERIFY_ERROR, 7);
  HHVM_RC_INT(IDNA_STRINGPREP_ERROR, 1);

  loadSystemlib("icu_misc");
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
