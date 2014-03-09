#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/unorm.h>
#include <unicode/utypes.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
// class Normalizer

const StaticString s_Normalizer("Normalizer");

static Variant HHVM_STATIC_METHOD(Normalizer, isNormalized,
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
      return uninit_null();
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uinput(u16(input, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error converting string to UTF-16.");
    return false;
  }

  error = U_ZERO_ERROR;
  UBool ret = unorm_isNormalizedWithOptions(uinput.getBuffer(), uinput.length(),
                                            (UNormalizationMode)form,
                                            0, &error);

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
      return uninit_null();
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString uinput(u16(input, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error converting string to UTF-16.");
    return uninit_null();
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
    return uninit_null();
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
      return uninit_null();
    }
  }
  dest.releaseBuffer(size_needed);

  error = U_ZERO_ERROR;
  String ret(u8(dest, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "normalizer_normalize: "
                                  "error converting normalized text UTF-8");
    return uninit_null();
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////

#define CONST_NORM(v) \
  Native::registerClassConstant<KindOfInt64> \
    (s_Normalizer.get(), makeStaticString("FORM_" #v), UNORM_NF ## v); \
  Native::registerClassConstant<KindOfInt64> \
    (s_Normalizer.get(), makeStaticString("NF" #v), UNORM_NF ## v)

const StaticString s_NONE("NONE");

void IntlExtension::initNormalizer() {
  HHVM_STATIC_ME(Normalizer, isNormalized);
  HHVM_STATIC_ME(Normalizer, normalize);

  Native::registerClassConstant<KindOfInt64>
    (s_Normalizer.get(), s_NONE.get(), UNORM_NONE);

  CONST_NORM(D);
  CONST_NORM(KD);
  CONST_NORM(C);
  CONST_NORM(KC);

  loadSystemlib("icu_normalizer");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
