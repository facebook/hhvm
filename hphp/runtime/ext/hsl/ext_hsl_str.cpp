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

#include "hphp/runtime/base/locale.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/runtime/ext/hsl/ext_hsl_locale.h"
#include "hphp/runtime/ext/hsl/hsl_locale_ops.h"
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

RDS_LOCAL(HSLLocale*, c_hsl_locale);

const HSLLocale* get_locale(const Variant& maybe_locale) {
  if (LIKELY(maybe_locale.isNull())) {
    // TODO:
    // - test perf with this default
    // - add runtime option to request vs C locale
    // - go back to this shortly after
    assertx(c_hsl_locale);
    return *c_hsl_locale;
  }
  if (!maybe_locale.isObject()) {
    raise_fatal_error("Locale is not null or an object");
  }
  return HSLLocale::fromObject(maybe_locale.asCObjRef());
}

int64_t HHVM_FUNCTION(strlen_l, const String& str, const Variant& maybe_loc) {
  return get_locale(maybe_loc)->ops()->strlen(str);
}

String HHVM_FUNCTION(uppercase_l, const String& str, const Variant& maybe_loc) {
  return get_locale(maybe_loc)->ops()->uppercase(str);
}

String HHVM_FUNCTION(lowercase_l, const String& str, const Variant& maybe_loc) {
  return get_locale(maybe_loc)->ops()->lowercase(str);
}

String HHVM_FUNCTION(titlecase_l, const String& str, const Variant& maybe_loc) {
  // Always use ICU because libc doesn't have this feature
  if (str.empty()) {
    return str;
  }

  icu::Locale ctype(
    get_locale(maybe_loc)->get()->querylocale(LocaleCategory, LC_CTYPE)
  );
  auto mut = icu::UnicodeString::fromUTF8(icu::StringPiece(str.data(), str.length()));
  mut.toTitle(nullptr, ctype);
  std::string ret;
  mut.toUTF8String(ret);
  return ret;
}

String HHVM_FUNCTION(foldcase_l, const String& str, const Variant& maybe_loc) {
  return get_locale(maybe_loc)->ops()->foldcase(str);
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
  return get_locale(maybe_loc)->ops()->chunk(str, chunk_size);
}

int64_t HHVM_FUNCTION(strcoll_l,
                      const String& a,
                      const String& b,
                      const Variant& maybe_loc) {
  return get_locale(maybe_loc)->ops()->strcoll(a, b);
}

int64_t HHVM_FUNCTION(strcasecmp_l,
                      const String& a,
                      const String& b,
                      const Variant& maybe_loc) {
  return get_locale(maybe_loc)->ops()->strcasecmp(a, b);
}

struct HSLStrExtension final : Extension {
  HSLStrExtension() : Extension("hsl_str", "0.1") {}

  void moduleInit() override {
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

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\chunk_l, chunk_l);

    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strcoll_l, strcoll_l);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Str\\strcasecmp_l, strcasecmp_l);
    loadSystemlib();
  }

  void requestInit() override {
    *c_hsl_locale.get() = new HSLLocale(Locale::getCLocale());
  }
} s_hsl_str_extension;

} // namespace
} // namespace HPHP
