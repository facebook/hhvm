/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/locale.h"
#include "hphp/runtime/base/thread-safe-setlocale.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/runtime/ext/hsl/ext_hsl_locale.h"
#include "hphp/runtime/ext/hsl/hsl_locale_byte_ops.h"
#include "hphp/runtime/ext/hsl/hsl_locale_ops.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"

#include <functional>

#include <unicode/locid.h>
#include <unicode/unistr.h>
#include <unicode/uchar.h>

#include <ctype.h>

namespace HPHP {
namespace {

const HSLLocale::Ops* s_byte_ops = nullptr;
const Locale* s_bytes_locale = nullptr;

ALWAYS_INLINE const Locale* get_locale(const Variant& maybe_locale) {
  if (LIKELY(maybe_locale.isNull())) {
    assert(s_bytes_locale);
    return s_bytes_locale;
  }

  if (!maybe_locale.isObject()) {
    raise_fatal_error("Locale is not null or an object");
  }

  return HSLLocale::fromObject(maybe_locale.asCObjRef())->get().get();
}

ALWAYS_INLINE const HSLLocale::Ops* get_ops(const Variant& maybe_locale) {
  if (LIKELY(maybe_locale.isNull())) {
    assert(s_byte_ops);
    return s_byte_ops;
  }

  if (!maybe_locale.isObject()) {
    raise_fatal_error("Locale is not null or an object");
  }

  return HSLLocale::fromObject(maybe_locale.asCObjRef())->ops();
}


int64_t HHVM_FUNCTION(strlen_l, const String& str, const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strlen(str);
}

String HHVM_FUNCTION(uppercase_l, const String& str, const Variant& maybe_loc) {
  return get_ops(maybe_loc)->uppercase(str);
}

String HHVM_FUNCTION(lowercase_l, const String& str, const Variant& maybe_loc) {
  return get_ops(maybe_loc)->lowercase(str);
}

String HHVM_FUNCTION(titlecase_l, const String& str, const Variant& maybe_loc) {
  // Always use ICU because libc doesn't have this feature
  if (str.empty()) {
    return str;
  }

  icu::Locale ctype(
    get_locale(maybe_loc)->querylocale(LocaleCategory, LC_CTYPE)
  );
  auto mut = icu::UnicodeString::fromUTF8(icu::StringPiece(str.data(), str.length()));
  mut.toTitle(nullptr, ctype);
  std::string ret;
  mut.toUTF8String(ret);
  return ret;
}

String HHVM_FUNCTION(foldcase_l, const String& str, const Variant& maybe_loc) {
  return get_ops(maybe_loc)->foldcase(str);
}

Array HHVM_FUNCTION(chunk_l,
                     const String& str,
                     int64_t chunk_size,
                     const Variant& maybe_loc) {
  if (chunk_size < 1) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "chunk size can not be less than 1"
    );
  }
  return get_ops(maybe_loc)->chunk(str, chunk_size);
}

int64_t HHVM_FUNCTION(strcoll_l,
                      const String& a,
                      const String& b,
                      const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strcoll(a, b);
}

int64_t HHVM_FUNCTION(strcasecmp_l,
                      const String& a,
                      const String& b,
                      const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strcasecmp(a, b);
}

bool HHVM_FUNCTION(starts_with_l,
                   const String& string,
                   const String& prefix,
                   const Variant& maybe_loc) {
  return get_ops(maybe_loc)->starts_with(string, prefix);
}

bool HHVM_FUNCTION(starts_with_ci_l,
                   const String& string,
                   const String& prefix,
                   const Variant& maybe_loc) {
  return get_ops(maybe_loc)->starts_with_ci(string, prefix);
}

bool HHVM_FUNCTION(ends_with_l,
                   const String& string,
                   const String& suffix,
                   const Variant& maybe_loc) {
  return get_ops(maybe_loc)->ends_with(string, suffix);
}

bool HHVM_FUNCTION(ends_with_ci_l,
                   const String& string,
                   const String& suffix,
                   const Variant& maybe_loc) {
  return get_ops(maybe_loc)->ends_with_ci(string, suffix);
}

String HHVM_FUNCTION(strip_prefix_l,
                     const String& string,
                     const String& prefix,
                     const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strip_prefix(string, prefix);
}

String HHVM_FUNCTION(strip_suffix_l,
                     const String& string,
                     const String& suffix,
                     const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strip_suffix(string, suffix);
}

int64_t HHVM_FUNCTION(strpos_l,
                      const String& haystack,
                      const String& needle,
                      int64_t offset,
                      const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strpos(haystack, needle, offset);
}

int64_t HHVM_FUNCTION(stripos_l,
                      const String& haystack,
                      const String& needle,
                      int64_t offset,
                      const Variant& maybe_loc) {
  return get_ops(maybe_loc)->stripos(haystack, needle, offset);
}

int64_t HHVM_FUNCTION(strrpos_l,
                      const String& haystack,
                      const String& needle,
                      int64_t offset,
                      const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strrpos(haystack, needle, offset);
}

int64_t HHVM_FUNCTION(strripos_l,
                      const String& haystack,
                      const String& needle,
                      int64_t offset,
                      const Variant& maybe_loc) {
  return get_ops(maybe_loc)->strripos(haystack, needle, offset);
}

namespace {
ALWAYS_INLINE int64_t normalize_length(int64_t length) {
  if (length < 0) {
    SystemLib::throwInvalidArgumentExceptionObject("Expected non-negative length.");
  }
  return MIN(length, StringData::MaxSize);
}
} // namespace

String HHVM_FUNCTION(slice_l,
                     const String& str,
                     int64_t offset,
                     int64_t length,
                     const Variant& maybe_loc) {
  length = normalize_length(length);
  return get_ops(maybe_loc)->slice(str, offset, length);
}

String HHVM_FUNCTION(splice_l,
                     const String& str,
                     const String& replacement,
                     int64_t offset,
                     const Variant& length,
                     const Variant& maybe_loc) {
  const int64_t int_length = normalize_length(
    length.isNull() ? StringData::MaxSize : length.asInt64Val()
  );
  return get_ops(maybe_loc)->splice(str, replacement, offset, int_length);
}

Array HHVM_FUNCTION(split_l,
                    const String& str,
                    const String& delimiter,
                    const Variant& limit,
                    const Variant& maybe_loc) {
  if (str.empty()) {
    return  make_vec_array(empty_string());;
  }

  int64_t int_limit = limit.isNull() ? k_PHP_INT_MAX : limit.asInt64Val();
  if (int_limit == 0) {
    return empty_vec_array();
  }
  if (int_limit < 0) {
    SystemLib::throwInvalidArgumentExceptionObject("Limit must be >= 0");
  }
  return get_ops(maybe_loc)->split(str, delimiter, int_limit);
}

String HHVM_FUNCTION(reverse_l,
                     const String& str,
                     const Variant& maybe_loc) {
  if (str.empty()) {
    return str;
  }
  return get_ops(maybe_loc)->reverse(str);
}

String HHVM_FUNCTION(pad_left_l,
                     const String& str,
                     int64_t len,
                     const String& pad,
                     const Variant& maybe_loc) {
  if (pad.empty()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "padding string can not be empty"
    );
  }
  len = normalize_length(len);
  return get_ops(maybe_loc)->pad_left(str, len, pad);
}

String HHVM_FUNCTION(pad_right_l,
                     const String& str,
                     int64_t len,
                     const String& pad,
                     const Variant& maybe_loc) {
  if (pad.empty()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "padding string can not be empty"
    );
  }
  len = normalize_length(len);
  return get_ops(maybe_loc)->pad_right(str, len, pad);
}

String HHVM_FUNCTION(vsprintf_l,
                     const Variant& maybe_loc,
                     const String& fmt,
                     const Array& args) {

  // Okay, quite a few layers here :p
  // 1. get an HSLLocale from a nullable Locale object
  // 2. get an std::shared_ptr<HPHP::Locale> from an HSL Locale
  // 3. get a locale_t from an HPHP::Locale

  // We're doing this manually instead of using `get_locale()` as we need
  // a shared_ptr; the cached fixed locale used by `get_locale()` is a raw
  // pointer for performance.
  if (!(maybe_loc.isNull() || maybe_loc.isObject())) {
    raise_fatal_error("Locale is not null or an object");
  }
  auto format_loc = maybe_loc.isNull()
    ? Locale::getCLocale()
    : HSLLocale::fromObject(maybe_loc.asCObjRef())->get();
  auto thread_loc = ThreadSafeLocaleHandler::getRequestLocale();
  // Compare locale_t
  if (LIKELY(format_loc->get() == thread_loc->get())) {
    return string_printf(fmt.data(), fmt.size(), args);
  }
  SCOPE_EXIT { ThreadSafeLocaleHandler::setRequestLocale(thread_loc); };
  ThreadSafeLocaleHandler::setRequestLocale(format_loc);

  return string_printf(fmt.data(), fmt.size(), args);
}

namespace {
String trim_impl(const String& str,
                 const Variant& what,
                 const Variant& maybe_loc,
                 HSLLocale::Ops::TrimSides sides) {
  if (str.empty()) {
    return str;
  }
  if (what.isNull()) {
    return get_ops(maybe_loc)->trim(str, sides);
  }
  assertx(what.isString());
  const auto& swhat = what.asCStrRef();

  if (swhat.empty()) {
    return str;
  }
  return get_ops(maybe_loc)->trim(str, swhat, sides);
}
} // namespace

String HHVM_FUNCTION(trim_l,
                     const String& str,
                     const Variant& what,
                     const Variant& maybe_loc) {
  return trim_impl(str, what, maybe_loc, HSLLocale::Ops::TrimSides::BOTH);
}

String HHVM_FUNCTION(trim_left_l,
                     const String& str,
                     const Variant& what,
                     const Variant& maybe_loc) {
  return trim_impl(str, what, maybe_loc, HSLLocale::Ops::TrimSides::LEFT);
}

String HHVM_FUNCTION(trim_right_l,
                     const String& str,
                     const Variant& what,
                     const Variant& maybe_loc) {
  return trim_impl(str, what, maybe_loc, HSLLocale::Ops::TrimSides::RIGHT);
}

String HHVM_FUNCTION(replace_l,
                     const String& haystack,
                     const String& needle,
                     const String& replacement,
                     const Variant& maybe_loc) {
  if (haystack.empty() || needle.empty()) {
    return haystack;
  }
  return get_ops(maybe_loc)->replace(haystack, needle, replacement);
}

String HHVM_FUNCTION(replace_ci_l,
                     const String& haystack,
                     const String& needle,
                     const String& replacement,
                     const Variant& maybe_loc) {
  if (haystack.empty() || needle.empty()) {
    return haystack;
  }
  return get_ops(maybe_loc)->replace_ci(haystack, needle, replacement);
}

namespace {
void check_replace_pairs(const Array& replacements) {
  IterateKV(replacements.get(), [](TypedValue needle, TypedValue replacement) {
    if(!isStringType(needle.type()) || !isStringType(replacement.type())) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Keys and values passed to str_replace*() must be strings"
      );
    }
    if (needle.val().pstr->empty()) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "Keys passed to str_replace*() must be non-empty strings"
      );
    }
  });
}
} // namespace

String HHVM_FUNCTION(replace_every_l,
                     const String& haystack,
                     const Array& replacements,
                     const Variant& maybe_loc) {
  if (haystack.empty() || replacements.empty()) {
    return haystack;
  }
  check_replace_pairs(replacements);
  return get_ops(maybe_loc)->replace_every(haystack, replacements);
}

String HHVM_FUNCTION(replace_every_ci_l,
                     const String& haystack,
                     const Array& replacements,
                     const Variant& maybe_loc) {
  if (haystack.empty() || replacements.empty()) {
    return haystack;
  }
  check_replace_pairs(replacements);
  return get_ops(maybe_loc)->replace_every_ci(haystack, replacements);
}

String HHVM_FUNCTION(replace_every_nonrecursive_l,
                     const String& haystack,
                     const Array& replacements,
                     const Variant& maybe_loc) {
  if (haystack.empty() || replacements.empty()) {
    return haystack;
  }
  check_replace_pairs(replacements);
  return get_ops(maybe_loc)->replace_every_nonrecursive(haystack, replacements);
}

String HHVM_FUNCTION(replace_every_nonrecursive_ci_l,
                     const String& haystack,
                     const Array& replacements,
                     const Variant& maybe_loc) {
  if (haystack.empty() || replacements.empty()) {
    return haystack;
  }
  check_replace_pairs(replacements);
  return get_ops(maybe_loc)->replace_every_nonrecursive_ci(haystack, replacements);
}

struct HSLStrExtension final : Extension {
  HSLStrExtension() : Extension("hsl_str", "0.1", NO_ONCALL_YET) {}

  void moduleInit() override {
    s_byte_ops = new HSLLocaleByteOps();
    s_bytes_locale = Locale::getCLocale().get();

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strlen_l, strlen_l);
    // - clang doesn't like the HHVM_FALIAS macro with \\u
    // - \\\\ gets different results in gcc, leading to 'undefined native function'
    HHVM_NAMED_FE_STR(
      "HH\\Lib\\_Private\\_Str\\uppercase_l",
      HHVM_FN(uppercase_l),
      nativeFuncs()
    );
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\lowercase_l, lowercase_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\titlecase_l, titlecase_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\foldcase_l, foldcase_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strcoll_l, strcoll_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strcasecmp_l, strcasecmp_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\starts_with_l, starts_with_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\starts_with_ci_l, starts_with_ci_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\ends_with_l, ends_with_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\ends_with_ci_l, ends_with_ci_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strip_prefix_l, strip_prefix_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strip_suffix_l, strip_suffix_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strpos_l, strpos_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strrpos_l, strrpos_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\stripos_l, stripos_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strripos_l, strripos_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\chunk_l, chunk_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\slice_l, slice_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\splice_l, splice_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\split_l, split_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\reverse_l, reverse_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\vsprintf_l, vsprintf_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\pad_left_l, pad_left_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\pad_right_l, pad_right_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\trim_l, trim_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\trim_left_l, trim_left_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\trim_right_l, trim_right_l);


    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\replace_l, replace_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\replace_ci_l, replace_ci_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\replace_every_l, replace_every_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\replace_every_ci_l, replace_every_ci_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\replace_every_nonrecursive_l, replace_every_nonrecursive_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\replace_every_nonrecursive_ci_l, replace_every_nonrecursive_ci_l);
  }
} s_hsl_str_extension;

} // namespace
} // namespace HPHP
