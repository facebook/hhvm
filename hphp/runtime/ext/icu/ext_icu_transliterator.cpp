#include "hphp/runtime/ext/icu/ext_icu_transliterator.h"

namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////

Class* Transliterator::c_Transliterator = nullptr;

#define FETCH_TRANS(data, obj) \
  auto data = Transliterator::Get(obj); \
  if (!data) { \
    throw s_intl_error->getException("Uninitialized Message Formatter"); \
  }


const StaticString s_Transliterator("Transliterator");
const icu::UnicodeString s_RulesTransPHP("RulesTransPHP",
                                         sizeof("RulesTransPHP") - 1, US_INV);

static bool HHVM_METHOD(Transliterator, __init,
                        const String&  idOrRules,
                        int64_t direction, bool rules) {
  auto data = Native::data<Transliterator>(this_.get());
  if ((direction != UTRANS_FORWARD) && (direction != UTRANS_REVERSE)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "transliterator_create: invalid direction");
    return false;
  }
  auto dir = (UTransDirection)direction;
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString idOrRules16(u16(idOrRules, error));
  if (U_FAILURE(error)) {
    data->setError(error,
                   "Unable to convert id to UTF-16");
    return false;
  }
  UParseError pe;
  auto trans = rules
    ? icu::Transliterator::createFromRules(s_RulesTransPHP, idOrRules16,
                                           dir, pe, error)
    : icu::Transliterator::createInstance(idOrRules16, dir, error);

  if (U_FAILURE(error)) {
    if (rules) {
      data->setError(error, "transliterator_create_from_rules: unable to "
                            "create ICU transliterator from rules");
    } else {
      data->setError(error, "transliterator_create: unable to open ICU "
                            "transliterator with id \"%s\"", idOrRules.c_str());
    }
    return false;
  }

  data->clearError();
  data->setTransliterator(trans);
  return true;
}

static Variant HHVM_METHOD(Transliterator, __createInverse) {
  FETCH_TRANS(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  auto trans = data->trans()->createInverse(error);
  if (U_FAILURE(error)) {
    data->setError(error, "transliterator_create_inverse: could not create ");
    return uninit_null;
  }
  data->clearError();
  return Transliterator::newInstance(trans);
}

static int64_t HHVM_METHOD(Transliterator, getErrorCode) {
  FETCH_TRANS(data, this_);
  return data->getErrorCode();
}

static String HHVM_METHOD(Transliterator, getErrorMessage) {
  FETCH_TRANS(data, this_);
  return data->getErrorMessage();
}

static Variant HHVM_METHOD(Transliterator, getId) {
  FETCH_TRANS(data, this_);
  auto id16 = data->trans()->getID();
  UErrorCode error = U_ZERO_ERROR;
  String ret(u8(id16, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Unable to convert ID to UTF-8");
    return false;
  }
  data->clearError();
  return ret;
}

static Variant HHVM_STATIC_METHOD(Transliterator, listIDs) {
  UErrorCode error = U_ZERO_ERROR;
  auto se = icu::Transliterator::getAvailableIDs(error);
  SCOPE_EXIT{ if (se) delete se; };
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "transliterator_list_ids: Failed to obtain "
                                  "registered transliterators");
    return false;
  }
  error = U_ZERO_ERROR;
  Array ret = Array::Create();
  for(;;) {
    int32_t len;
    auto str = se->next(&len, error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "transliterator_list_ids: Failed to build "
                                    "array of registered transliterators");
      return false;
    }
    if (!str) break;
    ret.append(String(str, len, CopyString));
  }
  s_intl_error->clearError();
  return ret;
}

static Variant HHVM_METHOD(Transliterator, transliterate,
                           const String& str, int64_t begin, int64_t end) {
  FETCH_TRANS(data, this_);
  if (end < -1) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "transliterator_transliterate: \"end\" argument should be "
                   "either non-negative or -1");
    return false;
  }
  if ((begin < 0) || ((end != -1) && (begin > end))) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "transliterator_transliterate: \"start\" argument should be "
                   "non-negative and not bigger than \"end\" (if defined)");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString str16(u16(str, error));
  if (U_FAILURE(error)) {
    data->setError(error, "String conversion of string to UTF-16 failed");
    return false;
  }
  if ((begin > str16.length()) || ((end != -1) && (end > str16.length()))) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "transliterator_transliterate: Neither \"start\" nor the "
                   "\"end\" arguments can exceed the number of UTF-16 code "
                   "units (in this case, %d)", (int)str16.length());
    return false;
  }
  data->trans()->transliterate(str16, begin, (end < 0) ? str16.length() : end);

  error = U_ZERO_ERROR;
  String ret(u8(str16, error));
  if (U_FAILURE(error)) {
    data->setError(error, "String conversion of string to UTF-8 failed");
    return false;
  }
  data->clearError();
  return ret;
}

//////////////////////////////////////////////////////////////////////////////

const StaticString
  s_FORWARD("FORWARD"),
  s_REVERSE("REVERSE");

void IntlExtension::initTransliterator() {
  HHVM_ME(Transliterator, __init);
  HHVM_ME(Transliterator, __createInverse);
  HHVM_ME(Transliterator, getErrorCode);
  HHVM_ME(Transliterator, getErrorMessage);
  HHVM_ME(Transliterator, getId);
  HHVM_STATIC_ME(Transliterator, listIDs);
  HHVM_ME(Transliterator, transliterate);

  Native::registerClassConstant<KindOfInt64>
    (s_Transliterator.get(), s_FORWARD.get(), UTRANS_FORWARD);
  Native::registerClassConstant<KindOfInt64>
    (s_Transliterator.get(), s_REVERSE.get(), UTRANS_REVERSE);

  Native::registerNativeDataInfo<Transliterator>(s_Transliterator.get());

  loadSystemlib("icu_transliterator");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
