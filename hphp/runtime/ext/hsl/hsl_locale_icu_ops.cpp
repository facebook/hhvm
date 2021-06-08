/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/hsl/hsl_locale_icu_ops.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/system/systemlib.h"

#include <unicode/unistr.h>
#include <unicode/uchar.h>

#include <functional>

namespace HPHP {

HSLLocaleICUOps::HSLLocaleICUOps(
  const Locale& locale
) : m_ctype(icu::Locale::getRoot()), m_caseFoldFlags(0) {
  auto ctype = locale.querylocale(LocaleCategory, LC_CTYPE);

  m_ctype = icu::Locale(ctype);

  if (strncmp(ctype, "tr", 2) == 0 || strncmp(ctype, "az", 2) == 0) {
    m_caseFoldFlags = U_FOLD_CASE_EXCLUDE_SPECIAL_I;
  }
}

HSLLocaleICUOps::~HSLLocaleICUOps() {
}

int64_t HSLLocaleICUOps::strlen(const String& str) const {
  return HHVM_FN(fb_utf8_strlen)(str);
}

namespace {

ALWAYS_INLINE String utf8_icu_op(
  const String& utf8_in,
  std::function<void(icu::UnicodeString&)> op
) {
  if (utf8_in.empty()) {
    return utf8_in;
  }

  auto mut = icu::UnicodeString::fromUTF8(icu::StringPiece(utf8_in.data(), utf8_in.length()));
  op(mut);
  std::string ret;
  mut.toUTF8String(ret);
  return String(ret);
}

} // namespace

String HSLLocaleICUOps::uppercase(const String& str) const {
  return utf8_icu_op(
    str,
    [this](icu::UnicodeString& mut) { mut.toUpper(this->m_ctype); }
  );
}

String HSLLocaleICUOps::lowercase(const String& str) const {
  return utf8_icu_op(
    str,
    [this](icu::UnicodeString& mut) { mut.toLower(this->m_ctype); }
  );
}

String HSLLocaleICUOps::foldcase(const String& str) const {
  return utf8_icu_op(
    str,
    [this](icu::UnicodeString& mut) { mut.foldCase(this->m_caseFoldFlags); }
  );
}

} // namespace HPHP
