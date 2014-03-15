#include "hphp/runtime/ext/icu/ext_icu_break_iterator.h"

namespace HPHP { namespace Intl {

const StaticString
  s_IntlBreakIterator("IntlBreakIterator"),
  s_IntlCodePointBreakIterator("IntlCodePointBreakIterator"),
  s_DONE("DONE");

Class* IntlBreakIterator::c_IntlBreakIterator = nullptr;
Class* IntlBreakIterator::c_IntlCodePointBreakIterator = nullptr;

//////////////////////////////////////////////////////////////////////////////
// class IntlBreakIterator

#define FETCH_BI(data, this_, def) \
  auto data = IntlBreakIterator::Get(this_); \
  if (!data) { \
    return def; \
  }

inline Object ibi_create(const char *funcname,
                         icu::BreakIterator *(*func)(const icu::Locale&,
                                                     UErrorCode&),
                         const String& locale) {
  UErrorCode error = U_ZERO_ERROR;
  auto bi = func(Locale::createFromName(locale.c_str()), error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "%s: error creating BreakIterator", funcname);
    return null_object;
  }

  return IntlBreakIterator::newInstance(bi);
}

/////////////////////////////////////////////////////////////////////////////

static Object HHVM_STATIC_METHOD(IntlBreakIterator, createCharacterInstance,
                                 const Variant& locale) {
  return ibi_create("breakiter_create_character_instance",
                    &BreakIterator::createCharacterInstance,
                    localeOrDefault(locale.toString()));
}

static Object HHVM_STATIC_METHOD(IntlBreakIterator, createCodePointInstance) {
  return IntlBreakIterator::newCodePointInstance(new CodePointBreakIterator());
}

static Object HHVM_STATIC_METHOD(IntlBreakIterator, createLineInstance,
                                 const Variant& locale) {
  return ibi_create("breakiter_create_line_instance",
                    &BreakIterator::createLineInstance,
                    localeOrDefault(locale.toString()));
}

static Object HHVM_STATIC_METHOD(IntlBreakIterator, createSentenceInstance,
                                 const Variant& locale) {
  return ibi_create("breakiter_create_sentence_instance",
                    &BreakIterator::createSentenceInstance,
                    localeOrDefault(locale.toString()));
}

static Object HHVM_STATIC_METHOD(IntlBreakIterator, createTitleInstance,
                                 const Variant& locale) {
  return ibi_create("breakiter_create_title_instance",
                    &BreakIterator::createTitleInstance,
                    localeOrDefault(locale.toString()));
}

static Object HHVM_STATIC_METHOD(IntlBreakIterator, createWordInstance,
                                 const Variant& locale) {
  return ibi_create("breakiter_create_word_instance",
                    &BreakIterator::createWordInstance,
                    localeOrDefault(locale.toString()));
}

static Variant HHVM_METHOD(IntlBreakIterator, current) {
  FETCH_BI(data, this_, false);
  int64_t ret = data->breakIterator()->current();
  data->setStarted(true);
  return ret;
}

static Variant HHVM_METHOD(IntlBreakIterator, first) {
  FETCH_BI(data, this_, false);
  int64_t ret = data->breakIterator()->first();
  data->setStarted(ret != UBRK_DONE);
  data->m_key = 0;
  return ret;
}

static Variant HHVM_METHOD(IntlBreakIterator, following, int64_t off) {
  FETCH_BI(data, this_, false);
  if ((off < INT32_MIN) || (off > INT32_MAX)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "breakiter_following: offset argument is outside bounds of"
                   " a 32-bit wide integer");
    return false;
  }
  int64_t ret = data->breakIterator()->following(off);
  data->setStarted(true);
  data->m_key = -1;
  return ret;
}

static int64_t HHVM_METHOD(IntlBreakIterator, getErrorCode) {
  FETCH_BI(data, this_, 0);
  return data->getErrorCode();
}

static String HHVM_METHOD(IntlBreakIterator, getErrorMessage) {
  FETCH_BI(data, this_, empty_string);
  return data->getErrorMessage();
}

static Variant HHVM_METHOD(IntlBreakIterator, getLocale, int64_t locale_type) {
  FETCH_BI(data, this_, false);
  if ((locale_type != ULOC_ACTUAL_LOCALE) &&
      (locale_type != ULOC_VALID_LOCALE)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "breakiter_get_locale: invalid locale type");
    return false;
  }
  UErrorCode error = U_ZERO_ERROR;
  auto locale = data->breakIterator()->
    getLocale((ULocDataLocaleType)locale_type, error);
  if (U_FAILURE(error)) {
    data->setError(error,
                   "breakiter_get_locale: Call to ICU method has failed");
    return false;
  }
  return String(locale.getName(), CopyString);
}

static Object HHVM_METHOD(IntlBreakIterator, getPartsIterator,
                          const String& key_type) {
  throw NotImplementedException("IntlBreakIterator::getPartsIterator");
}

static Variant HHVM_METHOD(IntlBreakIterator, getText) {
  FETCH_BI(data, this_, false);
  auto text = data->text();
  if (text.empty()) {
    return uninit_null();
  }
  return text;
}

static bool HHVM_METHOD(IntlBreakIterator, isBoundary, int64_t off) {
  FETCH_BI(data, this_, false);
  if ((off < INT32_MIN) || (off > INT32_MAX)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "breakiter_is_boundary: offset argument is outside bounds of"
                   " a 32-bit wide integer");
    return false;
  }
  return data->breakIterator()->isBoundary(off);
}

static Variant HHVM_METHOD(IntlBreakIterator, key) {
  FETCH_BI(data, this_, false);
  if (data->m_key >= 0) {
    return data->m_key;
  }
  return false;
}

static Variant HHVM_METHOD(IntlBreakIterator, last) {
  FETCH_BI(data, this_, false);
  int64_t ret = data->breakIterator()->last();
  data->setStarted(true);
  data->m_key = -1;
  return ret;
}

static Variant HHVM_METHOD(IntlBreakIterator, next, const Variant& offset) {
  FETCH_BI(data, this_, false);
  if (offset.isNull()) {
    int64_t ret = data->breakIterator()->next();
    if (data->started()) {
      if (data->m_key >= 0) {
        ++data->m_key;
      }
    } else {
      data->setStarted(true);
      data->m_key = 0;
    }
    if (ret == UBRK_DONE) {
      data->m_key = -1;
    }
    return ret;
  }
  auto off = offset.toInt64();
  if ((off < INT32_MIN) || (off > INT32_MAX)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "breakiter_next: offset argument is outside bounds of"
                   " a 32-bit wide integer");
    return false;
  }
  int64_t ret = data->breakIterator()->next(off);
  data->setStarted(true);
  if (ret == UBRK_DONE) {
    data->m_key = -1;
  }
  if (data->m_key >= 0) {
    data->m_key += off;
  }
  return ret;
}

static Variant HHVM_METHOD(IntlBreakIterator, preceding, int64_t off) {
  FETCH_BI(data, this_, false);
  if ((off < INT32_MIN) || (off > INT32_MAX)) {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "breakiter_preceding: offset argument is outside bounds of"
                   " a 32-bit wide integer");
    return false;
  }
  int64_t ret = data->breakIterator()->preceding(off);
  data->setStarted(ret != UBRK_DONE);
  data->m_key = -1;
  return ret;
}

static Variant HHVM_METHOD(IntlBreakIterator, previous) {
  FETCH_BI(data, this_, false);
  int64_t ret = data->breakIterator()->previous();
  data->setStarted(ret != UBRK_DONE);
  if (ret == UBRK_DONE) {
    data->m_key = -1;
  }
  if (data->m_key >= 0) {
    --data->m_key;
  }
  return ret;
}

static bool HHVM_METHOD(IntlBreakIterator, setText, const String& text) {
  FETCH_BI(data, this_, false);
  return data->setText(text);
}

//////////////////////////////////////////////////////////////////////////////

static int64_t HHVM_METHOD(IntlCodePointBreakIterator, getLastCodePoint) {
  FETCH_BI(data, this_, -1);
  return data->codePointBreakIterator()->getLastCodePoint();
}

//////////////////////////////////////////////////////////////////////////////

static void HHVM_METHOD(IntlRuleBasedBreakIterator, __construct,
                        const String& rules, bool compiled /*=false*/) {
  s_intl_error->clearError();
  auto data = Native::data<IntlBreakIterator>(this_.get());
  if (compiled) {
#if U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM >= 408
    UErrorCode error = U_ZERO_ERROR;
    auto bi = new RuleBasedBreakIterator((uint8_t*)rules.c_str(), rules.size(),
                                         error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "rbbi_create_instance: unable to "
                                    "create instance from compiled rules");
      return;
    }
    data->setBreakIterator(bi);
    return;
#else /* ICU < 4.8 */
    s_intl_error->setError(U_UNSUPPORTED_ERROR,
                           "rbbi_create_instance: "
                           "compiled rules require ICU >= 4.8");
    return;
#endif
  } else {
    UErrorCode error = U_ZERO_ERROR;
    auto rules16 = u16(rules, error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "rbbi_create_instance: rules were not "
                                    "a valid UTF-8 string");
      return;
    }
    error = U_ZERO_ERROR;
    UParseError parseError;
    auto bi = new RuleBasedBreakIterator(rules16, parseError, error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "rbbi_create_instance: unable to create "
                                    "RuleBasedBreakIterator from rules");
      return;
    }
    data->setBreakIterator(bi);
    return;
  }
}

static Variant HHVM_METHOD(IntlRuleBasedBreakIterator, getRules) {
  FETCH_BI(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  auto ret = u8(data->ruleBasedBreakIterator()->getRules(), error);
  if (U_FAILURE(error)) {
    data->setError(error, "rbbi_hash_code: Error converting result to "
                          "UTF-8 string");
    return false;
  }
  return ret;
}

static int64_t HHVM_METHOD(IntlRuleBasedBreakIterator, getRuleStatus) {
  FETCH_BI(data, this_, 0);
  return data->ruleBasedBreakIterator()->getRuleStatus();
}

static Variant HHVM_METHOD(IntlRuleBasedBreakIterator, getRuleStatusVec) {
  FETCH_BI(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  int32_t count =
    data->ruleBasedBreakIterator()->getRuleStatusVec(nullptr, 0, error);
  if (error != U_BUFFER_OVERFLOW_ERROR) {
    data->setError(error, "rbbi_get_rule_status_vec: failed "
                          " determining the number of status values");
    return false;
  }
  int32_t *rules = new int32_t[count];
  SCOPE_EXIT{ delete[] rules; };
  error = U_ZERO_ERROR;
  count = data->ruleBasedBreakIterator()->getRuleStatusVec(rules, count, error);
  if (U_FAILURE(error)) {
    data->setError(error, "rbbi_get_rule_status_vec: failed "
                          "obtaining the status values");
    return false;
  }
  Array ret = Array::Create();
  for (int32_t i = 0; i < count; ++i) {
    ret.append((int64_t)rules[i]);
  }
  return ret;
}

#if U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM >= 408
static Variant HHVM_METHOD(IntlRuleBasedBreakIterator, getBinaryRules) {
  FETCH_BI(data, this_, false);
  uint32_t rules_len;
  auto rules = data->ruleBasedBreakIterator()->getBinaryRules(rules_len);
  if (rules_len > (INT_MAX - 1)) {
    data->setError(U_BUFFER_OVERFLOW_ERROR,
                   "rbbi_get_binary_rules: the rules are too large");
    return false;
  }
  return String((const char*)rules, rules_len, CopyString);
}
#endif

//////////////////////////////////////////////////////////////////////////////

static Object HHVM_METHOD(IntlPartsIterator, getBreakIterator) {
  throw NotImplementedException("IntlPartsIterator::getBreakIterator");
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initBreakIterator() {
  HHVM_STATIC_ME(IntlBreakIterator, createCharacterInstance);
  HHVM_STATIC_ME(IntlBreakIterator, createCodePointInstance);
  HHVM_STATIC_ME(IntlBreakIterator, createLineInstance);
  HHVM_STATIC_ME(IntlBreakIterator, createSentenceInstance);
  HHVM_STATIC_ME(IntlBreakIterator, createTitleInstance);
  HHVM_STATIC_ME(IntlBreakIterator, createWordInstance);
  HHVM_ME(IntlBreakIterator, current);
  HHVM_ME(IntlBreakIterator, first);
  HHVM_ME(IntlBreakIterator, following);
  HHVM_ME(IntlBreakIterator, getErrorCode);
  HHVM_ME(IntlBreakIterator, getErrorMessage);
  HHVM_ME(IntlBreakIterator, getLocale);
  HHVM_ME(IntlBreakIterator, getPartsIterator);
  HHVM_ME(IntlBreakIterator, getText);
  HHVM_ME(IntlBreakIterator, isBoundary);
  HHVM_ME(IntlBreakIterator, key);
  HHVM_ME(IntlBreakIterator, last);
  HHVM_ME(IntlBreakIterator, next);
  HHVM_ME(IntlBreakIterator, preceding);
  HHVM_ME(IntlBreakIterator, previous);
  HHVM_ME(IntlBreakIterator, setText);

  HHVM_ME(IntlCodePointBreakIterator, getLastCodePoint);

  HHVM_ME(IntlRuleBasedBreakIterator, __construct);
  HHVM_ME(IntlRuleBasedBreakIterator, getRules);
  HHVM_ME(IntlRuleBasedBreakIterator, getRuleStatus);
  HHVM_ME(IntlRuleBasedBreakIterator, getRuleStatusVec);
#if U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM >= 408
  HHVM_ME(IntlRuleBasedBreakIterator, getBinaryRules);
#endif

  HHVM_ME(IntlPartsIterator, getBreakIterator);

  Native::registerClassConstant<KindOfInt64>(s_IntlBreakIterator.get(),
                                             s_DONE.get(),
                                             icu::BreakIterator::DONE);

#define BI_CONST(name) \
  Native::registerClassConstant<KindOfInt64>(s_IntlBreakIterator.get(), \
                                             makeStaticString(#name), \
                                             UBRK_##name);

  BI_CONST(WORD_NONE);
  BI_CONST(WORD_NONE_LIMIT);
  BI_CONST(WORD_NUMBER);
  BI_CONST(WORD_NUMBER_LIMIT);
  BI_CONST(WORD_LETTER);
  BI_CONST(WORD_LETTER_LIMIT);
  BI_CONST(WORD_KANA);
  BI_CONST(WORD_KANA_LIMIT);
  BI_CONST(WORD_IDEO);
  BI_CONST(WORD_IDEO_LIMIT);

  BI_CONST(LINE_SOFT);
  BI_CONST(LINE_SOFT_LIMIT);
  BI_CONST(LINE_HARD);
  BI_CONST(LINE_HARD_LIMIT);

  BI_CONST(SENTENCE_TERM);
  BI_CONST(SENTENCE_TERM_LIMIT);
  BI_CONST(SENTENCE_SEP);
  BI_CONST(SENTENCE_SEP_LIMIT);

#undef BI_CONST

  Native::registerNativeDataInfo<IntlBreakIterator>(s_IntlBreakIterator.get());

  loadSystemlib("icu_break_iterator");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
