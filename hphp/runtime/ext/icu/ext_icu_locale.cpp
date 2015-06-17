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
#include "hphp/runtime/ext/icu/ext_icu_locale.h"
#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include <unicode/ures.h>
#include <unicode/uloc.h>
#include <algorithm>
#include <utility>
#include <vector>

namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////
// class Locale

#define ULOC_CHECK(err, ret) \
  if (U_FAILURE(err)) { \
    s_intl_error->setError(err); \
    return ret; \
  }

#define MAX_LOCALE_LEN 80

/*returns TRUE if a is an ID separator FALSE otherwise*/
#define isIDSeparator(a) (a == '_' || a == '-')
#define isKeywordSeparator(a) (a == '@' )
#define isEndOfTag(a) (a == '\0' )

#define isPrefixLetter(a) ((a=='x')||(a=='X')||(a=='i')||(a=='I'))

/*returns TRUE if one of the special prefixes is here (s=string)
 *   'x-' or 'i-' */
#define isIDPrefix(s) (isPrefixLetter(s[0])&&isIDSeparator(s[1]))
#define isKeywordPrefix(s) ( isKeywordSeparator(s[0]) )

/* Dot terminates it because of POSIX form  where dot precedes the codepage
 *  * except for variant */
#define isTerminator(a)  ((a==0)||(a=='.')||(a=='@'))

static std::vector<std::string> g_grandfathered = {
  "art-lojban", "i-klingon", "i-lux", "i-navajo", "no-bok", "no-nyn",
  "cel-gaulish", "en-GB-oed", "i-ami", "i-bnn", "i-default", "i-enochian",
  "i-mingo", "i-pwn", "i-tao", "i-tay", "i-tsu", "sgn-BE-fr", "sgn-BE-nl",
  "sgn-CH-de", "zh-cmn", "zh-cmn-Hans", "zh-cmn-Hant", "zh-gan", "zh-guoyu",
  "zh-hakka", "zh-min", "zh-min-nan", "zh-wuu", "zh-xiang", "zh-yue"
};

/* Preferred locale codes for the first N entries of g_grandfathered
 * Must be kept in sync with above.
 */
static std::vector<std::string> g_grandfathered_preferred = {
  "jbo", "tlh", "lb", "nv", "nb", "nn"
};

static int getGrandfatheredOffset(const String& locale) {
  auto it = std::find(g_grandfathered.begin(),
                      g_grandfathered.end(), locale.data());
  if (it == g_grandfathered.end()) return -1;
  return (it - g_grandfathered.begin());
}

static String getGrandfatheredPreferred(int ofs) {
  if ((ofs < 0) || (ofs >= g_grandfathered.size())) {
    return String();
  }
  if (ofs < g_grandfathered_preferred.size()) {
    return g_grandfathered_preferred[ofs];
  }
  return g_grandfathered[ofs];
}

enum LocaleTag {
  LOC_SCRIPT,
  LOC_LANG,
  LOC_REGION,
  LOC_VARIANT,
  LOC_CANONICALIZE,
  LOC_PRIVATE,
  LOC_DISPLAY,
  LOC_EXTLANG,
};

const StaticString
  s_DEFAULT_LOCALE("DEFAULT_LOCALE"),
  s_LANG_TAG("LANG_TAG"),
  s_EXTLANG_TAG("EXTLANG_TAG"),
  s_SCRIPT_TAG("SCRIPT_TAG"),
  s_REGION_TAG("REGION_TAG"),
  s_VARIANT_TAG("VARIANT_TAG"),
  s_GRANDFATHERED_LANG_TAG("GRANDFATHERED_LANG_TAG"),
  s_PRIVATE_TAG("PRIVATE_TAG"),
  s_LOC_SCRIPT("script"),
  s_LOC_LANG("language"),
  s_LOC_REGION("region"),
  s_LOC_VARIANT("variant"),
  s_LOC_CANONICALIZE("canonicalize"),
  s_LOC_PRIVATE("private"),
  s_LOC_DISPLAY("display"),
  s_LOC_EXTLANG("extlang"),
  s_GRANDFATHERED("grandfathered"),
  s_SEPARATOR("_"),
  s_SEPARATOR_x("_x");

static const StaticString LocaleName(LocaleTag tag) {
  switch (tag) {
    case LOC_SCRIPT:       return s_LOC_SCRIPT;
    case LOC_LANG:         return s_LOC_LANG;
    case LOC_REGION:       return s_LOC_REGION;
    case LOC_VARIANT:      return s_LOC_VARIANT;
    case LOC_CANONICALIZE: return s_LOC_CANONICALIZE;
    case LOC_PRIVATE:      return s_LOC_PRIVATE;
    case LOC_DISPLAY:      return s_LOC_DISPLAY;
    case LOC_EXTLANG:      return s_LOC_EXTLANG;
  }
  not_reached();
}

static int singleton_pos(const String& str) {
  auto len = str.size();
  for (int i = 0; i < (len - 2); ++i) {
    if (!isIDSeparator(str[i])) continue;
    if (i == 1) return 0;
    if (isIDSeparator(str[i+2])) return i+1;
  }
  return -1;
}


static Variant get_icu_value(const String &locale, LocaleTag tag,
                             bool fromParseLocale = false) {
  String locale_name(locale);
  if (tag != LOC_CANONICALIZE) {
    if (getGrandfatheredOffset(locale) >= 0) {
      if (tag == LOC_LANG) {
        return locale;
      }
      return false;
    }
    if (fromParseLocale) {
      auto localecstr = locale.c_str();
      if (tag == LOC_LANG && locale.size() > 1 && isIDPrefix(localecstr)) {
        return locale;
      }
      int pos = singleton_pos(locale);
      if (pos == 0) {
        return init_null();
      } else if (pos > 0) {
        locale_name = locale.substr(0, pos - 1);
      }
    }
  }

  int32_t (*ulocfunc)(const char *loc, char *val, int32_t len, UErrorCode *err);
  switch (tag) {
    case LOC_SCRIPT:       ulocfunc = uloc_getScript;    break;
    case LOC_LANG:         ulocfunc = uloc_getLanguage;  break;
    case LOC_REGION:       ulocfunc = uloc_getCountry;   break;
    case LOC_VARIANT:      ulocfunc = uloc_getVariant;   break;
    case LOC_CANONICALIZE: ulocfunc = uloc_canonicalize; break;
    default:
      assert(false);
      return false;
  }

  String buf(64, ReserveString);
  do {
    UErrorCode error = U_ZERO_ERROR;
    int32_t len = ulocfunc(locale_name.c_str(),
                           buf.get()->mutableData(), buf.capacity() + 1,
                           &error);
    if (error != U_BUFFER_OVERFLOW_ERROR &&
        error != U_STRING_NOT_TERMINATED_WARNING) {
      if (U_FAILURE(error)) {
        s_intl_error->setError(error, "unable to get locale info");
        return false;
      }
      buf.setSize(len);
      return buf;
    }
    if (len <= buf.capacity() + 1) {
      // Avoid infinite loop
      s_intl_error->setError(U_INTERNAL_PROGRAM_ERROR,
                             "Got invalid response from ICU");
      return false;
    }
    buf = String(len, ReserveString);
  } while (true);

  not_reached();
  return false;
}

static Variant get_icu_display_value(const String& locale,
                                     const String& disp_locale,
                                     LocaleTag tag) {
  String locname(locale);
  if (tag != LOC_DISPLAY) {
    int ofs = getGrandfatheredOffset(locale);
    if (ofs >= 0) {
      if (tag == LOC_LANG) {
        locname = getGrandfatheredPreferred(ofs);
      } else {
        return false;
      }
    }
  }

  // Hack around buffer overflow in libicu. ures_getByKeyWithFallback is a
  // silly function.
  if (locname.size() >= 255 || disp_locale.size() >= 255) return false;

  int32_t (*ulocfunc)(const char *loc, const char *dloc,
                      UChar *dest, int32_t destcap, UErrorCode *err);
  switch (tag) {
    case LOC_LANG:     ulocfunc = uloc_getDisplayLanguage; break;
    case LOC_SCRIPT:   ulocfunc = uloc_getDisplayScript;   break;
    case LOC_REGION:   ulocfunc = uloc_getDisplayCountry;  break;
    case LOC_VARIANT:  ulocfunc = uloc_getDisplayVariant;  break;
    case LOC_DISPLAY:  ulocfunc = uloc_getDisplayName;     break;
    default:
      assert(false);
      return false;
  }

  icu::UnicodeString buf;
  auto ubuf = buf.getBuffer(64);
  do {
    UErrorCode error = U_ZERO_ERROR;
    int32_t len = ulocfunc(locname.c_str(), disp_locale.c_str(),
                           ubuf, buf.getCapacity(), &error);
    if (error != U_BUFFER_OVERFLOW_ERROR &&
        error != U_STRING_NOT_TERMINATED_WARNING) {
      if (U_FAILURE(error)) {
        s_intl_error->setError(error, "locale_get_display_%s : unable to "
                                      "get locale %s",
                                      LocaleName(tag).c_str(),
                                      LocaleName(tag).c_str());
        return false;
      }
      buf.releaseBuffer(len);

      error = U_ZERO_ERROR;
      String out(u8(buf, error));
      if (U_FAILURE(error)) {
        s_intl_error->setError(error, "Unable to convert result from "
                                      "locale_get_display_%s to UTF-8",
                                      LocaleName(tag).c_str());
        return false;
      }
      return out;
    }
    if (len <= buf.getCapacity()) {
      // Avoid infinite loop
      buf.releaseBuffer(0);
      s_intl_error->setError(U_INTERNAL_PROGRAM_ERROR,
                             "Got invalid response from ICU");
      return false;
    }

    // Grow the buffer to sufficient size
    buf.releaseBuffer(0);
    ubuf = buf.getBuffer(len);
  } while (true);

  not_reached();
  return false;
}

static Variant HHVM_STATIC_METHOD(Locale, acceptFromHttp,
                                  const String& header) {
  UErrorCode error = U_ZERO_ERROR;
  UEnumeration *avail = ures_openAvailableLocales(nullptr, &error);
  ULOC_CHECK(error, false);
  char out[MAX_LOCALE_LEN];
  UAcceptResult result;
  error = U_ZERO_ERROR;
  int len = uloc_acceptLanguageFromHTTP(out, sizeof(out), &result,
                                        header.c_str(), avail, &error);
  uenum_close(avail);
  ULOC_CHECK(error, false);
  if (len < 0 || result == ULOC_ACCEPT_FAILED) {
    return false;
  }
  return String(out, len, CopyString);
}

static Variant HHVM_STATIC_METHOD(Locale, canonicalize, const String& locale) {
  return get_icu_value(localeOrDefault(locale), LOC_CANONICALIZE);
}

inline void element_not_string() {
  s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                         "locale_compose: parameter array element is "
                         "not a string");
}

static bool append_key_value(String& ret,
                             const Array& subtags,
                             LocaleTag tag) {
  auto name = LocaleName(tag);
  if (!subtags.exists(name)) return true;
  auto val = subtags[name];
  if (!val.isString()) {
    element_not_string();
    return false;
  }
  ret += s_SEPARATOR + val.toString();
  return true;
}

static bool append_multiple_key_values(String& ret,
                                       const Array& subtags,
                                       LocaleTag tag) {
  auto name = LocaleName(tag);
  if (subtags.exists(name)) {
    // Sane version: [tag] => string, [tag] => array<tring>
    auto val = subtags[name];
    if (val.isString()) {
      if (tag == LOC_PRIVATE) {
        ret += s_SEPARATOR_x;
      }
      ret += s_SEPARATOR + val.toString();
      return true;
    }
    if (!val.isArray()) {
      return false;
    }
    bool first = true;
    for (ArrayIter it(val.toArray()); it; ++it) {
      auto v = it.second();
      if (!v.isString()) {
        element_not_string();
        return false;
      }
      if (first) {
        if (tag == LOC_PRIVATE) {
          ret += s_SEPARATOR + "x";
        }
        first = false;
      }
      ret += s_SEPARATOR + v.toString();
    }
    return true;
  }

  // clowny version [tag$n] => string
  // Only extlang, variant, and private
  if (tag != LOC_EXTLANG &&
      tag != LOC_VARIANT &&
      tag != LOC_PRIVATE) {
    return true;
  }

  int max = (tag == LOC_EXTLANG) ? 3 : 15;
  bool first = true;
  for (int i = 0; i < max; ++i) {
    auto namenum = name + String(i, CopyString);
    if (!subtags.exists(namenum)) continue;
    auto val = subtags[namenum];
    if (!val.isString()) {
      element_not_string();
      return false;
    }
    if (first) {
      if (tag == LOC_PRIVATE) {
        ret += s_SEPARATOR + "x";
      }
      first = false;
    }
    ret += s_SEPARATOR + val.toString();
  }
  return true;
}

static Variant HHVM_STATIC_METHOD(Locale, composeLocale, const Array& subtags) {
  s_intl_error->clearError();

  if (subtags.exists(s_GRANDFATHERED)) {
    auto val = subtags[s_GRANDFATHERED];
    if (val.isString()) {
      return val;
    }
  }

  if (!subtags.exists(s_LOC_LANG)) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "locale_compose: "
                           "parameter array does not contain 'language' tag.");
    return false;
  }
  String ret(subtags[s_LOC_LANG].toString());
  if (!append_multiple_key_values(ret, subtags, LOC_EXTLANG) ||
      !append_key_value(ret, subtags, LOC_SCRIPT) ||
      !append_key_value(ret, subtags, LOC_REGION) ||
      !append_multiple_key_values(ret, subtags, LOC_VARIANT) ||
      !append_multiple_key_values(ret, subtags, LOC_PRIVATE)) {
    return false;
  }
  return ret;
}

static Array HHVM_STATIC_METHOD(Locale, getAllVariants, const String& locale) {
  Variant val = get_icu_value(localeOrDefault(locale), LOC_VARIANT);
  String strval = val.toString();
  if (strval.empty()) {
    return Array();
  }
  Array ret = Array::Create();
  const char *s = strval.c_str(), *e = s + strval.size(), *p;
  for (p = s; p < e; ++p) {
    if (!isIDSeparator(*p)) continue;
    if ((p - s) <= 1) {
      return ret;
    }
    ret.append(String(s, p - s, CopyString));
    s = p + 1;
  }
  if ((e - s) > 1) {
    ret.append(String(s, e - s, CopyString));
  }
  return ret;
}

static String HHVM_STATIC_METHOD(Locale, getDefault) {
  return GetDefaultLocale();
}

static String HHVM_STATIC_METHOD(Locale, getDisplayLanguage,
                                 const String& locale,
                                 const String& in_locale) {
  return get_icu_display_value(localeOrDefault(locale),
                               localeOrDefault(in_locale), LOC_LANG);
}

static String HHVM_STATIC_METHOD(Locale, getDisplayName,
                                 const String& locale,
                                 const String& in_locale) {
  return get_icu_display_value(localeOrDefault(locale),
                               localeOrDefault(in_locale), LOC_DISPLAY);
}

static String HHVM_STATIC_METHOD(Locale, getDisplayRegion,
                                 const String& locale,
                                 const String& in_locale) {
  return get_icu_display_value(localeOrDefault(locale),
                               localeOrDefault(in_locale), LOC_REGION);
}

static String HHVM_STATIC_METHOD(Locale, getDisplayScript,
                                 const String& locale,
                                 const String& in_locale) {
  return get_icu_display_value(localeOrDefault(locale),
                               localeOrDefault(in_locale), LOC_SCRIPT);
}

static String HHVM_STATIC_METHOD(Locale, getDisplayVariant,
                                 const String& locale,
                                 const String& in_locale) {
  return get_icu_display_value(localeOrDefault(locale),
                               localeOrDefault(in_locale), LOC_VARIANT);
}

static Array HHVM_STATIC_METHOD(Locale, getKeywords, const String& locale) {
  UErrorCode error = U_ZERO_ERROR;
  String locname = localeOrDefault(locale);
  UEnumeration *e = uloc_openKeywords(locname.c_str(), &error);
  if (!e) return Array();

  Array ret = Array::Create();
  const char *key;
  int key_len;
  String val(128, ReserveString);
  char *ptr = val.get()->mutableData();
  error = U_ZERO_ERROR;
  while ((key = uenum_next(e, &key_len, &error))) {
tryagain:
    error = U_ZERO_ERROR;
    int val_len = uloc_getKeywordValue(locname.c_str(), key,
                                       ptr, val.capacity() + 1, &error);
    if (error == U_BUFFER_OVERFLOW_ERROR) {
      val = String(val_len + 128, ReserveString);
      ptr = val.get()->mutableData();
      goto tryagain;
    }
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "locale_get_keywords: Error encountered "
                                    "while getting the keyword  value for the "
                                    " keyword");
      return Array();
    }
    ret.set(String(key, key_len, CopyString), String(ptr, val_len, CopyString));
  }
  return ret;
}

static String HHVM_STATIC_METHOD(Locale, getPrimaryLanguage,
                                 const String& locale) {
  return get_icu_value(localeOrDefault(locale), LOC_LANG);
}

static Variant HHVM_STATIC_METHOD(Locale, getRegion, const String& locale) {
  return get_icu_value(localeOrDefault(locale), LOC_REGION);
}

static Variant HHVM_STATIC_METHOD(Locale, getScript, const String& locale) {
  return get_icu_value(localeOrDefault(locale), LOC_SCRIPT);
}

static String locale_suffix_strip(const String& locale) {
  for (int i = locale.size(); i >= 0; --i) {
    if (isIDSeparator(locale[i])) {
      if ((i >= 2) && isIDSeparator(locale[i - 2])) {
        return locale.substr(0, i - 2);
      } else {
        return locale.substr(0, i);
      }
    }
  }
  return String();
}

inline void normalize_for_match(String& v) {
  for (char *ptr = v.get()->mutableData(), *end = ptr + v.size(); ptr < end;
       ++ptr) {
    if (*ptr == '-') {
      *ptr = '_';
    } else {
      *ptr = tolower(*ptr);
    }
  }
  v.get()->invalidateHash();
}

static String HHVM_STATIC_METHOD(Locale, lookup, const Array& langtag,
                                 const String& locale,
                                 bool canonicalize, const String& def) {
  String locname(localeOrDefault(locale), CopyString);
  std::vector<std::pair<String,String>> cur_arr;
  for (ArrayIter iter(langtag); iter; ++iter) {
    auto val = iter.second();
    if (!val.isString()) {
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "lookup_loc_range: "
                             "locale array element is not a string");
      return def;
    }
    String normalized(val.toString(), CopyString);
    normalize_for_match(normalized);
    if (canonicalize) {
      normalized = get_icu_value(normalized, LOC_CANONICALIZE);
      if (normalized.isNull()) {
        s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "lookup_loc_range: "
                               "unable to canonicalize lang_tag");
        return def;
      }
      normalize_for_match(normalized);
    }
    cur_arr.push_back(std::make_pair(val.toString(),normalized));
  }

  if (canonicalize) {
    locname = get_icu_value(locname, LOC_CANONICALIZE);
    if (locname.isNull()) {
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "lookup_loc_range: "
                             "unable to canonicalize loc_range");
      return def;
    }
  }

  normalize_for_match(locname);
  while (locname.size() > 0) {
    for (auto &p : cur_arr) {
      if (locname.same(p.second)) {
        return canonicalize ? p.second : p.first;
      }
    }
    locname = locale_suffix_strip(locname);
  }
  return def;
}

static Variant get_private_subtags(const String& locname) {
  if (locname.empty()) return init_null();
  String locale(locname);
  int pos;
  while ((pos = singleton_pos(locale)) >= 0) {
    if ((locale[pos] == 'x') || (locale[pos] == 'X')) {
      if ((pos + 2) == locale.size()) {
        /* loc_name ends with '-x-' */
        return init_null();
      }
      return locale.substr(pos);
    }
    if ((pos + 1) >= locale.size()) {
      return init_null();
    }
    locale = locale.substr(pos + 1);
  }
  return init_null();
}

static void add_array_entry(Array& ret,
                            const String& locname,
                            LocaleTag tag) {
  Variant val;
  if (tag == LOC_PRIVATE) {
    val = get_private_subtags(locname);
  } else {
    val = get_icu_value(locname, tag, true);
  }
  if (val.isNull()) return;
  String strval = val.toString();
  if (strval.empty()) {
    return;
  }

  auto name = LocaleName(tag);
  if ((tag != LOC_PRIVATE) && (tag != LOC_VARIANT)) {
    ret.set(name, strval);
    return;
  }

  const char *s = strval.c_str(), *e = s + strval.size(), *p;
  int cnt;
  for (cnt = 0, p = s; p < e; ++p) {
    if (!isIDSeparator(*p)) continue;
    if ((p - s) > 1) {
      ret.set(name + String(cnt++), String(s, p - s, CopyString));
    }
    s = p + 1;
  }
  if ((e - s) > 1) {
    ret.set(name + String(cnt++), String(s, e - s, CopyString));
  }
}

static Array HHVM_STATIC_METHOD(Locale, parseLocale, const String& locale) {
  String locname = localeOrDefault(locale);
  Array ret = Array::Create();
  if (std::find(g_grandfathered.begin(),
                g_grandfathered.end(), locale.data()) !=
                g_grandfathered.end()) {
    ret.set(s_GRANDFATHERED, locname);
    return ret;
  }
  add_array_entry(ret, locname, LOC_LANG);
  add_array_entry(ret, locname, LOC_SCRIPT);
  add_array_entry(ret, locname, LOC_REGION);
  add_array_entry(ret, locname, LOC_VARIANT);
  add_array_entry(ret, locname, LOC_PRIVATE);
  return ret;
}

static bool HHVM_STATIC_METHOD(Locale, setDefault, const String& locale) {
  return SetDefaultLocale(locale);
}

//////////////////////////////////////////////////////////////////////////////

const StaticString s_Locale("Locale");

void IntlExtension::initLocale() {
  HHVM_STATIC_ME(Locale, acceptFromHttp);
  HHVM_STATIC_ME(Locale, canonicalize);
  HHVM_STATIC_ME(Locale, composeLocale);
  HHVM_STATIC_ME(Locale, getAllVariants);
  HHVM_STATIC_ME(Locale, getDefault);
  HHVM_STATIC_ME(Locale, getDisplayLanguage);
  HHVM_STATIC_ME(Locale, getDisplayName);
  HHVM_STATIC_ME(Locale, getDisplayRegion);
  HHVM_STATIC_ME(Locale, getDisplayScript);
  HHVM_STATIC_ME(Locale, getDisplayVariant);
  HHVM_STATIC_ME(Locale, getKeywords);
  HHVM_STATIC_ME(Locale, getPrimaryLanguage);
  HHVM_STATIC_ME(Locale, getRegion);
  HHVM_STATIC_ME(Locale, getScript);
  HHVM_STATIC_ME(Locale, lookup);
  HHVM_STATIC_ME(Locale, parseLocale);
  HHVM_STATIC_ME(Locale, setDefault);

#define ULOC_CONST(nm,val) Native::registerClassConstant<KindOfStaticString>\
                               (s_Locale.get(), s_##nm.get(), s_##val.get())

  Native::registerClassConstant<KindOfNull>(s_Locale.get(),
                                            s_DEFAULT_LOCALE.get());
  ULOC_CONST(LANG_TAG,               LOC_LANG);
  ULOC_CONST(EXTLANG_TAG,            LOC_EXTLANG);
  ULOC_CONST(SCRIPT_TAG,             LOC_SCRIPT);
  ULOC_CONST(REGION_TAG,             LOC_REGION);
  ULOC_CONST(VARIANT_TAG,            LOC_VARIANT);
  ULOC_CONST(GRANDFATHERED_LANG_TAG, GRANDFATHERED);
  ULOC_CONST(PRIVATE_TAG,            LOC_PRIVATE);

#undef ULOC_CONST

#define ULOC_LOCALE_CONST(cns) \
  Native::registerConstant<KindOfInt64>\
    (makeStaticString("ULOC_" #cns), ULOC_##cns); \
  Native::registerClassConstant<KindOfInt64>\
    (s_Locale.get(), makeStaticString(#cns), ULOC_##cns);

  ULOC_LOCALE_CONST(ACTUAL_LOCALE);
  ULOC_LOCALE_CONST(VALID_LOCALE);
#undef ULOC_LOCALE_CONST

  loadSystemlib("icu_locale");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
