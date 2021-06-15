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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/hsl/hsl_locale_icu_ops.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/system/systemlib.h"

#include <unicode/coll.h>
#include <unicode/errorcode.h>
#include <unicode/normalizer2.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/uvernum.h>

#include <functional>


namespace HPHP {

HSLLocaleICUOps::HSLLocaleICUOps(
  const Locale& locale
) {
  m_collate = icu::Locale(locale.querylocale(LocaleCategory, LC_COLLATE));
  auto ctype = locale.querylocale(LocaleCategory, LC_CTYPE);

  m_ctype = icu::Locale(ctype);

  if (strncmp(ctype, "tr", 2) == 0 || strncmp(ctype, "az", 2) == 0) {
    m_caseFoldFlags = U_FOLD_CASE_EXCLUDE_SPECIAL_I;
  }
}

HSLLocaleICUOps::~HSLLocaleICUOps() {
  delete m_collator;
}

int64_t HSLLocaleICUOps::strlen(const String& str) const {
  return HHVM_FN(fb_utf8_strlen)(str);
}

namespace {

ALWAYS_INLINE icu::StringPiece icu_string_piece(const HPHP::String& str) {
  return icu::StringPiece(str.data(), str.size());
}

ALWAYS_INLINE String utf8_icu_op(
  const String& utf8_in,
  std::function<void(icu::UnicodeString&)> op
) {
  if (utf8_in.empty()) {
    return utf8_in;
  }

  auto mut = icu::UnicodeString::fromUTF8(icu_string_piece(utf8_in));
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

Array HSLLocaleICUOps::chunk(const String& str, int64_t chunk_size) const {
  assert(chunk_size > 0);
  auto icustr = icu::UnicodeString::fromUTF8(icu_string_piece(str));
  const auto len = icustr.countChar32();
  VecInit ret { (size_t) (len / chunk_size + 1) };
  if (len <= chunk_size) {
    ret.append(str);
    return ret.toArray();
  }

  int next = 0;
  // i, next, and code_units are count of char16s, but chunk_size is count of
  // char32s
  const auto code_units = icustr.length();
  for (int i = 0; i < code_units; i = next) {
    next = icustr.moveIndex32(i, chunk_size);
    std::string buf;
    icustr.tempSubStringBetween(i, next).toUTF8String(buf);
    ret.append(buf);
  }

  return ret.toArray(); 
}

int64_t HSLLocaleICUOps::strcoll(const String& a, const String& b) const {
  assertx(!a.isNull() && !b.isNull());

  icu::ErrorCode err;
  auto coll = collator();
  coll->setStrength(icu::Collator::TERTIARY); // accent- and case-sensitive
  return coll->compareUTF8(icu_string_piece(a), icu_string_piece(b), err);
}

int64_t HSLLocaleICUOps::strcasecmp(const String& a, const String& b) const {
  assertx(!a.isNull() && !b.isNull());

  icu::ErrorCode err;
  auto coll = collator();
  coll->setStrength(icu::Collator::SECONDARY); // accent-sensitive, not case-sensitive
  return coll->compareUTF8(icu_string_piece(a), icu_string_piece(b), err);
}

bool HSLLocaleICUOps::starts_with(const String& str, const String& prefix) const {
  assertx(!str.isNull() && !prefix.isNull());
  if (str.substr(0, prefix.size()) == prefix) {
    return true;
  }

  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  auto ustr = icu::UnicodeString::fromUTF8(icu_string_piece(str));
  auto uprefix = icu::UnicodeString::fromUTF8(icu_string_piece(prefix));
  icu::UnicodeString nprefix;
  normalizer->normalize(uprefix, nprefix, err);
  if (ustr.startsWith(nprefix)) {
    return true;
  }
  icu::UnicodeString nstr;
  normalizer->normalize(ustr, nstr, err);
  return nstr.startsWith(nprefix);
}

bool HSLLocaleICUOps::ends_with(const String& str, const String& suffix) const {
  assertx(!str.isNull() && !suffix.isNull());
  int64_t off = str.size() - suffix.size();
  if (off >= 0 && str.substr((int) off, suffix.size()) == suffix) {
    return true;
  }

  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  auto ustr = icu::UnicodeString::fromUTF8(icu_string_piece(str));
  auto usuffix = icu::UnicodeString::fromUTF8(icu_string_piece(suffix));
  icu::UnicodeString nsuffix;
  normalizer->normalize(usuffix, nsuffix, err);
  if (ustr.endsWith(nsuffix)) {
    return true;
  }
  icu::UnicodeString nstr;
  normalizer->normalize(ustr, nstr, err);
  return nstr.endsWith(nsuffix);
}

bool HSLLocaleICUOps::starts_with_ci(const String& str, const String& prefix) const {
  assertx(!str.isNull() && !prefix.isNull());
  if (str.substr(0, prefix.size()) == prefix) {
    return true;
  }
  auto uprefix = icu::UnicodeString::fromUTF8(icu_string_piece(prefix));
  auto ustr = icu::UnicodeString::fromUTF8(icu_string_piece(str));

  uprefix.foldCase(m_caseFoldFlags);
  ustr.foldCase(m_caseFoldFlags);

  icu::ErrorCode err;
  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  icu::UnicodeString nprefix;
  icu::UnicodeString nstr;
  normalizer->normalize(uprefix, nprefix, err);
  normalizer->normalize(ustr, nstr, err);
  return nstr.startsWith(nprefix);
}

bool HSLLocaleICUOps::ends_with_ci(const String& str, const String& suffix) const {
  assertx(!str.isNull() && !suffix.isNull());
  int64_t off = str.size() - suffix.size();
  if (off >= 0 && str.substr(off, suffix.size()) == suffix) {
    return true;
  }
  auto usuffix = icu::UnicodeString::fromUTF8(icu_string_piece(suffix));
  auto ustr = icu::UnicodeString::fromUTF8(icu_string_piece(str));

  usuffix.foldCase(m_caseFoldFlags);
  ustr.foldCase(m_caseFoldFlags);

  icu::ErrorCode err;
  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  icu::UnicodeString nsuffix;
  icu::UnicodeString nstr;
  normalizer->normalize(usuffix, nsuffix, err);
  normalizer->normalize(ustr, nstr, err);
  return nstr.endsWith(nsuffix);
}

icu::Collator* HSLLocaleICUOps::collator() const {
  // const is a lie, but only because we're doing lazy initialization
  if (m_collator) {
    return m_collator;
  }
  icu::ErrorCode err;
  const_cast<HSLLocaleICUOps*>(this)->m_collator =
    icu::Collator::createInstance(this->m_collate, err);
  if (err.isFailure()) {
    SystemLib::throwErrorObject(
      folly::sformat(
        "Failed to create a collator: {}",
        err.errorName()
      )
    );
  }
  return m_collator;
}

} // namespace HPHP
