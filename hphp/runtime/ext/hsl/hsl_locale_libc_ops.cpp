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
#include "hphp/runtime/ext/hsl/hsl_locale_libc_ops.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/util/bstring.h"
#include "hphp/zend/zend-string.h"

#include <strings.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

namespace HPHP {

HSLLocaleLibcOps::HSLLocaleLibcOps(
  const Locale& locale
) : m_loc(locale.get()) {
}

HSLLocaleLibcOps::~HSLLocaleLibcOps() {
}

int64_t HSLLocaleLibcOps::strlen(const String& str) const {
  return str.length();
}

String HSLLocaleLibcOps::uppercase(const String& str) const {
  return str.forEachByteFast([this](char c) { return toupper_l(c, this->m_loc); });
}

String HSLLocaleLibcOps::lowercase(const String& str) const {
  return str.forEachByteFast([this](char c) { return tolower_l(c, this->m_loc); });
}

String HSLLocaleLibcOps::foldcase(const String& str) const {
  return lowercase(str);
}

Array HSLLocaleLibcOps::chunk(const String& str, int64_t chunk_size) const {
  assertx(chunk_size > 0);
  const auto len = str.size();
  VecInit ret { (size_t) (len / chunk_size + 1) };
  if (len <= chunk_size) {
    ret.append(str);
    return ret.toArray();
  }

  for (int i = 0; i < len; i += chunk_size) {
    ret.append(str.substr(i, chunk_size));
  }
  return ret.toArray();
}

int64_t HSLLocaleLibcOps::strcoll(const String& a, const String& b) const {
  // Overridden  in HSLLocaleByteOps for "C" locale
  assertx(!a.isNull() && !b.isNull());

  return strcoll_l(a.c_str(), b.c_str(), this->m_loc);
}

int64_t HSLLocaleLibcOps::strcasecmp(const String& a, const String& b) const {
  // Overridden  in HSLLocaleByteOps for "C" locale
  assertx(!a.isNull() && !b.isNull());
  const auto min_len = MIN(a.size(), b.size());
  // Defined on FreeBSD, but also an undocumented glibc extension 
  const auto res = strncasecmp_l(a.data(), b.data(), min_len, this->m_loc);
  if (res != 0) {
    return res;
  }

  if (a.size() < b.size()) {
    return -1;
  }
  if (a.size() > b.size()) {
    return 1;
  }
  return 0;
}

bool HSLLocaleLibcOps::starts_with(const String& str, const String& prefix) const {
  assertx(!str.isNull() & !prefix.isNull());
  if (str.size() < prefix.size()) {
    return false;
  }
  return string_ncmp(str.data(), prefix.data(), prefix.size()) == 0;
}

bool HSLLocaleLibcOps::starts_with_ci(const String& str, const String& prefix) const {
  assertx(!str.isNull() & !prefix.isNull());
  if (str.size() < prefix.size()) {
    return false;
  }
  return bstrcaseeq(str.data(), prefix.data(), prefix.size());
}

bool HSLLocaleLibcOps::ends_with(const String& str, const String& suffix) const {
  assertx(!str.isNull() & !suffix.isNull());
  if (str.size() < suffix.size()) {
    return false;
  }
  const auto offset = str.size() - suffix.size();
  return string_ncmp(str.data() + offset, suffix.data(), suffix.size()) == 0;
}

bool HSLLocaleLibcOps::ends_with_ci(const String& str, const String& suffix) const {
  assertx(!str.isNull() & !suffix.isNull());
  if (str.size() < suffix.size()) {
    return false;
  }
  const auto offset = str.size() - suffix.size();
  return bstrcaseeq(str.data() + offset, suffix.data(), suffix.size());
}

int64_t HSLLocaleLibcOps::strpos(const String& haystack, const String& needle, int64_t offset) const {
  if (needle.empty() || haystack.empty()) {
    return -1;
  }
  if (offset < 0) {
    offset += haystack.length();
    if (offset < 0) {
      return -1;
    }
  }
  auto pos = HHVM_FN(strpos)(haystack, needle, offset);
  if (pos.m_type == KindOfBoolean) {
    return -1;
  }
  return pos.m_data.num;
}

int64_t HSLLocaleLibcOps::strrpos(const String& haystack, const String& needle, int64_t offset) const {
  if (needle.empty() || haystack.empty()) {
    return -1;
  }
  auto pos = HHVM_FN(strrpos)(haystack, needle, offset);
  if (pos.m_type == KindOfBoolean) {
    return -1;
  }
  return pos.m_data.num;
}

int64_t HSLLocaleLibcOps::stripos(const String& haystack, const String& needle, int64_t offset) const {
  if (needle.empty() || haystack.empty()) {
    return -1;
  }
  if (offset < 0) {
    offset += haystack.length();
    if (offset < 0) {
      return -1;
    }
  }
  auto pos = HHVM_FN(stripos)(haystack, needle, offset);
  if (pos.m_type == KindOfBoolean) {
    return -1;
  }
  return pos.m_data.num;
}

int64_t HSLLocaleLibcOps::strripos(const String& haystack, const String& needle, int64_t offset) const {
  if (needle.empty() || haystack.empty()) {
    return -1;
  }
  auto pos = HHVM_FN(strripos)(haystack, needle, offset);
  if (pos.m_type == KindOfBoolean) {
    return -1;
  }
  return pos.m_data.num;
}

String HSLLocaleLibcOps::slice(const String& str, int64_t offset, int64_t length) const {
  if (length < 0) {
    length += str.length();
    if (length <= 0) {
      return empty_string();
    }
  }
  if (offset < 0) {
    offset += str.length();
  }
  if (offset < 0 || offset >= str.length()) {
    return empty_string();
  }
  return str.substr(offset, length);
}

String HSLLocaleLibcOps::reverse(const String& str) const {
  return HHVM_FN(strrev)(str);
}

} // namespace HPHP
