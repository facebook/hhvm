#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/unorm.h>
#include <unicode/utypes.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////
// class Normalizer

const StaticString s_Normalizer("Normalizer");

static bool HHVM_STATIC_METHOD(Normalizer, isNormalized,
                                  const String& input, int64_t form) {
  s_intl_error->clearError();
  switch (form) {
    case UNORM_NFD:
    case UNORM_NFKD:
    case UNORM_NFC:
    case UNORM_NFKC:
      break;
    default:
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                             "normalizer_isnormalized: "
                             "illegal normalization form");
      return false;
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uinput(u16(input, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error converting string to UTF-16.");
    return false;
  }

  error = U_ZERO_ERROR;
  bool ret = (unorm_isNormalizedWithOptions(uinput.getBuffer(), uinput.length(),
                                            (UNormalizationMode)form,
                                            0, &error) == 1 ? true : false);

  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error testing if string is the given "
                                   "normalization form.");
    return false;
  }

  return ret;
}

static Variant HHVM_STATIC_METHOD(Normalizer, normalize,
                                  const String& input, int64_t form) {
  s_intl_error->clearError();

  int expansion_factor = 1;
  switch(form) {
    case UNORM_NONE:
    case UNORM_NFC:
    case UNORM_NFKC:
      break;
    case UNORM_NFD:
    case UNORM_NFKD:
      expansion_factor = 3;
      break;
    default:
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                             "normalizer_normalize: "
                             "illegal normalization form");
      return false;
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uinput(u16(input, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error converting string to UTF-16");
    return false;
  }

  icu::UnicodeString dest;
  int32_t capacity = uinput.length() * expansion_factor;
  error = U_ZERO_ERROR;
  int32_t size_needed = unorm_normalize(uinput.getBuffer(), uinput.length(),
                                        (UNormalizationMode)form, (int32_t) 0,
                                        dest.getBuffer(capacity), capacity,
                                        &error);

  if (U_FAILURE(error) &&
      (error != U_BUFFER_OVERFLOW_ERROR) &&
      (error != U_STRING_NOT_TERMINATED_WARNING)) {
    return false;
  }

  if (size_needed > capacity) {
    dest.releaseBuffer(0);
    error = U_ZERO_ERROR;
    size_needed = unorm_normalize(uinput.getBuffer(), uinput.length(),
                                  (UNormalizationMode)form, (int32_t) 0,
                                  dest.getBuffer(size_needed), size_needed,
                                  &error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "Error normalizing string");
      return false;
    }
  }
  dest.releaseBuffer(size_needed);

  error = U_ZERO_ERROR;
  String ret(u8(dest, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "normalizer_normalize: "
                                  "error converting normalized text UTF-8");
    return false;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::registerNativeNormalizer() {
  HHVM_STATIC_ME(Normalizer, isNormalized);
  HHVM_STATIC_ME(Normalizer, normalize);

  HHVM_RCC_INT(Normalizer, NONE, UNORM_NONE);
  HHVM_RCC_INT(Normalizer, FORM_D, UNORM_NFD);
  HHVM_RCC_INT(Normalizer, NFD, UNORM_NFD);
  HHVM_RCC_INT(Normalizer, FORM_KD, UNORM_NFKD);
  HHVM_RCC_INT(Normalizer, NFKD, UNORM_NFKD);
  HHVM_RCC_INT(Normalizer, FORM_C, UNORM_NFC);
  HHVM_RCC_INT(Normalizer, NFC, UNORM_NFC);
  HHVM_RCC_INT(Normalizer, FORM_KC, UNORM_NFKC);
  HHVM_RCC_INT(Normalizer, NFKC, UNORM_NFKC);
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
