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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/hsl/hsl_locale_icu_ops.h"
#include "hphp/runtime/ext/hsl/replace_every_nonrecursive.h"
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

ALWAYS_INLINE icu::StringPiece icu_string_piece(const HPHP::StringData* str) {
  return icu::StringPiece(str->data(), str->size());
}

template<typename T>
ALWAYS_INLINE icu::UnicodeString ustr_from_utf8(const T& str) {
  return icu::UnicodeString::fromUTF8(icu_string_piece(str));
}
template<>
ALWAYS_INLINE icu::UnicodeString ustr_from_utf8<TypedValue>(const TypedValue& tv) {
  assertx(isStringType(tv.type()));
  return ustr_from_utf8(tv.val().pstr);
}

ALWAYS_INLINE String utf8_icu_op(
  const String& utf8_in,
  std::function<void(icu::UnicodeString&)> op
) {
  if (utf8_in.empty()) {
    return utf8_in;
  }

  auto mut = ustr_from_utf8(utf8_in);
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
  auto icustr = ustr_from_utf8(str);
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
  auto ustr = ustr_from_utf8(str);
  auto uprefix = ustr_from_utf8(prefix);
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
  auto ustr = ustr_from_utf8(str);
  auto usuffix = ustr_from_utf8(suffix);
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
  auto uprefix = ustr_from_utf8(prefix);
  auto ustr = ustr_from_utf8(str);

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
  auto usuffix = ustr_from_utf8(suffix);
  auto ustr = ustr_from_utf8(str);

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

String HSLLocaleICUOps::strip_prefix(const String& str, const String& prefix) const {
  if (str.substr(0, prefix.size()) == prefix) {
    return str.substr(prefix.size());
  }

  auto ustr = ustr_from_utf8(str);
  auto uprefix = ustr_from_utf8(prefix);
  icu::ErrorCode err;

  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  icu::UnicodeString nprefix;
  icu::UnicodeString nstr;
  normalizer->normalize(uprefix, nprefix, err);
  normalizer->normalize(ustr, nstr, err);

  if (!nstr.startsWith(nprefix)) {
    return str;
  }

  auto tail = nstr.tempSubString(nprefix.length());
  std::string ret;
  tail.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::strip_suffix(const String& str, const String& suffix) const {
  auto ustr = ustr_from_utf8(str);
  auto usuffix = ustr_from_utf8(suffix);
  icu::ErrorCode err;

  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  icu::UnicodeString nsuffix;
  icu::UnicodeString nstr;
  normalizer->normalize(usuffix, nsuffix, err);
  normalizer->normalize(ustr, nstr, err);

  if (!nstr.endsWith(nsuffix)) {
    return str;
  }

  auto tail = nstr.tempSubString(0, nstr.length() - nsuffix.length());
  std::string ret;
  tail.toUTF8String(ret);
  return ret;
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

namespace {

enum class Direction {
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT
};
enum class CaseSensitivity {
  CASE_SENSITIVE,
  CASE_INSENSITIVE
};

int64_t strpos_impl(const String& haystack,
                    const String& needle,
                    int64_t offset,
                    Direction dir,
                    CaseSensitivity ci,
                    uint32_t caseFoldFlags) {
  auto uhs = ustr_from_utf8(haystack);
  auto un = ustr_from_utf8(needle);

  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  uhs = normalizer->normalize(uhs, err);
  un = normalizer->normalize(un, err);

  auto char32_len = uhs.countChar32();
  if (offset >= 0) {
    offset = HSLLocale::Ops::normalize_offset(offset, char32_len);
    char32_len -= offset;
  } else {
    if (dir == Direction::LEFT_TO_RIGHT) {
      offset = HSLLocale::Ops::normalize_offset(offset, char32_len);
    } else {
      // Match PHP strrpos() behavior for now; RFC for new behavior at
      // https://github.com/facebook/hhvm/pull/8847
      char32_len += std::min(offset + un.countChar32(), (int64_t) 0);
      offset = 0;
    }
  }

  if (ci == CaseSensitivity::CASE_INSENSITIVE) {
    uhs.foldCase(caseFoldFlags);
    un.foldCase(caseFoldFlags);
  }

  const auto char16_len = uhs.moveIndex32(0, char32_len);
  const auto char16_offset = uhs.moveIndex32(0, offset);

  const auto char16_pos = (dir == Direction::LEFT_TO_RIGHT)
    ? uhs.indexOf(un, char16_offset, char16_len)
    : uhs.lastIndexOf(un, char16_offset, char16_len);

  if (char16_pos == -1) {
    return -1;
  }

  return uhs.countChar32(0, char16_pos);
}

} // namespace

int64_t HSLLocaleICUOps::strpos(const String& haystack, const String& needle, int64_t offset) const {
  return strpos_impl(haystack,
                     needle,
                     offset,
                     Direction::LEFT_TO_RIGHT,
                     CaseSensitivity::CASE_SENSITIVE,
                     /* n/a: case fold flags */ 0);
}

int64_t HSLLocaleICUOps::strrpos(const String& haystack, const String& needle, int64_t offset) const {
  return strpos_impl(haystack,
                     needle,
                     offset,
                     Direction::RIGHT_TO_LEFT,
                     CaseSensitivity::CASE_SENSITIVE,
                     /* n/a: case fold flags */ 0);
}

int64_t HSLLocaleICUOps::stripos(const String& haystack, const String& needle, int64_t offset) const {
  return strpos_impl(haystack,
                     needle,
                     offset,
                     Direction::LEFT_TO_RIGHT,
                     CaseSensitivity::CASE_INSENSITIVE,
                     m_caseFoldFlags);
}

int64_t HSLLocaleICUOps::strripos(const String& haystack, const String& needle, int64_t offset) const {
  return strpos_impl(haystack,
                     needle,
                     offset,
                     Direction::RIGHT_TO_LEFT,
                     CaseSensitivity::CASE_INSENSITIVE,
                     m_caseFoldFlags);
}

String HSLLocaleICUOps::slice(const String& str, int64_t offset, int64_t length) const {
  auto ustr = ustr_from_utf8(str);
  const auto char32_full_len = ustr.countChar32();
  if (length < 0) {
    length += char32_full_len;
    if (length <= 0) {
      return empty_string();
    }
  }

  offset = normalize_offset(offset, char32_full_len);

  const auto char16_start = ustr.moveIndex32(0, offset);
  const auto char16_end = ustr.moveIndex32(char16_start, MIN(length, std::numeric_limits<int32_t>::max()));
  auto uslice = ustr.tempSubStringBetween(char16_start, char16_end);
  std::string ret;
  uslice.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::reverse(const String& str) const {
  auto mut = ustr_from_utf8(str);
  mut.reverse();
  std::string ret;
  mut.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::pad_left(const String& str, int64_t desired_len, const String& pad) const {
  auto ustr = ustr_from_utf8(str);
  const auto strl = ustr.countChar32();
  if (strl >= desired_len) {
    return str;
  }

  const auto upad = ustr_from_utf8(pad);
  const auto padl = upad.countChar32();
  icu::UnicodeString uret;
  for (int64_t diff = desired_len - strl; diff >= 0; diff -= padl) {
    uret.append(upad);
  }
  uret.truncate(uret.moveIndex32(0, desired_len - strl));
  uret.append(ustr);
  std::string ret;
  uret.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::pad_right(const String& str, int64_t desired_len, const String& pad) const {
  auto ustr = ustr_from_utf8(str);
  const auto strl = ustr.countChar32();
  if (strl >= desired_len) {
    return str;
  }

  const auto upad = ustr_from_utf8(pad);
  const auto padl = upad.countChar32();
  for (int64_t diff = desired_len - strl; diff >= 0; diff -= padl) {
    ustr.append(upad);
  }
  ustr.truncate(ustr.moveIndex32(0, desired_len));
  std::string ret;
  ustr.toUTF8String(ret);
  return ret;
}

Array HSLLocaleICUOps::split(const String& str, const String& delimiter, int64_t limit) const {
  assertx(limit > 0);

  auto ustr = ustr_from_utf8(str);

  if (delimiter.empty()) {
    VecInit ret { (size_t) MIN(ustr.countChar32(), limit) };
    int32_t offset = 0;
    int count = 0;
    while (offset < ustr.length()) {
      std::string slice;
      if (count == limit - 1) {
        ustr.tempSubString(offset).toUTF8String(slice);
        ret.append(slice);
        break;
      }
      auto next = ustr.moveIndex32(offset, 1);
      if (next == offset) {
        break;
      }
      ustr.tempSubStringBetween(offset, next).toUTF8String(slice);
      ret.append(slice);
      offset = next;
      count++;
    }
    return ret.toArray();
  }

  auto udelim = ustr_from_utf8(delimiter);

  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  ustr = normalizer->normalize(ustr, err);

  udelim = normalizer->normalize(udelim, err);


  VecInit ret {(size_t) (ustr.length() / udelim.length()) + 1};

  int32_t offset = 0, next = 0, count = 0;
  while (next < ustr.length() && count + 1 < limit) {
    next = ustr.indexOf(udelim, offset);
    if (next == -1) {
      break;
    }

    std::string slice;
    ustr.tempSubStringBetween(offset, next).toUTF8String(slice);
    ret.append(slice);
    count++;

    next += udelim.length();
    offset = next;
  }

  std::string slice;
  ustr.tempSubString(offset).toUTF8String(slice);
  ret.append(slice);

  return ret.toArray();
}

String HSLLocaleICUOps::splice(const String& str,
                               const String& replacement,
                               int64_t offset,
                               int64_t length) const {

  assertx(length >= 0);
  auto mut = ustr_from_utf8(str);

  // Normalize: if we replace the first character of "éfg" with "X":
  // - we want "Xfg", not "X́fg"
  // - we don't want the result to depend on if the UTF8 string has been
  //   normalized already
  // - this also affects offset calculation, as normalization can reduce the
  //   length
  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  mut = normalizer->normalize(mut, err);

  const auto char32_len = mut.countChar32();
  const auto ureplacement = ustr_from_utf8(replacement);

  offset = normalize_offset(offset, char32_len);
  offset = mut.moveIndex32(0, offset);
  length = mut.moveIndex32(offset, length) - offset;
  mut.replace(offset, length, ureplacement);

  std::string ret;
  mut.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::trim_impl(const String& str,
                                  const std::function<bool(UChar32)>& test,
                                  TrimSides sides) const {
  auto ustr = ustr_from_utf8(str);
  int32_t start = 0;
  int32_t end = ustr.length();
  if ((uint8_t) sides & (uint8_t) TrimSides::LEFT) {
    while (start <= end) {
      if (!test(ustr.char32At(start))) {
        break;
      }
      start = ustr.moveIndex32(start, 1);
    }
  }
  if ((uint8_t) sides & (uint8_t) TrimSides::RIGHT) {
    while (start <= end) {
      auto prev = ustr.moveIndex32(end, -1);
      if (!test(ustr.char32At(prev))) {
        break;
      }
      end = prev;
    }
  }
  if (end <= start) {
    return empty_string();
  }
  std::string ret;
  ustr.tempSubStringBetween(start, end).toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::trim(const String& str, TrimSides sides) const {
  return trim_impl(str, u_isUWhiteSpace, sides);
}

String HSLLocaleICUOps::trim(const String& str, const String& what, TrimSides sides) const {
  std::set<UChar32> what_set;
  const auto uwhat = ustr_from_utf8(what);
  for (int32_t i = 0; i < uwhat.length(); i = uwhat.moveIndex32(i, 1)) {
    what_set.emplace(uwhat.char32At(i));
  }

  return trim_impl(
    str,
    [what_set](UChar32 ch) {
      return what_set.find(ch) != what_set.end();
    },
    sides
  );
}

String HSLLocaleICUOps::replace(const String& haystack,
                                 const String& needle,
                                 const String& replacement) const {
  auto uhaystack = ustr_from_utf8(haystack);
  auto uneedle = ustr_from_utf8(needle);
  const auto ureplacement = ustr_from_utf8(replacement);

  icu::ErrorCode err;
  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  uhaystack = normalizer->normalize(uhaystack, err);
  uneedle = normalizer->normalize(uneedle, err);
  // No need to normalize replacement

  uhaystack.findAndReplace(uneedle, ureplacement);
  std::string ret;
  uhaystack.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::replace_ci(const String& haystack,
                                    const String& needle,
                                    const String& replacement) const {
  auto uhaystack = ustr_from_utf8(haystack);
  auto uneedle = ustr_from_utf8(needle);
  const auto ureplacement = ustr_from_utf8(replacement);

  icu::ErrorCode err;
  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  uhaystack = normalizer->normalize(uhaystack, err);
  uneedle = normalizer->normalize(uneedle, err);
  // No need to normalize replacement

  auto usearch(uhaystack);
  usearch.foldCase(m_caseFoldFlags);
  uneedle.foldCase(m_caseFoldFlags);
  assertx(usearch.length() == uhaystack.length());

  int32_t i = 0;
  while (i < usearch.length()) {
    i = usearch.indexOf(uneedle, i);
    if (i < 0) {
      break;
    }
    // Mutate both so that offsets are in sync
    usearch.replace(i, uneedle.length(), ureplacement);
    uhaystack.replace(i, uneedle.length(), ureplacement);

    i += ureplacement.length();
  }

  std::string ret;
  uhaystack.toUTF8String(ret);

  return ret;
}

String HSLLocaleICUOps::replace_every(const String& haystack,
                                      const Array& replacements) const {
  auto uhaystack = ustr_from_utf8(haystack);
  icu::ErrorCode err;
  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  uhaystack = normalizer->normalize(uhaystack, err);

  IterateKV(replacements.get(), [&](TypedValue needle, TypedValue replacement) {
    auto uneedle = ustr_from_utf8(needle);
    const auto ureplacement = ustr_from_utf8(replacement);
    normalizer->normalize(uneedle, err);

    uhaystack.findAndReplace(uneedle, ureplacement);
  });

  std::string ret;
  uhaystack.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::replace_every_ci(const String& haystack,
                                         const Array& replacements) const {
  auto uhaystack = ustr_from_utf8(haystack);
  icu::ErrorCode err;
  // singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);
  uhaystack = normalizer->normalize(uhaystack, err);
  auto usearch(uhaystack);
  usearch.foldCase(m_caseFoldFlags);
  assertx(usearch.length() == uhaystack.length());

  IterateKV(replacements.get(), [&](TypedValue needle, TypedValue replacement) {
    auto uneedle = ustr_from_utf8(needle);
    const auto ureplacement = ustr_from_utf8(replacement);
    normalizer->normalize(uneedle, err);
    uneedle.foldCase(m_caseFoldFlags);
    auto ufoldedReplacement(ureplacement);
    ufoldedReplacement.foldCase(m_caseFoldFlags);

    int32_t i = 0;
    while (i < usearch.length()) {
      i = usearch.indexOf(uneedle, i);
      if (i < 0) {
        break;
      }
      usearch.replace(i, uneedle.length(), ufoldedReplacement);
      uhaystack.replace(i, uneedle.length(), ureplacement);
    }
  });

  std::string ret;
  uhaystack.toUTF8String(ret);
  return ret;
}

String HSLLocaleICUOps::replace_every_nonrecursive(const String& haystack,
                                                   const Array& replacements) const {
  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);

  return HPHP::replace_every_nonrecursive<icu::UnicodeString>(
    haystack,
    replacements,
    [](const HPHP::String& s) { return ustr_from_utf8(s); },
    [](const icu::UnicodeString& s) -> String {
      std::string ret;
      s.toUTF8String(ret);
      return ret;
    },
    [&](icu::UnicodeString* s) { *s = normalizer->normalize(*s, err); },
    [](icu::UnicodeString* s) { /* case sensitive, don't fold case */ },
    [](const icu::UnicodeString& hs, const icu::UnicodeString& n, int32_t offset) {
      return hs.indexOf(n, offset);
    },
    [](icu::UnicodeString* hs, int32_t offset, int32_t len, const icu::UnicodeString& r) {
      hs->replace(offset, len, r);
    }
  );
}

String HSLLocaleICUOps::replace_every_nonrecursive_ci(const String& haystack,
                                                      const Array& replacements) const {
  icu::ErrorCode err;
  // Singleton, do not free
  auto normalizer = icu::Normalizer2::getNFCInstance(err);

  return HPHP::replace_every_nonrecursive<icu::UnicodeString>(
    haystack,
    replacements,
    [](const HPHP::String& s) { return ustr_from_utf8(s); },
    [](const icu::UnicodeString& s) -> String {
      std::string ret;
      s.toUTF8String(ret);
      return ret;
    },
    [&](icu::UnicodeString* s) { *s = normalizer->normalize(*s, err); },
    [this](icu::UnicodeString* s) { s->foldCase(this->m_caseFoldFlags); },
    [](const icu::UnicodeString& hs, const icu::UnicodeString& n, int32_t offset) {
      return hs.indexOf(n, offset);
    },
    [](icu::UnicodeString* hs, int32_t offset, int32_t len, const icu::UnicodeString& r) {
      hs->replace(offset, len, r);
    }
  );
}

} // namespace HPHP
